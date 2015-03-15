/media/Dev/STB_Builds/openvix/experimental/builds/vuultimo/tmp/sysroots/x86_64-linux/usr/bin/xgettext -L Python --from-code=UTF-8 --add-comments="TRANSLATORS:" -d enigma2 -s -o crossepg.pot ../src/enigma2/python/plugin.py ../src/enigma2/python/crossepglib.py ../src/enigma2/python/crossepg_xepgdb_update.py ../src/enigma2/python/crossepg_setup.py ../src/enigma2/python/crossepg_rytec_update.py ../src/enigma2/python/crossepg_providers.py ../src/enigma2/python/crossepg_ordering.py ../src/enigma2/python/crossepg_menu.py ../src/enigma2/python/crossepg_main.py ../src/enigma2/python/crossepg_locale.py ../src/enigma2/python/crossepg_loader.py ../src/enigma2/python/crossepg_info.py ../src/enigma2/python/crossepg_importer.py ../src/enigma2/python/crossepg_downloader.py ../src/enigma2/python/crossepg_converter.py ../src/enigma2/python/crossepg_auto.py ../src/enigma2/python/crossepg_about.py ../src/enigma2/python/__init__.py
if [ -f ar.po ]; then
	/media/Dev/STB_Builds/openvix/experimental/builds/vuultimo/tmp/sysroots/x86_64-linux/usr/bin/msgmerge --backup=none --no-location -s -N -U ar.po crossepg.pot && touch ar.po;
else
	/media/Dev/STB_Builds/openvix/experimental/builds/vuultimo/tmp/sysroots/x86_64-linux/usr/bin/msginit -l ar.po -o ar.po -i crossepg.pot --no-translator;
fi
if [ -f bg.po ]; then
	/media/Dev/STB_Builds/openvix/experimental/builds/vuultimo/tmp/sysroots/x86_64-linux/usr/bin/msgmerge --backup=none --no-location -s -N -U bg.po crossepg.pot && touch bg.po;
else
	/media/Dev/STB_Builds/openvix/experimental/builds/vuultimo/tmp/sysroots/x86_64-linux/usr/bin/msginit -l bg.po -o bg.po -i crossepg.pot --no-translator;
fi
if [ -f ca.po ]; then
	/media/Dev/STB_Builds/openvix/experimental/builds/vuultimo/tmp/sysroots/x86_64-linux/usr/bin/msgmerge --backup=none --no-location -s -N -U ca.po crossepg.pot && touch ca.po;
else
	/media/Dev/STB_Builds/openvix/experimental/builds/vuultimo/tmp/sysroots/x86_64-linux/usr/bin/msginit -l ca.po -o ca.po -i crossepg.pot --no-translator;
fi
if [ -f cs.po ]; then
	/media/Dev/STB_Builds/openvix/experimental/builds/vuultimo/tmp/sysroots/x86_64-linux/usr/bin/msgmerge --backup=none --no-location -s -N -U cs.po crossepg.pot && touch cs.po;
else
	/media/Dev/STB_Builds/openvix/experimental/builds/vuultimo/tmp/sysroots/x86_64-linux/usr/bin/msginit -l cs.po -o cs.po -i crossepg.pot --no-translator;
fi
if [ -f da.po ]; then
	/media/Dev/STB_Builds/openvix/experimental/builds/vuultimo/tmp/sysroots/x86_64-linux/usr/bin/msgmerge --backup=none --no-location -s -N -U da.po crossepg.pot && touch da.po;
else
	/media/Dev/STB_Builds/openvix/experimental/builds/vuultimo/tmp/sysroots/x86_64-linux/usr/bin/msginit -l da.po -o da.po -i crossepg.pot --no-translator;
fi
if [ -f de.po ]; then
	/media/Dev/STB_Builds/openvix/experimental/builds/vuultimo/tmp/sysroots/x86_64-linux/usr/bin/msgmerge --backup=none --no-location -s -N -U de.po crossepg.pot && touch de.po;
else
	/media/Dev/STB_Builds/openvix/experimental/builds/vuultimo/tmp/sysroots/x86_64-linux/usr/bin/msginit -l de.po -o de.po -i crossepg.pot --no-translator;
fi
if [ -f el.po ]; then
	/media/Dev/STB_Builds/openvix/experimental/builds/vuultimo/tmp/sysroots/x86_64-linux/usr/bin/msgmerge --backup=none --no-location -s -N -U el.po crossepg.pot && touch el.po;
else
	/media/Dev/STB_Builds/openvix/experimental/builds/vuultimo/tmp/sysroots/x86_64-linux/usr/bin/msginit -l el.po -o el.po -i crossepg.pot --no-translator;
