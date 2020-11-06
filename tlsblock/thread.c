#include "offset.h"

#include <assert.h>
#include <pthread.h>
#include <stdio.h>

static void *thread(void *set) {
	uintptr_t off = (uintptr_t) set;
	unsigned *ptr = (unsigned *) (addressof_tcb() - off);
	assert(*ptr == 0xdeadbeef);
	return NULL;
}

int main(void) {
	static thread_local unsigned sentinel = 0xdeadbeef;
	size_t off = offsetof_tls(&sentinel);
	++sentinel;

	pthread_t tid;
	pthread_create(&tid, NULL, thread, (void *) off);
	pthread_join(tid, NULL);

	printf("offset: %lu\n", off);
	return 0;
}
