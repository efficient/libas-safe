override CFLAGS := -std=c99 -g -Og -Wall -Wextra -Wpedantic $(CFLAGS)

libctor: private LDFLAGS += -Wl,-R\$$ORIGIN
libctor: private LDLIBS += -ldl
libctor: libctor.so

.PHONY: clean
clean:
	git clean -fX

lib%.so: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -shared $< -o $@
