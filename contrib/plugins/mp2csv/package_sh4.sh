#!/bin/bash
sh ./clean.sh

make -f Makefiles/Makefile.sh4

# delete apple finder fastidious files
find . | grep "\._" | xargs rm 2>/dev/null
find . | grep "\.DS_Store" | xargs rm 2>/dev/null
find . | grep "\.o$" | xargs rm 2>/dev/null

# get the versione and generate filename
VERSION=0.1.0
FILENAME_DGS_SH4=../../../crossepg_packages/mp2csv-crossepg-$VERSION-dgs-sh4.tar.gz

#tar --exclude-vcs -zcf $FILENAME ../crossepg
cd dgs
tar --exclude-vcs -zcf mp2csv-crossepg.tar.gz var
tar --exclude-vcs -zcf ../$FILENAME_DGS_SH4 mp2csv-crossepg.tar.gz installer.sh
rm mp2csv-crossepg.tar.gz
cd ..

exit 0