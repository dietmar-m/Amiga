/**
 * \file   mandel/complex.h
 * \brief  complex arithmetic for Mandelbrot program
 * \author Dietmar Muscholik <d.muscholik@t-online.de>
 * \date   2016-07-04
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
#ifndef COMPLEX_H
#define COMPLEX_H

typedef struct {double r,i;} complex_t;

void complex_add(complex_t *sum,complex_t *add);
void complex_sub(complex_t *dif,complex_t *sub);
void complex_mul(complex_t *prod,complex_t *fact);
void complex_div(complex_t *quot,complex_t *div);
void complex_quad(complex_t *c);
double complex_abs(complex_t *c);

#endif
