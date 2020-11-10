#include "offset.h"

#include <assert.h>
#include <pthread.h>
#include <stdio.h>

void *ltrace_dl_allocate_tls_ret;

static void *thread(void *set) {
	uintptr_t off = (uintptr_t) set;
	uintptr_t tcb = addressof_tcb();
	unsigned *tls = (unsigned *) (tcb - off);
	if(ltrace_dl_allocate_tls_ret)
		assert((uintptr_t) ltrace_dl_allocate_tls_ret == tcb);
	assert(*tls == 0xdeadbeef);
	return NULL;
}

int main(void) {
	static thread_local unsigned sentinel = 0xdeadbeef;
	size_t off = offsetof_tls(&sentinel);
	++sentinel;

	pthread_t tid;
	for(unsigned count = 0; count < 2; ++count) {
		pthread_create(&tid, NULL, thread, (void *) off);
		pthread_join(tid, NULL);
	}

	printf("offset: %lu\n", off);
	return 0;
}
