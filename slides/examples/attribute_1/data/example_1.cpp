[[nodiscard, deprecated]] int get_answer() {return 42;}

struct Widget {
	[[nodiscard]] int get_answer() {return 0;}
	[[no_unique_address]] int x;
};

struct Thing {
	virtual void grok();
	virtual void snork();
};
struct Doodad final : public Thing {
	virtual void grok() override;
	[[deprecated]] virtual void snork() final;
	[[deprecated]] alignas(float) char old_crufty_value;
};

__attribute((deprecated))
int foobar;

[[gnu::unused]] int foobar_value;

[[maybe_unused, deprecated]] alignas(long double) int obsolete = 42;

#pragma GCC push_options
#pragma GCC optimize(9)
int fast() {return -1;}
#pragma GCC pop_options

[[using gnu : hot, const, always_inline]] [[nodiscard]]
float flaming() {return 42.42'42'42'42;}

[[gnu::noinline]]
long double slow() {return -1000;}
