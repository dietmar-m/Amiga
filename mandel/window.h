/**
 * \file   mandel/window.h
 * \brief  window functions for Mandelbrot program
 * \author Dietmar Muscholik <d.muscholik@t-online.de>
 * \date   2016-07-29
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
#ifndef WINDOW_H
#define WINDOW_H

#include <intuition/intuition.h>
#include "mandel.h"

struct Window *window_Create(int colors);
int window_Delete(struct Window *window);
int window_Process(struct Window *window,mandel_t *man);

#endif
