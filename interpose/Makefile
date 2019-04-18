override CFLAGS   := -std=c99 -O2 -Wall -Wextra -Wpedantic $(CFLAGS)
override CPPFLAGS := $(CPPFLAGS)
override LDFLAGS  := $(LDFLAGS)
override LDLIBS   := $(LDLIBS)

.PHONY: all
all: date libtime.so

.PHONY: clean
clean:
	git clean -fX

lib%.so: %.o
	$(CC) $(LDFLAGS) -shared -zdefs -ztext -o $@ $^ $(LDLIBS)
