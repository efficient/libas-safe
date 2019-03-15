RUSTC := rustc

override CFLAGS    := -std=c99 -O2 -Wall -Wextra -Wpedantic $(CFLAGS)
override CPPFLAGS  := $(CPPFLAGS)
override LDFLAGS   := $(LDFLAGS)
override LDLIBS    := $(LDLIBS)
override RUSTFLAGS := --edition 2018 -Copt-level=2 $(RUSTFLAGS)

.PHONY: all
all: bench libpreload.so

bench: libglobl.so libproxy.rlib
bench: private RUSTFLAGS += -L. -Crpath
libpreload.so: private LDLIBS += -ldl
libproxy.rlib: proxy.o
preload.o: private CFLAGS += -fpic
preload.o: private CPPFLAGS += -D_GNU_SOURCE
proxy.o: proxy.h
proxy.o: private CPPFLAGS += -D_GNU_SOURCE

.PHONY: clean
clean:
	$(RM) bench *.o *.rlib *.so

%: %.rs
	$(RUSTC) $(RUSTFLAGS) --test -Clink-args="$(LDFLAGS)" $< $(LDLIBS)

lib%.rlib: %.rs
	$(RUSTC) $(RUSTFLAGS) --crate-type rlib -Clink-args="$(LDFLAGS)" $< $(LDLIBS)
	if [ -e $*.o ]; then $(AR) rs $@ $*.o; fi

lib%.so: %.o
	$(CC) $(LDFLAGS) -shared $^ -o $@ $(LDLIBS)

lib%.so: %.rs
	$(RUSTC) $(RUSTFLAGS) --crate-type dylib -Cprefer-dynamic -Clink-args="$(LDFLAGS)" $< $(LDLIBS)
