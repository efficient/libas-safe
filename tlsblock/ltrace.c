#include "interpose.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define BOOTSTRAP_BYTES 32

static thread_local unsigned nesting;
static thread_local size_t callocnm;
static thread_local size_t callocsz;

static inline void indent(void) {
	for(unsigned count = 0; count < nesting; ++count)
		putc('\t', stderr);
}

static inline void dumpdtv(const void *tcb) {
	const uintptr_t (*dtv)[2] = ((const uintptr_t (**)[2]) tcb)[1];
	if(!dtv)
		return;

	static size_t dtvnm, dtvsz;
	if(!dtvnm || !dtvsz) {
		dtvnm = callocnm;
		dtvsz = callocsz;
	}
	assert(dtvnm);
	assert(dtvsz);
	assert(dtvsz == sizeof *dtv);
	for(ssize_t idx = -1; idx < (ssize_t) dtvnm - 1; ++idx) {
		indent();
		fprintf(stderr, "dtv[%ld]<-(%#lx, %#lx)\n", idx, dtv[idx][0], dtv[idx][1]);
		if(dtv[idx][0]) {
			indent();
			fprintf(stderr, "       offset %#lx\n", (uintptr_t) tcb - dtv[idx][0]);
		}
	}
}

INTERPOSE(int, pthread_create, uintptr_t tid, uintptr_t attr, void *(*start)(void *), void *arg) //{
	indent();
	fprintf(stderr, "pthread_create(%#lx, %#lx, %#lx, %#lx)\n", tid, attr, (uintptr_t) start, (uintptr_t) arg);
	++nesting;

	int res = pthread_create(tid, attr, start, arg);

	--nesting;
	indent();
	fprintf(stderr, "->%d\n", res);
	return res;
}

INTERPOSE(void *, _dl_allocate_tls, void *arg) //{
	indent();
	fprintf(stderr, "_dl_allocate_tls(%#lx)\n", (uintptr_t) arg);
	++nesting;

	callocnm = 0;
	callocsz = 0;

	#pragma weak ltrace_dl_allocate_tls_arg
	#pragma weak ltrace_dl_allocate_tls_ret
	extern void *ltrace_dl_allocate_tls_arg;
	extern void *ltrace_dl_allocate_tls_ret;
	void *res = _dl_allocate_tls(arg);
	if(&ltrace_dl_allocate_tls_arg)
		ltrace_dl_allocate_tls_arg = arg;
	if(&ltrace_dl_allocate_tls_ret)
		ltrace_dl_allocate_tls_ret = res;

	dumpdtv(res);
	callocnm = 0;
	callocsz = 0;

	--nesting;
	indent();
	fprintf(stderr, "->%#lx\n", (uintptr_t) res);
	return res;
}

INTERPOSE(void *, _dl_allocate_tls_init, void *arg) //{
	indent();
	fprintf(stderr, "_dl_allocate_tls_init(%#lx)\n", (uintptr_t) arg);
	++nesting;

	void *res = _dl_allocate_tls_init(arg);

	--nesting;
	indent();
	fprintf(stderr, "->%#lx\n", (uintptr_t) res);
	return res;
}

INTERPOSE(void, _dl_deallocate_tls, void *arg, bool full) //{
	indent();
	fprintf(stderr, "_dl_deallocate_tls(%#lx, %d)\n", (uintptr_t) arg, full);
	++nesting;
	_dl_deallocate_tls(arg, full);
	--nesting;
}

INTERPOSE(void, _dl_get_tls_static_info, size_t *size, size_t *align) //{
	indent();
	fprintf(stderr, "_dl_get_static_info(%#lx, %#lx)\n", (uintptr_t) size, (uintptr_t) align);
	++nesting;
	_dl_get_tls_static_info(size, align);
	--nesting;
	indent();
	fprintf(stderr, "->(%lu, %lu)\n", *size, *align);
}

INTERPOSE(void *, malloc, size_t arg) //{
	indent();
	fprintf(stderr, "malloc(%lu)\n", arg);
	++nesting;

	void *res = malloc(arg);

	--nesting;
	indent();
	fprintf(stderr, "->%#lx\n", (uintptr_t) res);
	return res;
}

INTERPOSE(void *, calloc, size_t nmemb, size_t size) //{
	if(bootstrapping) {
		static thread_local uint8_t buf[BOOTSTRAP_BYTES] = {0};
		if(nmemb * size != sizeof buf) {
			fputs("calloc() bootstrap size mismatch\n", stderr);
			abort();
		}
		return buf;
	}

	indent();
	fprintf(stderr, "calloc(%lu, %lu)\n", nmemb, size);
	++nesting;

	void *res = calloc(nmemb, size);
	callocnm = nmemb;
	callocsz = size;

	--nesting;
	indent();
	fprintf(stderr, "->%#lx\n", (uintptr_t) res);
	return res;
}

INTERPOSE(void *, memset, void *dest, int src, size_t cnt) //{
	indent();
	fprintf(stderr, "memset(%#lx, %d, %lu)\n", (uintptr_t) dest, src, cnt);
	++nesting;

	void *res = memset(dest ,src, cnt);

	--nesting;
	indent();
	fprintf(stderr, "->%#lx\n", (uintptr_t) res);
	return res;
}

INTERPOSE(void, free, void *arg) //{
	indent();
	fprintf(stderr, "free(%#lx)\n", (uintptr_t) arg);
	++nesting;
	free(arg);
	--nesting;
}

#define INTERPOSE_MMAP(sym) \
	INTERPOSE(void *, sym, void *addr, size_t length, int prot, int flags, int fd, off_t offset) \
		indent(); \
		fprintf(stderr, #sym "(%#lx, %lu, %#x, %#x, %d, %ld)\n", (uintptr_t) addr, length, prot, flags, fd, offset); \
		++nesting; \
		\
		void *res = sym(addr, length, prot, flags, fd, offset); \
		\
		--nesting; \
		indent(); \
		fprintf(stderr, "->%#lx\n", (uintptr_t) res); \
		return res; \
	} \

#define INTERPOSE_MPROTECT(sym) \
	INTERPOSE(int, sym, void *addr, size_t len, int prot) \
		indent(); \
		fprintf(stderr, #sym "(%#lx, %lu, %#x)\n", (uintptr_t) addr, len, prot); \
		++nesting; \
		\
		int res = sym(addr, len, prot); \
		\
		--nesting; \
		indent(); \
		fprintf(stderr, "->%d\n", res); \
		return res; \
	} \

#define INTERPOSE_MUNMAP(sym) \
	INTERPOSE(int, sym, void *addr, size_t length) \
		indent(); \
		fprintf(stderr, #sym "(%#lx, %lu)\n", (uintptr_t) addr, length); \
		++nesting; \
		\
		int res = sym(addr, length); \
		\
		--nesting; \
		indent(); \
		fprintf(stderr, "->%d\n", res); \
		return res; \
	} \

INTERPOSE_MMAP(mmap)
INTERPOSE_MMAP(__mmap)
INTERPOSE_MPROTECT(mprotect)
INTERPOSE_MPROTECT(__mprotect)
INTERPOSE_MUNMAP(munmap)
INTERPOSE_MUNMAP(__munmap)
