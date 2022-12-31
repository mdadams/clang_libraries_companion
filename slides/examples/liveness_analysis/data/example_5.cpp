int foo() {
	return 42;
	// unreachable
	int i = 43;
	return i;
}
