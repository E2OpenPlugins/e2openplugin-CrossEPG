#!/bin/sh

[ -d ./tmp ] && rm -rf ./tmp

DEVKIT_ROOT=/mnt/devel/workspace/openee_build/dm800se/build/tmp
CROSS=${DEVKIT_ROOT}/cross/mipsel/bin/mipsel-oe-linux-

export CFLAGS+="-I${DEVKIT_ROOT}/staging/mipsel-oe-linux/usr/include \
 -I${DEVKIT_ROOT}/staging/mipsel-oe-linux/usr/include/libxml2 \
 -I${DEVKIT_ROOT}/staging/mipsel-oe-linux/usr/include/python2.6"
export CC=${CROSS}gcc
export STRIP=${CROSS}strip
export SWIG=${DEVKIT_ROOT}/staging/x86_64-linux/usr/bin/swig
export D=./tmp

make && make install

if [ $? != 0 ]; then
	echo compile error
	exit 1
fi

mkdir -p tmp/CONTROL
cp contrib/control tmp/CONTROL/
VERSION=`cat src/version.h | grep RELEASE | sed "s/.*RELEASE \"//" | sed "s/\"//" | sed "s/\ /-/" | sed "s/\ /-/" | sed "s/(//" | sed "s/)//"`
echo "Package: enigma2-plugin-systemplugins-crossepg" >> tmp/CONTROL/control
echo "Version: $VERSION-r0" >> tmp/CONTROL/control
echo "Architecture: mipsel" >> tmp/CONTROL/control

sh ipkg-build -o root -g root tmp/

[ ! -d out ] && mkdir out
mv *.ipk out
echo "Package moved in `pwd`/out folder"
