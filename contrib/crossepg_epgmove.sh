#!/bin/sh
HOME=`echo $0 | sed "s/crossepg_epgmove\.sh//"`

if [ -f "$HOME/crossepg.config" ] # if configuration exist
then
	DBROOT=`cat $HOME/crossepg.config | grep db_root= | sed "s/db_root=//"`
else # else default path
	DBROOT="/hdd/crossepg"
fi

if [ -f "$DBROOT/ext.epg.dat" ] # try on configuration path
then
	echo copying ext.epg.dat from $DBROOT
	$HOME/crossepg_epgcopy "$DBROOT/ext.epg.dat" /hdd/epg.dat
elif [ -f "/hdd/crossepg/ext.epg.dat" ] # if we have a bad path try with default path
then
	echo copying ext.epg.dat from /hdd/crossepg/
	$HOME/crossepg_epgcopy /hdd/crossepg/ext.epg.dat /hdd/epg.dat
else # no epg found
	echo ext.epg.dat not found
fi
