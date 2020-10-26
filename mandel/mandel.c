/**
 * \file   mandel/mandel.h
 * \brief  calculate a single row of a Mandelbrot set
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
#include <intuition/intuition.h>
#include <pragmas/graphics_pragmas.h>

#include "mandel.h"


static int mandel_calcdot(complex_t *c,int i_max)
{
	int i;
	complex_t z={0,0};

	for(i=0; i<i_max; i++)
	{
		complex_quad(&z);
		complex_add(&z,c);
		if(complex_abs(&z) > MANDEL_LIMIT)
			break;
	}

	return i;
}

/*static int mandel_pen(struct Window *win,int i,int i_max)*/
static int mandel_pen(struct RastPort *rp,int i,int i_max)
{
/*
	int colors=win->WScreen->ViewPort.ColorMap->Count-NUMDRIPENS;
*/
	int colors=(1<<rp->BitMap->Depth)-FILLTEXTPEN-1;

	if(i<i_max)
		return i%(i_max<colors ? i_max : colors)+FILLTEXTPEN+1;
	return FILLTEXTPEN;
}

int mandel_calcrow(struct Window *win,mandel_t *man,int row)
{
	int width=win->Width-win->BorderLeft-win->BorderRight;
	int height=win->Height-win->BorderTop-win->BorderBottom;
	double dr=(man->max.r-man->min.r)/width;
	complex_t c;
	int col,i;

	c.r=man->min.r;
/*	c.i=man->max.i-row*(man->max.i-man->min.i)/height;*/
	c.i=man->min.i+row*(man->max.i-man->min.i)/height;
	/*printf("from %lf,%lf ",c.r,c.i);*/
	for(col=0; col<width; col++)
	{
		i=mandel_calcdot(&c,man->i_max);
		SetAPen(win->RPort,mandel_pen(win->RPort,i,man->i_max));
		WritePixel(win->RPort,win->BorderLeft+col,win->BorderTop+row);
		c.r+=dr;
	}
	/*printf("to %lf,%lf\n",c.r,c.i);*/

	return 0;
}
