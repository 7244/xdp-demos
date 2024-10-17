#include <linux/bpf.h>
#include <bpf/bpf_helpers.h>
#include <linux/if_ether.h>
#include <arpa/inet.h>

#include <linux/ip.h>
#include <linux/ipv6.h>

#include <linux/udp.h>

/* bound check */
#define BC(p) \
  if((unsigned long)&p[1] > ctx->data_end) do{ \
    return XDP_DROP; \
  }while(0)

SEC("start")
int _start(struct xdp_md *ctx){
  struct ethhdr *eth = (struct ethhdr *)(unsigned long)ctx->data;

  BC(eth);

  struct iphdr *ipv4 = (struct iphdr *)&eth[1];
  struct ipv6hdr *ipv6 = (struct ipv6hdr *)&eth[1];

  struct udphdr *udp;

  if(eth->h_proto == htons(ETH_P_IP)){
    BC(ipv4);

    if(ipv4->protocol != IPPROTO_UDP){
      return XDP_PASS;
    }

    udp = (struct udphdr *)&ipv4[1];
  }
  else if(eth->h_proto == htons(ETH_P_IPV6)){
    BC(ipv6);

    if(ipv6->nexthdr != IPPROTO_UDP){
      return XDP_PASS;
    }

    udp = (struct udphdr *)&ipv6[1];
  }
  else{
    /* TODO i dont know if next layers of this can be udp or not */
    return XDP_PASS;
  }

  BC(udp);

  if(udp->source == htons(1900)){
    return XDP_DROP;
  }

  return XDP_PASS;
}

char _license[] SEC("license") = "GPL";
