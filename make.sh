#!/bin/bash

# check error
cerr () {
  if [ $? -ne 0 ]; then
    echo "command failed. exiting."
    exit 1
  fi
}

if [ "$#" -ne 1 ]; then
  echo "need one path/to/file"
  exit 1
fi

echo -e "_:\n\tclang -O2 -g -Wall -Wextra -Wno-unused-parameter -Wno-unused-variable -target bpf -c $1 -o bpf.o" > Makefile
cerr

make
cerr

interface_name=$(ip route list | grep default | awk '{print $5} ')
if [ -z "${interface_name}" ]; then
  echo "failed to get interface name. change your load.sh and unload.sh manually."
  interface_name="enter_interface_name"
fi

echo -e "#!/bin/bash\nsudo ip link set $interface_name xdpgeneric obj bpf.o sec start" > load.sh
cerr
chmod +x load.sh
cerr

echo -e "#!/bin/bash\nsudo ip link set $interface_name xdpgeneric off" > unload.sh
cerr
chmod +x unload.sh
cerr
