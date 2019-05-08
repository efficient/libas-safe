#include <sys/time.h>
#include <errno.h>
#include <signal.h>
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

	for(;;)
		fflush(stdout);

	return 0;
}
