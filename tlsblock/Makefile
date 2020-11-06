override CFLAGS := -Og -g3 -Wall -Wextra -Wpedantic $(CFLAGS)
override CPPFLAGS := $(CPPFLAGS)
override LDFLAGS := $(LDFLAGS)
override LDLIBS := -lpthread $(LDLIBS)

CC := c99

BINS := libtlsblock.so libltrace.so layout thread libsobss.so libsodata.so

.PHONY: all
all: $(BINS)

libsobss.so: sotls.c
libsodata.so: sotls.c

layout: offset.h

libtlsblock.so libltrace.so: interpose.h
libtlsblock.so libltrace.so: private CFLAGS += -fpic
libtlsblock.so libltrace.so: private CPPFLAGS += -D_GNU_SOURCE
libtlsblock.so libltrace.so: private LDLIBS += -ldl

.PHONY: clean
clean:
	$(RM) $(BINS)

lib%.so: %.c
	$(CC) $(LDFLAGS) $(CPPFLAGS) $(CFLAGS) -shared -zdefs -ztext $< -o $@ $(LDLIBS)
