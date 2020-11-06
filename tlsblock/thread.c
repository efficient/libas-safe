#include <pthread.h>

static void *thread(void *ign) {
	(void) ign;
	return NULL;
}

int main(void) {
	pthread_t tid;
	pthread_create(&tid, NULL, thread, NULL);
	pthread_join(tid, NULL);
	return 0;
}
