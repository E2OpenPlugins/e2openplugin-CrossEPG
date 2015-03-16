OBJS = src/common/memory.o src/common/core/log.o src/common/core/interactive.o src/common/dvb/dvb.o src/common/aliases/aliases.o src/common/net/http.o src/common/gzip/gzip.o src/common/opentv/opentv.o src/common/opentv/huffman.o src/common/providers/providers.o src/common/epgdb/epgdb.o src/common/epgdb/epgdb_channels.o src/common/epgdb/epgdb_index.o src/common/epgdb/epgdb_titles.o src/common/epgdb/epgdb_aliases.o src/common/epgdb/epgdb_search.o src/common/xmltv/xmltv_encodings.o src/common/xmltv/xmltv_channels.o src/common/xmltv/xmltv_downloader.o src/common/xmltv/xmltv_parser.o src/common/dbmerge/dbmerge.o src/enigma2/enigma2_hash.o src/enigma2/enigma2_lamedb.o src/common/importer/csv.o src/common/importer/importer.o

CONVERTER_OBJS = src/enigma2/crossepg_dbconverter.o
DBINFO_OBJS = src/common/crossepg_dbinfo.o
DOWNLOADER_OBJS = src/common/crossepg_downloader.o
EPGCOPY_OBJS = src/enigma2/crossepg_epgcopy.o
IMPORTER_OBJS = src/common/crossepg_importer.o
EXPORTER_OBJS = src/common/crossepg_exporter.o
XMLTV_OBJS = src/common/crossepg_xmltv.o
DEFRAGMENTER_OBJS = src/common/crossepg_defragmenter.o

CONVERTER_BIN = bin/crossepg_dbconverter
DBINFO_BIN = bin/crossepg_dbinfo
DOWNLOADER_BIN = bin/crossepg_downloader
EPGCOPY_BIN = bin/crossepg_epgcopy
IMPORTER_BIN = bin/crossepg_importer
EXPORTER_BIN = bin/crossepg_exporter
XMLTV_BIN = bin/crossepg_xmltv
DEFRAGMENTER_BIN = bin/crossepg_defragmenter

SHARED_LIBS = bin/libcrossepg.so

SWIGS_OBJS = src/common/crossepg_wrap.o
SWIGS_LIBS = bin/_crossepg.so

VERSION_HEADER = src/version.h
VERSION_PYTHON = src/enigma2/python/version.py
VERSION=$(shell cat VERSION)

BIN_DIR = bin

TARGET_ARCH ?= mips

all: $(CONVERTER_BIN) $(DBINFO_BIN) $(DOWNLOADER_BIN) $(EPGCOPY_BIN) $(IMPORTER_BIN) $(EXPORTER_BIN) $(XMLTV_BIN) $(DEFRAGMENTER_BIN) $(SWIGS_LIBS) $(SHARED_LIBS)

$(BIN_DIR):
	mkdir -p $@

$(VERSION_HEADER):
	echo "#define RELEASE \"$(VERSION)\"" > $(VERSION_HEADER)
	echo "version = \"$(VERSION)\"" > $(VERSION_PYTHON)

$(SWIGS_OBJS):
	$(SWIG) -threads -python $(@:_wrap.o=.i)
	$(CC) $(CFLAGS) -c -fpic -o $@ $(@:.o=.c)

$(OBJS): $(VERSION_HEADER) $(BIN_DIR)
	$(CC) $(CFLAGS) -c -fpic -o $@ $(@:.o=.c)

$(CONVERTER_OBJS):
	$(CC) $(CFLAGS) -c -o $@ $(@:.o=.c)

$(DOWNLOADER_OBJS):
	$(CC) $(CFLAGS) -c -o $@ $(@:.o=.c)

$(EPGCOPY_OBJS):
	$(CC) $(CFLAGS) -c -o $@ $(@:.o=.c)

$(IMPORTER_OBJS):
	$(CC) $(CFLAGS) -c -o $@ $(@:.o=.c)

$(EXPORTER_OBJS):
	$(CC) $(CFLAGS) -c -o $@ $(@:.o=.c)

$(XMLTV_OBJS):
	$(CC) $(CFLAGS) -c -o $@ $(@:.o=.c)

$(DEFRAGMENTER_OBJS):
	$(CC) $(CFLAGS) -c -o $@ $(@:.o=.c)

$(SWIGS_LIBS): $(SWIGS_OBJS)
	$(CC) $(LDFLAGS) -shared -o $@ $(OBJS) $(SWIGS_OBJS) -lxml2 -lz -lm -lpthread -lcurl
	$(STRIP) $@

$(SHARED_LIBS): $(OBJS)
	$(CC) $(LDFLAGS) -shared -o $@ $(OBJS) -lxml2 -lz -lm -lpthread -lcurl
	$(STRIP) $@

$(DBINFO_OBJS):
	$(CC) $(CFLAGS) -c -o $@ $(@:.o=.c)

