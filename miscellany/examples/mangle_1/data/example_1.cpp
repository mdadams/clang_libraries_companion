#include <iostream>

int main()
{
	std::cout << "Hello, World!\n";
	return std::cout().flush() ? 0 : 1;
}
