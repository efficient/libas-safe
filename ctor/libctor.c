#include <stdint.h>
#include <stdio.h>

void nop(void);

int main(void) {
	printf(" nop:     %#lx\n", (uintptr_t) nop);
	nop();
	return 0;
}
