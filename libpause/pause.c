#include "pause.h"

#include "libgotcha/libgotcha_api.h"

#include <assert.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include <ucontext.h>

#define SIGNAL SIGALRM

static ucontext_t checkpoint;
static libgotcha_group_t preempt;
static timer_t timer;

bool finishes_within(void (*fun)(void), unsigned long usec) {
	volatile bool finished = false;
	getcontext(&checkpoint);
	if(!finished) {
		libgotcha_group_thread_set(preempt);
		finished = true;

		struct itimerspec it = {
			.it_value = {
				.tv_sec = usec / 1000000,
				.tv_nsec = usec * 1000,
			},
		};
		if(timer_settime(timer, 0, &it, NULL))
			assert(false && "timer_settime()");
		fun();
	} else
		finished = false;

	static const struct itimerspec it = {0};
	if(timer_settime(timer, 0, &it, NULL))
		assert(false && "timer_settime()");
	libgotcha_group_thread_set(LIBGOTCHA_GROUP_SHARED);
	return finished;
}

static void handler(int no, siginfo_t *si, void *co) {
	(void) no;
	(void) si;

	if(libgotcha_group_thread_get() != LIBGOTCHA_GROUP_SHARED) {
		ucontext_t *uc = co;
		checkpoint.uc_mcontext.gregs[REG_CSGSFS] = uc->uc_mcontext.gregs[REG_CSGSFS];
		checkpoint.uc_mcontext.fpregs = uc->uc_mcontext.fpregs;
		memcpy(co, &checkpoint, sizeof checkpoint);
	}
}

static void deferred(void) {
	ucontext_t uc = {0};
	handler(0, NULL, &uc);
	setcontext(&uc);
}

static void __attribute__((constructor)) ctor(void) {
	struct sigevent sigev = {
		.sigev_notify = SIGEV_SIGNAL,
		.sigev_signo = SIGNAL,
	};
	if(timer_create(CLOCK_REALTIME, &sigev, &timer))
		assert(false && "timer_create()");

	struct sigaction sa = {
		.sa_mask = SA_SIGINFO,
		.sa_sigaction = handler,
	};
	if(sigaction(SIGNAL, &sa, NULL))
		assert(false && "sigaction()");

	if((preempt = libgotcha_group_new()) == LIBGOTCHA_GROUP_ERROR)
		assert(false && "libgotcha_group_new()");

	libgotcha_shared_hook(deferred);
}

static void __attribute__((destructor)) dtor(void) {
	if(timer_delete(timer))
		assert(false && "timer_delete()");

	if(signal(SIGNAL, SIG_DFL) == SIG_ERR)
		assert(false && "signal()");
}
