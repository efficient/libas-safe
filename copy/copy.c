#include "copy.h"

#include "libgotcha/libgotcha_api.h"

#include <assert.h>
#include <threads.h>

static thread_local bool teardown;

static void callback(void) {
	if(teardown) {
		libgotcha_group_thread_set(LIBGOTCHA_GROUP_SHARED);
		assert(libgotcha_group_thread_get() == LIBGOTCHA_GROUP_SHARED);
	}
}

void init(void) {
	static libgotcha_group_t group = LIBGOTCHA_GROUP_SHARED;
	if(group == LIBGOTCHA_GROUP_SHARED)
		group = libgotcha_group_new();
	libgotcha_shared_hook(callback);

	assert(libgotcha_group_thread_get() == LIBGOTCHA_GROUP_SHARED);
	libgotcha_group_thread_set(group);
	assert(libgotcha_group_thread_get() == group);
}

static bool check(void) {
	return libgotcha_group_thread_get() != LIBGOTCHA_GROUP_SHARED;
}

bool (*checker(void))(void) {
	return check;
}

void cleanup(void) {
	teardown = true;
}
