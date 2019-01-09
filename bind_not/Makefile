override CFLAGS := -std=c99 -g -Og -Wall -Wextra -Wpedantic $(CFLAGS)

exe: private LDFLAGS += -Wl,-R\$$ORIGIN
exe: liblib.so
liblib.so: lib.h

.PHONY: clean
clean:
	git clean -fX

lib%.so: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -shared $< -o $@
