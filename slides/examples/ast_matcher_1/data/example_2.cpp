#define IDENTITY(x) x
IDENTITY(void) IDENTITY(foo)(IDENTITY(int x), IDENTITY(int y)) {}

#define FOO(x, y) foo(x, y)

#define FOOZ foo

int main() {
	IDENTITY(foo)(42, 42);
	FOO(42, 42);
	FOOZ(42, 42);
}
