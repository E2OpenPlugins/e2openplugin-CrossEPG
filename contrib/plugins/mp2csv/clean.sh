#!/bin/bash

# delete apple finder fastidious files
find . | grep "\._" | xargs rm 2>/dev/null
find . | grep "\.DS_Store" | xargs rm 2>/dev/null
find . | grep "\.o$" | xargs rm 2>/dev/null

make -f Makefiles/Makefile.sh4 clean
make -f Makefiles/Makefile_e2.sh4 clean
make -f Makefiles/Makefile_e2.mipsel clean

exit 0
