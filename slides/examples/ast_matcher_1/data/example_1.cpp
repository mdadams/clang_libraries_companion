#include <vector>
#include <iostream>
#include <cassert>

int foo(int a, int b, int c) {
	return a + b + c;
}

int bar(int a, int b) {
	return a * b;
}

int main() {
	int i = foo(
	  1,
	  2,
	  3
	);
	int j = foo(4, 5, 6);
	int k = bar(1, 2);
	int l = bar(
	  1, 2);
	std::vector<int> v;
	v.push_back(42);
	return v.size() + i;
}
