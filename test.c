#include "hook.h"

#include <stdio.h>
#include <string.h>

static void hook(size_t index) {
	printf("hook(%lu)\n", index);
}

int main(void) {
	hook_got(hook);
	puts("HERE");
	return 0;
}
