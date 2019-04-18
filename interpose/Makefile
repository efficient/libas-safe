override CFLAGS   := -std=c99 -O2 -Wall -Wextra -Wpedantic $(CFLAGS)
override CPPFLAGS := $(CPPFLAGS)
override LDFLAGS  := $(LDFLAGS)
override LDLIBS   := $(LDLIBS)

.PHONY: all
all: date libtime.so libdlinterp.so

libdlinterp.so: private LDLIBS += -ldl
libdlinterp.so: interpose.o
libtime.so: private LDLIBS += -ldl
libtime.so: interpose.o

dlinterp.o: private CFLAGS += -fno-optimize-sibling-calls
interpose.o: private CPPFLAGS += -D_GNU_SOURCE
time.o: private CFLAGS += -fno-optimize-sibling-calls

.PHONY: clean
clean:
	git clean -fX

lib%.so: %.o
	$(CC) $(LDFLAGS) -shared -zdefs -ztext -o $@ $^ $(LDLIBS)