fi
if [ -f en.po ]; then
	/media/Dev/STB_Builds/openvix/experimental/builds/vuultimo/tmp/sysroots/x86_64-linux/usr/bin/msgmerge --backup=none --no-location -s -N -U en.po crossepg.pot && touch en.po;
else
	/media/Dev/STB_Builds/openvix/experimental/builds/vuultimo/tmp/sysroots/x86_64-linux/usr/bin/msginit -l en.po -o en.po -i crossepg.pot --no-translator;
fi
if [ -f en_GB.po ]; then
	/media/Dev/STB_Builds/openvix/experimental/builds/vuultimo/tmp/sysroots/x86_64-linux/usr/bin/msgmerge --backup=none --no-location -s -N -U en_GB.po crossepg.pot && touch en_GB.po;
else
	/media/Dev/STB_Builds/openvix/experimental/builds/vuultimo/tmp/sysroots/x86_64-linux/usr/bin/msginit -l en_GB.po -o en_GB.po -i crossepg.pot --no-translator;
fi
if [ -f es.po ]; then
	/media/Dev/STB_Builds/openvix/experimental/builds/vuultimo/tmp/sysroots/x86_64-linux/usr/bin/msgmerge --backup=none --no-location -s -N -U es.po crossepg.pot && touch es.po;
else
	/media/Dev/STB_Builds/openvix/experimental/builds/vuultimo/tmp/sysroots/x86_64-linux/usr/bin/msginit -l es.po -o es.po -i crossepg.pot --no-translator;
fi
if [ -f et.po ]; then
	/media/Dev/STB_Builds/openvix/experimental/builds/vuultimo/tmp/sysroots/x86_64-linux/usr/bin/msgmerge --backup=none --no-location -s -N -U et.po crossepg.pot && touch et.po;
else
	/media/Dev/STB_Builds/openvix/experimental/builds/vuultimo/tmp/sysroots/x86_64-linux/usr/bin/msginit -l et.po -o et.po -i crossepg.pot --no-translator;
fi
if [ -f fi.po ]; then
	/media/Dev/STB_Builds/openvix/experimental/builds/vuultimo/tmp/sysroots/x86_64-linux/usr/bin/msgmerge --backup=none --no-location -s -N -U fi.po crossepg.pot && touch fi.po;
else
	/media/Dev/STB_Builds/openvix/experimental/builds/vuultimo/tmp/sysroots/x86_64-linux/usr/bin/msginit -l fi.po -o fi.po -i crossepg.pot --no-translator;
fi
if [ -f fr.po ]; then
	/media/Dev/STB_Builds/openvix/experimental/builds/vuultimo/tmp/sysroots/x86_64-linux/usr/bin/msgmerge --backup=none --no-location -s -N -U fr.po crossepg.pot && touch fr.po;
else
	/media/Dev/STB_Builds/openvix/experimental/builds/vuultimo/tmp/sysroots/x86_64-linux/usr/bin/msginit -l fr.po -o fr.po -i crossepg.pot --no-translator;
fi
if [ -f fy.po ]; then
	/media/Dev/STB_Builds/openvix/experimental/builds/vuultimo/tmp/sysroots/x86_64-linux/usr/bin/msgmerge --backup=none --no-location -s -N -U fy.po crossepg.pot && touch fy.po;
else
	/media/Dev/STB_Builds/openvix/experimental/builds/vuultimo/tmp/sysroots/x86_64-linux/usr/bin/msginit -l fy.po -o fy.po -i crossepg.pot --no-translator;
fi
if [ -f hr.po ]; then
	/media/Dev/STB_Builds/openvix/experimental/builds/vuultimo/tmp/sysroots/x86_64-linux/usr/bin/msgmerge --backup=none --no-location -s -N -U hr.po crossepg.pot && touch hr.po;
else
	/media/Dev/STB_Builds/openvix/experimental/builds/vuultimo/tmp/sysroots/x86_64-linux/usr/bin/msginit -l hr.po -o hr.po -i crossepg.pot --no-translator;
fi
if [ -f hu.po ]; then
	/media/Dev/STB_Builds/openvix/experimental/builds/vuultimo/tmp/sysroots/x86_64-linux/usr/bin/msgmerge --backup=none --no-location -s -N -U hu.po crossepg.pot && touch hu.po;
else
	/media/Dev/STB_Builds/openvix/experimental/builds/vuultimo/tmp/sysroots/x86_64-linux/usr/bin/msginit -l hu.po -o hu.po -i crossepg.pot --no-translator;
