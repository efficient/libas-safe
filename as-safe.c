#include "libgotcha_api.h"
#include "libgotcha_repl.h"

#include <assert.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <threads.h>

struct handler {
	void (*handler)(int, siginfo_t *, void *);
	sigset_t mask;
};

struct pending {
	siginfo_t info;
	sigset_t mask;
};

static struct handler *handlers;
static int (*mainfunc)(int, char **, char **);
static libgotcha_group_t nonsignal;
static thread_local struct pending pending;
static bool verbose;

static int as_safe(int, char **, char **);
#pragma weak libassafe_libc_start_main = __libc_start_main
int __libc_start_main(int (*main)(int, char **, char **), int argc, char **argv, int (*init)(int, char **, char **), void (*fini)(void), void (*rtld_fini)(void), void *stack_end) {
	mainfunc = main;
	return __libc_start_main(as_safe, argc, argv, init, fini, rtld_fini, stack_end);
}

static void restorer(void);
static int as_safe(int argc, char **argv, char **envp) {
	if(getenv("LIBASSAFE_VERBOSE"))
		verbose = true;

	if(verbose) fputs("LIBAS-SAFE: as_safe() initializating...\n", stderr);
	handlers = calloc(SIGRTMAX, sizeof *handlers);
	libgotcha_group_thread_set(nonsignal = libgotcha_group_new());
	libgotcha_shared_hook(restorer);
	return mainfunc(argc, argv, envp);
}

static void stub(int, siginfo_t *, void *);
int sigaction(int signum, const struct sigaction *act, struct sigaction *oldact) {
	assert(signum >= 0);
	assert(signum < SIGRTMAX);

	struct sigaction sa;
	struct handler backup;
	if(act) {
		if(verbose) fprintf(
			stderr,
			"LIBAS-SAFE: sigaction() installing new handler for signal %d\n",
			signum);

		struct handler *record = handlers + signum;
		memcpy(&backup, record, sizeof backup);
		memcpy(&sa, act, sizeof sa);
		record->handler = sa.sa_sigaction;
		sa.sa_sigaction = stub;
		record->mask = sa.sa_mask;
		libgotcha_sigfillset(&sa.sa_mask);

		sa.sa_flags |= SA_SIGINFO;
		if(sa.sa_flags & SA_NODEFER) {
			sigdelset(&record->mask, signum);
			sa.sa_flags &= ~SA_NODEFER;
		} else
			libgotcha_sigaddset(&record->mask, signum);

		act = &sa;
	}

	int status = libgotcha_sigaction(signum, act, oldact);
	if(status)
		memcpy(handlers + signum, &backup, sizeof backup);
	if(oldact && oldact->sa_sigaction == stub)
		oldact->sa_sigaction = handlers[signum].handler;
	return status;
}

static void stub(int no, siginfo_t *si, void *co) {
	libgotcha_group_t group = libgotcha_group_thread_get();
	siginfo_t extra = {
		.si_signo = 0,
	};
	if(pending.info.si_signo) {
		assert(pending.info.si_signo == no);

		ucontext_t *uc = co;
		memcpy(&extra, &pending.info, sizeof extra);
		memcpy(&uc->uc_sigmask, &pending.mask, sizeof uc->uc_sigmask);
		pending.info.si_signo = 0;
	} else if(group == LIBGOTCHA_GROUP_SHARED) {
		if(verbose) fprintf(
			stderr,
			"LIBAS-SAFE: stub() deferring handling of signal %d to after shared code\n",
			no);

		ucontext_t *uc = co;
		memcpy(&pending.info, si, sizeof pending.info);
		memcpy(&pending.mask, &uc->uc_sigmask, sizeof pending.mask);
		libgotcha_sigfillset(&uc->uc_sigmask);
		return;
	}

	struct handler *record = handlers + no;
	libgotcha_pthread_sigmask(SIG_SETMASK, &record->mask, NULL);
	libgotcha_group_thread_set(LIBGOTCHA_GROUP_SHARED);
	if(extra.si_signo)
		record->handler(no, &extra, co);
	record->handler(no, si, co);
	libgotcha_group_thread_set(group);
}

static void restorer(void) {
	if(pending.info.si_signo) {
		sigset_t full;
		libgotcha_sigfillset(&full);
		sigdelset(&full, pending.info.si_signo);
		sigsuspend(&full);
	}
}
