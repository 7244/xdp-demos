#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <arpa/inet.h>

#include <bpf/bpf.h>

#include "../../extra/country_code.hpp"

#include "stats.h"

uint8_t id_from_alpha2(const char *alpha2str, uintptr_t length = 0){
  length = length ? length : strlen(alpha2str);
  /* TODO strlen(alpha2str) can be less than 2 when length is 2 */
  if(length != 2){
    fprintf(stderr, "alpha2 length is not 2\n");
    exit(1);
  }

  for(uint32_t i = 0; i < country_code.GetMemberAmount(); i++){
    if(strncasecmp(alpha2str, country_code.NA(i)->alpha2, length) == 0){
      return i;
    }
  }

  return country_code.AN(&country_code_t::Unknown);
}

int get_map_fd_by_id(uint32_t id){
  if(id == 0){
    fprintf(stderr, "map_id cant be 0\n");
    exit(1);
  }

  int fd = bpf_map_get_fd_by_id(id);
  if(fd < 0){
    perror("bpf_map_get_fd_by_id");
    exit(1);
  }

  return fd;
}

int write_prefixes(int argc, char **argv){
  if(argc < 2){
    fprintf(stderr, "need ipv4pcountrymap_id ipv6pcountrymap_id as parameter\n");
    return 1;
  }

  int ipv4pcountrymap_fd = get_map_fd_by_id(atoi(argv[0]));
  int ipv6pcountrymap_fd = get_map_fd_by_id(atoi(argv[1]));

  for(int iarg = 2; iarg < argc; iarg++){
    char *arg = argv[iarg];

    FILE *f = fopen(arg, "rb");
    if(fseek(f, 0, SEEK_END)){
      perror("fseek");
      return 1;
    }
    long f_size = ftell(f);
    if(f_size < 0){
      perror("ftell");
      return 1;
    }
    if(fseek(f, 0, SEEK_SET)){
      perror("fseek");
      return 1;
    }

    char *fdata = (char *)malloc(f_size + 1);
    if(fread(fdata, 1, f_size, f) != f_size){
      perror("fread");
      return 1;
    }

    /* for sscanf */
    fdata[f_size] = 0;

    fclose(f);

    char *slash = strrchr(arg, '/');
    if(slash != NULL){
      arg = slash + 1;
    }
    uintptr_t arg_length = strlen(arg);
    char *dot = strchr(arg, '.');
    if(dot != NULL){
      arg_length = (uintptr_t)dot - (uintptr_t)arg;
    }
    if(arg_length != 2){
      fprintf(stderr, "filename doesnt have country alpha2 length\n");
      return 1;
    }

    uint8_t country_id = id_from_alpha2(arg, 2);

    if(country_id == country_code.AN(&country_code_t::Unknown)){
      fprintf(stderr, "country_id is Unknown %.*s\n", 2, arg);
      return 1;
    }

    for(uintptr_t i = 0; fdata[i]; i++){
      struct{
        uint32_t prefix;
        uint8_t ip[4];
      }key;
      uint32_t ip[4];

      if(sscanf(&fdata[i], "%u.%u.%u.%u/%u", &ip[0], &ip[1], &ip[2], &ip[3], &key.prefix) != 5){
        fprintf(stderr, "failed to get ip and prefix from file\n");
        return 1;
      }

      key.ip[0] = ip[0];
      key.ip[1] = ip[1];
      key.ip[2] = ip[2];
      key.ip[3] = ip[3];

      if(bpf_map_update_elem(ipv4pcountrymap_fd, &key, &country_id, BPF_ANY)){
        perror("bpf_map_update_elem");
        return 1;
      }

      while(1){
        if(!fdata[i]){
          i--;
          break;
        }
        if(fdata[i] == '\n'){
          break;
        }
        i++;
      }
    }


    free(fdata);
  }

  return 0;
}

int update_country(int argc, char **argv){
  if(argc != 3){
    fprintf(stderr, "need map_id alpha2 0/1 as parameter\n");
    return 1;
  }

  int map_fd = get_map_fd_by_id(atoi(argv[0]));

  uint32_t id = id_from_alpha2(argv[1]);
  if(id == country_code.AN(&country_code_t::Unknown)){
    fprintf(stderr, "id_from_alpha2 returns Unknown for %s\n", argv[1]);
    return 1;
  }
  uint8_t value = atoi(argv[2]);

  if(bpf_map_update_elem(map_fd, &id, &value, BPF_ANY)){
    perror("bpf_map_update_elem");
    return 1;
  }

  return 0;
}

