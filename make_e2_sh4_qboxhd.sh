#!/bin/sh

[ -d ./tmp ] && rm -rf ./tmp

DEVKIT_ROOT=/opt/STM/STLinux-2.3/devkit/sh4
CROSS=${DEVKIT_ROOT}/bin/sh4-linux-

export CFLAGS="-I${DEVKIT_ROOT}/target/usr/include \
 -I${DEVKIT_ROOT}/target/usr/include/libxml2 \
 -I${DEVKIT_ROOT}/target/usr/include/python2.4"

export CC=${CROSS}gcc
export STRIP=${CROSS}strip
export SWIG=swig
export D=./tmp/qboxhd_update
export TARGET_ARCH=sh4

make && make install-var-qboxhd

if [ $? != 0 ]; then
	echo compile error
	exit 1
fi

VERSION=`cat src/version.h | grep RELEASE | sed "s/.*RELEASE \"//" | sed "s/\"//" | sed "s/\ /-/" | sed "s/\ /-/" | sed "s/(//" | sed "s/)//"`

[ ! -d out ] && mkdir out

# make qboxhd_update
cp contrib/qboxhd_update.sh tmp/qboxhd_update/update.sh
chmod 755 tmp/qboxhd_update/update.sh
chown -R root:root tmp/*
cd tmp
tar zcf ../out/crossepg-${VERSION}-qboxhd_update.tar.gz *
cd ..

# make addon
rm -f tmp/qboxhd_update/update.sh
mkdir -p tmp/qboxhd_update/var/addons/uninstall
cp contrib/qboxhd_uninstall.sh tmp/qboxhd_update/var/addons/uninstall/crossepg_delfile.sh
chmod 755 tmp/qboxhd_update/var/addons/uninstall/crossepg_delfile.sh
mkdir tmp/qboxhd_update/usr/bin 
mkdir tmp/qboxhd_update/usr/local/bin 
cd tmp/qboxhd_update/usr/bin
ln -sf ../local/bin/python python
cd ../..
tar zcf ../../out/crossepg-${VERSION}-qboxhd_addon.tar.gz *
cd ../..


echo "Package moved in `pwd`/out folder"
