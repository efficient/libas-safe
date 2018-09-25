#include "lib.h"

#include <assert.h>
#include <dlfcn.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

extern void *_GLOBAL_OFFSET_TABLE_[];
extern void *data_start[];

int main(void) {
	const size_t GOT_LEN = data_start - _GLOBAL_OFFSET_TABLE_;
	printf("GOT entries: %lu\n", GOT_LEN);

	assert(!get());
	set(true);
	assert(get());

	void *handle = NULL;
	for(unsigned entry = 2; entry < GOT_LEN; ++entry) {
		void *addr = _GLOBAL_OFFSET_TABLE_[entry];

		Dl_info symb;
		int err = dladdr(addr, &symb);
		if(err)
			printf("GOT[%u] = %#lx (%s:%s)\n",
				entry,
				(uintptr_t) addr,
				symb.dli_fname,
				symb.dli_sname);
		else
			fprintf(stderr, "%s\n", dlerror());

		if(strstr(symb.dli_fname, "/liblib.so\0")) {
			if(!handle)
				handle = dlmopen(LM_ID_NEWLM, symb.dli_fname, RTLD_LAZY);
			assert(handle);

			_GLOBAL_OFFSET_TABLE_[entry] = dlsym(handle, symb.dli_sname);
		}
	}

	assert(!get());
	set(true);
	assert(get());

	return 0;
}
