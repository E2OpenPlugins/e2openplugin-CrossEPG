OBJS += src/common/memory.o
OBJS += src/common/core/log.o
OBJS += src/common/core/interactive.o
OBJS += src/common/dvb/dvb.o
OBJS += src/common/aliases/aliases.o
OBJS += src/common/net/http.o
OBJS += src/common/opentv/opentv.o
OBJS += src/common/opentv/huffman.o
OBJS += src/common/providers/providers.o
OBJS += src/common/epgdb/epgdb.o
OBJS += src/common/epgdb/epgdb_channels.o
OBJS += src/common/epgdb/epgdb_index.o
OBJS += src/common/epgdb/epgdb_titles.o
OBJS += src/common/epgdb/epgdb_aliases.o
OBJS += src/common/epgdb/epgdb_search.o
OBJS += src/common/xmltv/xmltv_channels.o
OBJS += src/common/xmltv/xmltv_downloader.o
OBJS += src/common/xmltv/xmltv_parser.o
OBJS += src/enigma2/enigma2_hash.o
OBJS += src/enigma2/enigma2_lamedb.o
OBJS += src/common/importer/csv.o
OBJS += src/common/importer/gzip.o
OBJS += src/common/importer/importer.o

CONVERTER_OBJS += src/enigma2/crossepg_dbconverter.o
DBINFO_OBJS += src/common/crossepg_dbinfo.o
DOWNLOADER_OBJS += src/common/crossepg_downloader.o
EPGCOPY_OBJS += src/enigma2/crossepg_epgcopy.o
IMPORTER_OBJS += src/common/crossepg_importer.o

CONVERTER_BIN = bin/crossepg_dbconverter
DBINFO_BIN = bin/crossepg_dbinfo
DOWNLOADER_BIN = bin/crossepg_downloader
EPGCOPY_BIN = bin/crossepg_epgcopy
IMPORTER_BIN = bin/crossepg_importer

VERSION_HEADER = src/version.h

SVN=$(shell svn info | grep Revision | sed "s/.*:\ //")
VERSION=$(shell cat VERSION)

BIN_DIR = bin

all: clean $(CONVERTER_BIN) $(DBINFO_BIN) $(DOWNLOADER_BIN) $(EPGCOPY_BIN) $(IMPORTER_BIN)

$(BIN_DIR):
	mkdir -p $@
	
$(VERSION_HEADER):
	echo "#define RELEASE \"$(VERSION) (svn $(SVN))\"" > $(VERSION_HEADER)

$(OBJS): $(VERSION_HEADER) $(BIN_DIR)
	$(CC) $(CFLAGS) -c -o $@ $(@:.o=.c) -DE2 -DSTANDALONE

$(CONVERTER_OBJS):
	$(CC) $(CFLAGS) -c -o $@ $(@:.o=.c) -DE2 -DSTANDALONE

$(DOWNLOADER_OBJS):
	$(CC) $(CFLAGS) -c -o $@ $(@:.o=.c) -DE2 -DSTANDALONE

$(EPGCOPY_OBJS):
	$(CC) $(CFLAGS) -c -o $@ $(@:.o=.c) -DE2 -DSTANDALONE

$(IMPORTER_OBJS):
	$(CC) $(CFLAGS) -c -o $@ $(@:.o=.c) -DE2 -DSTANDALONE

$(DBINFO_OBJS):
	$(CC) $(CFLAGS) -c -o $@ $(@:.o=.c) -DE2 -DSTANDALONE

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
	
clean:
	rm -f $(OBJS) $(CONVERTER_OBJS) $(DOWNLOADER_OBJS) $(EPGCOPY_OBJS) $(IMPORTER_OBJS) $(DBINFO_OBJS) $(CONVERTER_BIN) $(DBINFO_BIN) $(DOWNLOADER_BIN) $(EPGCOPY_BIN) $(IMPORTER_BIN) $(VERSION_HEADER)

install:
	install -d /usr/crossepg/aliases
	install -d /usr/crossepg/import
	install -d /usr/crossepg/providers
	install -d /usr/lib/enigma2/python/Plugins/SystemPlugins/CrossEPG/skins
	install -d /usr/lib/enigma2/python/Plugins/SystemPlugins/CrossEPG/po/it/LC_MESSAGES
	install -m 755 bin/crossepg_dbconverter /usr/crossepg/
	install -m 755 bin/crossepg_dbinfo /usr/crossepg/
	install -m 755 bin/crossepg_downloader /usr/crossepg/
	install -m 755 bin/crossepg_epgcopy /usr/crossepg/
	install -m 755 bin/crossepg_importer /usr/crossepg/
	install -m 755 contrib/crossepg_epgmove.sh /usr/crossepg/
	install -m 644 contrib/po/it/LC_MESSAGES/CrossEPG.mo /usr/lib/enigma2/python/Plugins/SystemPlugins/CrossEPG/po/it/LC_MESSAGES
	