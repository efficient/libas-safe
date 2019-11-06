#include <sys/auxv.h>
#include <link.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static inline bool bootstrap(void) {
	#pragma weak _start
	void _start(void);

	void (*start)(void) = (void (*)(void)) getauxval(AT_ENTRY);
	if(start != _start) {
		const char *preload = getenv("LD_PRELOAD");
		if(!preload)
			return false;

		const struct link_map *l;
		for(l = dlopen(NULL, RTLD_LAZY); l->l_ld != _DYNAMIC; l = l->l_next)
			if(!l)
				return false;

		const char *name = strrchr(l->l_name, '/');
		return strstr(preload, name ? name + 1 : l->l_name);
	}

	return true;
}

static void __attribute__((constructor)) ctor(void) {
	puts("ctor()");
	if(bootstrap()) {
		const char *arg0 = (char *) getauxval(AT_EXECFN);
		puts(arg0);
		dlmopen(LM_ID_NEWLM, arg0, RTLD_LAZY);
	}
}
