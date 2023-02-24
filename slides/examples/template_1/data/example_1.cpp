#include <tuple>

#if 1
template<class... Ts> class not_a_tuple {};
template<class... Ts> using tuple = not_a_tuple<Ts...>;
#else
template<class... Ts> class tuple {};
#endif

namespace foo {
	using Thing = int;
	using Doodad = float;
	struct Widget {};
	struct Gadget {Widget w;};
	std::tuple<Gadget, foo::Gadget, Widget, foo::Widget> f_1;
	std::tuple<Thing, Doodad> f_2;
	std::tuple<tuple<std::tuple<tuple<Thing, Doodad>>>> f_3;
}
struct Gadget {};
std::tuple<> g_0;
std::tuple<char, short, int, long> g_1;
std::tuple<std::tuple<int, float>, std::tuple<>> g_2;
std::tuple<::Gadget> g_3;

int main() {
	std::tuple<std::tuple<std::tuple<bool, char, int>, float, double>,
	  void*> m_1;
	std::tuple<unsigned char, char> m_2;
	tuple<int> m_3;
}
