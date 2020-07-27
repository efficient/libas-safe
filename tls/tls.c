#include "libinger.h"

#include <assert.h>
#include <pthread.h>
#include <threads.h>

static void timed(void *args) {
	static thread_local volatile bool ean;
	(void) args;

	assert(!ean);
	ean = true;
	assert(ean);
	pause();
	assert(ean);
}

static void *threaded(void *timed) {
	linger_t *state = timed;
	resume(state, UINT64_MAX);
	return NULL;
}

int main(void) {
	linger_t state = launch(timed, UINT64_MAX, NULL);
	pthread_t nation;
	pthread_create(&nation, NULL, threaded, &state);
	pthread_join(nation, NULL);
	return 0;
}
