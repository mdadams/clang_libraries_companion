template<class T>
T identity(T x) {
	return x;
}

template<class T>
T abs(T x) {
	if (x >= T(0)) {
		return x;
	}
	return -x;
}

int foo(int x) {
	if (x > 0) {
		if (x < 30) {
			if (x > 5) {
				if (x < 2) {
					return x;
				}
			} else {
				if (x == 5) {
					return 3;
				}
			}
		}
	}
	return x;
}
