#include <assert.h>
#include <dlfcn.h>
#include <link.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char **argv) {
	(void) argv;

	const struct link_map *l;
	for(l = dlopen(NULL, RTLD_LAZY); !strstr(l->l_name, "/librrpreload.so"); l = l->l_next)
		assert(l->l_next);

	const ElfW(Dyn) *d;
	for(d = l->l_ld; d->d_tag != DT_PLTGOT; ++d)
		assert(d->d_tag != DT_NULL);

	const struct link_map **got = (const struct link_map **) d->d_un.d_ptr;
	if(argc == 1) {
		if(!got[1]) {
			fputs("gots[1] = l ... ", stdout);
			fflush(stdout);
			// Protection violation when run with LD_BIND_NOW!  Pass any command-line
			// arg(s) to skip the assignment/check and move on to the dynamic call.
			got[1] = l;
			puts("done!");
		}
		assert(got[1] == l);
	}

	fputs("sysconf(_SC_PAGESIZE) ... ", stdout);
	fflush(stdout);
	// Protection violation in _dl_fixup() when run without LD_BIND_NOW!
	assert(sysconf(_SC_PAGESIZE) == 4096);
	puts("done!");

	return 0;
}
