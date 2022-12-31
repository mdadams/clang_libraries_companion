#include <iostream>

int main() {
	std::cout << "hello\n";
	return !std::cout.flush();
}
