#ifndef DEREF_H_
#define DEREF_H_

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

static inline const uint8_t **r(const char *reg) {
	extern const uint8_t *p[];
	extern const uint8_t *x[];
	extern const uint8_t *i[];
	extern const uint8_t *nul[];
	extern const uint8_t *one[];

	assert(reg);
	assert(reg[0] == '%');
	assert(reg[1] == 'r');
	switch(reg[3]) {
	case 'p':
		return p;
	case 'x':
		return x + (reg[2] - 'a');
	case 'i':
		return i + (reg[2] == 's');
	case '\0':
		return nul + (reg[2] - '8');
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
		return one + (reg[1] - '0');
	default:
		assert(false);
	}
}

#endif
