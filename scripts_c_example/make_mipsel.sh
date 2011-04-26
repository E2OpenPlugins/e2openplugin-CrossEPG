#!/bin/sh

DEVKIT_ROOT=/home/skaman/gecko/et9000/build/tmp
CROSS=${DEVKIT_ROOT}/cross/mipsel/bin/mipsel-oe-linux-

export CFLAGS+="-I${DEVKIT_ROOT}/staging/mipsel-oe-linux/usr/include \
 -I${DEVKIT_ROOT}/staging/mipsel-oe-linux/usr/include/libxml2 \
 -I${DEVKIT_ROOT}/staging/mipsel-oe-linux/usr/include/python2.6"
export CC=${CROSS}gcc
export STRIP=${CROSS}strip

make
