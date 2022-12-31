#include <vector>

class widget {
public:
	widget() noexcept {}
	widget(int x) {}
	~widget() = default;
};

int func_1(int, ...);
int func_1(int x, ...) {
	return x;
}

template<typename T> void func_2(T);
template<typename T>
void func_2(T x) {
}

std::vector<int> foo();
std::vector<int> foo() {
	return {};
}

void never_throw() noexcept;
void never_throw() noexcept {
}

int func_4(int) noexcept;
auto func_4(int x) noexcept -> int {return 42;}

int main() {
	func_2(42);
}
