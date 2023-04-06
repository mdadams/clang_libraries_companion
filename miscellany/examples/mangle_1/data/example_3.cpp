#include "example_3.hpp"
#include "example_3.h"

int main() {
	int x = abs(-42) + fabs(42.0) + slow_fabs(0.0) + forty_two<long>;
	return x;
}
