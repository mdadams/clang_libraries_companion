int abs(int i) {
	return i < 0 ? -i : i;
}

#if 0
void looper() {

	for (int i = 0; i < 10; ++i) {}

	{
		int i = 0;
		while (i < 10) {
			++i;
		}
	}

	{
		int i = 0;
		do {
			++i;
		} while(i < 10);
	}

}
#endif

#if 0
int iffer(int i) {
	int result = 0;
	if (i == 0) {
		result = 1;
	} else if (i == 2) {
		result = 2;
	} else {
		result = 42;
	}

	switch(i) {
	case 0:
		result *= 10;
		break;
	case 1:
		result *= 2;
		break;
	default:
		result *= 3;
		break;
	}

	return result;
}
#endif

#if 1
void expressor(int i) {
	auto r1 = i + i;
	auto r2 = i++;
	auto r3 = abs(i);
}
#endif
