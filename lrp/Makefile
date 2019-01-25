override CFLAGS  := -O2 -Wall -Wextra -Wpedantic $(CFLAGS)
override LDFLAGS := -L. -Wl,-R\$$ORIGIN
override LDLIBS  := -ldl

lrp: librrpreload.so

librrpreload.so: librrpreload.so.hex
	xxd -r $< $@

librrpreload.so.hex: librrpreload.so.patch
	xxd /usr/lib/rr/librrpreload.so $@
	patch $@ $<

.PHONY: clean
clean:
	git clean -fX
