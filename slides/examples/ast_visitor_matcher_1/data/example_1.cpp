#if 0
#include <iostream>
#include <cassert>
#include <algorithm>
#include <string>
#endif
#include <vector>

void foo1a() {
	for (int i = 0; i < 16; ++i) {
	}
}

void bar1a() {
	std::vector<int> v{1, 2, 3};
	for (auto i : v) {
	}
}

void foo1b() {
	int i = 0;
	while (i) {
		for (int j = 0; j < 8; ++j) {
		}
	}
}

void foo1c() {
	int i = 42;
	while (i) {
		++i;
		--i;
		for (int j = 0; j < 8; ++j)
			;
		--i;
	}
}

void foo2a() {
	for (int i = 0; i < 16; ++i)
		for (int j = 0; j < 16; ++j) {}
}

void foo3a() {
	for (int i = 0; i < 8; ++i) {
		for (int j = 0; j < 8; ++j) {
			for (int k = 0; k < 8; ++k) {
			}
		}
	}
}

void foo3b() {
	for (int i = 0; i < 8; ++i)
		for (int j = 0; j < 8; ++j)
			for (int k = 0; k < 8; ++k)
				;
}

void bar3a() {
	std::vector<int> v{1, 2, 3};
	for (auto i : v) {
		for (auto j : v) {
			for (auto k : v) {
			}
		}
	}
}

void bar3b() {
	std::vector<int> v{1, 2, 3};
	for (auto i : v)
		for (auto j : v)
			for (auto k : v)
				;
}

void foo0() {
}

void foobar2() {
	std::vector<int> v{1, 2, 3};
	for (auto i : v)
		for (int i = 0; i < 8; ++i) {}
}

auto baz1 = []() {
	for (int i = 0; i < 8; ++i) {
	}
};

int main() {
	auto f = []() {
		for (int i = 0; i < 8; ++i) {
		}
	};
	auto g = []() {
		for (int i = 0; i < 8; ++i) {
			for (int j = 0; j < 8; ++j) {
			}
		}
	};
	foo1a();
	foo1b();
	foo1c();
	foo3a();
	foo3b();
}
