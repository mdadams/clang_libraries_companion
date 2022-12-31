#define FORTY_TWO forty_two
#define IDENTITY(x) x

struct gadget {
	gadget& operator=(const gadget&) {return *this;}
};
struct widget {
	widget& operator=(const widget&) = default;
};

constexpr int forty_two() {
	return 42;
}

int main() {
	for (int i = 1; i < 10; ++i) {
		forty_two();
		FORTY_TWO();
		IDENTITY(forty_two)();
	}
}


