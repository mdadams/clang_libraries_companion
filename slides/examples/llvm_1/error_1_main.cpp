#include "error_1.cpp"
#include <cstdlib>
#include <iostream>

int main(int argc, char** argv) {
	int test = -1;
	if (argc >= 2) {
		test = std::atoi(argv[1]);
	}
	switch (test) {
	case 1:
		std::cout << "calling doWork1\n";
		doWork1(); // ERROR: aborts
		break;
	case 2:
		std::cout << "calling doWork2\n";
		doWork2(); // ERROR: aborts
		break;
	case 3:
	default:
		std::cout << "calling doWork3\n";
		doWork3(); // OK
		break;
	}
}
