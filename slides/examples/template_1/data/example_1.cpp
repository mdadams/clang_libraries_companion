#include <tuple>

#if 0
template<class... Ts> class not_a_tuple {};
template<class... Ts> using tuple = not_a_tuple<Ts...>;
#else
template<class... Ts> class tuple {};
#endif

int main() {
	std::tuple<std::tuple<std::tuple<bool, char, int>, float, double>,
	  void*> t_1;
	std::tuple<unsigned char, char> t_2;
	tuple<int> t_3;
}
