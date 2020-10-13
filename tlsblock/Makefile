override CFLAGS := -Og -g3 -Wall -Wextra -Wpedantic $(CFLAGS)
override CPPFLAGS := $(CPPFLAGS)
override LDFLAGS := $(LDFLAGS)
override LDLIBS := -lpthread $(LDLIBS)

CC := c99

BINS := layout libsotls.so

.PHONY: all
all: $(BINS)

.PHONY: clean
clean:
	$(RM) $(BINS)

lib%.so: %.c
	$(CC) $(LDFLAGS) $(CPPFLAGS) $(CFLAGS) -shared -zdefs -ztext $< -o $@ $(LDLIBS)
