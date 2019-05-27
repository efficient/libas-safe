#include <sys/ioctl.h>
#include <sys/time.h>
#include <assert.h>
#include <errno.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define FRACTION 75

#define CSI "\x1b["

static void handler(int ign) {
	(void) ign;

	int erryes = errno;

	static char acter = '|';
	printf(CSI "s" CSI "1;1H%c" CSI "u", acter);
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

	errno = erryes;
}

int usleep(useconds_t usec) {
	struct timeval start, now, end = {
		.tv_sec  = usec / 1000000,
		.tv_usec = usec % 1000000,
	};
	gettimeofday(&start, NULL);
	timeradd(&start, &end, &end);
	while(!gettimeofday(&now, NULL) && timercmp(&now, &end, <));
}

int main(void) {
	struct sigaction sa = {
		.sa_handler = handler,
	};
	sigaction(SIGALRM, &sa, NULL);

	struct timeval tv = {
		.tv_usec = 100000,
	};
	struct itimerval it = {
		.it_interval = tv,
		.it_value = tv,
	};
	setitimer(ITIMER_REAL, &it, NULL);

	struct winsize ws;
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws);
	ws.ws_row -= 2;
	srand(time(NULL));

	for(int delay = 10000;; delay && (delay -= 10)) {
		printf(CSI "s" CSI "%d;%dH." CSI "u", rand() % ws.ws_row + 2, rand() % ws.ws_col + 1);
		fflush(stdout);
		usleep(delay);
	}

	return 0;
}
