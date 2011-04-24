OBJS += src/common/memory.o
OBJS += src/common/core/log.o
OBJS += src/common/core/interactive.o
OBJS += src/common/dvb/dvb.o
OBJS += src/common/aliases/aliases.o
OBJS += src/common/net/http.o
OBJS += src/common/gzip/gzip.o
OBJS += src/common/opentv/opentv.o
OBJS += src/common/opentv/huffman.o
OBJS += src/common/providers/providers.o
OBJS += src/common/epgdb/epgdb.o
OBJS += src/common/epgdb/epgdb_channels.o
OBJS += src/common/epgdb/epgdb_index.o
OBJS += src/common/epgdb/epgdb_titles.o
OBJS += src/common/epgdb/epgdb_aliases.o
OBJS += src/common/epgdb/epgdb_search.o
OBJS += src/common/xmltv/xmltv_encodings.o
OBJS += src/common/xmltv/xmltv_channels.o
OBJS += src/common/xmltv/xmltv_downloader.o
OBJS += src/common/xmltv/xmltv_parser.o
OBJS += src/common/dbmerge/dbmerge.o
OBJS += src/enigma2/enigma2_hash.o
OBJS += src/enigma2/enigma2_lamedb.o
OBJS += src/common/importer/csv.o
OBJS += src/common/importer/importer.o

CONVERTER_OBJS += src/enigma2/crossepg_dbconverter.o
DBINFO_OBJS += src/common/crossepg_dbinfo.o
DOWNLOADER_OBJS += src/common/crossepg_downloader.o
EPGCOPY_OBJS += src/enigma2/crossepg_epgcopy.o
IMPORTER_OBJS += src/common/crossepg_importer.o
EXPORTER_OBJS += src/common/crossepg_exporter.o
XMLTV_OBJS += src/common/crossepg_xmltv.o

CONVERTER_BIN = bin/crossepg_dbconverter
DBINFO_BIN = bin/crossepg_dbinfo
DOWNLOADER_BIN = bin/crossepg_downloader
EPGCOPY_BIN = bin/crossepg_epgcopy
IMPORTER_BIN = bin/crossepg_importer
EXPORTER_BIN = bin/crossepg_exporter
XMLTV_BIN = bin/crossepg_xmltv

SWIGS_OBJS = src/common/crossepg_wrap.o
SWIGS_LIBS = bin/_crossepg.so

VERSION_HEADER = src/version.h
VERSION_PYTHON = src/enigma2/python/version.py

SVN=$(shell sh get_svn_version.sh)
VERSION=$(shell cat VERSION)

BIN_DIR = bin

FTP_HOST = 172.16.1.139
FTP_USER = root
FTP_PASSWORD = sifteam

all: clean $(CONVERTER_BIN) $(DBINFO_BIN) $(DOWNLOADER_BIN) $(EPGCOPY_BIN) $(IMPORTER_BIN) $(EXPORTER_BIN) $(XMLTV_BIN) $(SWIGS_LIBS)

$(BIN_DIR):
	mkdir -p $@
	
$(VERSION_HEADER):
	echo "#define RELEASE \"$(VERSION) (svn $(SVN))\"" > $(VERSION_HEADER)
	echo "version = \"$(VERSION) (svn $(SVN))\"" > $(VERSION_PYTHON)

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

$(SWIGS_LIBS): $(SWIGS_OBJS)
	$(CC) $(LDFLAGS) -shared -o $@ $(OBJS) $(SWIGS_OBJS) -lxml2 -lz -lm -lpthread
	$(STRIP) $@
	
$(DBINFO_OBJS):
	$(CC) $(CFLAGS) -c -o $@ $(@:.o=.c)

