#include <link.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

static inline void rela(const ElfW(Rela) *r, uintptr_t addr, const ElfW(Sym) *st, const char *s) {
	st += ELF64_R_SYM(r->r_info);
	if(st->st_shndx != SHN_UNDEF && ELF64_ST_TYPE(r->r_info) != STT_OBJECT) {
		const void *imp = dlsym(RTLD_NEXT, s += st->st_name);
		if(imp)
			*(const void **) (addr + r->r_offset) = imp;
	}
}

static inline const void *dyn(unsigned tag) {
	for(const ElfW(Dyn) *d = _DYNAMIC; d->d_tag != DT_NULL; ++d)
		if(d->d_tag == tag)
			return (void *) d->d_un.d_ptr;
	return NULL;
}

static void __attribute__((constructor)) ctor(void) {
	const struct link_map *l;
	for(l = dlopen(NULL, RTLD_LAZY); l && l->l_ld != _DYNAMIC; l = l->l_next);

	uintptr_t addr = l->l_addr;
	const ElfW(Rela) *rel = dyn(DT_RELA);
	const ElfW(Rela) *rele = (ElfW(Rela) *) ((uintptr_t) rel + (size_t) dyn(DT_RELASZ));
	const ElfW(Rela) *jmprel = dyn(DT_JMPREL);
	const ElfW(Rela) *jmprele = (ElfW(Rela) *) ((uintptr_t) jmprel + (size_t) dyn(DT_PLTRELSZ));
	const ElfW(Sym) *symtab = dyn(DT_SYMTAB);
	const char *strtab = dyn(DT_STRTAB);

	for(const ElfW(Rela) *r = rel; r != rele; ++r)
		if(ELF64_R_TYPE(r->r_info) == R_X86_64_GLOB_DAT)
			rela(r, addr, symtab, strtab);
	for(const ElfW(Rela) *r = jmprel; r != jmprele; ++r)
		rela(r, addr, symtab, strtab);
}
