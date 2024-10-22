#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <bpf/bpf.h>

#include "../../extra/country_code.hpp"

uint8_t id_from_alpha2(const char *alpha2str){
  for(uint32_t i = 0; i < country_code.GetMemberAmount(); i++){
    if(strncasecmp(alpha2str, country_code.NA(i)->alpha2, 2) == 0){
      return i;
    }
  }
  return country_code.AN(&country_code_t::Unknown);
}

int write_prefixes(int argc, char **argv){
  if(argc < 4){
    fprintf(stderr, "need ipv4pcountrymap_id ipv6pcountrymap_id as parameter\n");
    return 1;
  }

  uint32_t ipv4pcountrymap_id = atoi(argv[2]);
  uint32_t ipv6pcountrymap_id = atoi(argv[3]);

  if(!ipv4pcountrymap_id){
    fprintf(stderr, "ipv4pcountrymap_id cant be 0\n");
    return 1;
  }
  if(!ipv6pcountrymap_id){
    fprintf(stderr, "ipv6pcountrymap_id cant be 0\n");
    return 1;
  }

  int ipv4pcountrymap_fd = bpf_map_get_fd_by_id(ipv4pcountrymap_id);
  if(ipv4pcountrymap_fd < 0){
    perror("bpf_map_get_fd_by_id");
    return 1;
  }
  int ipv6pcountrymap_fd = bpf_map_get_fd_by_id(ipv6pcountrymap_id);
  if(ipv6pcountrymap_fd < 0){
    perror("bpf_map_get_fd_by_id");
    return 1;
  }

  for(int iarg = 4; iarg < argc; iarg++){
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

    uint8_t country_id = id_from_alpha2(arg);

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
  if(argc < 5){
    fprintf(stderr, "need map_id ip 0/1 as parameter\n");
    return 1;
  }

  uint32_t map_id = atoi(argv[2]);

  if(!map_id){
    fprintf(stderr, "map_id cant be 0\n");
    return 1;
  }

  int map_fd = bpf_map_get_fd_by_id(map_id);
  if(map_fd < 0){
    perror("bpf_map_get_fd_by_id");
    return 1;
  }

  uint32_t id = id_from_alpha2(argv[3]);
  uint8_t value = atoi(argv[4]);

  if(bpf_map_update_elem(map_fd, &id, &value, BPF_ANY)){
    perror("bpf_map_update_elem");
    return 1;
  }

  return 0;
}

int main(int argc, char **argv){
  if(argc < 2){
    fprintf(stderr, "need parameters\n");
    return 1;    
  }

  if(atoi(argv[1]) == 0){
    return write_prefixes(argc, argv);
  }
  else if(atoi(argv[1]) == 1){
    return update_country(argc, argv);
  }
  else{
    fprintf(stderr, "wrong parameter in begin\n");
    return 1;
  }
}
