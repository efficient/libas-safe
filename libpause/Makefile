override CFLAGS   := -std=c99 -O2 -Wall -Wextra -Wpedantic $(CFLAGS)
override CPPFLAGS := -D_GNU_SOURCE $(CPPFLAGS)
override LDFLAGS  := $(LDFLAGS)
override LDLIBS   := $(LDLIBS)

libpause.so: private LDLIBS += -lrt
libpause.so:

test: private LDFLAGS += -Wl,-R\$$ORIGIN
test: libpause.so

include libgotcha/libgotcha.mk

.PHONY: clean
clean:
	$(RM) test *.a *.o *.so
