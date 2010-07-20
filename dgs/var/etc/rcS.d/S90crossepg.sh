#!/bin/sh
export PATH=$PATH:/var/bin
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/var/lib

#[ ! -e /dev/weboutput ] && mkfifo -m 600 /dev/weboutput
sleep 120
[ -e /var/crossepg/crossepgd ] && /var/crossepg/crossepgd &

exit 0
