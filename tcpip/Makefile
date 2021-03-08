override CFLAGS := -g3 -Og -Wall -Wextra -Wpedantic $(CFLAGS)
override CPPFLAGS := -D_DEFAULT_SOURCE $(CPPFLAGS)

BINS := echo

.PHONY: all
all: $(BINS)

.PHONY: clean
clean:
	$(RM) $(BINS)
