#if 0
#include <cassert>
#endif

// declaration for function template
template<class T> T foo(T);

// definition for function template
template<class T> T foo(T x) {
	return x + 1;
}

// declaration for explicit specialization of function template
template<> int foo(int);

// definition for explicit specialization of function template
#if 1
template<> int foo(int x) {
	return x + 2;
}
#endif

// explicit instantiation of function template
template long double foo(long double);

int main() {
#if 1
	auto a1 = foo(0);
	auto a2 = foo(0.0);
#endif
#if 0
	assert(a1 == 1);
	assert(a2 == 2.0);
#endif
}
