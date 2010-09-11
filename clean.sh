#!/bin/bash

# delete apple finder fastidious files
find . | grep "\._" | xargs rm 2>/dev/null
find . | grep "\.DS_Store" | xargs rm 2>/dev/null
find . | grep "\.o$" | xargs rm 2>/dev/null

make clean

exit 0
