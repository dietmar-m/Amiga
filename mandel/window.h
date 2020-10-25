#ifndef WINDOW_H
#define WINDOW_H

#include <intuition/intuition.h>
#include "mandel.h"

struct Window *window_Create(int colors);
int window_Delete(struct Window *window);
int window_Process(struct Window *window,mandel_t *man);

#endif
