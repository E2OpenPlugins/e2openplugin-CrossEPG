#!/bin/sh
HOME=`echo $0 | sed "s/crossepg_prepare_pre_start\.sh//"`
ENIGMA2PRESTART="/usr/bin/enigma2_pre_start.sh"

if [ -f "/var/crossepg/crossepg.config" ]
then
	HOME="/var/crossepg/"
else
	HOME="/usr/crossepg/"
fi

if [ -f "$ENIGMA2PRESTART" ]
then
	TMP=`cat /usr/bin/enigma2_pre_start.sh  | grep "crossepg_epgmove\.sh"`
	if [ ! -n "$TMP" ]
	then
		echo "" >> $ENIGMA2PRESTART
		echo "${HOME}crossepg_epgmove.sh" >> $ENIGMA2PRESTART
		chmod +x $ENIGMA2PRESTART
	fi
else
	echo "#!/bin/sh" > $ENIGMA2PRESTART
	echo "${HOME}crossepg_epgmove.sh" >> $ENIGMA2PRESTART
	chmod +x $ENIGMA2PRESTART
fi
	