#include <WITCH/WITCH.h>

#include <stdlib.h>
#include <stdio.h>
#include <GeoIP.h>

#include "country_code.hpp"

const char *alpha2_from_id(uint8_t id){
  return country_code.NA(id)->alpha2;
}

int main(){
  FILE *fout = fopen("geoiparray", "rb");
  if(fout == NULL){
    printf("failed to open output file\n");
    return 1;
  }

  uint8_t *ip24_country_array = (uint8_t *)malloc(1 << 24);

  if(fread(ip24_country_array, 1, 1 << 24, fout) != 1 << 24){
    perror("fread");
    return 1;
  }

  fclose(fout);

  GeoIP *gi = GeoIP_new(0);
  if(gi == NULL){
    return 1;
  }

  uint32_t error = 0;

  uint32_t ip = 0;
  while(ip != 0xffffffff){
    uint8_t id = ip24_country_array[ip >> 8];
    auto ptr = GeoIP_country_code_by_ipnum(gi, ip);
    if(ptr != NULL){
      if(strcasecmp(ptr, alpha2_from_id(id)) != 0){
        error++;
        //printf("error at ip not same %08x %s %s\n", ip, alpha2_from_id(id), ptr);
        //return 1;
      }
    }

    ip++;
  }

  free(ip24_country_array);

  GeoIP_delete(gi);

  printf("total errors: %u\n", error);

  return 0;
}
