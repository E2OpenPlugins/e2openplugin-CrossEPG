#!/bin/bash
SVN=`svn info | grep Revision | sed "s/.*:\ //"`
SVN=`expr $SVN + 1`
VERSION=`cat VERSION`

cat src/common.h  | sed "s/RELEASE\ \".*\"/RELEASE \"$VERSION (svn $SVN)\"/" > src/common.h.tmp
mv src/common.h.tmp src/common.h
