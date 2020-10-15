#include "tlsblock.h"

#include <sys/mman.h>
#include <assert.h>
#include <dlfcn.h>
#include <stdatomic.h>
#include <stdint.h>
#include <stdlib.h>
#include <threads.h>
#include <unistd.h>

#define BACKING_FILENAME "/tmp/libtlsblock-XXXXXX"

#define INTERPOSE(ret, fun, ...) \
	ret fun(__VA_ARGS__) { \
		static ret (*fun)(__VA_ARGS__) = NULL; \
		if(!fun) \
			*(void **) &fun = dlsym(RTLD_NEXT, #fun); \

static bool template;
static int template_fd;
static size_t template_size;
static char template_filename[] = BACKING_FILENAME;

static thread_local bool recording;
static thread_local const uint8_t *recording_addr;
static thread_local size_t recording_size;

void _dl_deallocate_tls(void *, bool);

INTERPOSE(void *, _dl_allocate_tls, void *existing) //{
	if(template)
		return mmap(NULL, template_size, PROT_READ | PROT_WRITE, MAP_PRIVATE, template_fd, 0);

	if(!existing)
		recording = true;
	return _dl_allocate_tls(existing);
}

INTERPOSE(void, _dl_deallocate_tls, void *addr, bool full) //{
	if(template)
		munmap(addr, template_size);
	else
		_dl_deallocate_tls(addr, full);
}

INTERPOSE(void *, malloc, size_t arg) //{
	void *res = malloc(arg);
	if(recording) {
		recording_addr = res;
		recording_size = arg;
		recording = false;
	}
	return res;
}

bool tlsblock_init(void) {
	static bool idempotent;
	if(atomic_exchange(&idempotent, true))
		return true;

	int fd = mkstemp(template_filename);
	if(fd < 0)
		return false;

	void *handle = _dl_allocate_tls(NULL);
	assert(handle);
	assert(recording_addr);
	assert(recording_size);

	const uint8_t *addr = recording_addr;
	while(addr != recording_addr + recording_size)
		addr += write(fd, addr, recording_addr + recording_size - addr);

	_dl_deallocate_tls(handle, true);
	handle = NULL;

	template_fd = fd;
	template_size = recording_size;
	__asm__("" ::: "memory");
	template = true;

	return true;
}

void tlsblock_cleanup(void) {
	if(atomic_exchange(&template, false)) {
		close(template_fd);
		template_fd = 0;
		unlink(template_filename);
	}
}

static void __attribute__((constructor)) ctor(void) {
	bool ean = tlsblock_init();
	assert(ean);
}

static void __attribute__((destructor)) dtor(void) {
	tlsblock_cleanup();
}
