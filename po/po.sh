BINDIR="/home/malo/OEDev/Openpli/openpli4/openpli-oe-core/build/tmp/sysroots/x86_64-linux/usr/bin"
LANGUAGES="ar bg ca cs da de el en en_GB es et fi fr fy hr hu is it lt lv nl no pl pt ru sv sk sl sr th tr uk"

$BINDIR/xgettext -L Python --from-code=UTF-8 --add-comments="TRANSLATORS:" -d enigma2 -s -o crossepg.pot ../src/enigma2/python/plugin.py ../src/enigma2/python/crossepglib.py ../src/enigma2/python/crossepg_xepgdb_update.py ../src/enigma2/python/crossepg_setup.py ../src/enigma2/python/crossepg_rytec_update.py ../src/enigma2/python/crossepg_providers.py ../src/enigma2/python/crossepg_ordering.py ../src/enigma2/python/crossepg_menu.py ../src/enigma2/python/crossepg_main.py ../src/enigma2/python/crossepg_locale.py ../src/enigma2/python/crossepg_loader.py ../src/enigma2/python/crossepg_info.py ../src/enigma2/python/crossepg_importer.py ../src/enigma2/python/crossepg_downloader.py ../src/enigma2/python/crossepg_converter.py ../src/enigma2/python/crossepg_auto.py ../src/enigma2/python/crossepg_about.py ../src/enigma2/python/__init__.py

for lang in $LANGUAGES ; do
	if [ -f $lang.po ]; then
		$BINDIR/msgmerge --backup=none --no-location -s -N -U $lang.po crossepg.pot && touch $lang.po;
	else
		$BINDIR/msginit -l $lang.po -o $lang.po -i crossepg.pot --no-translator;
	fi
done

for lang in $LANGUAGES ; do
	$BINDIR/msgfmt -o $lang.mo $lang.po
done

