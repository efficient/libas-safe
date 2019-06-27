#include "pause.h"

#include <assert.h>
#include <stdlib.h>
#include <unistd.h>

static void sleep_20us(void) {
	usleep(20);
}

static void halt(void) {
	for(;;);
}

static void allocate(void) {
	for(;;)
		free(malloc(rand() % (1024 * 1024)));
}

int main(void) {
	assert(!finishes_within(sleep_20us, 10));
	assert( finishes_within(sleep_20us, 100));
	assert(!finishes_within(halt,       10));
	assert(!finishes_within(allocate,   10));

	return 0;
}
