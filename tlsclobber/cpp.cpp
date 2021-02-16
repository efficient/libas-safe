#include <iostream>

using std::cout;
using std::endl;

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
}
