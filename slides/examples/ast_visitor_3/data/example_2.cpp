#define ALL
namespace foo {
	struct A1 {
		struct A2 {
			struct A3 {
#ifdef ALL
				struct A4 {
				};
#endif
			};
		};
	};
#ifdef ALL
	struct B {};
#endif
}

#ifdef ALL
struct Foo {
	struct Bar {
	};
};
#endif

union Something {
	union Wazzit {
		float f;
		char c;
	};
	union {
		char c;
	};
	int i;
	float f;
};
