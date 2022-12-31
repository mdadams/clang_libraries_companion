#include <format>
#include <iostream>
int main(int argc, char** argv) {
	for (int i = 0; i < argc; ++i) {
		std::cout << std::format("argv[{}]: {}\n", i, argv[i]);
	}
}
