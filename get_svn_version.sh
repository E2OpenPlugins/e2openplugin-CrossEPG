#!/bin/sh
cat .svn/all-wcprops | grep "^\/svn\/\!svn\/ver" | grep "trunk$" | tail -n 1 | sed "s/.*ver\///" | sed "s/\/trunk//"
