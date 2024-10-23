#include <linux/bpf.h>
#include <bpf/bpf_helpers.h>
#include <linux/if_ether.h>
#include <arpa/inet.h>

#include <linux/ip.h>
#include <linux/ipv6.h>

/* bound check */
#define BC(p) \
  if((unsigned long)&p[1] > ctx->data_end) do{ \
    return XDP_DROP; \
  }while(0)

struct{
  __uint(type, BPF_MAP_TYPE_ARRAY);
  __uint(max_entries, 256);
  __type(key, __u32);
  __type(value, __u8); /* non zero for whitelist */
}countryblockarr SEC(".maps");

struct key_ipv4prefix{
  __u32 prefix;
  __u32 address;
};

struct{
  __uint(type, BPF_MAP_TYPE_LPM_TRIE);
  __uint(max_entries, 0x1000000);
  __type(key, struct key_ipv4prefix);
  __type(value, __u8); /* country */
  __uint(map_flags, BPF_F_NO_PREALLOC);
}ipv4pcountrymap SEC(".maps");

struct key_ipv6prefix{
  __u32 prefix;
  struct in6_addr address;
};

struct{
  __uint(type, BPF_MAP_TYPE_LPM_TRIE);
  __uint(max_entries, 0x1000000);
  __type(key, struct key_ipv6prefix);
  __type(value, __u8); /* country */
  __uint(map_flags, BPF_F_NO_PREALLOC);
}ipv6pcountrymap SEC(".maps");

struct ratelimit{
  __u64 wanted_pps;
  __u64 pps;
  __u64 last_time;
};

struct{
  __uint(type, BPF_MAP_TYPE_HASH);
  __uint(max_entries, 0x10000);
  __type(key, __u32);
  __type(value, struct ratelimit);
  __uint(map_flags, BPF_F_NO_PREALLOC);
}ipv4ratemap SEC(".maps");
struct{
  __uint(type, BPF_MAP_TYPE_HASH);
  __uint(max_entries, 0x10000);
  __type(key, struct in6_addr);
  __type(value, struct ratelimit);
  __uint(map_flags, BPF_F_NO_PREALLOC);
}ipv6ratemap SEC(".maps");

SEC("start")
int _start(struct xdp_md *ctx){
  struct ethhdr *eth = (struct ethhdr *)(unsigned long)ctx->data;

  BC(eth);

  struct ratelimit *ratelimit;

  struct iphdr *ipv4 = (struct iphdr *)&eth[1];
  struct ipv6hdr *ipv6 = (struct ipv6hdr *)&eth[1];

  __u8 *lookup_country;

  if(eth->h_proto == htons(ETH_P_IP)){
    BC(ipv4);

    struct key_ipv4prefix key;
    key.prefix = 32;
    key.address = ipv4->saddr;

    lookup_country = bpf_map_lookup_elem(&ipv4pcountrymap, &key);
    ratelimit = bpf_map_lookup_elem(&ipv4ratemap, &ipv4->saddr);
  }
  else if(eth->h_proto == htons(ETH_P_IPV6)){
    BC(ipv6);

    struct key_ipv6prefix key;
    key.prefix = 128;
    key.address = ipv6->saddr;

    lookup_country = bpf_map_lookup_elem(&ipv6pcountrymap, &key);
    ratelimit = bpf_map_lookup_elem(&ipv6ratemap, &ipv6->saddr);
  }
  else{
    /* TODO can countries access outside of ipv4 ipv6 ? */
    return XDP_PASS;
  }

  if(lookup_country == NULL){
    return XDP_DROP;
  }

  __u32 country = *lookup_country;

  __u8 *whitelisted = bpf_map_lookup_elem(&countryblockarr, &country);
  if(whitelisted == NULL){
    return XDP_DROP; /* verifier doesnt shut up */
    __builtin_unreachable();
  }
  if(!*whitelisted){
    return XDP_DROP;
  }

  /* spinlock free rate limiter */
  if(ratelimit != NULL){
    /* bpf values are allocated as 0. */
    /* and getting 1 from bpf_ktime_get_ns() is impossible (hehe) */
    __u64 last_time = 1; 

    __u64 pps;
    last_time = __sync_lock_test_and_set(&ratelimit->last_time, last_time);
    if(last_time != 1) do{
      __u64 ctime = bpf_ktime_get_ns();
      __u64 diff = ctime - last_time;
      if(diff < 10000000){
        pps = __sync_add_and_fetch(&ratelimit->pps, 0);
        __sync_lock_test_and_set(&ratelimit->last_time, last_time);
        break;
      }
      __u64 diff_scale = diff / 10000000;
      if(diff_scale > 100){
        diff_scale = 100;
      }
      pps = __sync_add_and_fetch(&ratelimit->pps, 0);
      __u64 pps_to_sub = pps * diff_scale / 100;

      if(pps_to_sub){
        pps = __sync_sub_and_fetch(&ratelimit->pps, pps_to_sub);
        __sync_lock_test_and_set(&ratelimit->last_time, ctime);
      }
      else{ /* we did nothing. lets revert */
        __sync_lock_test_and_set(&ratelimit->last_time, last_time);
      }
    }while(0);
    else{
      pps = __sync_add_and_fetch(&ratelimit->pps, 0);
    }

    __u64 wanted_pps = __sync_add_and_fetch(&ratelimit->wanted_pps, 0);
    if(pps >= wanted_pps){
      return XDP_DROP;
    }
    __sync_fetch_and_add(&ratelimit->pps, 1);
  }

  return XDP_PASS;
}

char _license[] SEC("license") = "GPL";
