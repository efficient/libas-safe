#include <dlfcn.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

static void __attribute__((constructor)) deferred(void) {
	setenv("LD_PRELOAD", "./libinger.so", true);
	if(!dlopen("./libinger.so", RTLD_LAZY | RTLD_GLOBAL)) {
		fputs("libdeferred.so: Error! Try increasing GLIBC_TUNABLES=glibc.rtld.optional_static_tls?\n", stderr);
		abort();
	}
}
