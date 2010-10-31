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
export D=./tmp/qboxhd_update

make && make install-var

if [ $? != 0 ]; then
	echo compile error
	exit 1
fi

mkdir -p tmp/qboxhd_update/var/addons/uninstall
cp contrib/qboxhd_update.sh tmp/update.sh
cp contrib/qboxhd_uninstall.sh tmp/qboxhd_update/var/addons/uninstall/crossepg_delfile.sh
chmod 755 tmp/update.sh
chmod 755 tmp/qboxhd_update/var/addons/uninstall/crossepg_delfile.sh

VERSION=`cat src/version.h | grep RELEASE | sed "s/.*RELEASE \"//" | sed "s/\"//" | sed "s/\ /-/" | sed "s/\ /-/" | sed "s/(//" | sed "s/)//"`

[ ! -d out ] && mkdir out

cd tmp
tar zcf ../out/enigma2-plugin-systemplugins-crossepg-qboxhd_${VERSION}_sh4.tar.gz *
cd ..

echo "Package moved in `pwd`/out folder"
