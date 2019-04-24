#include "deref.h"

#include <elfutils/libasm.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <link.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static inline const ElfW(Shdr) *next_shdr(const ElfW(Shdr) *shdr, const ElfW(Shdr) *shdre, uint32_t type) {
	for(const ElfW(Shdr) *sh = shdr; sh != shdre; ++sh)
		if(sh->sh_type == type)
			return sh;
	return NULL;
}

struct insertion {
	size_t offset;
	const uint8_t *inst;
	size_t size;
};

static int deref(char *reg, size_t len, void *out) {
	(void) len;
	(void) out;

	struct insertion *insert = out;
	const uint8_t **mov = r(reg);
	insert->inst = *mov;
	insert->size = mov[1] - mov[0];

	// Stop after processing the first instruction.
	return 1;
}

int main(int argc, char **argv) {
	char *flags[argc + 3];
	flags[0] = "cc";
	memcpy(flags + 1, argv + 1, (argc - 1) * sizeof *flags);
	flags[argc] = NULL;
	for(char **arg = argv; arg != argv + argc; ++arg)
		if(strlen(*arg) == 2 && (*arg)[0] == '-')
			switch((*arg)[1]) {
			case 'c':
			case 'E':
			case 'S':
				execvp(*flags, flags);
				abort();
			}
	flags[argc] = "-znocopyreloc";
	flags[argc + 1] = "-ztext";
	flags[argc + 2] = NULL;

	Ebl ebl;
	memset(&ebl, 0, sizeof ebl);
	x86_64_init(NULL, 0, &ebl, sizeof ebl);

	DisasmCtx_t *ctx = disasm_begin(&ebl, NULL, NULL);
	for(char **arg = argv + 1; arg != argv + argc; ++arg) {
		int fd = -1;
		struct stat st;
		ElfW(Ehdr) *e;
		if((fd = open(*arg, O_RDWR)) >= 0 && !fstat(fd, &st) &&
			(e = mmap(NULL, st.st_size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0)) !=
			MAP_FAILED) {
			struct insertion *insertions = NULL;
			size_t ninsertions = 0;
			size_t growth = 0;
			if(!memcmp(e->e_ident, ELFMAG, SELFMAG) && e->e_type == ET_REL) {
				ElfW(Shdr) *shdr = (ElfW(Shdr) *) ((uintptr_t) e + e->e_shoff);
				const ElfW(Shdr) *shdre = shdr + e->e_shnum;
				const ElfW(Shdr) *load = next_shdr(shdr, shdre, SHT_PROGBITS);
				const ElfW(Shdr) *syh = next_shdr(shdr, shdre, SHT_SYMTAB);
				ElfW(Sym) *sym = (ElfW(Sym) *) ((uintptr_t) e + syh->sh_offset);
				for(const ElfW(Shdr) *sh = shdr;
					(sh = next_shdr(sh, shdre, SHT_RELA)); ++sh) {
					ElfW(Rela) *re = (ElfW(Rela) *)
						((uintptr_t) e + sh->sh_offset + sh->sh_size);
					for(ElfW(Rela) *r = (ElfW(Rela) *)
						((uintptr_t) e + sh->sh_offset); r != re; ++r) {
						size_t oldninsertions = ninsertions;
						bool nonextended = false;
						switch(ELF64_R_TYPE(r->r_info)) {
						case R_X86_64_PC32:
							nonextended = true;
							// Fall through.
						case R_X86_64_PC64: {
							const ElfW(Sym) *st =
								sym + ELF64_R_SYM(r->r_info);
							if(st->st_shndx == SHN_UNDEF) {
								size_t offset = load->sh_offset +
									r->r_offset;
								const uint8_t *inst =
									(uint8_t *) e + offset - 3;
								r->r_info =
									ELF64_R_INFO(ELF64_R_SYM(
									r->r_info), nonextended ?
									R_X86_64_GOTPCREL :
									R_X86_64_GOTPCREL64);
								insertions = realloc(insertions,
									++ninsertions *
									sizeof *insertions);
								insertions[ninsertions - 1].offset =
									offset - r->r_addend;
								disasm_cb(ctx, &inst,
									inst + X86_64_MAX_INSTR_LEN,
									0, "%.2o", deref,
									insertions + ninsertions - 1,
									NULL);
								growth += insertions[ninsertions - 1].size;
							}
						}}

						for(size_t ins = 0; ins < oldninsertions; ++ins)
							if(insertions[ins].offset <=
								r->r_offset - load->sh_offset)
								r->r_offset += insertions[ins].size;
					}
				}

				if(insertions) {
					if(e->e_shoff >= insertions->offset)
						e->e_shoff += growth;
					for(ElfW(Shdr) *sh = shdr; sh != shdre; ++sh)
						if(sh->sh_offset >= insertions->offset)
							sh->sh_offset += growth;
						else if(sh->sh_size && sh->sh_offset + sh->sh_size >
							insertions->offset)
							sh->sh_size += growth;

					const ElfW(Sym) *syme = (ElfW(Sym) *)
						((uintptr_t) sym + syh->sh_size);
					for(ElfW(Sym) *st = sym; st != syme; ++st) {
						const ElfW(Shdr) *sh = shdr + st->st_shndx;
						if(st->st_shndx != SHN_UNDEF &&
							st->st_shndx != SHN_ABS &&
							sh->sh_offset < insertions->offset) {
							uintptr_t addr =
								sh->sh_offset + st->st_value;
							for(size_t ins = 0; ins < ninsertions;
								++ins) {
								if(insertions[ins].offset <= addr)
									st->st_value +=
										insertions[ins].size;
								else if(st->st_size &&
									insertions[ins].offset <=
									addr + st->st_size)
									st->st_size +=
										insertions[ins].size;
							}
						}
					}

					const uint8_t *file = (uint8_t *) e;
					uint8_t *buf = malloc(st.st_size + growth);
					growth = 0;
					memcpy(buf, file, insertions->offset);
					for(size_t ins = 0; ins < ninsertions; ++ins) {
						if(ins) {
							size_t offset = insertions[ins - 1].offset;
							size_t size =
								insertions[ins].offset - offset;
							memcpy(buf + growth + offset +
								insertions[ins].size,
								file + offset, size);
						}

						size_t offset = insertions[ins].offset;
						size_t size = insertions[ins].size;
						memcpy(buf + growth + offset, insertions[ins].inst,
							size);
						growth += size;
					}

					size_t offset = insertions[ninsertions - 1].offset;
					memcpy(buf + growth + offset, file + offset,
						st.st_size - offset);

					write(fd, buf, st.st_size + growth);
					free(buf);
					free(insertions);
				}
			}
			munmap(e, st.st_size);
		}
		if(fd >= 0)
			close(fd);
	}
	disasm_end(ctx);

	execvp(*flags, flags);
	abort();
}
