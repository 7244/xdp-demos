#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <bpf/bpf.h>
#include <errno.h>

uint8_t hex_digit_to_number(uint8_t digit){
  switch(digit){
    case '0':case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8':case '9':
      return digit - '0';
    case 'a':case 'b':case 'c':case 'd':case 'e':case 'f':
      return digit - 'a' + 10;
    case 'A':case 'B':case 'C':case 'D':case 'E':case 'F':
      return digit - 'A' + 10;
  }

  printf("hex_digit_to_number expected hex digit got %c\n", digit);
  exit(1);

  /* TOOD unreachable is okay too */
  return 0;
}

void read_hex_to_left(void *dst, const void *src, uintptr_t hex_length){
  uint8_t *_dst = (uint8_t *)dst;
  const uint8_t *_src = (const uint8_t *)src;

  _src += hex_length;

  uint8_t ls = 0;
  while(hex_length--){
    *_dst |= hex_digit_to_number(*--_src) << ls;

    _dst -= ls == 4;
    ls ^= 4;
  }
}

int main(int argc, char **argv){
  int ret = 0;

  if(argc != 3){
    printf("usage: ./exe map_id key_in_hex\n");
    return 1;
  }

  int err;

  int map_fd = bpf_map_get_fd_by_id(atoi(argv[1]));
  if(map_fd < 0){
    perror("");
  }

  struct bpf_map_info info;
  memset(&info, 0, sizeof(info));
  __u32 info_len = sizeof(struct bpf_map_info);
  err = bpf_obj_get_info_by_fd(map_fd, &info, &info_len);
  if(err){
    perror("");
    return 1;
  }

  /* TODO need to bound check info_len */

  if(strlen(argv[2]) > info.key_size * 2){
    printf("key_in_hex is longer than info.key_size * 2\n");
    ret = 1;
    goto gt_cleanup;
  }

  /* TODO can key_size be 0 ? */

  uint8_t *key = (uint8_t *)malloc(info.key_size);

  memset(key, 0, info.key_size);

  read_hex_to_left(&key[info.key_size - 1], argv[2], strlen(argv[2]));

  if(bpf_map_delete_elem(map_fd, key)){
    perror("bpf_map_delete_elem");
    return 1;
  }

  free(key);

  gt_cleanup:

  if(close(map_fd)){
    perror("");
    return 1;
  }

  return ret;
}
