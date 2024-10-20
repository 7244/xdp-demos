#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <bpf/bpf.h>
#include <errno.h>

void print_hexstring(const void *ptr, uintptr_t size){
  for(uintptr_t i = 0; i < size; i++){
    printf("%02x", ((const uint8_t *)ptr)[i]);
  }
}

int main(){
  int err;

  __u32 start_id = 0;
  while(bpf_map_get_next_id(start_id, &start_id) == 0){

    int map_fd = bpf_map_get_fd_by_id(start_id);
    if(map_fd < 0){
      perror("");
    }

    struct bpf_map_info info;
    __u32 info_len = sizeof(struct bpf_map_info);
    err = bpf_map_get_info_by_fd(map_fd, &info, &info_len);
    if(err){
      perror("");
      return 0;
    }

    printf("map info:\n");
    printf(" type: %u\n", info.type);
    printf(" id: %u\n", info.id);
    printf(" key_size: %u\n", info.key_size);
    printf(" value_size: %u\n", info.value_size);
    printf(" max_entries: %u\n", info.max_entries);
    printf(" map_flags: %u\n", info.map_flags);
    printf(" name: %s\n", info.name);
    printf(" ifindex: %u\n", info.ifindex);
    printf(" btf_vmlinux_value_type_id: %u\n", info.btf_vmlinux_value_type_id);
    printf(" netns_dev: %llu\n", (unsigned long long)info.netns_dev);
    printf(" netns_ino: %llu\n", (unsigned long long)info.netns_ino);
    printf(" btf_id: %u\n", info.btf_id);
    printf(" btf_key_type_id: %u\n", info.btf_key_type_id);
    printf(" btf_value_type_id: %u\n", info.btf_value_type_id);
    printf(" btf_vmlinux_id: %u\n", info.btf_vmlinux_id);
    printf(" map_extra: %llu\n", (unsigned long long)info.map_extra);

    uint8_t *key = (uint8_t *)malloc(info.key_size);
    uint8_t *value = (uint8_t *)malloc(info.value_size);

    printf(" key value:\n");

    void *key_ptr = NULL;
    while(1){
      err = bpf_map_get_next_key(map_fd, key_ptr, key);
      if(err < 0){
        if(errno != ENOENT){
          /* TODO what is other errors? */
          perror("");
          return 1;
        }
        break;
      }

      key_ptr = (void *)key;

      printf("  ");

      print_hexstring(key, info.key_size);

      printf(" ");

      if(bpf_map_lookup_elem(map_fd, key, value)){
        perror("");
        return 1;
      }

      print_hexstring(value, info.value_size);

      printf("\n");
    }

    free(value);
    free(key);

    if(close(map_fd)){
      perror("");
      return 1;
    }
  }

  return 0;
}
