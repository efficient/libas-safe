#include <sys/mman.h>
#include <assert.h>
#include <dlfcn.h>
#include <link.h>
#include <signal.h>
#include <stdio.h>
#include <ucontext.h>
#include <unistd.h>

#define MASK     0x0badf00d0000
#define NOBJECTS 4

static inline const void *dyn(const struct link_map *l, ElfW(Sword) t) {
	const ElfW(Dyn) *d;
	for(d = l->l_ld; d->d_tag != t; ++d)
		assert(d->d_tag != DT_NULL);
	return (void *) d->d_un.d_ptr;
}

static inline const ElfW(Phdr) *seg(const struct link_map *l, const void *a, const ElfW(Phdr) *p) {
	uintptr_t addr = (uintptr_t) a - l->l_addr;
	const ElfW(Ehdr) *e = (ElfW(Ehdr) *) l->l_addr;
	for(p = p ? p + 1 : (ElfW(Phdr) *) (l->l_addr + e->e_phoff);
		addr < p->p_vaddr || p->p_vaddr + p->p_memsz <= addr;
		++p)
		if(p->p_type == PT_NULL)
			return NULL;
	return p;
}

static inline int prot(const ElfW(Phdr) *p) {
	uint32_t pf = p->p_flags;
	return ((pf & PF_R) ? PROT_READ : 0) | ((pf & PF_W) ? PROT_WRITE : 0) |
		((pf & PF_X) ? PROT_EXEC : 0);
}

static void *objects[NOBJECTS];

static void segv(int no, siginfo_t *si, void *co) {
	(void) no;

	putchar_unlocked('.');

	uintptr_t magic = (uintptr_t) si->si_addr;
	assert(si->si_code == SEGV_MAPERR);
	assert((magic & MASK) == MASK);

	size_t index = magic ^ MASK;
	ucontext_t *uc = co;
	greg_t *reg, *regl;
	for(reg = uc->uc_mcontext.gregs, regl = reg + NGREG - 1; *reg != (greg_t) magic; ++reg)
		assert(reg != regl);
	*reg = (greg_t) objects[index];
}

static void __attribute__((constructor)) ctor(void) {
	struct sigaction handler = {
		.sa_flags = SA_SIGINFO,
		.sa_sigaction = segv,
	};
	sigaction(SIGSEGV, &handler, NULL);

	const struct link_map *l = dlopen(NULL, RTLD_LAZY);
	const ElfW(Rela) *rela = dyn(l, DT_RELA);
	const ElfW(Rela) *rele = (ElfW(Rela) *) ((uintptr_t) rela + (uintptr_t) dyn(l, DT_RELASZ));
	const ElfW(Sym) *sym = dyn(l, DT_SYMTAB);
	const char *str = dyn(l, DT_STRTAB);

	size_t alloc = 0;
	size_t pgoffs = 0;
	int protect = PROT_WRITE;
	const ElfW(Phdr) *segment = NULL;
	puts("Shadowing globals...");
	for(const ElfW(Rela) *r = rela; r != rele; ++r)
		if(ELF64_R_TYPE(r->r_info) == R_X86_64_GLOB_DAT) {
			const ElfW(Sym) *st = sym + ELF64_R_SYM(r->r_info);
			if(st->st_shndx == SHN_UNDEF && ELF64_ST_TYPE(st->st_info) == STT_OBJECT) {
				puts(str + st->st_name);
				assert(alloc < NOBJECTS);

				void **loc = (void **) (l->l_addr + r->r_offset);
				if(!pgoffs) {
					pgoffs = sysconf(_SC_PAGESIZE) - 1;
					while((segment = seg(l, loc, segment)) &&
						(protect = prot(segment)) & PROT_WRITE);
					if(segment) {
						pgoffs &= l->l_addr + segment->p_vaddr;
						mprotect((void *)
							(l->l_addr + segment->p_vaddr - pgoffs),
							segment->p_memsz + pgoffs,
							protect | PROT_WRITE);
					}
				}

				objects[alloc] = *loc;
				*loc = (void *) (alloc++ | MASK);
			}
		}
	if(!(protect & PROT_WRITE))
		mprotect((void *) (l->l_addr + segment->p_vaddr - pgoffs),
			segment->p_memsz + pgoffs, protect);
	putchar('\n');
}
