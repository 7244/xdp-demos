#!/bin/bash

# check error
cerr () {
  if [ $? -ne 0 ]; then
    echo "command failed. exiting."
    exit 1
  fi
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

countryblockarr_id=
ipv4pcountrymap_id=
ipv6pcountrymap_id=

while read line ;
do
  if [ $(echo $line | awk '{print $1}') -eq $prog_id ]; then
    if [ $(echo $line | awk '{print $4}') = "countryblockarr" ]; then
      countryblockarr_id=$(echo $line | awk '{print $3}')
      break
    fi
  fi
done < <(sudo ./list-maps.exe | grep -i "map_info" | sed 's/.*map_info//')
while read line ;
do
  if [ $(echo $line | awk '{print $1}') -eq $prog_id ]; then
    if [ $(echo $line | awk '{print $4}') = "ipv4pcountrymap" ]; then
      ipv4pcountrymap_id=$(echo $line | awk '{print $3}')
      break
    fi
  fi
done < <(sudo ./list-maps.exe | grep -i "map_info" | sed 's/.*map_info//')
while read line ;
do
  if [ $(echo $line | awk '{print $1}') -eq $prog_id ]; then
    if [ $(echo $line | awk '{print $4}') = "ipv6pcountrymap" ]; then
      ipv6pcountrymap_id=$(echo $line | awk '{print $3}')
      break
    fi
  fi
done < <(sudo ./list-maps.exe | grep -i "map_info" | sed 's/.*map_info//')

if [ -z "$countryblockarr_id" ]; then
  echo "failed to get countryblockarr_id."
  exit 1;
fi
if [ -z "$ipv4pcountrymap_id" ]; then
  echo "failed to get ipv4pcountrymap_id."
  exit 1;
fi
if [ -z "$ipv6pcountrymap_id" ]; then
  echo "failed to get ipv6pcountrymap_id."
  exit 1;
fi

clang++ -std=c++2a control_xdp.cpp -o control_xdp.exe -lbpf
cerr

sudo ./control_xdp.exe 0 $ipv4pcountrymap_id $ipv6pcountrymap_id GeoIP-Country-Lists/Build/*
cerr

echo -e "#!/bin/bash\nsudo ./control_xdp.exe 1 $countryblockarr_id \$1 \$2" > set-country.sh
cerr

chmod +x set-country.sh
cerr
