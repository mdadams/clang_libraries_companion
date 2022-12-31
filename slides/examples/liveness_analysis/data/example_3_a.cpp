int foo(int x, int y) {
	int t = x * y;
	if ((x + 1) * (x - 1) == y) {
		t = 1;
	}
	if (x * x - 1 != y) {
		t = 2;
	}
	return t;
}
