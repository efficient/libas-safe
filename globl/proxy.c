#include "proxy.h"

#include <link.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

static struct link_map *dlget(Lmid_t lmid, const char *filename) {
	struct link_map *l = dlmopen(lmid, filename, RTLD_LAZY | RTLD_NOLOAD);
	if(l)
		dlclose(l);
	return l;
}

extern const char *__progname_full;

void *proxy(unsigned lmid, void *(*fun)(void *), void *arg) {
	Dl_info dli;
	bool succ = dladdr((void *) (uintptr_t) fun, &dli);
	if(!succ)
		return NULL;
	if(!lmid && !strcmp(dli.dli_fname, __progname_full))
		dli.dli_fname = NULL;

	struct link_map *l = dlget(lmid, dli.dli_fname);
	if(!l)
		return NULL;
	fun = (void *(*)(void *)) ((uintptr_t) fun - (uintptr_t) dli.dli_fbase + l->l_addr);
	return fun(arg);
}
