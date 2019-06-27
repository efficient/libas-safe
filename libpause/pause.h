#ifndef LIBPAUSE_H_
#define LIBPAUSE_H_

#include <stdbool.h>

bool finishes_within(void (*)(void), unsigned long usec);

#endif
