#include <WITCH/WITCH.h>

#include <stdlib.h>
#include <stdio.h>
#include <GeoIP.h>

#include "country_code.hpp"

uint8_t id_from_alpha2(const char *alpha2str){
  for(uint32_t i = 0; i < country_code.GetMemberAmount(); i++){
    if(strcasecmp(alpha2str, country_code.NA(i)->alpha2) == 0){
      return i;
    }
  }
  return country_code.AN(&country_code_t::Unknown);
}

int main(){
  FILE *fout = fopen("geoiparray", "wb");
  if(fout == NULL){
    printf("failed to open output file\n");
    return 1;
  }

  GeoIP *gi = GeoIP_new(0);
  if(gi == NULL){
    return 1;
  }

  uint8_t *ip24_country_array = (uint8_t *)malloc(1 << 24);

  uint32_t ip = 0;
  while(ip < 1 << 24){
    auto ptr = GeoIP_country_code_by_ipnum(gi, ip << 8);
    if(ptr == NULL){
      ip24_country_array[ip] = country_code.AN(&country_code_t::Unknown);
    }
    else{
      auto id = id_from_alpha2(ptr);
      if(id == country_code.AN(&country_code_t::Unknown)){
        printf("warning failed to find country code \"%s\" %08x\n", ptr, ip);
      }
      ip24_country_array[ip] = id;
    }

    ip++;
  }

  if(fwrite(ip24_country_array, 1, 1 << 24, fout) != 1 << 24){
    perror("fwrite");
    return 1;
  }

  free(ip24_country_array);

  GeoIP_delete(gi);

  fclose(fout);

  return 0;
}
