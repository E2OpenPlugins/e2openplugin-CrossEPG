#!/bin/sh
[ -f /tmp/crossepgd.pid ] && kill `cat /tmp/crossepgd.pid`
sleep 1
cd /
rm -rf /var/crossepg
rm /var/plug-in/crossepg.plugin
rm /var/plug-in/crossepg.downloader.plugin
rm /var/plug-in/crossepg.sync.plugin
rm /var/plug-in/crossepg.launcher.plugin
rm /var/plug-in/crossepg.config
rm /var/etc/rcS.d/S90crossepg.sh
