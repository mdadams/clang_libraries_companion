int f(int x) {
	return x;
}
int main() {
	int a;
	int b;
	int c;
	b = 3;
	if (b) {f(42);}
	c = 5;
	if (c) {f(42);}
	a = f(b * c);
}
