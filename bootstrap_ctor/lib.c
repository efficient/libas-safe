#include <sys/auxv.h>
#include <dlfcn.h>
#include <stdint.h>
#include <stdio.h>

#pragma weak _start
void _start(void);

static void __attribute__((constructor)) ctor(void) {
	void (*start)(void) = (void (*)(void)) getauxval(AT_ENTRY);
	printf("%#lx\n", start);
	printf("%#lx\n", _start);

	if(start == _start) {
		const char *arg0 = (char *) getauxval(AT_EXECFN);
		puts(arg0);
		dlmopen(LM_ID_NEWLM, arg0, RTLD_LAZY);
	}
}
