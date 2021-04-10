#include <assert.h>
#include <dlfcn.h>
#include <iconv.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

typedef __typeof__(iconv_open) *iconv_open_t;
typedef __typeof__(iconv_close) *iconv_close_t;

static void iconv_worker(iconv_open_t iconv_open, iconv_close_t iconv_close) {
	puts("iconv_open()");

	iconv_t id = iconv_open("UTF-8", "ISO-8859-1");
	assert(id != (iconv_t) -1);

	puts("iconv_close()");

	iconv_close(id);
}

static void *libc = RTLD_NEXT;

int main(int argc, char **argv) {
	if(argc != 2) {
		printf("%s <true|false>\n", argv[0]);
		return 1;
	}

	bool mitigation = !strcmp(argv[1], "true");
	if(!mitigation && strcmp(argv[1], "false")) {
		fprintf(stderr, "%s: expected 'true' or 'false'\n", argv[1]);
		return 2;
	}

	iconv_worker(iconv_open, iconv_close);

	void *lc = dlmopen(LM_ID_NEWLM, "libc.so.6", RTLD_LAZY);
	assert(lc);

	iconv_open_t iconv_open = (iconv_open_t) (uintptr_t) dlsym(lc, "iconv_open");
	iconv_close_t iconv_close = (iconv_close_t) (uintptr_t) dlsym(lc, "iconv_close");
	assert(iconv_open);
	assert(iconv_close);

	if(mitigation)
		libc = lc;
	iconv_worker(iconv_open, iconv_close);

	dlclose(libc);
	return 0;
}

extern const char *__progname;

void _dl_signal_exception(int error, const char *const *module, const char *message) {
	void (*_dl_signal_exception)(int, const char *const *, const char *) =
		(void (*)(int, const char *const *, const char *)) (uintptr_t)
		dlsym(libc, "_dl_signal_exception");
	fprintf(stderr, "./%s: symbol lookup warning: %s: %s (code %d)\n", __progname, *module, message, error);
	_dl_signal_exception(error, module, message);
}
