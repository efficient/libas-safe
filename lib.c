#include "lib.h"

static bool ean;

bool get(void) {
	return ean;
}

void set(bool val) {
	ean = val;
}
