#include <stdio.h>
#include <time.h>

time_t time(time_t *tloc) {
	puts("time() from libtime");
	return time(tloc);
}
