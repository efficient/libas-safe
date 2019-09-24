#include <assert.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define START_US   5
#define STEP_NS  100

#define STAMPS  1000
#define WARMUP 50000

#define TAIL      99

#if STAMPS % 2
#error "STAMPS must be even"
#endif

static timer_t timer;
static volatile ssize_t index;
static unsigned long stamps[STAMPS];

static unsigned long nsnow(void) {
	struct timespec tv;
	clock_gettime(CLOCK_REALTIME, &tv);
	return tv.tv_sec * 1000000000 + tv.tv_nsec;
}

static void handler(int ign) {
	(void) ign;

	assert(index <= STAMPS);
	if(index == STAMPS || index++ < 0)
		return;

	stamps[index - 1] = nsnow();
	if(index == STAMPS) {
		struct itimerspec it = {0};
		timer_settime(timer, 0, &it, NULL);
	}
}

static int compare(const void *left, const void *right) {
	const unsigned long *l = left, *r = right;
	return *l - *r;
}

int main(void) {
	struct sigaction sa = {
		.sa_handler = handler,
	};
	sigaction(SIGALRM, &sa, NULL);

	struct sigevent sigev = {
		.sigev_notify = SIGEV_SIGNAL,
		.sigev_signo = SIGALRM,
	};
	timer_create(CLOCK_REALTIME, &sigev, &timer);

	for(struct timespec tv = {.tv_nsec = START_US * 1000}; tv.tv_nsec != 0; tv.tv_nsec -= STEP_NS) {
		index = -WARMUP;

		struct itimerspec it = {
			.it_value = tv,
			.it_interval = tv,
		};
		timer_settime(timer, 0, &it, NULL);

		while(index != STAMPS);

		unsigned long mean = 0;
		for(size_t each = 1; each != STAMPS; ++each)
			mean += stamps[each - 1] = stamps[each] - stamps[each - 1];
		mean /= STAMPS - 1;
		printf("%6.3f: mean %6.3f ", tv.tv_nsec / 1000.0, mean / 1000.0);

		qsort(stamps, STAMPS - 1, sizeof *stamps, compare);
		printf("median %6.3f ", stamps[STAMPS / 2 - 1] / 1000.0);
		printf("%d%% %6.3f\n", TAIL, stamps[(STAMPS - 1) * TAIL / 100] / 1000.0);
	}

	return 0;
}
