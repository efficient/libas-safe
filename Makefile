BINDGEN := bindgen
RUSTC := rustc

override BINDFLAGS := --raw-line "\#![allow(dead_code, non_camel_case_types, non_snake_case, non_upper_case_globals)]" $(BINDFLAGS)
override CFLAGS := -std=c99 -g -Og -Wall -Wextra -Wpedantic $(CFLAGS)
override RUSTFLAGS := -g -O $(RUSTFLAGS)

bench: private LDFLAGS += -L. -Wl,-R,\$$ORIGIN -znow
bench: private RUSTFLAGS += --test
bench: dl.rs dlfcn.rs got.rs link.rs mman.rs liblib.so

refcount: private CPPFLAGS += -D_GNU_SOURCE
refcount: private LDLIBS += -ldl

rel: private LDFLAGS += -Wl,-R,\$$ORIGIN
rel: private LDLIBS += -ldl
rel: librela.so

dlfcn.rs: private CPPFLAGS += -D_GNU_SOURCE
link.rs: private BINDFLAGS += --no-rustfmt-bindings
lib.o: lib.h
rel.o: private CFLAGS += -Wno-pedantic
rela.o: private CPPFLAGS += -D_GNU_SOURCE

.PHONY: clean
clean:
	$(RM) bench refcount rel dlfcn.rs link.rs mman.rs *.o *.so

%: %.rs
	$(RUSTC) -Clink-args="$(LDFLAGS)" $(RUSTFLAGS) $< $(LDLIBS)

%.rs: %.h
	$(BINDGEN) $(BINDFLAGS) -o $@ $^ -- $(CPPFLAGS)

lib%.so: %.o
	$(CC) $(LDFLAGS) -shared -o $@ $^ $(LDLIBS)
