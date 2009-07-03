#!/bin/sh
[ -f /tmp/crossepgd.pid ] && kill `cat /tmp/crossepgd.pid`
sleep 1
tar zxvf crossepg.tar.gz -C /
rm crossepg.tar.gz
rm installer.sh
chmod +x /var/crossepg/crossepgd
chmod +x /var/crossepg/crossepg_alias
chmod +x /var/crossepg/crossepg_importer
chmod +x /var/crossepg/uninstall.sh
/var/crossepg/crossepgd &