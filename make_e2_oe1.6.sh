#!/bin/sh
CROSS=/opt/dmm/dm800/build/tmp/cross/mipsel/bin/mipsel-oe-linux-
CFLAGS+=-I/opt/dmm/dm800/build/tmp/staging/mipsel-oe-linux/usr/include/libxml2 \
CC=${CROSS}gcc \
STRIP=${CROSS}strip \
make

mkdir -p tmp/usr/crossepg/aliases
mkdir -p tmp/usr/crossepg/providers
mkdir -p tmp/usr/crossepg/import
mkdir -p tmp/usr/lib/enigma2/python/Plugins/SystemPlugins/CrossEPG/skins

cp bin/* tmp/usr/crossepg/
chmod 755 tmp/usr/crossepg/*

cp providers/* tmp/usr/crossepg/providers/

cp src/enigma2/python/skins/*.xml tmp/usr/lib/enigma2/python/Plugins/SystemPlugins/CrossEPG/skins/

cp -rp contrib/po tmp/usr/lib/enigma2/python/Plugins/SystemPlugins/CrossEPG/

python compileb.py