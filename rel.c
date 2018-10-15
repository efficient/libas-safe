#include <link.h>

extern ElfW(Addr) _GLOBAL_OFFSET_TABLE_[];

void fixup(void *, ElfW(Dyn) *, ElfW(Addr) *);

int main(void) {
	fixup(main, _DYNAMIC, _GLOBAL_OFFSET_TABLE_);
	return 0;
}
