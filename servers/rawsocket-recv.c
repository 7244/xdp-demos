#include <stdint.h>
#include <stdio.h>
#include <inttypes.h>
#include <pthread.h>

#include <sys/socket.h>
#include <netinet/in.h>

#include <linux/if_ether.h>
#include <time.h>

uint64_t getnanotime(){
  struct timespec t;
  clock_gettime(CLOCK_MONOTONIC, &t);
  return (uint64_t)t.tv_sec * 1000000000 + t.tv_nsec;
}
void donanosleep(uint64_t t){
  struct timespec ts;
  ts.tv_sec = t / 1000000000;
  ts.tv_nsec = t % 1000000000;
  nanosleep(&ts, &ts);
}

uint64_t pps = 0;
uint64_t Bps = 0;
uint64_t big = 0;

void *info_func(){
  uint64_t t = getnanotime();

  while(1){
    donanosleep(1000000000);

    uint64_t tnew = getnanotime();
    uint64_t tdiff = tnew - t;
    t = tnew;

    uint64_t _pps = __atomic_load_n(&pps, __ATOMIC_SEQ_CST);
    uint64_t _Bps = __atomic_load_n(&Bps, __ATOMIC_SEQ_CST);
    uint64_t _big = __atomic_load_n(&big, __ATOMIC_SEQ_CST);

    printf(
      "pps %" PRIu64 ", Bps %" PRIu64 ", big %" PRIu64 ". in %lf seconds.\n",
      _pps,
      _Bps,
      _big,
      (double)tdiff / 1000000000
    );

    __atomic_sub_fetch(&pps, _pps, __ATOMIC_SEQ_CST);
    __atomic_sub_fetch(&Bps, _Bps, __ATOMIC_SEQ_CST);
    __atomic_sub_fetch(&big, _big, __ATOMIC_SEQ_CST);
  }

  return NULL;
}

int main(){
  int s = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
  if(s < 0){
    perror("socket");
    return 1;
  }

  pthread_t pthid;
  int ptherr = pthread_create(&pthid, NULL, info_func, NULL);

  while(1){
    uint8_t buf[4096];
    ssize_t r = recvfrom(s, buf, sizeof(buf), 0, NULL, NULL);
    if(r < 0){
      perror("recvfrom");
      return 1;
    }

    __atomic_add_fetch(&pps, 1, __ATOMIC_SEQ_CST);
    __atomic_add_fetch(&Bps, r, __ATOMIC_SEQ_CST);
    if(r == sizeof(buf)){
      __atomic_add_fetch(&big, 1, __ATOMIC_SEQ_CST);
    }
  }

  return 0;
}
