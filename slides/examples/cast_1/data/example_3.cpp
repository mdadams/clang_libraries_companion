class Widget {
	virtual void grok() {}
};
class BigWidget : public Widget {};

void* address_forty_two() {
	return reinterpret_cast<void*>(42);
}
BigWidget* big(Widget* w) {
	return dynamic_cast<BigWidget*>(w);
}
BigWidget* big2(Widget* w) {
	return static_cast<BigWidget*>(w);
}
char* dangerous(const char* s) {
	return const_cast<char*>(s);
}
