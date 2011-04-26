CROSSEPG_HEADERS ?= ../src/
CROSSEPG_BINARIES ?= ../bin/
CC ?= gcc
STRIP ?= strip

OBJS = testapp.o
BIN = testapp

all: clean $(BIN)

$(OBJS):
	$(CC) $(CFLAGS) -I$(CROSSEPG_HEADERS) -c -o $@ $(@:.o=.c)

$(BIN): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) -lcrossepg -lxml2 -lz -lm -lpthread -L$(CROSSEPG_BINARIES)
	$(STRIP) $@

clean:
	rm -f $(OBJS) $(BIN)