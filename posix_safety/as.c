#include <sys/time.h>
#include <assert.h>
#ifdef _GNU_SOURCE
#      include <dlfcn.h>
#endif
#include <errno.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>

static void handler(int ign) {
	(void) ign;

	int erryes = errno;

	static char acter = '|';
	printf("%c\b", acter);
	switch(acter) {
	case '|':
		acter = '/';
		break;
	case '/':
		acter = '-';
		break;
	case '-':
		acter = '\\';
		break;
	case '\\':
		acter = '|';
		break;
	}

	errno = erryes;
}

int main(void) {
	struct sigaction sa = {
		.sa_handler = handler,
	};
	sigaction(SIGALRM, &sa, NULL);

	struct timeval tv = {
		.tv_usec = 500000,
	};
	struct itimerval it = {
		.it_interval = tv,
		.it_value = tv,
	};
	setitimer(ITIMER_REAL, &it, NULL);

#ifdef _GNU_SOURCE
	void *l = dlmopen(LM_ID_NEWLM, "libc.so.6", RTLD_LAZY);
	void (*fflush)(FILE *) = (void (*)(FILE *)) (uintptr_t) dlsym(l, "fflush");
#endif
	for(;;)
		fflush(stdout);

	return 0;
}
