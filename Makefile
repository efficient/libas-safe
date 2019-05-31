CFLAGS   += -std=c99 -g -O2 -fpic -fno-optimize-sibling-calls -Wall -Wextra -Wpedantic
CPPFLAGS += -I$(LIBGOTCHA_PATH) -D_DEFAULT_SOURCE

CP := objcopy -Wsigaction --globalize-symbol libgotcha_sigaction --globalize-symbol libgotcha_pthread_sigmask --globalize-symbol libgotcha_sigaddset --globalize-symbol libgotcha_sigfillset

.PHONY: all
all: libas-safe.so

.PHONY: clean
clean:
	$(RM) *.a *.o *.so

include libgotcha/libgotcha.mk
