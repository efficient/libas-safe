#include <sys/mman.h>
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>

static size_t mal;
//static size_t cal;
static size_t map;

int main(void) {
	printf("malloc()'d %lu B\n", mal);
	//printf("calloc()'d %lu B\n", cal);
	printf("mmap()'d   %lu B\n", map);
	return 0;
}

#define STATIC_REPLACE(ret, fun, ...) \
	ret fun(__VA_ARGS__) { \
		static ret (*fun)(__VA_ARGS__) = NULL; \
		if(!fun) \
			*(void **) &fun = dlsym(RTLD_NEXT, #fun); \

STATIC_REPLACE(void *, malloc, size_t size) //{
	mal += size;
	return malloc(size);
}

/*STATIC_REPLACE(void *, calloc, size_t nmemb, size_t size) //{
	cal += nmemb * size;
	return calloc(nmemb, size);
}
*/

STATIC_REPLACE(void *, mmap, void *addr, size_t length, int prot, int flags, int fd, off_t offset) //{
	map += length;
	return mmap(addr, length, prot, flags, fd, offset);
}
