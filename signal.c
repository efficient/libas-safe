#include <sys/mman.h>
#include <errno.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

static size_t pagesz;
static uint8_t *stack;

static int enablepage(void *page) {
	return mprotect(page, pagesz, PROT_READ | PROT_WRITE);
}

static void growstack(int signum) {
	int erryes = errno;
	(void) signum;

	puts("Growing stack...");
	enablepage(stack);

	errno = erryes;
}

int main(void) {
	struct sigaction sa = {
		.sa_handler = growstack,
		.sa_flags = SA_ONSTACK,
	};
	sigaction(SIGSEGV, &sa, NULL);

	stack_t ss = {
		.ss_sp = (void *) ((uintptr_t) &ss - SIGSTKSZ),
		.ss_size = SIGSTKSZ,
	};
	sigaltstack(&ss, NULL);

	pagesz = sysconf(_SC_PAGE_SIZE);
	stack = mmap(NULL, pagesz * 2, PROT_NONE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	enablepage(stack + pagesz);

	puts("Installing custom stack...");
	__asm__ volatile("mov %0, %%rsp" :: "r" (stack + pagesz + 0x40));

	puts("Reinstalling original stack...");
	__asm__ volatile("mov %0, %%rsp" :: "r" (&ss));

	munmap(stack, pagesz * 2);
	return 0;
}
