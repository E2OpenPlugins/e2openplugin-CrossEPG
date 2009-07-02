#!/bin/sh
export PATH=$PATH:/var/bin
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/var/lib

sleep 60

if [ -e /var/myepg/myepgd ]; then
	/var/myepg/myepgd &
fi

exit 0
