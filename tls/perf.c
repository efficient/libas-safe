#include <stdio.h>
#include <time.h>

#define ITERS 10000000

void *_dl_allocate_tls(void *);

static unsigned long nsnow(void) {
	struct timespec tv;
	clock_gettime(CLOCK_REALTIME, &tv);
	return tv.tv_sec * 1000000000 + tv.tv_nsec;
}

int main(void) {
	unsigned long nsthen = nsnow();
	for(int iter = 0; iter < ITERS; ++iter)
		_dl_allocate_tls(NULL);

	unsigned long nswhen = (nsnow() - nsthen) / ITERS;
	printf("%ld.%03ld μs\n", nswhen / 1000, nswhen % 1000);

	return 0;
}
