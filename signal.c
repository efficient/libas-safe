#include <sys/time.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>

static void handler(int ignored) {
	(void) ignored;
	printf("In signal handler\n");
}

int main(void) {
	struct sigaction sa = {
		.sa_handler = handler,
	};
	sigaction(SIGALRM, &sa, NULL);

	struct timeval tv = {
		.tv_usec = 10000,
	};
	struct itimerval it = {
		.it_interval = tv,
		.it_value = tv,
	};
	setitimer(ITIMER_REAL, &it, NULL);

	char acter = '|';
	while(true) {
		printf("%c\b", acter);
		fflush(stdout);
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
	}

	return 0;
}
