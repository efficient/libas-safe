#include "libinger.h"

#include <stdio.h>
#include <time.h>

#define ITERS 511

static unsigned long nsnow(void) {
	struct timespec tv;
	clock_gettime(CLOCK_REALTIME, &tv);
	return tv.tv_sec * 1000000000 + tv.tv_nsec;
}

static void nop(void *nope) {
	(void) nope;
}

static void bop(void *nope) {
	(void) nope;
	pause();
}

int main(void) {
	unsigned long nsthen = nsnow();
	for(int iter = 0; iter < ITERS; ++iter)
		launch(nop, -1, NULL);

	unsigned long nswhen = (nsnow() - nsthen) / ITERS;
	printf("%ld.%03ld μs\n", nswhen / 1000, nswhen % 1000);

	return 0;
}
