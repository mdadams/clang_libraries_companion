int foo(int x) {
	if (x > 0) {
		return 1;
	} else if (x < 0) {
		return 2;
	} else {
		return 3;
	}
}

int foo_bar() {
	return 42;
}

template<class T>
T get_forty_two() {
	return T(42);
}

int foobar() {
	return get_forty_two<int>();
}
