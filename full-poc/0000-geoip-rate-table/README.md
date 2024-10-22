### request
1 - based on geoip database, make whitelist for specific countries. 

2 - make rate limit (eg PPS) based on specific ip which will be provided from CLI.

3 - make table of sessions that will be related with last 360 second traffic.

4 - make high performance.

### response
1 - done. country to whitelist is changeable by CLI in runtime.

2 - TODO

3 - TODO

4 - TODO

###
how to use:

`./build_load_setup.sh`

then for add to whitelist `./set-country.sh DE 1` to remove from whitelist `./set-country.sh DE 0`

`./unload.sh` for unload the module.
