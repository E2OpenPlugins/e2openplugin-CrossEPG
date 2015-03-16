#!/bin/sh

MACHINE="vuduo2"
OPENPLI_PATH="/home/Openpli/openpli4"

[ -d ./tmp ] && rm -rf ./tmp

DEVKIT_ROOT=$OPENPLI_PATH/openpli-oe-core/build/tmp
CROSS=${DEVKIT_ROOT}/sysroots/x86_64-linux/usr/bin/mipsel-oe-linux/mipsel-oe-linux-

export CFLAGS+="-I${DEVKIT_ROOT}/sysroots/$MACHINE/usr/include \
 -I${DEVKIT_ROOT}/sysroots/$MACHINE/usr/include/libxml2 \
 -I${DEVKIT_ROOT}/sysroots/$MACHINE/usr/include/python2.7"
export CC=${CROSS}gcc
export STRIP=${CROSS}strip
export SWIG=${DEVKIT_ROOT}/sysroots/x86_64-linux/usr/bin/swig
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
