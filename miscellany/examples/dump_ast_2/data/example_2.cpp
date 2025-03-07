#if 0
int forty_two() {
	return 42;
}
#endif

#if 0
int zero() {
	return [](){return 0;}();
}
#endif

#if 0
struct Widget {
	int i;
};

Widget foo(Widget w) {
	return Widget(w.i + 1);
}
#endif

#if 0
namespace A { namespace B { class C {}; } }
class A::B::C obj;

namespace ABC = A::B;
#endif

#if 0
template<class T>
T abs(T x) {
	return x >= T(0) ? x : -x;
}
void foo()
{
	abs(-42);
}
#endif

#if 1
struct Base {};
struct Derived : Base {};
#endif
