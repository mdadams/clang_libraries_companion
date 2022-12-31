int foo2(int x) {
	if (x > 0) {
		for (int i = 0; i < 4; ++i) {
			if (x > 5) {
				for (int j = 0; j < 10; ++j) {
				}
				return 1;
			}
		}
	}
	return 0;
}

int bar3(int x) {
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
		}
	}
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			for (int k = 0; k < 4; ++k) {
			}
		}
	}
	for (int i = 0; i < 4; ++i) {
	}
	return 42;
}

void bar2() {
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
		}
	}
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
		}
	}
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
		}
	}
}
