RUSTC := rustc

override CFLAGS    := -std=c99 -O2 -Wall -Wextra -Wpedantic $(CFLAGS)
override CPPFLAGS  := $(CPPFLAGS)
override LDFLAGS   := $(LDFLAGS)
override LDLIBS    := $(LDLIBS)
override RUSTFLAGS := -Copt-level=2 $(RUSTFLAGS)

.PHONY: all
all: bench libpreload.so

bench: libglobl.so
bench: private RUSTFLAGS += -L. -Crpath
libpreload.so: private LDLIBS += -ldl
preload.o: private CFLAGS += -fpic
preload.o: private CPPFLAGS += -D_DEFAULT_SOURCE

.PHONY: clean
clean:
	$(RM) bench *.o *.so

%: %.rs
	$(RUSTC) $(RUSTFLAGS) --test -Clink-args="$(LDFLAGS)" $< $(LDLIBS)

lib%.so: %.o
	$(CC) $(LDFLAGS) -shared $^ -o $@ $(LDLIBS)

lib%.so: %.rs
	$(RUSTC) $(RUSTFLAGS) --crate-type dylib -Cprefer-dynamic -Clink-args="$(LDFLAGS)" $< $(LDLIBS)