$(CONVERTER_BIN): $(OBJS) $(CONVERTER_OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(CONVERTER_OBJS) -lxml2 -lz -lm -lpthread -lcurl
	$(STRIP) $@

$(DBINFO_BIN): $(OBJS) $(DBINFO_OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(DBINFO_OBJS) -lxml2 -lz -lm -lpthread -lcurl
	$(STRIP) $@

$(DOWNLOADER_BIN): $(OBJS) $(DOWNLOADER_OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(DOWNLOADER_OBJS) -lxml2 -lz -lm -lpthread -lcurl
	$(STRIP) $@

$(EPGCOPY_BIN): $(OBJS) $(EPGCOPY_OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(EPGCOPY_OBJS) -lxml2 -lz -lm -lpthread -lcurl
	$(STRIP) $@

$(IMPORTER_BIN): $(OBJS) $(IMPORTER_OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(IMPORTER_OBJS) -lxml2 -lz -lm -lpthread -lcurl
	$(STRIP) $@

$(EXPORTER_BIN): $(OBJS) $(EXPORTER_OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(EXPORTER_OBJS) -lxml2 -lz -lm -lpthread -lcurl
	$(STRIP) $@

$(XMLTV_BIN): $(OBJS) $(XMLTV_OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(XMLTV_OBJS) -lxml2 -lz -lm -lpthread -lcurl
	$(STRIP) $@

$(DEFRAGMENTER_BIN): $(OBJS) $(DEFRAGMENTER_OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(DEFRAGMENTER_OBJS) -lxml2 -lz -lm -lpthread -lcurl
	$(STRIP) $@

clean:
	rm -f $(OBJS) $(CONVERTER_OBJS) $(DOWNLOADER_OBJS) $(EPGCOPY_OBJS) $(IMPORTER_OBJS) \
	$(EXPORTER_OBJS) $(XMLTV_OBJS) $(DEFRAGMENTER_OBJS) $(DBINFO_OBJS) $(CONVERTER_BIN) $(DBINFO_BIN) $(DOWNLOADER_BIN) \
	$(EPGCOPY_BIN) $(IMPORTER_BIN) $(EXPORTER_BIN) $(XMLTV_BIN) $(DEFRAGMENTER_BIN) $(VERSION_HEADER) \
	$(SWIGS_OBJS) $(SWIGS_LIBS) $(SHARED_LIBS)

install-python:
	install -d $(D)/usr/lib/python2.7/lib-dynload
	#install -m 644 src/common/crossepg.py $(D)/usr/lib/python2.7
	install -m 644 src/common/crossepg.pyo $(D)/usr/lib/python2.7
	install -m 644 bin/_crossepg.so $(D)/usr/lib/python2.7/lib-dynload

install-standalone:
	install -d $(D)/usr/crossepg/aliases
	install -d $(D)/usr/crossepg/import
	install -d $(D)/usr/crossepg/providers
	install -d $(D)/usr/crossepg/scripts
	install -d $(D)/usr/crossepg/scripts/lib
	install -d $(D)/usr/crossepg/scripts/rai
	install -d $(D)/usr/crossepg/scripts/alias
	install -d $(D)/usr/crossepg/scripts/mediaprem
	install -d $(D)/usr/crossepg/scripts/mediaset
	# install -d $(D)/usr/crossepg/scripts/mhw2epgdownloader
	install -m 755 bin/crossepg_dbconverter $(D)/usr/crossepg/
	install -m 755 bin/crossepg_dbinfo $(D)/usr/crossepg/
	install -m 755 bin/crossepg_downloader $(D)/usr/crossepg/
	install -m 755 bin/crossepg_epgcopy $(D)/usr/crossepg/
	install -m 755 bin/crossepg_importer $(D)/usr/crossepg/
	install -m 755 bin/crossepg_exporter $(D)/usr/crossepg/
	install -m 755 bin/crossepg_xmltv $(D)/usr/crossepg/
	install -m 755 bin/crossepg_defragmenter $(D)/usr/crossepg/
	install -m 644 bin/libcrossepg.so $(D)/usr/crossepg/
	install -m 755 contrib/crossepg_epgmove.sh $(D)/usr/crossepg/
	install -m 755 contrib/crossepg_prepare_pre_start.sh $(D)/usr/crossepg/
	install -m 644 providers/* $(D)/usr/crossepg/providers/
	#install -m 755 scripts/*.py $(D)/usr/crossepg/scripts/
	install -m 755 scripts/*.pyo $(D)/usr/crossepg/scripts/
	install -m 755 scripts/lib/* $(D)/usr/crossepg/scripts/lib/
	install -m 755 scripts/rai/* $(D)/usr/crossepg/scripts/rai/
	install -m 755 scripts/alias/* $(D)/usr/crossepg/scripts/alias/
	install -m 755 scripts/mediaprem/* $(D)/usr/crossepg/scripts/mediaprem/
	install -m 755 scripts/mediaset/* $(D)/usr/crossepg/scripts/mediaset/
	install -m 755 aliases/aliases.conf $(D)/usr/crossepg/aliases/
	# install -m 644 scripts/mhw2epgdownloader/*.conf $(D)/usr/crossepg/scripts/mhw2epgdownloader/
	# install -m 644 scripts/mhw2epgdownloader/version.txt $(D)/usr/crossepg/scripts/mhw2epgdownloader/
	# install -m 755 scripts/mhw2epgdownloader/mhw2epgdownloader.$(TARGET_ARCH) $(D)/usr/crossepg/scripts/mhw2epgdownloader/mhw2epgdownloader

install-plugin:
	install -d $(D)/usr/lib/enigma2/python/Plugins/SystemPlugins/CrossEPG/skins
	install -d $(D)/usr/lib/enigma2/python/Plugins/SystemPlugins/CrossEPG/images
	install -d $(D)/usr/lib/enigma2/python/Plugins/SystemPlugins/CrossEPG/po/cs/LC_MESSAGES
	install -d $(D)/usr/lib/enigma2/python/Plugins/SystemPlugins/CrossEPG/po/nl/LC_MESSAGES
	install -d $(D)/usr/lib/enigma2/python/Plugins/SystemPlugins/CrossEPG/po/it/LC_MESSAGES
	install -d $(D)/usr/lib/enigma2/python/Plugins/SystemPlugins/CrossEPG/po/uk/LC_MESSAGES
	install -d $(D)/usr/lib/enigma2/python/Plugins/SystemPlugins/CrossEPG/po/ru/LC_MESSAGES
	install -d $(D)/usr/lib/enigma2/python/Plugins/SystemPlugins/CrossEPG/po/pl/LC_MESSAGES
	install -d $(D)/usr/lib/enigma2/python/Plugins/SystemPlugins/CrossEPG/po/fa/LC_MESSAGES
	install -d $(D)/usr/lib/enigma2/python/Plugins/SystemPlugins/CrossEPG/po/es/LC_MESSAGES
	install -d $(D)/usr/lib/enigma2/python/Plugins/SystemPlugins/CrossEPG/po/sv/LC_MESSAGES
	install -d $(D)/usr/lib/enigma2/python/Plugins/SystemPlugins/CrossEPG/po/de/LC_MESSAGES
	install -m 644 contrib/po/cs/LC_MESSAGES/CrossEPG.mo $(D)/usr/lib/enigma2/python/Plugins/SystemPlugins/CrossEPG/po/cs/LC_MESSAGES/
	install -m 644 contrib/po/nl/LC_MESSAGES/CrossEPG.mo $(D)/usr/lib/enigma2/python/Plugins/SystemPlugins/CrossEPG/po/nl/LC_MESSAGES/
	install -m 644 contrib/po/it/LC_MESSAGES/CrossEPG.mo $(D)/usr/lib/enigma2/python/Plugins/SystemPlugins/CrossEPG/po/it/LC_MESSAGES/
	install -m 644 contrib/po/uk/LC_MESSAGES/CrossEPG.mo $(D)/usr/lib/enigma2/python/Plugins/SystemPlugins/CrossEPG/po/uk/LC_MESSAGES/
	install -m 644 contrib/po/ru/LC_MESSAGES/CrossEPG.mo $(D)/usr/lib/enigma2/python/Plugins/SystemPlugins/CrossEPG/po/ru/LC_MESSAGES/
	install -m 644 contrib/po/pl/LC_MESSAGES/CrossEPG.mo $(D)/usr/lib/enigma2/python/Plugins/SystemPlugins/CrossEPG/po/pl/LC_MESSAGES/
	install -m 644 contrib/po/fa/LC_MESSAGES/CrossEPG.mo $(D)/usr/lib/enigma2/python/Plugins/SystemPlugins/CrossEPG/po/fa/LC_MESSAGES/
	install -m 644 contrib/po/es/LC_MESSAGES/CrossEPG.mo $(D)/usr/lib/enigma2/python/Plugins/SystemPlugins/CrossEPG/po/es/LC_MESSAGES/
	install -m 644 contrib/po/sv/LC_MESSAGES/CrossEPG.mo $(D)/usr/lib/enigma2/python/Plugins/SystemPlugins/CrossEPG/po/sv/LC_MESSAGES/
	install -m 644 contrib/po/de/LC_MESSAGES/CrossEPG.mo $(D)/usr/lib/enigma2/python/Plugins/SystemPlugins/CrossEPG/po/de/LC_MESSAGES/
	#install -m 644 src/enigma2/python/*.py $(D)/usr/lib/enigma2/python/Plugins/SystemPlugins/CrossEPG/
	install -m 644 src/enigma2/python/*.pyo $(D)/usr/lib/enigma2/python/Plugins/SystemPlugins/CrossEPG/
	install -m 644 src/enigma2/python/skins/*.xml $(D)/usr/lib/enigma2/python/Plugins/SystemPlugins/CrossEPG/skins/
	install -m 644 src/enigma2/python/images/*.png $(D)/usr/lib/enigma2/python/Plugins/SystemPlugins/CrossEPG/images/

install: install-python install-standalone install-plugin

