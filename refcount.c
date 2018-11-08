#include <assert.h>
#include <dlfcn.h>
#include <link.h>
#include <stdio.h>
#include <string.h>

int main(void) {
	const char *libc = NULL;
	for(struct link_map *l = dlopen(NULL, RTLD_LAZY); l; l = l->l_next)
		if(strstr(l->l_name, "/libc.so")) {
			libc = l->l_name;
			break;
		}

	Lmid_t lmid = LM_ID_BASE + 1;
	struct link_map *l = dlmopen(LM_ID_NEWLM, libc, RTLD_LAZY);
	assert(l);
	assert(dlmopen(lmid, libc, RTLD_LAZY | RTLD_NOLOAD));
	dlclose(l);
	dlclose(l);
	assert(!dlmopen(lmid, libc, RTLD_LAZY | RTLD_NOLOAD));

	return 0;
}
