#include "/usr/include/hg2g/main.hpp"

int foo() {
	constexpr int x = hg2g::get_answer();
	return 2 * x;
}

void bar()
{
}
