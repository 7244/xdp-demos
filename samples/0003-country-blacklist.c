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
  __type(value, __u8); /* non zero for blacklist */
}countryblockarr SEC(".maps");

struct key_ipv4prefix{
  __u32 prefix;
  __u32 address;
};

struct{
  __uint(type, BPF_MAP_TYPE_LPM_TRIE);
  __uint(max_entries, 0x10000);
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
  __uint(max_entries, 0x10000);
  __type(key, struct key_ipv6prefix);
  __type(value, __u8); /* country */
  __uint(map_flags, BPF_F_NO_PREALLOC);
}ipv6pcountrymap SEC(".maps");

SEC("start")
int _start(struct xdp_md *ctx){
  struct ethhdr *eth = (struct ethhdr *)(unsigned long)ctx->data;

  BC(eth);

  struct iphdr *ipv4 = (struct iphdr *)&eth[1];
  struct ipv6hdr *ipv6 = (struct ipv6hdr *)&eth[1];

  __u8 *lookup_country;

  if(eth->h_proto == htons(ETH_P_IP)){
    BC(ipv4);

    struct key_ipv4prefix key;
    key.prefix = 32;
    key.address = ipv4->saddr;

    lookup_country = bpf_map_lookup_elem(&ipv4pcountrymap, &key);
  }
  else if(eth->h_proto == htons(ETH_P_IPV6)){
    BC(ipv6);

    struct key_ipv6prefix key;
    key.prefix = 128;
    key.address = ipv6->saddr;

    lookup_country = bpf_map_lookup_elem(&ipv6pcountrymap, &key);
  }
  else{
    return XDP_PASS;
  }

  if(lookup_country == NULL){
    return XDP_PASS;
  }

  __u32 country = *lookup_country;

  __u8 *blacklisted = bpf_map_lookup_elem(&countryblockarr, &country);
  if(blacklisted == NULL){
    return XDP_PASS; /* verifier doesnt shut up */
    __builtin_unreachable();
  }
  if(*blacklisted){
    return XDP_DROP;
  }

  return XDP_PASS;
}

char _license[] SEC("license") = "GPL";
