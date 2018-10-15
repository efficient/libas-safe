#include <assert.h>
#include <dlfcn.h>
#include <link.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

extern ElfW(Addr) _GLOBAL_OFFSET_TABLE_[];

static Lmid_t min(Lmid_t left, Lmid_t right) {
	return left <= right ? left : right;
}

void fixup(void *caller, ElfW(Dyn) *dynamic, ElfW(Addr) *global_offset_table) {
	assert(dynamic != _DYNAMIC);
	assert(global_offset_table != _GLOBAL_OFFSET_TABLE_);

	Dl_info dli;
	int succ = dladdr(caller, &dli);
	assert(succ);
	assert(dli.dli_fname && "Info structure contains no filename");

	struct link_map *l = dlopen(NULL, RTLD_LAZY | RTLD_NOLOAD);
	struct link_map *m = NULL;
	assert(!dlinfo(l, RTLD_DI_LINKMAP, &m) && l == m && "Handles point to opaque data");
	assert(!*l->l_name && "Root handle has nonempty name");
	for(struct link_map *it = l->l_next; it && !*l->l_name; it = it->l_next)
		if(!strcmp(it->l_name, dli.dli_fname))
			l = it;
	assert(l->l_ld == dynamic && "Dynamic section address mismatch");

	const char *strs = NULL;
	const ElfW(Sym) *syms = NULL;
	const ElfW(Addr) *got = NULL;
	const ElfW(Rela) *rel = NULL;
	const ElfW(Rela) *rela = NULL;
	size_t relsz = 0;
	size_t relasz = 0;
	bool relent = false;
	bool relaent = false;
	for(const ElfW(Dyn) *d = l->l_ld; !strs || !syms || !got || !rel || !rela || !relsz || !relasz || !relent || !relaent; ++d)
		switch(d->d_tag) {
		case DT_STRTAB:
			assert(!strs && "Multiple STRTAB entries in dynamic section");
			strs = (char *) d->d_un.d_ptr;
			break;
		case DT_SYMTAB:
			assert(!syms && "Multiple SYMTAB entries in dynamic section");
			syms = (ElfW(Sym) *) d->d_un.d_ptr;
			break;
		case DT_PLTGOT:
			assert(!got && "Multiple PLTGOT entries in dynamic section");
			got = (ElfW(Addr) *) d->d_un.d_ptr;
			assert(got == global_offset_table && "GOT address mismatch");
			break;
		case DT_JMPREL:
			assert(!rel && "Multiple JMPREL entries in dynamic section");
			rel = (ElfW(Rela) *) d->d_un.d_ptr;
			break;
		case DT_PLTREL:
			assert(!relent && "Mutliple PLTREL entries in dynamic section");
			relent = true;
			assert(d->d_un.d_val == DT_RELA && "PLT relocation entry type mismatch");
			break;
		case DT_PLTRELSZ:
			assert(!relsz && "Multiple PLTRELSZ entries in dynamic section");
			relsz = d->d_un.d_val;
			break;
		case DT_RELA:
			assert(!rela && "Multiple RELA entries in dynamic section");
			rela = (ElfW(Rela) *) d->d_un.d_ptr;
			break;
		case DT_RELAENT:
			assert(!relaent && "Multiple RELAENT entries in dynamic section");
			relaent = true;
			assert(d->d_un.d_val == sizeof *rela && "Relocation entry size mismatch");
			break;
		case DT_RELASZ:
			assert(!relasz && "Multiple RELASZ entries in dynamic section");
			relasz = d->d_un.d_val;
			break;
		case DT_NULL:
			assert(false && "Dynamic section missing required entries");
			break;
		}

	const ElfW(Rela) *end = rela + relasz / sizeof *end;
	for(const ElfW(Rela) *r = rela; r != end; ++r)
		assert(ELF64_R_TYPE(r->r_info) != R_X86_64_COPY && "Found COPY relocation entry");

	Lmid_t count = LM_ID_NEWLM;
	end = rel + relsz / sizeof *end;
	for(const ElfW(Rela) *r = rel; r != end; ++r) {
		size_t sym = ELF64_R_SYM(r->r_info);
		size_t str = syms[sym].st_name;
		const char *sname = strs + str;
		void *addr = dlsym(l, sname);
		assert(addr);

		int succ = dladdr(addr, &dli);
		assert(succ);
		assert(dli.dli_fname);
		printf("%s %s\n", dli.dli_fname, sname);

		Lmid_t n = LM_ID_BASE + 1;
		for(void *handle = NULL; n != count && (handle = dlmopen(min(n, count), dli.dli_fname, RTLD_LAZY)); ++n) {
			void *resolv = dlsym(handle, sname);
			assert(resolv);
		}
		if(count == LM_ID_NEWLM)
			count = n + 1;
	}
}
