#include <assert.h>
#include <link.h>
#include <stddef.h>

static inline const struct link_map *dlget(const char *f) {
	struct link_map *l = dlopen(f, RTLD_LAZY | RTLD_NOLOAD);
	if(l)
		dlclose(l);
	return l;
}

static inline size_t dynget(const struct link_map *l, ElfW(Sxword) t) {
	assert(l);
	for(const ElfW(Dyn) *d = l->l_ld; d->d_tag != DT_NULL; ++d)
		if(d->d_tag == t)
			return d->d_un.d_ptr;
	return 0;
}

int main(void) {
	dlmopen(LM_ID_NEWLM, "libpthread.so.0", RTLD_LAZY);

	const struct link_map *l = dlget("libpthread.so.0");
	void (*init)(void) = (void (*)(void)) (l->l_addr + dynget(l, DT_INIT));
	init();

	return 0;
}
