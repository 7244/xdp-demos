#if defined(__cplusplus)
  struct stats_e : __dme_inherit<stats_e>{
  #define d(name, ...) __dme(name,);
#else
  enum{
  #define c ,
  #define d(name, comma) name comma
#endif
  d(stats_proto_ipv4_e,c)
  d(stats_proto_ipv6_e,c)
  d(stats_proto_other_e,c)
  d(stats_iplookupfail_e,c)
  d(stats_countrylookupfail_e,c)
  d(stats_dropcountry_e,c)
  d(stats_droplimit_e,c)
  d(_stats_last_e,)
}
#if defined(__cplusplus)
  stats_e
#endif
;

#undef d
#undef c
