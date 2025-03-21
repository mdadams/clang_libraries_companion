#include <concepts>

template<class T>
requires std::floating_point<T>
class Interval
{
public:
	Interval() : lower_(0), upper_(0) {}
	// ...
private:
	T lower_;
	T upper_;
};

Interval<float> the_interval;

void foo() {
	int forty_two = 42;
	auto x = [=](){return forty_two;};
	x();
}
