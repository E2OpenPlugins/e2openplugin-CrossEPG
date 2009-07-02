#!/bin/sh
[ -f /tmp/myepgd.pid ] && kill `cat /tmp/myepgd.pid`
sleep 1
tar zxf myepg.tar.gz -C /
rm myepg.tar.gz
rm installer.sh
/var/myepg/myepgd &