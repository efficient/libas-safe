CFLAGS   := -std=c99 -fpic -g -O2
CPPFLAGS := -D_DEFAULT_SOURCE

as:

as_fixed: LDFLAGS += -L. -Wl,-R\$$ORIGIN
as_fixed: LDLIBS  += -las-safe

as_fixed.o: as.o
	cp $< $@

.PHONY: clean
clean:
	$(RM) as as_fixed *.o
