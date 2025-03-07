template<class T> struct Widget {
	T value;
};

template<class T> T abs(T x) {
	return x >= 0 ? x : (-x);
}

template<class T> T zero_var = T(0);

int forty_two(Widget<int> w)
{
	return 42;
}
