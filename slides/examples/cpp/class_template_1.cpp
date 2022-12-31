// declaration
template<class, class> class Widget;

// definition
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

// ???
template class Widget<int, int>;

// ???
template<class T>
class Widget<bool, T>;

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

// explicit instantiation
template class Widget<long double, long double>;

int main() {
	constexpr auto a = Widget<int, float>::value();
	constexpr auto b = Widget<bool, int>::value();
	constexpr auto c = Widget<bool, bool>::value();
	static_assert(a == 0);
	static_assert(b == 1);
	static_assert(c == 2);
}
