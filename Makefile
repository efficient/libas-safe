CFLAGS   += -std=c99 -g -O2 -fpic -fno-optimize-sibling-calls -Wall -Wextra -Wpedantic
CPPFLAGS += -D_DEFAULT_SOURCE

-include libgotcha.mk

.PHONY: all
all: libas-safe.so

libas-safe.so: libgotcha.h

libgotcha.a: libgotcha/libgotcha.a
	objcopy -Wsigaction --globalize-symbol libgotcha_sigaction --globalize-symbol libgotcha_pthread_sigmask --globalize-symbol libgotcha_sigaddset --globalize-symbol libgotcha_sigfillset $< $@

.PHONY: clean
clean:
	$(RM) *.a *.o *.so
