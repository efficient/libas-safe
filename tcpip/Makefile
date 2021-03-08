override CFLAGS := -g3 -Og -fpic -Wall -Wextra -Wpedantic $(CFLAGS)
override CPPFLAGS := -D_DEFAULT_SOURCE $(CPPFLAGS)
override LDFLAGS := -L$(wildcard libinger/target/*/) -Wl,-R$(wildcard libinger/target/*/) -Wl,-Ilibinger/ld.so $(LDFLAGS)
override LDLIBS := -linger $(LDLIBS)

BINS := echo

.PHONY: all
all: $(BINS)

.PHONY: clean
clean:
	$(RM) $(BINS)
