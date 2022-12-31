#include <cassert>

class Gadget {
public:
	Gadget() {}
	virtual ~Gadget() {}
	virtual void grok() {}
	virtual void snarf() {}
	void fooz1(int) {}
	void fooz2(int, int) {}
	void fooz3(int, int, int) {}
	void fooz4(int, int, int, int) {}
	void fooz5(int, int, int, int, int) {}
};

class Widget : public Gadget {
public:
	Widget() {}
	virtual ~Widget() {}
	void grok() override {}
	virtual void snarf() override {}
};

int main() {
	Widget w;
}
