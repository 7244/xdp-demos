#!/bin/bash

# check error
cerr () {
  if [ $? -ne 0 ]; then
    echo "command failed. exiting."
    exit 1
  fi
}

get_map_id () {
  while read line ;
  do
    if [ $(echo $line | awk '{print $1}') -eq $prog_id ]; then
      if [ $(echo $line | awk '{print $4}') = $1 ]; then
        echo $line | awk '{print $3}'
        return 0
      fi
    fi
  done < <(sudo ./list-maps.exe | grep -i "map_info" | sed 's/.*map_info//')

  echo "failed to get map_id $1."
  exit 1
}

../../make.sh xdp.c
cerr

if [ -d "GeoIP-Country-Lists" ]; then
  cd GeoIP-Country-Lists
  cerr
  git pull
  cerr
  cd ..
else
  git clone --depth 1 "https://github.com/AndiDittrich/GeoIP-Country-Lists.git"
  cerr
fi

./load.sh
cerr

interface_name=$(cat load.sh | grep "ip link set" | sed 's/.*ip link set//' | awk '{print $1}')
if [ -z "${interface_name}" ]; then
  echo "failed to get interface name."
  exit 1;
fi

prog_id=$(ip link show dev $interface_name | grep "xdp id" | sed 's/.*xdp id//' | awk '{print $1}')
if [ -z "${interface_name}" ]; then
  echo "failed to get prog_id."
  exit 1;
fi

clang ../../user/list-maps.c -o list-maps.exe -lbpf
cerr

statsarr_id=$(get_map_id statsarr)
countryblockarr_id=$(get_map_id countryblockarr)
ipv4pcountrymap_id=$(get_map_id ipv4pcountrymap)
ipv6pcountrymap_id=$(get_map_id ipv6pcountrymap)
ipv4ratemap_id=$(get_map_id ipv4ratemap)
ipv6ratemap_id=$(get_map_id ipv6ratemap)
ipv4sessionmap_id=$(get_map_id ipv4sessionmap)
ipv6sessionmap_id=$(get_map_id ipv6sessionmap)

cd control_xdp
cerr
cmake .
cerr
make
cerr
cd ..
cerr

sudo ./control_xdp.exe 0 $ipv4pcountrymap_id $ipv6pcountrymap_id GeoIP-Country-Lists/Build/*
cerr

echo -e "#!/bin/bash\nsudo ./control_xdp.exe 1 $countryblockarr_id \$1 \$2" > set-country.sh
cerr
chmod +x set-country.sh
cerr

echo -e "#!/bin/bash\nsudo ./control_xdp.exe 2 $ipv4ratemap_id \$1 \$2" > set-ratelimit.sh
cerr
chmod +x set-ratelimit.sh
cerr

echo -e "#!/bin/bash\nsudo ./control_xdp.exe 3 $ipv4ratemap_id \$1" > delete-ratelimit.sh
cerr
chmod +x delete-ratelimit.sh
cerr

echo -e "#!/bin/bash\nsudo ./control_xdp.exe 4 $statsarr_id $ipv4sessionmap_id $ipv6sessionmap_id" > list-stats.sh
cerr
chmod +x list-stats.sh
cerr
