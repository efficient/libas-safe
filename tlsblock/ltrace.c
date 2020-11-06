#include "interpose.h"

#include <stdint.h>
#include <stdio.h>

static thread_local unsigned nesting;

static inline void indent(void) {
	for(unsigned count = 0; count < nesting; ++count)
		putc('\t', stderr);
}

INTERPOSE(void *, _dl_allocate_tls, void *arg) //{
	indent();
	fprintf(stderr, "_dl_allocate_tls(%#lx)\n", (uintptr_t) arg);
	++nesting;

	void *res = _dl_allocate_tls(arg);

	--nesting;
	indent();
	fprintf(stderr, "->%#lx\n", (uintptr_t) res);
	return res;
}

INTERPOSE(void, _dl_deallocate_tls, void *arg, bool dtv) //{
	indent();
	fprintf(stderr, "_dl_deallocate_tls(%#lx, %d)\n", (uintptr_t) arg, dtv);
	++nesting;
	_dl_deallocate_tls(arg, dtv);
	--nesting;
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

INTERPOSE(void, free, void *arg) //{
	indent();
	fprintf(stderr, "free(%#lx)\n", (uintptr_t) arg);
	++nesting;
	free(arg);
	--nesting;
}
