#include "nest.h"

#include "libgotcha/libgotcha_api.h"
#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <threads.h>

static thread_local bool calledback;

static void callback(void) {
	calledback = true;
}

void nest(void) {
	static libgotcha_group_t group = LIBGOTCHA_GROUP_SHARED;
	if(group == LIBGOTCHA_GROUP_SHARED) {
		libgotcha_shared_hook(callback);
		group = libgotcha_group_new();
	} else if(!calledback) {
		static bool once = true;
		assert(once);
		once = false;
	} else
		calledback = false;

	libgotcha_group_thread_set(group);
	free(NULL);
	assert(calledback);
	calledback = false;
}
