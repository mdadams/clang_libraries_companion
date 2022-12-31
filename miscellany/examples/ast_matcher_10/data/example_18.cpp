#include <new>
int main() {
	char buf[1];
	auto p = new (buf) char;
}
