#!/bin/bash
sh ./clean.sh

#make -f Makefiles/crossepg.sh4
#make -f Makefiles/crossepgd.sh4
#make -f Makefiles/crossepg_alias.sh4
#make -f Makefiles/crossepg_alias.enigma2.sh4
#make -f Makefiles/crossepg_alias.enigma2.mipsel
make -f Makefiles/crossepg_dbconverter.enigma2.sh4
make -f Makefiles/crossepg_dbconverter.enigma2.mipsel
#make -f Makefiles/crossepg_downloader.sh4
make -f Makefiles/crossepg_downloader.enigma2.sh4
make -f Makefiles/crossepg_downloader.enigma2.mipsel
#make -f Makefiles/crossepg_importer.sh4
#make -f Makefiles/crossepg_importer.enigma2.sh4
#make -f Makefiles/crossepg_importer.enigma2.mipsel
#make -f Makefiles/crossepg_sync.sh4
#make -f Makefiles/crossepg_launcher.sh4


cp -r src/enigma2/python/* enigma2/sh4/usr/lib/enigma2/python/Plugins/SystemPlugins/CrossEPG/
cp -r src/enigma2/python/* enigma2/mipsel/usr/lib/enigma2/python/Plugins/SystemPlugins/CrossEPG/

# delete apple finder fastidious files
find . | grep "\._" | xargs rm 2>/dev/null
find . | grep "\.DS_Store" | xargs rm 2>/dev/null
find . | grep "\.o$" | xargs rm 2>/dev/null

# get the versione and generate filename
FILENAME=../crossepg_packages/crossepg-`cat src/common.h | grep RELEASE | sed "s/.*RELEASE \"//" | sed "s/\"//"`-full.tar.gz
FILENAME_DGS_SH4=../crossepg_packages/crossepg-`cat src/common.h | grep RELEASE | sed "s/.*RELEASE \"//" | sed "s/\"//"`-dgs-sh4.tar.gz
FILENAME_E2_SH4=../crossepg_packages/crossepg-`cat src/common.h | grep RELEASE | sed "s/.*RELEASE \"//" | sed "s/\"//"`-enigma2-sh4.tar.gz
FILENAME_E2_MIPSEL=../crossepg_packages/crossepg-`cat src/common.h | grep RELEASE | sed "s/.*RELEASE \"//" | sed "s/\"//"`-enigma2-mipsel.tar.gz

tar zcf $FILENAME ../crossepg
cd dgs
tar zcf crossepg.tar.gz var
tar zcf ../$FILENAME_DGS_SH4 crossepg.tar.gz installer.sh
rm crossepg.tar.gz
cd ..
cd enigma2/sh4
tar zcf ../../$FILENAME_E2_SH4 *
cd ../..
cd enigma2/mipsel
tar zcf ../../$FILENAME_E2_MIPSEL *
cd ../..

exit 0
