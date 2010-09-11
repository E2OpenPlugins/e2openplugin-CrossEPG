#!/bin/bash
sh ./clean.sh

make -f Makefiles/Makefile_e2.mipsel

chmod +x enigma2/mipsel/usr/crossepg/import/mp2csv.bin

# delete apple finder fastidious files
find . | grep "\._" | xargs rm 2>/dev/null
find . | grep "\.DS_Store" | xargs rm 2>/dev/null
find . | grep "\.o$" | xargs rm 2>/dev/null

rm -rf /tmp/crossepg_ipkg
mkdir /tmp/crossepg_ipkg
cd enigma2/mipsel
tar --exclude-vcs -c * | tar -x -C /tmp/crossepg_ipkg
cd ../..
ipkg-build -o root -g root /tmp/crossepg_ipkg

rm -rf /tmp/crossepg_ipkg
mv *.ipk ../../../crossepg_packages

exit 0
