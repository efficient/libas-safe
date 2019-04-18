#include <dlfcn.h>
#include <stdio.h>

void *dlopen(const char *filename, int flags) {
	puts("dlopen() from libdlinterp");
	return dlopen(filename, flags);
}
