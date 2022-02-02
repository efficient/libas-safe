CFLAGS   += -std=c99 -g -O2 -fpic -fno-optimize-sibling-calls -Wall -Wextra -Wpedantic
CPPFLAGS += -I$(LIBGOTCHA_PATH) -D_DEFAULT_SOURCE
override LDFLAGS  := $(LDFLAGS)

CP := objcopy -Wsigaction --globalize-symbol libgotcha_sigaction --globalize-symbol libgotcha_pthread_sigmask --globalize-symbol libgotcha_sigaddset --globalize-symbol libgotcha_sigfillset

.PHONY: all
all: libas-safe.so

ifneq ($(LDFLAGS),)
LIBSTDRUST_SONAME := $(shell sed -n "s/LIBSTDRUST_SONAME := //p" libgotcha/libgotcha.mk)

libas-safe.so: lib$(LIBSTDRUST_SONAME).so
libas-safe.so: private LDFLAGS += -Wl,-R\$$ORIGIN

lib$(LIBSTDRUST_SONAME).so: libnull.a
	$(CC) $(LDFLAGS) -shared -zdefs -ztext -Wl,--whole-archive /usr/lib/rustlib/x86_64-unknown-linux-gnu/lib/libstd-*.rlib /usr/lib/rustlib/x86_64-unknown-linux-gnu/lib/libhashbrown-*.rlib -Wl,--no-whole-archive -o $@ $^ -ldl -lm -lpthread

libnull.a:
	$(RUSTC) --crate-type staticlib /dev/null
endif

.PHONY: clean
clean:
	$(RM) *.a *.o *.so signal

include libgotcha/libgotcha.mk
