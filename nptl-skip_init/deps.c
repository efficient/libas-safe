#include <assert.h>
#include <link.h>
#include <stdio.h>
#include <string.h>

#define LIBC  "libc.so.6"
#define LIBDL "libdl.so.2"

static inline struct link_map *dlmget(Lmid_t n, const char *s, int f) {
	struct link_map *l = dlmopen(n, s, f | RTLD_NOLOAD);
	if(l)
		dlclose(l);
	return l;
}

int main(int argc, char **argv) {
	assert(argc == 1);

	const struct link_map *clib;
	for(clib = dlopen(NULL, RTLD_LAZY); !strstr(clib->l_name, LIBC); clib = clib->l_next);
	assert(clib);

	struct link_map *cpwd = dlmopen(LM_ID_NEWLM, "./" LIBC, RTLD_LAZY);
	if(!cpwd) {
		printf("USAGE: %s (but first put a copy of " LIBC " in the working directory!)\n",
			argv[0]);
		return 1;
	}

	struct link_map *dl = dlmopen(1, LIBDL, RTLD_LAZY);
	assert(dl);

	dlclose(cpwd);
	// This assertion ensures that libdl is depending on our *local* copy of libc.
	assert(dlmget(1, "./" LIBC, RTLD_LAZY));

	// Now let's be absolutely sure the system one isn't loaded.
	assert(!dlmget(1, clib->l_name, RTLD_LAZY));
	assert(!dlerror());

	dlclose(dl);
	// This assertion ensures that closing libdl unloaded it...
	assert(!dlmget(1, LIBDL, RTLD_LAZY));
	// ... as well as libc!
	assert(!strcmp(dlerror(), LIBDL ": invalid target namespace in dlmopen(): Invalid argument"));

	return 0;
}
