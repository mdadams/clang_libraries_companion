class Widget {
	virtual void func();
};
class RedWidget : public Widget {
};

int main() {
	const char * const * const * const * xp = nullptr;
	int i = 42;
	int* ip = &i;
	char* cp = reinterpret_cast<char*>(ip);
	const char* ccp = cp;
	cp = const_cast<char*>(ccp);
	ccp = const_cast<const char*>(cp);
	RedWidget g;
	Widget* wp = &g;
	RedWidget* rwp;
	const RedWidget* crwp;
	const RedWidget* crw;
	rwp = static_cast<RedWidget*>(wp);
	rwp = (RedWidget*) wp;
	crwp = (const RedWidget*) wp;
	rwp = dynamic_cast<RedWidget*>(wp);
	auto x1 = reinterpret_cast<char*>(const_cast<char****>(xp));
}
