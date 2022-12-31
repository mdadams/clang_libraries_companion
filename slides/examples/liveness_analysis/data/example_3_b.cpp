int foo(int x, int y) {
	int t = x * y; // t is dead
	if ((x + 1) * (x - 1) == y) {
		t = 1;
	} else {
		t = 2;
	}
	return t;
}
