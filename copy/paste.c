#include "copy.h"

#include <assert.h>

// FIXME: It is presently impossible to write cleanup() correctly: any attempt to change the
//        namespace from within a statically-linked client library is doomed to be reversed on the
//        return back out of said library.  The return trampoline logic somehow needs to know when
//        the namespace has been changed---even to the indiscernible identifier 0---since entering.
int main(void) {
	assert(!checker()());
	init();
	assert(checker()());
	cleanup();
	assert(!checker()());
}
