#include "copy.h"

#include <assert.h>

int main(void) {
	assert(!checker()());
	init();
	assert(checker()());
	cleanup();
	assert(!checker()());
}
