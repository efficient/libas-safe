CFLAGS   := -std=c99 -fpic -g -O2
CPPFLAGS := -D_DEFAULT_SOURCE

all: as snow

as:
snow:

as_fixed: LDFLAGS += -L. -Wl,-R\$$ORIGIN
as_fixed: LDLIBS  += -las-safe

snow_fixed: LDFLAGS += -L. -Wl,-R\$$ORIGIN
snow_fixed: LDLIBS  += -las-safe

.PHONY: clean
clean:
	$(RM) as as_fixed snow snow_fixed *.o

%_fixed.o: %.o
	cp $< $@
