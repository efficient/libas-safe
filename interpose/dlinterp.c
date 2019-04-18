#include <sys/mman.h>
#include <dlfcn.h>
#include <stdio.h>
#include <unistd.h>

void *dlopen(const char *filename, int flags) {
	puts("dlopen() from libdlinterp");
	return dlopen(filename, flags);
}

void *dlsym(void *handle, const char *symbol) {
	puts("dlsym() from libdlinterp");
	return dlsym(handle, symbol);
}

int mprotect(void *addr, size_t len, int prot) {
	puts("mprotect() from libdlinterp");
	return mprotect(addr, len, prot);
}

long sysconf(int name) {
	puts("sysconf() from libdlinterp");
	return sysconf(name);
}
