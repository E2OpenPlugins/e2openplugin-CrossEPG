#!/bin/sh
CROSS=/opt/dmm/dm800/build/tmp/cross/mipsel/bin/mipsel-oe-linux-
CFLAGS+=-I/opt/dmm/dm800/build/tmp/staging/mipsel-oe-linux/usr/include/libxml2 \
CC=${CROSS}gcc \
STRIP=${CROSS}strip \
make