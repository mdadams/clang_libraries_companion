#include <iostream>
#include <vector>

namespace foo {

int max(int m, int n) {
	return m > n ? m : n;
}

int abs(int n) {
	return n >= 0 ? n : -n;
}

}

std::vector<int> get_values() {
	return std::vector<int>{1, 2, 4, 8};
}