fi
if [ -f is.po ]; then
	/media/Dev/STB_Builds/openvix/experimental/builds/vuultimo/tmp/sysroots/x86_64-linux/usr/bin/msgmerge --backup=none --no-location -s -N -U is.po crossepg.pot && touch is.po;
else
	/media/Dev/STB_Builds/openvix/experimental/builds/vuultimo/tmp/sysroots/x86_64-linux/usr/bin/msginit -l is.po -o is.po -i crossepg.pot --no-translator;
fi
if [ -f it.po ]; then
	/media/Dev/STB_Builds/openvix/experimental/builds/vuultimo/tmp/sysroots/x86_64-linux/usr/bin/msgmerge --backup=none --no-location -s -N -U it.po crossepg.pot && touch it.po;
else
	/media/Dev/STB_Builds/openvix/experimental/builds/vuultimo/tmp/sysroots/x86_64-linux/usr/bin/msginit -l it.po -o it.po -i crossepg.pot --no-translator;
fi
if [ -f lt.po ]; then
	/media/Dev/STB_Builds/openvix/experimental/builds/vuultimo/tmp/sysroots/x86_64-linux/usr/bin/msgmerge --backup=none --no-location -s -N -U lt.po crossepg.pot && touch lt.po;
else
	/media/Dev/STB_Builds/openvix/experimental/builds/vuultimo/tmp/sysroots/x86_64-linux/usr/bin/msginit -l lt.po -o lt.po -i crossepg.pot --no-translator;
fi
if [ -f lv.po ]; then
	/media/Dev/STB_Builds/openvix/experimental/builds/vuultimo/tmp/sysroots/x86_64-linux/usr/bin/msgmerge --backup=none --no-location -s -N -U lv.po crossepg.pot && touch lv.po;
else
	/media/Dev/STB_Builds/openvix/experimental/builds/vuultimo/tmp/sysroots/x86_64-linux/usr/bin/msginit -l lv.po -o lv.po -i crossepg.pot --no-translator;
fi
if [ -f nl.po ]; then
	/media/Dev/STB_Builds/openvix/experimental/builds/vuultimo/tmp/sysroots/x86_64-linux/usr/bin/msgmerge --backup=none --no-location -s -N -U nl.po crossepg.pot && touch nl.po;
else
	/media/Dev/STB_Builds/openvix/experimental/builds/vuultimo/tmp/sysroots/x86_64-linux/usr/bin/msginit -l nl.po -o nl.po -i crossepg.pot --no-translator;
fi
if [ -f no.po ]; then
	/media/Dev/STB_Builds/openvix/experimental/builds/vuultimo/tmp/sysroots/x86_64-linux/usr/bin/msgmerge --backup=none --no-location -s -N -U no.po crossepg.pot && touch no.po;
else
	/media/Dev/STB_Builds/openvix/experimental/builds/vuultimo/tmp/sysroots/x86_64-linux/usr/bin/msginit -l no.po -o no.po -i crossepg.pot --no-translator;
fi
if [ -f pl.po ]; then
	/media/Dev/STB_Builds/openvix/experimental/builds/vuultimo/tmp/sysroots/x86_64-linux/usr/bin/msgmerge --backup=none --no-location -s -N -U pl.po crossepg.pot && touch pl.po;
else
	/media/Dev/STB_Builds/openvix/experimental/builds/vuultimo/tmp/sysroots/x86_64-linux/usr/bin/msginit -l pl.po -o pl.po -i crossepg.pot --no-translator;
fi
if [ -f pt.po ]; then
	/media/Dev/STB_Builds/openvix/experimental/builds/vuultimo/tmp/sysroots/x86_64-linux/usr/bin/msgmerge --backup=none --no-location -s -N -U pt.po crossepg.pot && touch pt.po;
else
	/media/Dev/STB_Builds/openvix/experimental/builds/vuultimo/tmp/sysroots/x86_64-linux/usr/bin/msginit -l pt.po -o pt.po -i crossepg.pot --no-translator;
fi
if [ -f ru.po ]; then
	/media/Dev/STB_Builds/openvix/experimental/builds/vuultimo/tmp/sysroots/x86_64-linux/usr/bin/msgmerge --backup=none --no-location -s -N -U ru.po crossepg.pot && touch ru.po;
else
	/media/Dev/STB_Builds/openvix/experimental/builds/vuultimo/tmp/sysroots/x86_64-linux/usr/bin/msginit -l ru.po -o ru.po -i crossepg.pot --no-translator;
