CFLAGS := -std=c99 -O2

nested: private LDFLAGS += -Wl,-R\$$ORIGIN
nested: libnest.so

nest.o: private CFLAGS += -fpic
nest.o: nest.h
nested.o: nest.h

.PHONY: clean
clean:
	$(RM) nested *.o *.so

include libgotcha/libgotcha.mk
