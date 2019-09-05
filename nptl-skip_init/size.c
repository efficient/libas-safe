#include <assert.h>
//#include <elf.h>
#include <link.h>
//#include <stdint.h>
#include <stdio.h>
//#include <string.h>

extern uint8_t _rtld_global[];

int main(void) {
	/*const struct link_map *l;
	for(l = dlopen(NULL, RTLD_LAZY); !strstr(l->l_name, "ld-linux"); l = l->l_next);

	const char *s = NULL;
	const ElfW(Sym) *st = NULL;
	const ElfW(Dyn) *d;
	for(d = l->l_ld; !s || !st; ++d)
		switch(d->d_tag) {
		case DT_STRTAB:
			s = (char *) d->d_un.d_ptr;
			break;
		case DT_SYMTAB:
			st = (ElfW(Sym) *) d->d_un.d_ptr;
			break;
		}

	for(; strcmp(s + st->st_name, "_rtld_global"); ++st);

	uint64_t rtld[st->st_size];
	printf("%#lx %#lx\n", _rtld_global, st->st_size);
	memcpy(rtld, _rtld_global, st->st_size);
	*/

	struct link_map *nptl = dlmopen(LM_ID_NEWLM, "./libpthread.so.0", RTLD_LAZY);

	// If you failed this assertion, you forgot to put a copy of libpthread in PWD!
	assert(nptl);

	// This does not solve the segfault because libpthread's constructor somehow
	// causes _rtld_local to be updated (in addition to _rtld_global)!
	//memcpy(_rtld_global, rtld, st->st_size);

	// This also does not solve the segfault because libpthread's constructor doesn't
	// make the same change when run from the base namespace?!
	//dlopen("libpthread.so.0", RTLD_LAZY);

	// This segfaults if libpthread's constructor hasn't been disabled (i.e., it was
	// patched by a libgotcha revision older than 358c535).
	dlclose(nptl);

	// This assertion ensures we loaded a libpthread that was not marked with the
	// NODELETE flag (i.e., it was patched by libgotcha revision cacb776 or newer).
	assert(!dlmopen(1, "./libpthread.so.0", RTLD_LAZY | RTLD_NOLOAD));

	return 0;
}
