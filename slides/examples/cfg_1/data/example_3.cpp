#include <cstdlib>
int foo(int i) {
	switch (i) {
	case 1:
		return 10;
	case 2:
		return 20;
	case 3:
		return 30;
	case 4:
		return 40;
	case 5:
		return 50;
	default:
		std::abort();
	}
}
