#ifndef EXAMPLE_3_HPP
#define EXAMPLE_3_HPP

template<class T>
inline T abs(T x) {
	return x >= T(0) ? x : -x;
}

template<class T>
constexpr T forty_two = T(42);

#endif
