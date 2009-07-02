#!/bin/sh
HOME=`echo $0 | sed "s/crossepg_epgmove\.sh//"`
DBROOT=`cat $HOME/crossepg.config | grep db_root= | sed "s/db_root=//"`
if [ -f "$DBROOT/ext.epg.dat" ]
then
	echo copying ext.epg.dat from $DBROOT
	cp "$DBROOT/ext.epg.dat" /hdd/epg.dat
elif [ -f "/hdd/crossepg/ext.epg.dat" ]
then
	echo copying ext.epg.dat from /hdd/crossepg/
	cp /hdd/crossepg/ext.epg.dat /hdd/epg.dat
else
	echo ext.epg.dat not found
fi
