#include "lib.h"

#include <sys/mman.h>
#include <assert.h>
#include <link.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>

static bool subvert(void (**)(void), const char *);

int main(void) {
	void (*ptr)(void) = NULL;

	bool ok = subvert(&ptr, "helper");
	assert(ok);

	helper();
	assert(ptr);

	ptr();
	ptr = NULL;

	helper();
	assert(ptr);

	ptr();

	return 0;
}

extern const struct link_map *_GLOBAL_OFFSET_TABLE_[];

static bool subvert(void (**fun)(void), const char *sym) {
	const ElfW(Dyn) *d;

	for(d = _DYNAMIC; d->d_tag != DT_STRTAB; ++d)
		if(d->d_tag == DT_NULL)
			return false;
	const char *str = (char *) d->d_un.d_ptr;

	for(d = _DYNAMIC; d->d_tag != DT_SYMTAB; ++d)
		if(d->d_tag == DT_NULL)
			return false;
	const ElfW(Sym) *st = (ElfW(Sym) *) d->d_un.d_ptr;

	for(d = _DYNAMIC; d->d_tag != DT_JMPREL; ++d)
		if(d->d_tag == DT_NULL)
			return false;
	ElfW(Rela) *rel = (ElfW(Rela) *) d->d_un.d_ptr;

	for(d = _DYNAMIC; d->d_tag != DT_PLTRELSZ; ++d)
		if(d->d_tag == DT_NULL)
			return false;
	const ElfW(Rela) *end = (ElfW(Rela) *) ((uintptr_t) rel + d->d_un.d_val);

	ElfW(Rela) *r;
	for(r = rel; strcmp(sym, str + st[ELF64_R_SYM(r->r_info)].st_name); ++r)
		if(r + 1 == end)
			return false;

	size_t pgsz = sysconf(_SC_PAGESIZE);
	ElfW(Addr) *page = (ElfW(Addr) *) ((uintptr_t) &r->r_offset & ~(pgsz - 1));
	assert(page);

	const struct link_map *l = _GLOBAL_OFFSET_TABLE_[1];
	int fail = mprotect(page, pgsz, PROT_READ | PROT_WRITE);
	assert(!fail);
	r->r_offset = (ElfW(Addr)) ((uintptr_t) fun - l->l_addr);
	fail = mprotect(page, pgsz, PROT_READ);
	assert(!fail);

	return true;
}
