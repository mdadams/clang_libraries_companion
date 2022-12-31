#include <string>

int foo(int x) {
	{
		std::string x;
		{
			int y = 42;
		}
	}
	return 42;
}
