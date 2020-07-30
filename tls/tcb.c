#include <asm/prctl.h>
#include <assert.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <threads.h>

static thread_local bool ean;

int arch_prctl(int, const void ***);
void **_dl_allocate_tls(void **);
void _dl_deallocate_tls(void **, bool);

#pragma weak timed_plugin
void timed_plugin(void);
#pragma weak threaded_plugin
void threaded_plugin(void);

static void *timed(void) {
	void **lpfcb;
	const void **tcb;
	lpfcb = _dl_allocate_tls(NULL);
	*lpfcb = lpfcb;
	arch_prctl(ARCH_GET_FS, &tcb);
	arch_prctl(ARCH_SET_FS, (const void ***) lpfcb);

	assert(!ean);
	ean = true;
	assert(ean);
	if(timed_plugin)
		timed_plugin();

	arch_prctl(ARCH_SET_FS, (const void ***) tcb);
	return lpfcb;
}

static void *threaded(void *lpfcb) {
	assert(!ean);

	const void **tcb;
	arch_prctl(ARCH_GET_FS, &tcb);
	arch_prctl(ARCH_SET_FS, (const void ***) lpfcb);

	assert(ean);
	if(threaded_plugin)
		threaded_plugin();

	arch_prctl(ARCH_SET_FS, (const void ***) tcb);
	_dl_deallocate_tls(lpfcb, true);
	return NULL;
}

int main(void) {
	void *state = timed();
	pthread_t nation;
	pthread_create(&nation, NULL, threaded, state);
	pthread_join(nation, NULL);
	return 0;
}
