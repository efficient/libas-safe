override CFLAGS := -Og -g3 -Wall -Wextra -Wpedantic $(CFLAGS)
override CPPFLAGS := $(CPPFLAGS)
override LDFLAGS := $(LDFLAGS)
override LDLIBS := $(LDLIBS)

CC := c99

BINS := libtlsblock.so libltrace.so layout thread libsobss.so libsodata.so

.PHONY: all
all: $(BINS)

libsobss.so: sotls.c
libsodata.so: sotls.c

layout: offset.h
layout: private LDLIBS += -Wl,--no-as-needed -lpthread -Wl,--as-needed

thread: offset.h
thread: private LDLIBS += -lpthread

libtlsblock.so libltrace.so: interpose.h
libtlsblock.so libltrace.so: private CFLAGS += -fpic
libtlsblock.so libltrace.so: private CPPFLAGS += -D_GNU_SOURCE
libtlsblock.so libltrace.so: private LDLIBS += -ldl

.PHONY: clean
clean:
	$(RM) $(BINS)

lib%.so: %.c
	$(CC) $(LDFLAGS) $(CPPFLAGS) $(CFLAGS) -shared -zdefs -ztext $< -o $@ $(LDLIBS)
