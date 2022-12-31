// not specialization
template<class T, class U>
class Widget {
public:
	Widget(T x, U y) : x_(x), y_(y) {}
	const T& x() const {return x_;}
	const U& y() const {return y_;}
	static constexpr int value() {return 0;}
private:
	T x_;
	U y_;
};

#if 1
// partial specialization
template<class T>
class Widget<bool, T> {
public:
	static constexpr int value() {return 1;}
};

// full specialization
template<>
class Widget<bool, bool>
{
public:
	static constexpr int value() {return 2;}
};
#endif

int main() {
#if 1
	constexpr auto a0 = Widget<int, float>::value();
	static_assert(a0 == 0);
#endif
#if 1
	constexpr auto a1 = Widget<bool, int>::value();
	static_assert(a1== 1);
	constexpr auto a2 = Widget<bool, bool>::value();
	static_assert(a2== 2);
	constexpr auto a3 = Widget<char, short>::value();
	constexpr auto a4 = Widget<char, int>::value();
	constexpr auto a5 = Widget<char, long>::value();
#endif
}
