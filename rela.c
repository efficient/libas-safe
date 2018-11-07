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

// This function gets ready to fix up the dynamic library containing caller.  If caller's address is
// already recognized, it need not do anything; however, we must also maintain a mapping from link
// maps to lists of caller addresses so we can invalidate such entries in case their libraries are
// later unloaded.
void fixup(void *caller, ElfW(Dyn) *dynamic, ElfW(Addr) *global_offset_table) {
	assert(dynamic != _DYNAMIC);
	assert(global_offset_table != _GLOBAL_OFFSET_TABLE_);

	Dl_info dli;
	// Look up the shared object containing caller.
	int succ = dladdr(caller, &dli);
	assert(succ);
	assert(dli.dli_fname && "Info structure contains no filename");

	// Get a handle to the global link map.  This will be used to search for the specific link
	// map corresponding to the shared object found above, and also as a fallback in case the
	// library isn't found (in which case caller must've been statically linked into the
	// executable.)
	struct link_map *l = dlopen(NULL, RTLD_LAZY | RTLD_NOLOAD);
	struct link_map *m = NULL;
	// Ensure that handles *are* link maps so that we can cast between the two to save calls.
	assert(!dlinfo(l, RTLD_DI_LINKMAP, &m) && l == m && "Handles point to opaque data");
	assert(!*l->l_name && "Root handle has nonempty name");
	// Search for the shared object among all those loaded.  This is a slow path that is
	// necessary in order to avoid spurious dlopen() calls that would erroneously increment
	// the dynamic loader's reference counts.  We could add a fast path by memoizing lookups
	// using a hashmap, although we'd need to be sure to purge stale entries upon a dlclose().
	for(struct link_map *it = l->l_next; it && !*l->l_name; it = it->l_next)
		if(!strcmp(it->l_name, dli.dli_fname))
			l = it;
	assert(l->l_ld == dynamic && "Dynamic section address mismatch");

	// Read the dynamic section of the dynamic library in question to find its relocation
	// tables, symbol table, string table, and global offset table.
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

	// Check to make sure there are no COPY relocation entries.  These indicate that a global is
	// exposed directly via the shared library's public API, and is therefore located outside
	// that library.
	const ElfW(Rela) *end = rela + relasz / sizeof *end;
	for(const ElfW(Rela) *r = rela; r != end; ++r)
		assert(ELF64_R_TYPE(r->r_info) != R_X86_64_COPY && "Found COPY relocation entry");

	// High-water mark showing the first namespace identifier that has caused an error.
	Lmid_t count = LM_ID_NEWLM;
	// Iterate over each PLT relocation, looking up its name and the shared object containing
	// it, then opening as many copies of that library as possible.
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
