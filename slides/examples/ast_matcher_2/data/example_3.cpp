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

