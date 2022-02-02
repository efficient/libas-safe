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
		.tv_sec = 1,
	};
	struct itimerval it = {
		.it_interval = tv,
		.it_value = tv,
	};
	setitimer(ITIMER_REAL, &it, NULL);

	while(true)
		fflush(stdout);

	return 0;
}
