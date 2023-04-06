#ifndef EXAMPLE_3_H
#define EXAMPLE_3_H

#ifdef __cplusplus
extern "C" {
#endif

const int c_forty_two = 42;

extern int c_var;

static inline double fabs(double x) {
	return x >= 0.0 ? x : -x;
}

double slow_fabs(double x);

#ifdef __cplusplus
}
#endif

#endif
