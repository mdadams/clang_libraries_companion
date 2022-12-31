#define foo1(x) foo(x)
int foo(int x) {return x;}
int main() {
	return foo1(42);
}