fi
if [ -f sv.po ]; then
	/media/Dev/STB_Builds/openvix/experimental/builds/vuultimo/tmp/sysroots/x86_64-linux/usr/bin/msgmerge --backup=none --no-location -s -N -U sv.po crossepg.pot && touch sv.po;
else
	/media/Dev/STB_Builds/openvix/experimental/builds/vuultimo/tmp/sysroots/x86_64-linux/usr/bin/msginit -l sv.po -o sv.po -i crossepg.pot --no-translator;
fi
if [ -f sk.po ]; then
	/media/Dev/STB_Builds/openvix/experimental/builds/vuultimo/tmp/sysroots/x86_64-linux/usr/bin/msgmerge --backup=none --no-location -s -N -U sk.po crossepg.pot && touch sk.po;
else
	/media/Dev/STB_Builds/openvix/experimental/builds/vuultimo/tmp/sysroots/x86_64-linux/usr/bin/msginit -l sk.po -o sk.po -i crossepg.pot --no-translator;
fi
if [ -f sl.po ]; then
	/media/Dev/STB_Builds/openvix/experimental/builds/vuultimo/tmp/sysroots/x86_64-linux/usr/bin/msgmerge --backup=none --no-location -s -N -U sl.po crossepg.pot && touch sl.po;
else
	/media/Dev/STB_Builds/openvix/experimental/builds/vuultimo/tmp/sysroots/x86_64-linux/usr/bin/msginit -l sl.po -o sl.po -i crossepg.pot --no-translator;
fi
if [ -f sr.po ]; then
	/media/Dev/STB_Builds/openvix/experimental/builds/vuultimo/tmp/sysroots/x86_64-linux/usr/bin/msgmerge --backup=none --no-location -s -N -U sr.po crossepg.pot && touch sr.po;
else
	/media/Dev/STB_Builds/openvix/experimental/builds/vuultimo/tmp/sysroots/x86_64-linux/usr/bin/msginit -l sr.po -o sr.po -i crossepg.pot --no-translator;
fi
if [ -f th.po ]; then
	/media/Dev/STB_Builds/openvix/experimental/builds/vuultimo/tmp/sysroots/x86_64-linux/usr/bin/msgmerge --backup=none --no-location -s -N -U th.po crossepg.pot && touch th.po;
else
	/media/Dev/STB_Builds/openvix/experimental/builds/vuultimo/tmp/sysroots/x86_64-linux/usr/bin/msginit -l th.po -o th.po -i crossepg.pot --no-translator;
fi
if [ -f tr.po ]; then
	/media/Dev/STB_Builds/openvix/experimental/builds/vuultimo/tmp/sysroots/x86_64-linux/usr/bin/msgmerge --backup=none --no-location -s -N -U tr.po crossepg.pot && touch tr.po;
else
	/media/Dev/STB_Builds/openvix/experimental/builds/vuultimo/tmp/sysroots/x86_64-linux/usr/bin/msginit -l tr.po -o tr.po -i crossepg.pot --no-translator;
fi
if [ -f uk.po ]; then
	/media/Dev/STB_Builds/openvix/experimental/builds/vuultimo/tmp/sysroots/x86_64-linux/usr/bin/msgmerge --backup=none --no-location -s -N -U uk.po crossepg.pot && touch uk.po;
else
	/media/Dev/STB_Builds/openvix/experimental/builds/vuultimo/tmp/sysroots/x86_64-linux/usr/bin/msginit -l uk.po -o uk.po -i crossepg.pot --no-translator;
