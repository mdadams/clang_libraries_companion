#include <type_traits>

template<class T>
std::enable_if_t<sizeof(T) == 1>
func(T x) {}

int main() {
	func('a');
}
