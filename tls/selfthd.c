#include <asm/prctl.h>
#include <bits/pthreadtypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

int arch_prctl(int, const void ***);
pthread_t pthread_self(void);
void **_dl_allocate_tls(void **);
void _dl_deallocate_tls(void **, bool);

int main(void) {
	const void **fs;
	arch_prctl(ARCH_GET_FS, &fs);
	printf("%#lx\n", pthread_self());

	void **nfs = _dl_allocate_tls(NULL);
	*nfs = nfs; // TLS
	nfs[2] = nfs; // self
	arch_prctl(ARCH_SET_FS, (const void ***) nfs);
	printf("%#lx\n", pthread_self());

	arch_prctl(ARCH_SET_FS, (const void ***) fs);
	_dl_deallocate_tls(nfs, true);

	return 0;
}
