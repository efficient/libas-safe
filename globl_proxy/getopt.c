#include <sys/mman.h>
#include <assert.h>
#include <dlfcn.h>
#include <link.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define ANCILLARY_GETOPT
#define INDIRECT_OPTARG
#define ANCILLARY_OPTARG

static void *optarg_location(void *ign) {
	(void) ign;
	return &optarg;
}

static struct link_map *dlget(Lmid_t lmid, const char *filename) {
	struct link_map *l = dlmopen(lmid, filename, RTLD_LAZY | RTLD_NOLOAD);
	if(l)
		dlclose(l);
	return l;
}

static void *proxy(Lmid_t lmid, void *(*fun)(void *), void *arg) {
	extern const char *__progname_full;

	Dl_info dli;
	bool succ = dladdr((void *) (uintptr_t) fun, &dli);
	assert(succ);
	if(!lmid && !strcmp(dli.dli_fname, __progname_full))
		dli.dli_fname = NULL;

	struct link_map *l = dlget(lmid, dli.dli_fname);
	assert(l);
	fun = (void *(*)(void *)) ((uintptr_t) fun - (uintptr_t) dli.dli_fbase + l->l_addr);
	return fun(arg);
}

int main(int argc, char **argv) {
	typedef int (*getopt_t)(int, char *const *, const char *);
	extern getopt_t _GLOBAL_OFFSET_TABLE_[];

	struct link_map *l = dlmopen(LM_ID_NEWLM, argv[0], RTLD_LAZY);
	getopt_t *it;
	for(it = _GLOBAL_OFFSET_TABLE_ - 1; *it != getopt; --it);
	mprotect((void *) ((uintptr_t) it & ~0xfff), 0xfff, PROT_WRITE);
#ifdef ANCILLARY_GETOPT
	*it = (getopt_t) (uintptr_t) dlsym(l, "getopt");
#endif
	mprotect((void *) ((uintptr_t) it & ~0xfff), 0xfff, PROT_READ);

#ifndef INDIRECT_OPTARG
	optarg = "";
#else
#ifndef ANCILLARY_OPTARG
	char **optarg = proxy(0, optarg_location, NULL);
#else
	char **optarg = proxy(1, optarg_location, NULL);
#endif
	*optarg = "";
#endif

	const char *fmt = argv[1];
	size_t pos = 0;
	int arg;
	while((arg = getopt(argc - 1, argv + 1, fmt)) != -1)
		if(arg != '?') {
			putchar(arg);
			if(fmt[++pos] == ':') {
#ifndef INDIRECT_OPTARG
				fputs(optarg, stdout);
#else
				fputs(*optarg, stdout);
#endif
				++pos;
			}
			putchar('\n');
		}

	return 0;
}
