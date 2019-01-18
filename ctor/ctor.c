#include <sys/mman.h>
#include <assert.h>
#include <dlfcn.h>
#include <link.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void nop(void) {}

static void pwn(void) {
	puts("\x1b[41mpwn'd!\x1b[49m");
	exit(1);
}

static __attribute__((constructor)) void ctor(void) {
	const struct link_map *l = dlopen(NULL, RTLD_LAZY);
	const ElfW(Dyn) *d;

	for(d = l->l_ld; d->d_tag != DT_STRTAB; ++d)
		assert(d->d_tag != DT_NULL);
	const char *s = (char *) d->d_un.d_ptr;

	for(d = l->l_ld; d->d_tag != DT_SYMTAB; ++d)
		assert(d->d_tag != DT_NULL);
	const ElfW(Sym) *st = (ElfW(Sym) *) d->d_un.d_ptr;

	for(d = l->l_ld; d->d_tag != DT_RELA; ++d)
		assert(d->d_tag != DT_NULL);
	const ElfW(Rela) *r = (ElfW(Rela) *) d->d_un.d_ptr;

	for(d = l->l_ld; d->d_tag != DT_RELASZ; ++d)
		assert(d->d_tag != DT_NULL);
	const ElfW(Rela) *re;

	for(re = (ElfW(Rela) *) ((uintptr_t) r + d->d_un.d_val); strcmp(s + st[ELF64_R_SYM(r->r_info)].st_name, "nop"); ++r)
		assert(r + 1 != re);

	void (**g)(void) = (void (**)(void)) (l->l_addr + r->r_offset);
	printf(" nop@got: %#lx\n", (uintptr_t) g);
	printf("*nop@got: %#lx\n", (uintptr_t) *g);

	size_t pgsz = sysconf(_SC_PAGESIZE);
	void *page = (void *) ((uintptr_t) g & ~(pgsz - 1));
	assert(!mprotect(page, pgsz, PROT_READ | PROT_WRITE));

	*g = pwn;

	mprotect(page, pgsz, PROT_READ);
}
