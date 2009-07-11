#!/bin/bash
svn log -v --xml | python svn2log.py -s -O -A -L -H -p '/(branches/[^/]+|trunk)/'

