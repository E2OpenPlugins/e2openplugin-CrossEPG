#!/bin/sh

echo "Removing CrossEPG plugin ..." > /.remove_progress

rm -rf /usr/local/lib/enigma2/python/Plugins/SystemPlugins/CrossEPG
rm -rf /var/crossepg
rm -rf /usr/local/lib/python2.6/crossepg.py
rm -rf /usr/local/lib/python2.6/lib-dynload/_crossepg.so

sync

echo "Done." > /.remove_progress

exit 0
