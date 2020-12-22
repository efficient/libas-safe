#ifndef INTERPOSE_H_
#define INTERPOSE_H_

#include <dlfcn.h>
#include <stdbool.h>
#include <threads.h>

#define INTERPOSE(ret, fun, ...) \
	ret fun(__VA_ARGS__) { \
		static ret (*fun)(__VA_ARGS__) = NULL; \
		static thread_local volatile bool bootstrapping = false; \
		if(!fun && !bootstrapping) { \
			bootstrapping = true; \
			*(void **) &fun = dlsym(RTLD_NEXT, #fun); \
			bootstrapping = false; \
		} \

#endif
