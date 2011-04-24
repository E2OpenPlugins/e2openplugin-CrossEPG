#!/bin/sh
HOME=`echo $0 | sed "s/crossepg_epgmove\.sh//"`

if [ -f "$HOME/crossepg.config" ] # if configuration exist
then
	DBROOT=`cat $HOME/crossepg.config | grep db_root= | sed "s/db_root=//"`
else # else default path
	DBROOT="/hdd/crossepg"
fi

if [ -f "/etc/enigma2/settings" ] # if enigma2 configuration exist
then
	EPGDAT=`cat /etc/enigma2/settings | grep "config\.misc\.epgcache_filename=" | sed "s/config\.misc\.epgcache_filename=//"`
else # else default path
	EPGDAT="/hdd/epg.dat"
fi
echo $EPGDAT
if [ -f "$DBROOT/ext.epg.dat" ] # try on configuration path
then
	echo copying ext.epg.dat from $DBROOT
	$HOME/crossepg_epgcopy "$DBROOT/ext.epg.dat" $EPGDAT
elif [ -f "/hdd/crossepg/ext.epg.dat" ] # if we have a bad path try with default path
then
	echo copying ext.epg.dat from /hdd/crossepg/
	$HOME/crossepg_epgcopy /hdd/crossepg/ext.epg.dat /hdd/epg.dat
else # no epg found
	echo ext.epg.dat not found
fi
