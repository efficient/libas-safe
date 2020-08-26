override CFLAGS := -Og -Wall -Wextra -Wpedantic $(CFLAGS)
override CPPFLAGS := $(CPPFLAGS)
override LDFLAGS := $(LDFLAGS)
override LDLIBS := -lpthread $(LDLIBS)
override RUSTFLAGS := --edition 2018 $(RUSTFLAGS)

CC := c99
RUSTC := rustc

BINS := tls tlrs selfref selfthd tcb tcbpic libplugin.so perf spawn

.PHONY: all
all: $(BINS)

tls: private CFLAGS += -fpic
tls: private CPPFLAGS += -Ilibinger
tls: private LDFLAGS += -L$(wildcard libinger/target/*) -Wl,-R\$$ORIGIN/$(wildcard libinger/target/*) -Wl,-Ilibinger/ld.so
tls: private LDLIBS := -linger $(LDLIBS)

tlrs: private LDFLAGS += -Wl,-R\$$ORIGIN/$(wildcard libinger/target/*) -Wl,-Ilibinger/ld.so
tlrs: private RUSTFLAGS += -L$(wildcard libinger/target/*/deps)

perf: private CFLAGS += -O2
perf: private CPPFLAGS += -D_DEFAULT_SOURCE
perf: private LDFLAGS += -Wl,-Ilibinger/ld-bench.so

spawn: private LDFLAGS += -Wl,-Ilibinger/ld-bench.so
spawn: private RUSTC := rustdev rustc
spawn: private RUSTFLAGS += --test -O

selfthd: private CPPFLAGS += -D_DEFAULT_SOURCE

tcbpic: private CFLAGS += -fpic

.PHONY: clean
clean:
	$(RM) $(BINS)

%: %.rs
	$(RUSTC) $(RUSTFLAGS) $(LDFLAGS:%=-Clink-arg=%) $< $(RUSTLIBS)

lib%.so: %.rs
	$(RUSTC) $(RUSTFLAGS) --crate-type dylib $(LDFLAGS:%=-Clink-arg=%) $< $(RUSTLIBS)
