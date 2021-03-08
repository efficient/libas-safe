#include "libinger/libinger.h"

#define pause libc_pause
#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#undef pause

#define US
#define MS * 1000
#define S * 1000000

#define TIMEOUT 3 S

static void listener(int *socketstatus) {
	struct sockaddr sa;
	socklen_t sz = sizeof sa;
	int fd = accept(*socketstatus, &sa, &sz);
	if(fd < 0) {
		perror("accept()");
		*socketstatus = -6;
		return;
	}

	char host[NI_MAXHOST];
	if(getnameinfo(&sa, sz, host, sizeof host, NULL, 0, NI_NAMEREQD))
		dprintf(fd, "getnameinfo(): %s\n", strerror(errno));
	else
		dprintf(fd, "HELLO %s\n", host);

	char wait;
	read(fd, &wait, sizeof wait);
	close(fd);
}

int main(int argc, char **argv) {
	if(argc < 2 || argc > 3) {
		printf("USAGE: %s <port> [ip]\n", argv[0]);
		return 1;
	}

	struct addrinfo *ai;
	int error = getaddrinfo(argc >= 3 ? argv[2] : NULL, argv[1], &(struct addrinfo) {
		.ai_family = argc >= 3 ? 0 : AF_INET,
		.ai_socktype = SOCK_STREAM,
	}, &ai);
	if(error) {
		fprintf(stderr, "getaddrinfo(): %s\n", gai_strerror(error));
		return 2;
	}

	int status = 0;
	int sock = socket(ai->ai_family, ai->ai_socktype, 0);
	if(sock < 0) {
		perror("socket()");
		status = 3;
		goto cleanup;
	}
	if(bind(sock, ai->ai_addr, ai->ai_addrlen)) {
		perror("bind()");
		status = 4;
		goto cleanup;
	}
	if(listen(sock, 0)) {
		perror("listen()");
		status = 5;
		goto cleanup;
	}

	int sockstat = sock;
	linger_t listening = launch((void (*)(void *)) listener, TIMEOUT, &sockstat);
	if(!listening.is_complete)
		cancel(&listening);
	if(sockstat < 0)
		status = -sockstat;

	cleanup:
	if(sock >= 0)
		close(sock);
	freeaddrinfo(ai);
	return status;
}
