#define LEFT_BRACE {
#define RIGHT_BRACE }

#define MACRO_1(x) x
#define MACRO_2(x) MACRO_1(x)
#define MACRO_3(x) MACRO_2(x)
#define MACRO_4(x) MACRO_3(x)

MACRO_4(int) x = 42;

MACRO_4(float) yyy = 42.0f / 54321.0f;

MACRO_4(int) MACRO_3(xxx) MACRO_2(=) MACRO_2(42.0) MACRO_2(;)

int z = 42;

void foo() {}MACRO_4(extern) int foobarzzzzz;

struct widget LEFT_BRACE
#include "example_4.hpp"
RIGHT_BRACE;

widget w_1;

#define LONGINT long int
LONGINT big_int = 42;

#define FOOBAZ struct {int x; int y; int z;} sss;
FOOBAZ
