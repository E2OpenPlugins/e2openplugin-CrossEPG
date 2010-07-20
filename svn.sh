#!/bin/bash
sh ./clean.sh
sh ./version.sh

# delete apple finder fastidious files
find . | grep "\._" | xargs rm 2>/dev/null
find . | grep "\.DS_Store" | xargs rm 2>/dev/null
find . | grep "\.o$" | xargs rm 2>/dev/null

./svnadd .
svn commit --username sandro.cavazzoni
