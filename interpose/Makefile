override CFLAGS   := -std=c99 -O2 -Wall -Wextra -Wpedantic $(CFLAGS)
override CPPFLAGS := $(CPPFLAGS)
override LDFLAGS  := $(LDFLAGS)
override LDLIBS   := $(LDLIBS)

.PHONY: all
all: date libtime.so

libtime.so: private LDLIBS += -ldl
libtime.so: interpose.o

interpose.o: private CPPFLAGS += -D_GNU_SOURCE
time.o: private CFLAGS += -fno-optimize-sibling-calls

.PHONY: clean
clean:
	git clean -fX

lib%.so: %.o
	$(CC) $(LDFLAGS) -shared -zdefs -ztext -o $@ $^ $(LDLIBS)
