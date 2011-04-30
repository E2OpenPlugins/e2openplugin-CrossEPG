#!/bin/sh

[ -d ./tmp ] && rm -rf ./tmp

DEVKIT_ROOT=/opt/STM/STLinux-2.3/devkit/sh4
CROSS=${DEVKIT_ROOT}/bin/sh4-linux-

export CFLAGS+="-I${DEVKIT_ROOT}/target/usr/include \
 -I${DEVKIT_ROOT}/target/usr/include/libxml2 \
 -I${DEVKIT_ROOT}/target/usr/include/python2.6"
export CC=${CROSS}gcc
export STRIP=${CROSS}strip
export SWIG=swig
export D=./tmp
export TARGET_ARCH=sh4

make && make install-var

if [ $? != 0 ]; then
	echo compile error
	exit 1
fi

mkdir -p tmp/CONTROL
cp contrib/control tmp/CONTROL/
VERSION=`cat src/version.h | grep RELEASE | sed "s/.*RELEASE \"//" | sed "s/\"//" | sed "s/\ /-/" | sed "s/\ /-/" | sed "s/(//" | sed "s/)//"`
echo "Package: enigma2-plugin-systemplugins-crossepg" >> tmp/CONTROL/control
echo "Version: $VERSION" >> tmp/CONTROL/control
echo "Architecture: sh4" >> tmp/CONTROL/control

sh ipkg-build -o root -g root tmp/

[ ! -d out ] && mkdir out
mv *.ipk out
echo "Package moved in `pwd`/out folder"
