#ifndef MANDEL_H
#define MANDEL_H

#include "complex.h"

#define MANDEL_LIMIT 4

#define MANDEL_DEFAULT_MIN_RE -2.5
#define MANDEL_DEFAULT_MIN_IM 1.25
#define MANDEL_DEFAULT_MAX_RE 1.0
#define MANDEL_DEFAULT_MAX_IM -1.25
#define MANDEL_DEFAULT_ITER_MAX 100

typedef struct {
	complex_t min,max;
	int i_max;
} mandel_t;

int mandel_calcrow(struct Window *win,mandel_t *man,int row);

#endif