int update_ratelimit(int argc, char **argv){
  if(argc != 3){
    fprintf(stderr, "update_ratelimit needs 3 parameter\n");
    return 1;
  }

  int map_fd = get_map_fd_by_id(atoi(argv[0]));

  uint32_t ip = inet_addr(argv[1]);
  if(ip == (uint32_t)-1){
    fprintf(stderr, "inet_addr is -1\n");
    return 1;
  }

  struct{
    uint64_t pps;
    uint64_t internal0;
    uint64_t internal1;
  }value;
  value.pps = atoi(argv[2]);
  value.internal0 = 0;
  value.internal1 = 0;

  if(bpf_map_update_elem(map_fd, &ip, &value, BPF_ANY)){
    perror("bpf_map_update_elem");
    return 1;
  }

  return 0;
}

int delete_ratelimit(int argc, char **argv){
  if(argc != 2){
    fprintf(stderr, "delete_ratelimit needs 1 parameter\n");
    return 1;
  }

  int map_fd = get_map_fd_by_id(atoi(argv[0]));

  uint32_t ip = inet_addr(argv[1]);
  if(ip == (uint32_t)-1){
    fprintf(stderr, "inet_addr is -1\n");
    return 1;
  }

  if(bpf_map_delete_elem(map_fd, &ip)){
    perror("bpf_map_delete_elem");
    return 1;
  }

  return 0;
}

struct sessiondata{
  __u64 last_time;
  __u64 total_packet;
};

int list_stats(int argc, char **argv){
  if(argc != 3){
    fprintf(stderr, "delete_ratelimit needs 1 parameter\n");
    return 1;
  }

  int statsarr_fd = get_map_fd_by_id(atoi(argv[0]));
  int ipv4sessionmap_fd = get_map_fd_by_id(atoi(argv[1]));
  int ipv6sessionmap_fd = get_map_fd_by_id(atoi(argv[2]));

  for(uint32_t istat = 0; istat < stats_e.AN(&stats_e::_stats_last_e); istat++){
    uint64_t value;
    if(bpf_map_lookup_elem(statsarr_fd, &istat, &value)){
      perror("");
      return 1;
    }

    printf("%s: %llu\n", stats_e.NA(istat)->sn, (unsigned long long)value);
  }
  printf("\n");

  struct sessiondata sessiondata;
  void *key_ptr;

  printf("ipv4sessionmap:\n");

  uint8_t ip4[4];
  key_ptr = NULL;
  while(1){
    if(bpf_map_get_next_key(ipv4sessionmap_fd, key_ptr, ip4)){
      if(errno != ENOENT){
        perror("bpf_map_get_next_key");
        exit(1);
      }
      break;
    }

    key_ptr = (void *)ip4;

    if(bpf_map_lookup_elem(ipv4sessionmap_fd, ip4, &sessiondata)){
      continue;
    }

    printf(" %u.%u.%u.%u lasttime:%llu total_packet:%llu\n",
      ip4[0], ip4[1], ip4[2], ip4[3],
      (unsigned long long)sessiondata.last_time,
      (unsigned long long)sessiondata.total_packet
    );
  }

  return 0;
}

int main(int argc, char **argv){
  if(argc < 2){
    fprintf(stderr, "need parameters\n");
    return 1;    
  }

  if(atoi(argv[1]) == 0){
    return write_prefixes(argc - 2, &argv[2]);
  }
  else if(atoi(argv[1]) == 1){
    return update_country(argc - 2, &argv[2]);
  }
  else if(atoi(argv[1]) == 2){
    return update_ratelimit(argc - 2, &argv[2]);
  }
  else if(atoi(argv[1]) == 3){
    return delete_ratelimit(argc - 2, &argv[2]);
  }
  else if(atoi(argv[1]) == 4){
    return list_stats(argc - 2, &argv[2]);
  }
  else{
    fprintf(stderr, "wrong parameter in begin\n");
    return 1;
  }
}