$(CONVERTER_BIN): $(OBJS) $(CONVERTER_OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(CONVERTER_OBJS) -lxml2 -lz -lm -lpthread
	$(STRIP) $@

$(DBINFO_BIN): $(OBJS) $(DBINFO_OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(DBINFO_OBJS) -lxml2 -lz -lm -lpthread
	$(STRIP) $@
	
$(DOWNLOADER_BIN): $(OBJS) $(DOWNLOADER_OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(DOWNLOADER_OBJS) -lxml2 -lz -lm -lpthread
	$(STRIP) $@

$(EPGCOPY_BIN): $(OBJS) $(EPGCOPY_OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(EPGCOPY_OBJS) -lxml2 -lz -lm -lpthread
	$(STRIP) $@

$(IMPORTER_BIN): $(OBJS) $(IMPORTER_OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(IMPORTER_OBJS) -lxml2 -lz -lm -lpthread
	$(STRIP) $@

$(EXPORTER_BIN): $(OBJS) $(EXPORTER_OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(EXPORTER_OBJS) -lxml2 -lz -lm -lpthread
	$(STRIP) $@

$(XMLTV_BIN): $(OBJS) $(XMLTV_OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(XMLTV_OBJS) -lxml2 -lz -lm -lpthread
	$(STRIP) $@
	
clean:
	rm -f $(OBJS) $(CONVERTER_OBJS) $(DOWNLOADER_OBJS) $(EPGCOPY_OBJS) $(IMPORTER_OBJS) \
	$(EXPORTER_OBJS) $(XMLTV_OBJS) $(DBINFO_OBJS) $(CONVERTER_BIN) $(DBINFO_BIN) $(DOWNLOADER_BIN) \
	$(EPGCOPY_BIN) $(IMPORTER_BIN) $(EXPORTER_BIN) $(XMLTV_BIN) $(VERSION_HEADER) \
	$(SWIGS_OBJS) $(SWIGS_LIBS)

install-python:
	install -d $(D)/usr/lib/python2.6/lib-dynload
	install -m 644 src/common/crossepg.py $(D)/usr/lib/python2.6
	install -m 644 bin/_crossepg.so $(D)/usr/lib/python2.6/lib-dynload

install-python-qboxhd:
	install -d $(D)/usr/local/lib/python2.6/lib-dynload
	install -m 644 src/common/crossepg.py $(D)/usr/local/lib/python2.6
	install -m 644 bin/_crossepg.so $(D)/usr/local/lib/python2.6/lib-dynload

install-python-2.5:
	install -d $(D)/usr/lib/python2.5/lib-dynload
	install -m 644 src/common/crossepg.py $(D)/usr/lib/python2.5
	install -m 644 bin/_crossepg.so $(D)/usr/lib/python2.5/lib-dynload

install-standalone:
	install -d $(D)/usr/crossepg/aliases
	install -d $(D)/usr/crossepg/import
	install -d $(D)/usr/crossepg/providers
	install -d $(D)/usr/crossepg/scripts
	install -d $(D)/usr/crossepg/scripts/lib
	install -d $(D)/usr/crossepg/scripts/rai
	install -d $(D)/usr/crossepg/scripts/alias
	install -d $(D)/usr/crossepg/scripts/mediaprem
	install -m 755 bin/crossepg_dbconverter $(D)/usr/crossepg/
	install -m 755 bin/crossepg_dbinfo $(D)/usr/crossepg/
	install -m 755 bin/crossepg_downloader $(D)/usr/crossepg/
	install -m 755 bin/crossepg_epgcopy $(D)/usr/crossepg/
	install -m 755 bin/crossepg_importer $(D)/usr/crossepg/
	install -m 755 bin/crossepg_exporter $(D)/usr/crossepg/
	install -m 755 bin/crossepg_xmltv $(D)/usr/crossepg/
	install -m 755 contrib/crossepg_epgmove.sh $(D)/usr/crossepg/
	install -m 755 contrib/crossepg_prepare_pre_start.sh $(D)/usr/crossepg/
	install -m 644 providers/* $(D)/usr/crossepg/providers/
	install -m 755 scripts/*.py $(D)/usr/crossepg/scripts/
	install -m 755 scripts/lib/* $(D)/usr/crossepg/scripts/lib/
	install -m 755 scripts/rai/* $(D)/usr/crossepg/scripts/rai/
	install -m 755 scripts/alias/* $(D)/usr/crossepg/scripts/alias/
	install -m 755 scripts/mediaprem/* $(D)/usr/crossepg/scripts/mediaprem/

install-standalone-var:
	install -d $(D)/var/crossepg/aliases
	install -d $(D)/var/crossepg/import
	install -d $(D)/var/crossepg/providers
	install -d $(D)/var/crossepg/scripts
	install -d $(D)/var/crossepg/scripts/lib
	install -d $(D)/var/crossepg/scripts/rai
	install -d $(D)/var/crossepg/scripts/alias
	install -d $(D)/var/crossepg/scripts/mediaprem
	install -m 755 bin/crossepg_dbconverter $(D)/var/crossepg/
	install -m 755 bin/crossepg_dbinfo $(D)/var/crossepg/
	install -m 755 bin/crossepg_downloader $(D)/var/crossepg/
	install -m 755 bin/crossepg_epgcopy $(D)/var/crossepg/
	install -m 755 bin/crossepg_importer $(D)/var/crossepg/
	install -m 755 bin/crossepg_exporter $(D)/var/crossepg/
	install -m 755 bin/crossepg_xmltv $(D)/var/crossepg/
	install -m 755 contrib/crossepg_epgmove.sh $(D)/var/crossepg/
	install -m 755 contrib/crossepg_prepare_pre_start.sh $(D)/var/crossepg/
	install -m 644 providers/* $(D)/var/crossepg/providers/
	install -m 755 scripts/*.py $(D)/var/crossepg/scripts/
	install -m 755 scripts/lib/* $(D)/var/crossepg/scripts/lib/
	install -m 755 scripts/rai/* $(D)/var/crossepg/scripts/rai/
	install -m 755 scripts/alias/* $(D)/var/crossepg/scripts/alias/
	install -m 755 scripts/mediaprem/* $(D)/var/crossepg/scripts/mediaprem/

install-plugin:
	install -d $(D)/usr/lib/enigma2/python/Plugins/SystemPlugins/CrossEPG/skins
	install -d $(D)/usr/lib/enigma2/python/Plugins/SystemPlugins/CrossEPG/images
	install -d $(D)/usr/lib/enigma2/python/Plugins/SystemPlugins/CrossEPG/po/it/LC_MESSAGES
	install -d $(D)/usr/lib/enigma2/python/Plugins/SystemPlugins/CrossEPG/po/uk/LC_MESSAGES
	install -d $(D)/usr/lib/enigma2/python/Plugins/SystemPlugins/CrossEPG/po/ru/LC_MESSAGES
	install -m 644 contrib/po/it/LC_MESSAGES/CrossEPG.mo $(D)/usr/lib/enigma2/python/Plugins/SystemPlugins/CrossEPG/po/it/LC_MESSAGES/
	install -m 644 contrib/po/uk/LC_MESSAGES/CrossEPG.mo $(D)/usr/lib/enigma2/python/Plugins/SystemPlugins/CrossEPG/po/uk/LC_MESSAGES/
	install -m 644 contrib/po/ru/LC_MESSAGES/CrossEPG.mo $(D)/usr/lib/enigma2/python/Plugins/SystemPlugins/CrossEPG/po/ru/LC_MESSAGES/
	install -m 644 src/enigma2/python/*.py $(D)/usr/lib/enigma2/python/Plugins/SystemPlugins/CrossEPG/
	install -m 644 src/enigma2/python/skins/*.xml $(D)/usr/lib/enigma2/python/Plugins/SystemPlugins/CrossEPG/skins/
	install -m 644 src/enigma2/python/images/*.png $(D)/usr/lib/enigma2/python/Plugins/SystemPlugins/CrossEPG/images/

install-plugin-qboxhd:
	install -d $(D)/usr/local/lib/enigma2/python/Plugins/SystemPlugins/CrossEPG/skins
	install -d $(D)/usr/local/lib/enigma2/python/Plugins/SystemPlugins/CrossEPG/images
	install -d $(D)/usr/local/lib/enigma2/python/Plugins/SystemPlugins/CrossEPG/po/it/LC_MESSAGES
	install -d $(D)/usr/local/lib/enigma2/python/Plugins/SystemPlugins/CrossEPG/po/uk/LC_MESSAGES
	install -d $(D)/usr/local/lib/enigma2/python/Plugins/SystemPlugins/CrossEPG/po/ru/LC_MESSAGES
	install -m 644 contrib/po/it/LC_MESSAGES/CrossEPG.mo $(D)/usr/local/lib/enigma2/python/Plugins/SystemPlugins/CrossEPG/po/it/LC_MESSAGES/
	install -m 644 contrib/po/uk/LC_MESSAGES/CrossEPG.mo $(D)/usr/local/lib/enigma2/python/Plugins/SystemPlugins/CrossEPG/po/uk/LC_MESSAGES/
	install -m 644 contrib/po/ru/LC_MESSAGES/CrossEPG.mo $(D)/usr/local/lib/enigma2/python/Plugins/SystemPlugins/CrossEPG/po/ru/LC_MESSAGES/
	install -m 644 src/enigma2/python/*.py $(D)/usr/local/lib/enigma2/python/Plugins/SystemPlugins/CrossEPG/
	install -m 644 src/enigma2/python/skins/*.xml $(D)/usr/local/lib/enigma2/python/Plugins/SystemPlugins/CrossEPG/skins/
	install -m 644 src/enigma2/python/images/*.png $(D)/usr/local/lib/enigma2/python/Plugins/SystemPlugins/CrossEPG/images/

install: install-python install-standalone install-plugin
install-var: install-python install-standalone-var install-plugin
install-var-qboxhd: install-python-qboxhd install-standalone-var install-plugin-qboxhd
install-py25: install-python-2.5 install-standalone install-plugin

remote-install:
	ncftpput -m -u $(FTP_USER) -p $(FTP_PASSWORD) $(FTP_HOST) /usr/lib/python2.6 src/common/crossepg.py
	ncftpput -m -u $(FTP_USER) -p $(FTP_PASSWORD) $(FTP_HOST) /usr/lib/python2.6/lib-dynload bin/_crossepg.so

	ncftpput -m -u $(FTP_USER) -p $(FTP_PASSWORD) $(FTP_HOST) /usr/crossepg bin/crossepg_dbconverter
	ncftpput -m -u $(FTP_USER) -p $(FTP_PASSWORD) $(FTP_HOST) /usr/crossepg bin/crossepg_dbinfo
	ncftpput -m -u $(FTP_USER) -p $(FTP_PASSWORD) $(FTP_HOST) /usr/crossepg bin/crossepg_downloader
	ncftpput -m -u $(FTP_USER) -p $(FTP_PASSWORD) $(FTP_HOST) /usr/crossepg bin/crossepg_epgcopy
	ncftpput -m -u $(FTP_USER) -p $(FTP_PASSWORD) $(FTP_HOST) /usr/crossepg bin/crossepg_importer
	ncftpput -m -u $(FTP_USER) -p $(FTP_PASSWORD) $(FTP_HOST) /usr/crossepg bin/crossepg_exporter
	ncftpput -m -u $(FTP_USER) -p $(FTP_PASSWORD) $(FTP_HOST) /usr/crossepg bin/crossepg_xmltv
	ncftpput -m -u $(FTP_USER) -p $(FTP_PASSWORD) $(FTP_HOST) /usr/crossepg contrib/crossepg_epgmove.sh

	ncftpput -m -u $(FTP_USER) -p $(FTP_PASSWORD) $(FTP_HOST) /usr/crossepg/providers providers/*
	ncftpput -m -u $(FTP_USER) -p $(FTP_PASSWORD) $(FTP_HOST) /usr/crossepg/scripts scripts/*
	ncftpput -m -u $(FTP_USER) -p $(FTP_PASSWORD) $(FTP_HOST) /usr/lib/enigma2/python/Plugins/SystemPlugins/CrossEPG/po/it/LC_MESSAGES contrib/po/it/LC_MESSAGES/CrossEPG.mo
	ncftpput -m -u $(FTP_USER) -p $(FTP_PASSWORD) $(FTP_HOST) /usr/lib/enigma2/python/Plugins/SystemPlugins/CrossEPG src/enigma2/python/*.py
	ncftpput -m -u $(FTP_USER) -p $(FTP_PASSWORD) $(FTP_HOST) /usr/lib/enigma2/python/Plugins/SystemPlugins/CrossEPG/skins src/enigma2/python/skins/*.xml
	ncftpput -m -u $(FTP_USER) -p $(FTP_PASSWORD) $(FTP_HOST) /usr/lib/enigma2/python/Plugins/SystemPlugins/CrossEPG/images src/enigma2/python/images/*.png
	
