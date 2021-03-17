override CFLAGS := -g3 -Og -Wall -Wextra -Wpedantic $(CFLAGS)
override CPPFLAGS := $(CPPFLAGS)
override LDFLAGS := -Llibgotcha -Wl,-R\$$ORIGIN/libgotcha $(LDFLAGS)
override LDLIBS := -lgotcha $(LDLIBS)

BINS := getfn getopt

.PHONY: all
all: $(BINS)

getopt: private CFLAGS += -fpic -Wno-pedantic
getopt: private CPPFLAGS += -D_DEFAULT_SOURCE

.PHONY: clean
clean:
	$(RM) $(BINS) Cargo.lock
	$(RM) -r target
