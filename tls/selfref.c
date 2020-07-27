#include <asm/prctl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

int arch_prctl(int, const void ***);
void **_dl_allocate_tls(void **);
void _dl_deallocate_tls(void **, bool);

int main(void) {
	const void **fs;
	arch_prctl(ARCH_GET_FS, &fs);
	printf(" %%fs  = %#lx\n", (uintptr_t) fs);
	printf("(%%fs) = %#lx\n", (uintptr_t) *fs);

	void **nfs = _dl_allocate_tls(NULL);
	*nfs = nfs;
	printf(" %%fs  = %#lx\n", (uintptr_t) nfs);
	printf("(%%fs) = %#lx\n", (uintptr_t) *nfs);
	_dl_deallocate_tls(nfs, true);

	return 0;
}
