#!/bin/sh
export PATH=$PATH:/var/bin
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/var/lib

sleep 60

[ -e /var/crossepg/crossepgd ] && /var/crossepg/crossepgd &

exit 0
