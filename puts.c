#include <pthread.h>

int puts(const char *text) {
	int _IO_puts(const char *);

	static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
	pthread_mutex_lock(&lock);

	int res = _IO_puts(text);

	pthread_mutex_unlock(&lock);
	return res;
}
