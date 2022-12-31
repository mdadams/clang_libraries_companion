#define min(x, y) (((x) < (y)) ? (x) : (y))
#define max(x, y) (((x) > (y)) ? (x) : (y))

int max3(int x, int y, int z) {
	return max(max(x, y), z);
}

template<class T>
T max3(T x, T y, T z) {
	return max(max(x, y), z);
	// comment
}

int foo() {
	// comment
	auto i = [](){
		return 42;
	};
	return i();
}

struct Widget {
	int foo() {
		/*
		This is a comment.
		*/
		return 42;
	}
};
