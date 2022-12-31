struct Widget {
	void foo1() {
		for (;;) {
		}
	}
	void bar2(int x) {
		if (x < 0) {
			for (int i = 0; i < 4; ++i) {
				for (int j = 0; j < 4; ++j) {
				}
			}
		}
	}
};
