override CFLAGS := -Og -fpic -Wall -Wextra -Wpedantic $(CFLAGS)
override LDFLAGS := $(LDFLAGS)
override LDLIBS := -lpthread $(LDLIBS)

CC := c99

BINS := tls

.PHONY: all
all: $(BINS)

tls: private LDFLAGS += -L. -Wl,-R\$$ORIGIN
tls: private LDLIBS += -linger

.PHONY: clean
clean:
	$(RM) $(BINS)
