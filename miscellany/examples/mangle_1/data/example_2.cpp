class widget {
public:
	widget(int i) : i_(i) {}
	int value() const {return i_;}
private:
	int i_;
};

int main()
{
	widget w(42);
	return w.value();
}
