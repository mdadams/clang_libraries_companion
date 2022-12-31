#define cat(x,y) x##y

#define mac1 int
#define mac2 xxxxxxxxxxxxxxxxxxxxxxxxxxx
#define mac3 =
#define mac4 42

mac1 mac2 mac3 mac4;

#define FOO(T) \
class Gadget { \
public: \
	Gadget() {} \
	virtual ~Gadget() {} \
	virtual void grok() {} \
	virtual void snarf() {} \
	void fooz1(T) {} \
	void fooz2(T, T) {} \
	void fooz3(T, T, T) {} \
	void fooz4(T, T, T, T) {} \
	void fooz5(T, T, T, T, T) {} \
};

FOO(int)

cat(i,nt) cat(yyyyyyyy,yyyyyyyyyyyyyyyy) = 42;

#define MACRO_1(x) x
#define MACRO_2(x) MACRO_1(x)
#define MACRO_3(x) MACRO_2(x)
#define MACRO_4(x) MACRO_3(x)

#define EMPTY
MACRO_4(int) the_variable = 123456789;
int forty_two = 42;
MACRO_3(int) blah = 42;

auto foo() -> void {}
