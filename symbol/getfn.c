#include "libgotcha/libgotcha_api.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

static volatile bool expected;

static void callback(void) {
	assert(expected);
	expected = false;
}

int main(void) {
	libgotcha_group_t group = libgotcha_group_new();
	void (*free0)(void *) = (void (*)(void *)) (uintptr_t)
		libgotcha_group_symbol(LIBGOTCHA_GROUP_SHARED, "free");
	void (*free1)(void *) = (void (*)(void *)) (uintptr_t)
		libgotcha_group_symbol(group, "free");
	assert(free0 != free);
	assert(free0 != free1);
	libgotcha_shared_hook(callback);

	static void *volatile nullptr = NULL;
	free(nullptr);
	free0(nullptr);
	free1(nullptr);

	libgotcha_group_thread_set(group);
	expected = true;
	free(nullptr);
	if(expected)
		abort();
	free0(nullptr);
	free1(nullptr);

	expected = true;
	libgotcha_group_thread_set(LIBGOTCHA_GROUP_SHARED);
	return 0;
}
