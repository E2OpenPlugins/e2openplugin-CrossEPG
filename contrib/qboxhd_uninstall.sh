#!/bin/sh

echo "Removing CrossEPG plugin ..." > /.remove_progress

rm -rf /usr/lib/enigma2/python/Plugins/SystemPlugins/CrossEPG
rm -rf /var/crossepg

sync

echo "Done." > /.remove_progress

exit 0
