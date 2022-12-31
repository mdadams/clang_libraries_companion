int factorial(int n) noexcept {
	int result = 1;
	while (n >= 2) {
		result *= n--;
	}
	return result;
}
