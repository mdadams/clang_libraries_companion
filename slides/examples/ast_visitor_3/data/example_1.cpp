namespace foo {
	struct S1A {
		union U2 {
			struct S3A {};
			struct S3B {};
		};
		union {
			int i;
			float f;
		};
		struct S2 {};
	};
	struct S1B {};
}
