CFLAGS := -std=c99 -Wall -Wextra -Wpedantic -g -Og

paste: private LDFLAGS += -Wl,-R\$$ORIGIN
paste: copy.h libcopy.so

copy.o: private CFLAGS += -fpic

.PHONY: clean
clean:
	$(RM) paste *.o *.so

include libgotcha/libgotcha.mk
