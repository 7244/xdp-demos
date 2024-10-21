#include <linux/bpf.h>
#include <bpf/bpf_helpers.h>
#include <linux/if_ether.h>
#include <arpa/inet.h>

#include <linux/ip.h>

/* bound check */
#define BC(p) \
  if((unsigned long)&p[1] > ctx->data_end) do{ \
    return XDP_DROP; \
  }while(0)

struct key_ipv4prefix{
  __u32 prefix;
  __u32 address;
};

struct{
  __uint(type, BPF_MAP_TYPE_LPM_TRIE);
  __uint(max_entries, 4096);
  __type(key, struct key_ipv4prefix);
  __type(value, __u8); /* filler */
  __uint(map_flags, BPF_F_NO_PREALLOC);
}ipv4pblacklist SEC(".maps");

SEC("start")
int _start(struct xdp_md *ctx){
  struct ethhdr *eth = (struct ethhdr *)(unsigned long)ctx->data;

  BC(eth);

  if(eth->h_proto != htons(ETH_P_IP)){
    return XDP_PASS;
  }

  struct iphdr *ipv4 = (struct iphdr *)&eth[1];

  BC(ipv4);

  struct key_ipv4prefix key;
  key.prefix = 32;
  key.address = ipv4->saddr;

  if(bpf_map_lookup_elem(&ipv4pblacklist, &key)){
    return XDP_DROP;
  }

  return XDP_PASS;
}

char _license[] SEC("license") = "GPL";
