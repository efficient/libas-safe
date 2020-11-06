#ifndef OFFSET_H_
#define OFFSET_H_

#include <asm/prctl.h>
#include <stdint.h>
#include <threads.h>

static inline uintptr_t addressof_tcb(void) {
	int arch_prctl(int, uintptr_t *);

	uintptr_t tcb;
	arch_prctl(ARCH_GET_FS, &tcb);
	return tcb;
}

static inline size_t offsetof_tls(void *tls) {
	return addressof_tcb() - (uintptr_t) tls;
}

#endif
