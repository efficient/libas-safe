override CFLAGS := -std=c99 -g -Og -Wall -Wextra -Wpedantic $(CFLAGS)

bin: private LDFLAGS += -L. -Wl,-R,\$$ORIGIN
bin: private LDLIBS += -ldl -llib
bin: liblib.so

bin.o: lib.h
bin.o: private CPPFLAGS += -D_GNU_SOURCE
lib.o: lib.h

.PHONY: clean
clean:
	$(RM) bin *.o *.so

lib%.so: %.o
	$(CC) $(LDFLAGS) -shared -o $@ $^ $(LDLIBS)
