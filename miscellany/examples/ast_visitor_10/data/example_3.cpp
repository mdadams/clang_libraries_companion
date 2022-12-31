template<class T>
T abs(T x);

template<class T>
T abs(T x) {
	return (x >= 0) ? x : -x;
}

template<int> int abs(int);
