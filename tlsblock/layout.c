#include "offset.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>

void *_dl_allocate_tls(void *);
void _dl_deallocate_tls(void *, bool);

int main(void) {
	static thread_local unsigned sentinel = 0xdeadbeef;
	size_t off = offsetof_tls(&sentinel);
	void *tcb = _dl_allocate_tls(NULL);
	unsigned *ptr = (unsigned *) ((uintptr_t) tcb - off);
	++sentinel;
	assert(*ptr == 0xdeadbeef);
	_dl_deallocate_tls(tcb, true);

	printf("offset: %lu\n", off);
	return 0;
}
