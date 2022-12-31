namespace foo {

class Widget {
public:
	Widget(int i) : i_(i) {}
	int value() const {
		return i_;
	}
private:
	int i_;
};

}

int main() {
	foo::Widget w(42);
	return w.value();
}
