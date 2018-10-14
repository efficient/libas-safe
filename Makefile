BINDGEN := bindgen
RUSTC := rustc

override BINDFLAGS := --raw-line "\#![allow(dead_code, non_camel_case_types, non_snake_case, non_upper_case_globals)]" $(BINDFLAGS)
override CFLAGS := -std=c99 -g -Og -Wall -Wextra -Wpedantic $(CFLAGS)
override RUSTFLAGS := -g -O $(RUSTFLAGS)

bin: private LDFLAGS += -Wl,-R,\$$ORIGIN
bin: private LDLIBS += -ldl
bin: liblib.so

bench: private LDFLAGS += -L. -Wl,-R,\$$ORIGIN -znow
bench: private RUSTFLAGS += --test
bench: dl.rs dlfcn.rs got.rs link.rs mman.rs liblib.so

test: private LDFLAGS += -Wl,-R,\$$ORIGIN
test: libhook.so

libhook.so: hook_internal.o

bin.o: private CPPFLAGS += -D_GNU_SOURCE
bin.o: lib.h
dlfcn.rs: private CPPFLAGS += -D_GNU_SOURCE
hook.o: private CFLAGS += -fpic
link.rs: private BINDFLAGS += --no-rustfmt-bindings
lib.o: lib.h

.PHONY: clean
clean:
	$(RM) bench bin test dlfcn.rs link.rs mman.rs *.o *.so

%: %.rs
	$(RUSTC) -Clink-args="$(LDFLAGS)" $(RUSTFLAGS) $< $(LDLIBS)

%.rs: %.h
	$(BINDGEN) $(BINDFLAGS) -o $@ $^ -- $(CPPFLAGS)

lib%.so: %.o
	$(CC) $(LDFLAGS) -shared -o $@ $^ $(LDLIBS)
