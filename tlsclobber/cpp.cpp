#include <asm/prctl.h>
#include <iostream>

using std::cout;
using std::endl;
extern "C" {
	struct tcb_t;
	void _dl_allocate_tls_init(tcb_t *);
	void arch_prctl(int, tcb_t **);
}

struct Ure {
	const char *name;

	Ure(const char *name):
		name(name) {
		cout << "Ure(" << name << ')' << endl;
	}

	virtual ~Ure() {
		cout << "~Ure(" << name << ')' << endl;
	}
};

static Ure global = "global";
static thread_local Ure local = "local";

int main() {
	cout << &local << endl;

	tcb_t *tcb;
	arch_prctl(ARCH_GET_FS, &tcb);
	_dl_allocate_tls_init(tcb);

	cout << &local << endl;
}
