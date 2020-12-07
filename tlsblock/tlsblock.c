#include "interpose.h"

#include <sys/mman.h>
#include <sys/types.h>
#include <assert.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <threads.h>
#include <unistd.h>

// NB: This library assumes that no further shared libraries will be dynamically loaded after it
//     is initialized.  Should this condition be violated, Bad Things might happen, including:
//     * A realloc() call might move the shared DTV, leaving dangling pointers!
//     * The free() interposition overflow the stack due to a __tls_get_addr() bootstrapping cycle.
//     * Etc., etc.
//     In order to support this use case, we would need to support (lazily?) individualized DTVs.

#define FILENAME "/tmp/libtlsblock-XXXXXX"

int pthread_join(pthread_t, void **);

// Thread-specific state machine.
//
// Allocating fresh thread stack:
// pthread_create() -> __mmap() -> _dl_allocate_tls()
//   (SPAWNING)     (INITIALIZING)
//
// Reusing existing thread stack from pool:
// pthread_create() -> free().../memset() -> _dl_allocate_tls_init() -> _dl_allocate_tls()
//   (SPAWNING)          [nop]    [nop]          (INITIALIZING)
//
// Allocating stackless thread-control block:
// _dl_allocate_tls(NULL) -> malloc()
//    (ALLOCATING)
enum state {
	NONE = 0,
	SPAWNING,
	INITIALIZING,
	ALLOCATING,
};

static thread_local enum state state;
static thread_local bool recording;

static int template_fd;
static char template_filename[] = FILENAME;
static size_t template_size;
static size_t template_off;
static size_t template_tlssz;

static const void *alloc_dealloc;

static size_t pagesize(void) {
	static size_t pagesize;
	if(!pagesize)
		pagesize = sysconf(_SC_PAGE_SIZE);
	return pagesize;
}

static inline size_t pagemask(void) {
	return pagesize() - 1;
}

static inline size_t stackless_filoff(void) {
	return (template_off - template_tlssz) & ~pagemask();
}

static inline size_t stackless_memoff(void) {
	return template_tlssz + ((template_off - template_tlssz) & pagemask());
}

INTERPOSE(int, pthread_create, void *tid, void *attr, void *(*start)(void *), void *arg) //{
	assert(state == NONE);
	if(template_fd || recording)
		state = SPAWNING;

	int res = pthread_create(tid, attr, start, arg);

	assert(state == NONE);
	return res;
}

INTERPOSE(void *, __mmap, void *addr, size_t len, int prot, int flags, int fd, off_t offset) //{
	if(recording) {
		void *res = __mmap(addr, len, prot, flags, fd, offset);
		assert(!prot);
		template_off = (uintptr_t) res;
		template_size = len;
		state = INITIALIZING;
		return res;
	}

	if(state != SPAWNING || !template_fd)
		return __mmap(addr, len, prot, flags, fd, offset);

	if(len == template_size) {
		assert(!addr);
		flags = (flags & ~MAP_ANONYMOUS) | MAP_PRIVATE;
		fd = template_fd;
		offset = 0;
		state = INITIALIZING;
	} else {
		fputs("libtlsblock warning: Inconsistent stack allocation sizes\n", stderr);
		state = NONE;
	}

	return __mmap(addr, len, prot, flags, fd, offset);
}

