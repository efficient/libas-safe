override CFLAGS := -O2 -Wall -Wextra -Wpedantic $(CFLAGS)
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
thread: private CFLAGS += -rdynamic
thread: private LDLIBS += -lpthread

libltrace.so: interpose.h
libltrace.so: private CFLAGS += -fpic
libltrace.so: private CPPFLAGS += -D_GNU_SOURCE
libltrace.so: private LDLIBS += -ldl

libtlsblock.so: interpose.h
libtlsblock.so: private CFLAGS += -fpic
libtlsblock.so: private CPPFLAGS += -D_GNU_SOURCE
libtlsblock.so: private LDLIBS += -ldl -lpthread

.PHONY: clean
clean:
	$(RM) $(BINS)

lib%.so: %.c
	$(CC) $(LDFLAGS) $(CPPFLAGS) $(CFLAGS) -shared -zdefs -ztext $< -o $@ $(LDLIBS)