fi
/media/Dev/STB_Builds/openvix/experimental/builds/vuultimo/tmp/sysroots/x86_64-linux/usr/bin/msgfmt -o ar.mo ar.po
/media/Dev/STB_Builds/openvix/experimental/builds/vuultimo/tmp/sysroots/x86_64-linux/usr/bin/msgfmt -o bg.mo bg.po
/media/Dev/STB_Builds/openvix/experimental/builds/vuultimo/tmp/sysroots/x86_64-linux/usr/bin/msgfmt -o ca.mo ca.po
/media/Dev/STB_Builds/openvix/experimental/builds/vuultimo/tmp/sysroots/x86_64-linux/usr/bin/msgfmt -o cs.mo cs.po
/media/Dev/STB_Builds/openvix/experimental/builds/vuultimo/tmp/sysroots/x86_64-linux/usr/bin/msgfmt -o da.mo da.po
/media/Dev/STB_Builds/openvix/experimental/builds/vuultimo/tmp/sysroots/x86_64-linux/usr/bin/msgfmt -o de.mo de.po
/media/Dev/STB_Builds/openvix/experimental/builds/vuultimo/tmp/sysroots/x86_64-linux/usr/bin/msgfmt -o el.mo el.po
/media/Dev/STB_Builds/openvix/experimental/builds/vuultimo/tmp/sysroots/x86_64-linux/usr/bin/msgfmt -o en.mo en.po
/media/Dev/STB_Builds/openvix/experimental/builds/vuultimo/tmp/sysroots/x86_64-linux/usr/bin/msgfmt -o en_GB.mo en_GB.po
/media/Dev/STB_Builds/openvix/experimental/builds/vuultimo/tmp/sysroots/x86_64-linux/usr/bin/msgfmt -o es.mo es.po
/media/Dev/STB_Builds/openvix/experimental/builds/vuultimo/tmp/sysroots/x86_64-linux/usr/bin/msgfmt -o et.mo et.po
/media/Dev/STB_Builds/openvix/experimental/builds/vuultimo/tmp/sysroots/x86_64-linux/usr/bin/msgfmt -o fi.mo fi.po
/media/Dev/STB_Builds/openvix/experimental/builds/vuultimo/tmp/sysroots/x86_64-linux/usr/bin/msgfmt -o fr.mo fr.po
/media/Dev/STB_Builds/openvix/experimental/builds/vuultimo/tmp/sysroots/x86_64-linux/usr/bin/msgfmt -o fy.mo fy.po
/media/Dev/STB_Builds/openvix/experimental/builds/vuultimo/tmp/sysroots/x86_64-linux/usr/bin/msgfmt -o hr.mo hr.po
/media/Dev/STB_Builds/openvix/experimental/builds/vuultimo/tmp/sysroots/x86_64-linux/usr/bin/msgfmt -o hu.mo hu.po
/media/Dev/STB_Builds/openvix/experimental/builds/vuultimo/tmp/sysroots/x86_64-linux/usr/bin/msgfmt -o is.mo is.po
/media/Dev/STB_Builds/openvix/experimental/builds/vuultimo/tmp/sysroots/x86_64-linux/usr/bin/msgfmt -o it.mo it.po
/media/Dev/STB_Builds/openvix/experimental/builds/vuultimo/tmp/sysroots/x86_64-linux/usr/bin/msgfmt -o lt.mo lt.po
/media/Dev/STB_Builds/openvix/experimental/builds/vuultimo/tmp/sysroots/x86_64-linux/usr/bin/msgfmt -o lv.mo lv.po
/media/Dev/STB_Builds/openvix/experimental/builds/vuultimo/tmp/sysroots/x86_64-linux/usr/bin/msgfmt -o nl.mo nl.po
/media/Dev/STB_Builds/openvix/experimental/builds/vuultimo/tmp/sysroots/x86_64-linux/usr/bin/msgfmt -o no.mo no.po
/media/Dev/STB_Builds/openvix/experimental/builds/vuultimo/tmp/sysroots/x86_64-linux/usr/bin/msgfmt -o pl.mo pl.po
/media/Dev/STB_Builds/openvix/experimental/builds/vuultimo/tmp/sysroots/x86_64-linux/usr/bin/msgfmt -o pt.mo pt.po
/media/Dev/STB_Builds/openvix/experimental/builds/vuultimo/tmp/sysroots/x86_64-linux/usr/bin/msgfmt -o ru.mo ru.po
/media/Dev/STB_Builds/openvix/experimental/builds/vuultimo/tmp/sysroots/x86_64-linux/usr/bin/msgfmt -o sv.mo sv.po
/media/Dev/STB_Builds/openvix/experimental/builds/vuultimo/tmp/sysroots/x86_64-linux/usr/bin/msgfmt -o sk.mo sk.po
/media/Dev/STB_Builds/openvix/experimental/builds/vuultimo/tmp/sysroots/x86_64-linux/usr/bin/msgfmt -o sl.mo sl.po
/media/Dev/STB_Builds/openvix/experimental/builds/vuultimo/tmp/sysroots/x86_64-linux/usr/bin/msgfmt -o sr.mo sr.po
/media/Dev/STB_Builds/openvix/experimental/builds/vuultimo/tmp/sysroots/x86_64-linux/usr/bin/msgfmt -o th.mo th.po
/media/Dev/STB_Builds/openvix/experimental/builds/vuultimo/tmp/sysroots/x86_64-linux/usr/bin/msgfmt -o tr.mo tr.po
/media/Dev/STB_Builds/openvix/experimental/builds/vuultimo/tmp/sysroots/x86_64-linux/usr/bin/msgfmt -o uk.mo uk.po