INTERPOSE(void *, _dl_allocate_tls, void *arg) //{
	if(recording) {
		if(state == NONE)
			state = ALLOCATING;

		void *res = _dl_allocate_tls(arg);
		if(state == INITIALIZING) {
			state = NONE;

			const uint8_t *reference_addr = (uint8_t *) template_off;
			template_off = (uintptr_t) res - template_off;
			alloc_dealloc = res;
			assert(!((uintptr_t *) ((uintptr_t) reference_addr + template_size))[-1]);

			int fd = mkstemp(template_filename);
			if(fd < 0) {
				fputs("libtlsblock error: Failed to create backing temporary file\n", stderr);
				abort();
			}
			ftruncate(fd, pagesize());
			lseek(fd, 0, SEEK_END);

			const uint8_t *addr = reference_addr + pagesize();
			while(addr != reference_addr + template_size)
				addr += write(fd, addr, reference_addr + template_size - addr);
			fsync(fd);

			recording = false;
			template_fd = fd;
		} else {
			assert(state == NONE);
			assert(template_tlssz);
			template_tlssz = (uintptr_t) res - template_tlssz;
		}
		return res;
	}

	if((arg && state != INITIALIZING) || !template_fd)
		return _dl_allocate_tls(arg);

	if(arg) {
		state = NONE;

		// Reinitialize everything but the guard page and the control block
		uintptr_t tcb = (uintptr_t) arg;
		mmap((void *) (tcb - template_off + pagesize()), template_size - 2 * pagesize(), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_FIXED, template_fd, pagesize());
		pread(template_fd, (void *) (tcb & ~pagemask()), template_off & pagemask(), template_off & ~pagemask());
		return arg;
	} else {
		size_t offset = stackless_filoff();
		uint8_t *res = mmap(NULL, template_size - offset, PROT_READ | PROT_WRITE, MAP_PRIVATE, template_fd, offset);
		const uint8_t **free = (const uint8_t **) (res + template_size - offset - sizeof free);
		*free = res;
		return res + stackless_memoff();
	}
}

INTERPOSE(void *, malloc, size_t arg) //{
	if(state != ALLOCATING)
		return malloc(arg);

	assert(recording);
	assert(!template_tlssz);

	void *res = malloc(arg);
	template_tlssz = (uintptr_t) res;
	state = NONE;
	return res;
}

INTERPOSE(void *, _dl_allocate_tls_init, void *arg) //{
	if(state != SPAWNING || !template_fd)
		return _dl_allocate_tls_init(arg);

	assert(arg);
	state = INITIALIZING;
	return _dl_allocate_tls(arg);
}

INTERPOSE(void, _dl_deallocate_tls, void *arg, bool mallocated) //{
	// We assume that the stack used to record the template is allocated before and deallocated
	// after all block-allocated stacks.  Note that this is a production stack owned by the pool
	// in libpthread!
	if(!alloc_dealloc) {
		_dl_deallocate_tls(arg, mallocated);
		return;
	}

	if(arg == alloc_dealloc) {
		assert(!mallocated);
		_dl_deallocate_tls(arg, false);
		alloc_dealloc = NULL;
	} else if(mallocated) {
		// We assume that the non-TLS part of the TCB is smaller than a page, and therefore
		// look for the free location pointer at the end of the same page.
		void **free = (void **) (((uintptr_t) arg & ~pagemask()) + pagesize() - sizeof free);
		uint8_t *addr = arg;
		size_t size = template_size;
		if(*free) {
			addr = *free;
			size -= stackless_filoff();
		} else
			addr -= template_off;
		munmap(addr, size);
	} // else noop, since the DTV is shared
}

INTERPOSE(void, free, void *arg) //{
	if(state != SPAWNING)
		free(arg);
}

INTERPOSE(void *, memset, void *dest, int src, size_t cnt) //{
	if(state != SPAWNING)
		return memset(dest, src, cnt);

	return dest;
}

static void *dummy(void *ign) {
	return ign;
}

static void tlsblock_init(void) {
	recording = true;

	_dl_deallocate_tls(_dl_allocate_tls(NULL), true);
	assert(template_tlssz);

	pthread_t tid;
	pthread_create(&tid, NULL, dummy, NULL);
	pthread_join(tid, NULL);
	assert(template_size);
	assert(template_off);
	assert(template_fd);

	assert(!recording);
}

static void tlsblock_cleanup(void) {
	if(!template_fd)
		return;

	close(atomic_exchange(&template_fd, 0));
	unlink(template_filename);
	// Leave sizes and offset as they may still be needed for deallocation!
}

static void __attribute__((constructor)) ctor(void) {
	tlsblock_init();
}

static void __attribute__((destructor)) dtor(void) {
	tlsblock_cleanup();
}
