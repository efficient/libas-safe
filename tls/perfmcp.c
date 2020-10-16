#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#define ITERS 2500000

static unsigned long nsnow(void) {
	struct timespec tv;
	clock_gettime(CLOCK_REALTIME, &tv);
	return tv.tv_sec * 1000000000 + tv.tv_nsec;
}

int main(int argc, char **argv) {
	if(argc != 2) {
		printf("USAGE: %s <bytes>\n", argv[0]);
		return 1;
	}

	size_t bytes;
	sscanf(argv[1], "%lu", &bytes);

	unsigned long nsthen = nsnow();
	uint8_t src[bytes];
	uint8_t dest[bytes];
	for(int iter = 0; iter < ITERS; ++iter) {
		memcpy(dest, src, bytes);
		__asm__("" ::: "memory");
	}

	unsigned long nswhen = (nsnow() - nsthen) / ITERS;
	printf("%ld.%03ld μs\n", nswhen / 1000, nswhen % 1000);

	return 0;
}
