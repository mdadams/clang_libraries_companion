#include <vector>

/*
If a function template contains a templated class type that is iterated over
with a range-based for loop, building a CFG will fail.
*/

template<typename T>
void foo_with_range_based_for(std::vector<T> vec) {
	for(auto el : vec){ }
}

/* This function template is identical to foo except a traditional for loop
is used instead of a range-based for loop. */
template<typename T>
void foo_with_traditional_for(std::vector<T> vec){
	for (auto i = vec.begin(); i != vec.end(); ++i) {}
}

int main() {
	foo_with_range_based_for(std::vector<int>{1, 2, 3});
	foo_with_traditional_for(std::vector<int>{1, 2, 3});
}
