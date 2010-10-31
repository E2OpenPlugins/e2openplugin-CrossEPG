#!/bin/sh

[ -d ./tmp ] && rm -rf ./tmp

export CFLAGS+="-I/usr/include \
 -I/usr/include/libxml2 \
 -I/usr/include/python2.6"
export CC=gcc
export STRIP=strip
export SWIG=swig
export D=./tmp

make && make install-standalone && make install-python

if [ $? != 0 ]; then
	echo compile error
	exit 1
fi

VERSION=`cat src/version.h | grep RELEASE | sed "s/.*RELEASE \"//" | sed "s/\"//" | sed "s/\ /-/" | sed "s/\ /-/" | sed "s/(//" | sed "s/)//"`

[ ! -d out ] && mkdir out

cd tmp
tar zcf ../out/crossepg-standalone_${VERSION}_x86.tar.gz *
cd ..

echo "Package moved in `pwd`/out folder"
