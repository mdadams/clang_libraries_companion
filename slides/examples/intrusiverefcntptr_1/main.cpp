#include <cassert>
#include <iostream>
#include <llvm/ADT/IntrusiveRefCntPtr.h>

class Widget : public llvm::RefCountedBase<Widget> {
public:
	Widget() {++liveCount; std::cout << "Widget created\n";}
	Widget(const Widget&) = delete;
	Widget& operator=(const Widget&) = delete;
	~Widget() {--liveCount; std::cout << "Widget destroyed\n";}
	inline static int liveCount = 0;
};

int main() {
	{
		llvm::IntrusiveRefCntPtr<Widget> w1(new Widget());
		assert(Widget::liveCount == 1 && w1.useCount() == 1);
		llvm::IntrusiveRefCntPtr<Widget> w2 = w1;
		assert(w1.useCount() == 2 && &*w1 == &*w2);
		w2.reset();
		assert(!w2 && w1 && w1.useCount() == 1);
		{
			llvm::IntrusiveRefCntPtr<Widget> w3 = w1;
			assert(w1.useCount() == 2 && &*w3 == &*w1);
		}
		assert(w1.useCount() == 1);
		assert(Widget::liveCount == 1);
	} // Widget object destroyed
	assert(Widget::liveCount == 0);
}
