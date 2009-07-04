#!/bin/bash
sh ./clean.sh

make -f Makefiles/crossepg.sh4
make -f Makefiles/crossepgd.sh4
make -f Makefiles/crossepg_alias.sh4
#make -f Makefiles/crossepg_alias.enigma2.sh4
#make -f Makefiles/crossepg_alias.enigma2.mipsel
make -f Makefiles/crossepg_dbconverter.enigma2.sh4
make -f Makefiles/crossepg_dbconverter.enigma2.mipsel
make -f Makefiles/crossepg_downloader.sh4
make -f Makefiles/crossepg_downloader.enigma2.sh4
make -f Makefiles/crossepg_downloader.enigma2.mipsel
make -f Makefiles/crossepg_importer.sh4
#make -f Makefiles/crossepg_importer.enigma2.sh4
#make -f Makefiles/crossepg_importer.enigma2.mipsel
make -f Makefiles/crossepg_sync.sh4
make -f Makefiles/crossepg_launcher.sh4


cp src/enigma2/python/*.py enigma2/sh4/usr/lib/enigma2/python/Plugins/SystemPlugins/CrossEPG/
mkdir enigma2/sh4/usr/lib/enigma2/python/Plugins/SystemPlugins/CrossEPG/skins
cp src/enigma2/python/skins/*.xml enigma2/sh4/usr/lib/enigma2/python/Plugins/SystemPlugins/CrossEPG/skins
cp src/enigma2/python/*.py enigma2/mipsel/usr/lib/enigma2/python/Plugins/SystemPlugins/CrossEPG/
mkdir enigma2/mipsel/usr/lib/enigma2/python/Plugins/SystemPlugins/CrossEPG/skins
cp src/enigma2/python/skins/*.xml enigma2/mipsel/usr/lib/enigma2/python/Plugins/SystemPlugins/CrossEPG/skins

# delete apple finder fastidious files
find . | grep "\._" | xargs rm 2>/dev/null
find . | grep "\.DS_Store" | xargs rm 2>/dev/null
find . | grep "\.o$" | xargs rm 2>/dev/null

# get the versione and generate filename
VERSION=`cat src/common.h | grep RELEASE | sed "s/.*RELEASE \"//" | sed "s/\"//" | sed "s/\ /-/" | sed "s/\ /-/" | sed "s/(//" | sed "s/)//"`
FILENAME=../crossepg_packages/crossepg-$VERSION-full.tar.gz
FILENAME_DGS_SH4=../crossepg_packages/crossepg-$VERSION-dgs-sh4.tar.gz
FILENAME_E2_SH4=../crossepg_packages/crossepg-$VERSION-enigma2-sh4.tar.gz
FILENAME_E2_MIPSEL=../crossepg_packages/crossepg-$VERSION-enigma2-mipsel.tar.gz
FILENAME_E1_PPC=../crossepg_packages/crossepg-$VERSION-enigma1-ppc.tar.gz

tar --exclude-vcs -zcf $FILENAME ../crossepg
cd dgs
tar --exclude-vcs -zcf crossepg.tar.gz var
tar --exclude-vcs -zcf ../$FILENAME_DGS_SH4 crossepg.tar.gz installer.sh
rm crossepg.tar.gz
cd ..
cd enigma2/sh4
tar --exclude-vcs -zcf ../../$FILENAME_E2_SH4 *
cd ../..
cd enigma2/mipsel
tar --exclude-vcs -zcf ../../$FILENAME_E2_MIPSEL *
cd ../..
cd enigma1/ppc
tar --exclude-vcs -zcf ../../$FILENAME_E1_PPC *
cd ../..

exit 0
