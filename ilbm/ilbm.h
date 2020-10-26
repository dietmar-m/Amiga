/**
 * \file   ilbm/ilbm.h
 * \brief  read and write ilbm files
 * \author Dietmar Muscholik <d.muscholik@t-online.de>
 * \date   2016-08-05
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
#ifndef ILBM_H
#define ILBM_H

/*#include <graphics/gfx.h>*/
#include <graphics/rastport.h>
#include <graphics/view.h>

typedef struct {
	char id[4];
	LONG size;
} ilbm_header_t;

LONG ilbm_Write(char *name,struct RastPort *rp,
				struct Rectangle *rect,struct ColorMap *cm,
				ULONG mode,
				UBYTE *extra,size_t extra_size);
LONG ilbm_Read(char *name,struct RastPort *rp,
				struct Rectangle *rect,struct ColorMap *cm,
				UBYTE *extra,size_t extra_size);
#endif
