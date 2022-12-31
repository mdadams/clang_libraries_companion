int func(int, int, int, int);
int main() {
	auto func_ptr = &func;
	int a = 0;
	int b = 0;
	int c = 0;
	int d = 0;
	auto r1 = (*func_ptr)(a, b, c, d);
	auto r2 = func(a, b, c, d);
	return r1 + r2;
}
