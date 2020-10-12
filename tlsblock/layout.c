#include <stdbool.h>
#include <stddef.h>

void *_dl_allocate_tls(void *);
void _dl_deallocate_tls(void *, bool);

int main(void) {
	_dl_deallocate_tls(_dl_allocate_tls(NULL), true);
	return 0;
}
