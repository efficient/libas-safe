#include "hook.h"

#include <sys/mman.h>
#include <stdint.h>

#undef hook_got

void (*hook_resolver)(void);
void (*hook_installed)(size_t);

void hook_internal_trampoline(void);

void hook_got(void (*got[])(void), void (*hook)(size_t)) {
	hook_installed = hook;
	if(!hook_resolver) {
		uintptr_t entry = (uintptr_t) (got + 2) & ~0xfffL;
		mprotect((void *) entry, 8, PROT_READ | PROT_WRITE);
		hook_resolver = got[2];
		got[2] = hook_internal_trampoline;
	}
}
