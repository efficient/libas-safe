override CFLAGS := -Og -Wall -Wextra -Wpedantic $(CFLAGS)
override CPPFLAGS := $(CPPFLAGS)
override LDFLAGS := $(LDFLAGS)
override LDLIBS := -lpthread $(LDLIBS)
override RUSTFLAGS := --edition 2018 $(RUSTFLAGS)

CC := c99
LN := ln
RUSTC := rustc

BINS := tls tlrs selfref selfthd tcb tcbpic libplugin.so libdeferred.so perf rperf rperfb create spawn perflazy perfmcp mem

.PHONY: all
all: $(BINS)

tls: private CFLAGS += -fpic
tls: private CPPFLAGS += -Ilibinger
tls: private LDFLAGS += -L$(wildcard libinger/target/*) -Wl,-R\$$ORIGIN/$(wildcard libinger/target/*) -Wl,-Ilibinger/ld.so
tls: private LDLIBS := -linger $(LDLIBS)

tlrs: private LDFLAGS += -Wl,-R\$$ORIGIN/$(wildcard libinger/target/*) -Wl,-Ilibinger/ld.so
tlrs: private RUSTFLAGS += -L$(wildcard libinger/target/*/deps)

perf create: private CFLAGS += -O2
perf create: private CPPFLAGS += -D_DEFAULT_SOURCE
perf create: private LDFLAGS += -Wl,-Ilibinger/ld-bench.so

rperf: private LDFLAGS += -Wl,-Ilibinger/ld-bench.so
rperf: private RUSTC := rustdev rustc
rperf: private RUSTFLAGS += -O

rperfb spawn: private LDFLAGS += -Wl,-Ilibinger/ld-bench.so
rperfb spawn: private RUSTC := rustdev rustc
rperfb spawn: private RUSTFLAGS += --test -O

libdeferred.so: libinger.so
libdeferred.so: private LDLIBS += -ldl

deferred.o: private CFLAGS += -fpic -O2
deferred.o: private CPPFLAGS += -D_DEFAULT_SOURCE

libinger.so:
	$(LN) -s libinger/target/release/libinger.so

perflazy: private CFLAGS += -fpic -O2
perflazy: private CPPFLAGS += -D_GNU_SOURCE
perflazy: private LDFLAGS += -Wl,-Ilibinger/ld-bench.so
perflazy: private LDLIBS += -ldl

perfmcp: private CFLAGS += -O2
perfmcp: private CPPFLAGS += -D_DEFAULT_SOURCE

mem: private CFLAGS += -O2
mem: private CPPFLAGS += -D_GNU_SOURCE
mem: private LDLIBS += -ldl

selfthd: private CPPFLAGS += -D_DEFAULT_SOURCE

tcbpic: private CFLAGS += -fpic

.PHONY: clean
clean:
	$(RM) $(BINS) *.o *.so

%: %.rs
	$(RUSTC) $(RUSTFLAGS) $(LDFLAGS:%=-Clink-arg=%) $< $(RUSTLIBS)

lib%.so: %.o
	$(CC) $(LDFLAGS) -shared -zdefs -o $@ $< $(LDLIBS)

lib%.so: %.rs
	$(RUSTC) $(RUSTFLAGS) --crate-type dylib $(LDFLAGS:%=-Clink-arg=%) $< $(RUSTLIBS)
