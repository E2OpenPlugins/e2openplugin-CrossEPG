#!/bin/bash

# delete apple finder fastidious files
find . | grep "\._" | xargs rm 2>/dev/null
find . | grep "\.DS_Store" | xargs rm 2>/dev/null
find . | grep "\.o$" | xargs rm 2>/dev/null

make -f Makefiles/crossepg.sh4 clean
make -f Makefiles/crossepgd.sh4 clean
make -f Makefiles/crossepg_alias.sh4 clean
make -f Makefiles/crossepg_alias.enigma2.sh4 clean
make -f Makefiles/crossepg_alias.enigma2.mipsel clean
make -f Makefiles/crossepg_dbconverter.enigma2.sh4 clean
make -f Makefiles/crossepg_dbconverter.enigma2.mipsel clean
make -f Makefiles/crossepg_downloader.sh4 clean
make -f Makefiles/crossepg_downloader.enigma2.sh4 clean
make -f Makefiles/crossepg_downloader.enigma2.mipsel clean
make -f Makefiles/crossepg_downloader.enigma1.ppc clean
make -f Makefiles/crossepg_dbinfo.enigma2.sh4 clean
make -f Makefiles/crossepg_dbinfo.enigma2.mipsel clean
make -f Makefiles/crossepg_epgcopy.enigma2.sh4 clean
make -f Makefiles/crossepg_epgcopy.enigma2.mipsel clean
make -f Makefiles/crossepg_importer.sh4 clean
make -f Makefiles/crossepg_importer.enigma2.sh4 clean
make -f Makefiles/crossepg_importer.enigma2.mipsel clean
make -f Makefiles/crossepg_sync.sh4 clean
make -f Makefiles/crossepg_launcher.sh4 clean

rm -rf enigma2/sh4/usr/lib/enigma2/python/Plugins/SystemPlugins/CrossEPG/*
rm -rf enigma2/mipsel/usr/lib/enigma2/python/Plugins/SystemPlugins/CrossEPG/*
rm src/enigma2/python/*.pyc
rm src/enigma2/python/*.pyo

exit 0
