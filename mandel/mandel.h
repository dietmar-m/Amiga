/**
 * \file   mandel/mandel.h
 * \brief  calculate a single row of a Mandelbrot set
 * \author Dietmar Muscholik <d.muscholik@t-online.de>
 * \date   2016-07-09
 *         latest update
 */
/*
    Copyright (C) 2016  Dietmar Muscholik

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
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
