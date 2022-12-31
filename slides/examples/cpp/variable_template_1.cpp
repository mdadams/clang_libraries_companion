template<class T>
T forty_two = T(42);

template<int X, int Y>
constexpr int foo = -1;

template<>
constexpr float foo<1, 0> = 1.0f;

int main() {
	auto x = forty_two<int>;
}
