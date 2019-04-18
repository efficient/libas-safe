#include <dlfcn.h>
#include <stdio.h>

void *dlopen(const char *filename, int flags) {
	puts("dlopen() from libdlinterp");
	return dlopen(filename, flags);
}

void *dlsym(void *handle, const char *symbol) {
	puts("dlsym() from libdlinterp");
	return dlsym(handle, symbol);
}
