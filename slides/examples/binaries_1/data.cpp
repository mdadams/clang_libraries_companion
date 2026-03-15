#define EXTERN_C extern "C"

// Platform-specific section attributes.
#if defined(__GNUC__) || defined(__clang__)
	#define SECTION(x) __attribute__((__section__(x)))
#elif defined(_MSC_VER)
	#define SECTION(x) __declspec(allocate(x))
#else
	#define SECTION(x)  // Fallback: no section attribute
	#warning "Custom sections not supported on this compiler"
#endif

// Platform-specific section naming.
#if defined(__APPLE__)
	#define MY_SECTION SECTION("__DATA,__mysection")
#elif defined(_WIN32)
	#define MY_SECTION SECTION(".mysection")
	#pragma section(".mysection", read)
#else // Linux/ELF
	#define MY_SECTION SECTION(".mysection")
#endif

EXTERN_C
MY_SECTION
const char hello[] = "Hello, World!";

EXTERN_C
MY_SECTION
const char goodbye[] = "Goodbye, Cruel World!";

EXTERN_C
MY_SECTION
const char text[] = R"(To be, or not to be, that is the question:
Whether 'tis nobler in the mind to suffer
The slings and arrows of outrageous fortune,
Or to take arms against a sea of troubles,
And by opposing end them?
)";

EXTERN_C
MY_SECTION
const char* const helloPtr = "Hello, World!";

EXTERN_C
MY_SECTION
const double reals[] = {
	1.0,
	1.5,
	2.0,
	2.5,
};
