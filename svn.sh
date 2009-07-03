#!/bin/bash
sh ./clean.sh
sh ./version.sh
rm -rf enigma2/sh4/usr/lib/enigma2/python/Plugins/SystemPlugins/CrossEPG/*
rm -rf enigma2/mipsel/usr/lib/enigma2/python/Plugins/SystemPlugins/CrossEPG/*

# delete apple finder fastidious files
find . | grep "\._" | xargs rm 2>/dev/null
find . | grep "\.DS_Store" | xargs rm 2>/dev/null
find . | grep "\.o$" | xargs rm 2>/dev/null

./svnadd
svn commit
