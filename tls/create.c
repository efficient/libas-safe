#include <pthread.h>
#include <stdio.h>
#include <time.h>

#define ITERS 1000000

static unsigned long nsnow(void) {
	struct timespec tv;
	clock_gettime(CLOCK_REALTIME, &tv);
	return tv.tv_sec * 1000000000 + tv.tv_nsec;
}

static void *dummy(void *ign) {
	return ign;
}

int main(void) {
	unsigned long nsthen = nsnow();
	for(int iter = 0; iter < ITERS; ++iter) {
		pthread_t tid;
		pthread_create(&tid, NULL, dummy, NULL);
		pthread_join(tid, NULL);
	}

	unsigned long nswhen = (nsnow() - nsthen) / ITERS;
	printf("%ld.%03ld μs\n", nswhen / 1000, nswhen % 1000);

	return 0;
}
