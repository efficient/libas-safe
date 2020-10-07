#include <dlfcn.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#define ITERS 1000000

void *_dl_allocate_tls(void *);
void _dl_deallocate_tls(void *, bool);

extern uint8_t _rtld_global[];

static size_t *dtv_max_idx(void) {
	return (size_t *) (_rtld_global + 83264);
}

static unsigned long nsnow(void) {
	struct timespec tv;
	clock_gettime(CLOCK_REALTIME, &tv);
	return tv.tv_sec * 1000000000 + tv.tv_nsec;
}

int main(int argc, char **argv) {
	if(argc > 1) {
		size_t *tls_max_dtv_idx = dtv_max_idx();
		size_t snapshot = *tls_max_dtv_idx;
		printf("snap: %lu\n", snapshot);
		for(int i = 0; i < 511; ++i)
			if(!dlmopen(LM_ID_NEWLM, argv[1], RTLD_NOW)) {
				fprintf(stderr, "%s\n", dlerror());
				return 2;
			}
		printf("done: %lu\n", *tls_max_dtv_idx);
		if(argc > 2 && !strcmp(argv[2], "yes"))
			*tls_max_dtv_idx = snapshot;
	} else {
		printf("USAGE: %s <filename> [yes]\n", argv[0]);
		return 1;
	}

	unsigned long nsthen = nsnow();
	for(int iter = 0; iter < ITERS; ++iter)
		_dl_deallocate_tls(_dl_allocate_tls(NULL), true);

	unsigned long nswhen = (nsnow() - nsthen) / ITERS;
	printf("%ld.%03ld μs\n", nswhen / 1000, nswhen % 1000);

	return 0;
}
