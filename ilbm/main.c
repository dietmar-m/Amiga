/**
 * \file   ilbm/main.c
 * \brief  simple screenshot program writing ilbm files
 * \author Dietmar Muscholik <d.muscholik@t-online.de>
 * \date   2016-07-10
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
#include <stdio.h>

#include <dos/dos.h>
#include <graphics/gfxbase.h>
#include <intuition/intuition.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/intuition_pragmas.h>

/*#include "Mandel_rev.h"*/
/*#include "mandel.h"*/
/*#include "window.h"*/

#define INTUITIONNAME "intuition.library"
#define INTUITIONVERSION 0
#define GRAPHICSVERSION 0
#define GADTOOLSNAME "gadtools.library"
#define GADTOOLSVERSION 0


/*char *verstag=VERSTAG;*/

struct Library *IntuitionBase=NULL;
struct Library *GfxBase=NULL;
/*
struct Library *GadToolsBase=NULL;
*/

int open_libs()
{
	if(!(IntuitionBase=OpenLibrary(INTUITIONNAME,INTUITIONVERSION)))
	{
		fprintf(stderr,"failed to open %s\n",INTUITIONNAME);
		return -1;
	}
	if(!(GfxBase=OpenLibrary(GRAPHICSNAME,GRAPHICSVERSION)))
	{
		fprintf(stderr,"failed to open %s\n",GRAPHICSNAME);
		return -1;
	}
/*
	if(!(GadToolsBase=OpenLibrary(GADTOOLSNAME,GADTOOLSVERSION)))
	{
		fprintf(stderr,"failed to open %s\n",GADTOOLSNAME);
		return -1;
	}
*/
	return 0;
}

int close_libs()
{
/*
	if(GadToolsBase)
		CloseLibrary(GadToolsBase);
*/
	if(GfxBase)
		CloseLibrary(GfxBase);
	if(IntuitionBase)
		CloseLibrary(IntuitionBase);
	return 0;
}

int main(int argc,char **argv)
{
	int status=RETURN_OK;
	if(!open_libs())
	{
		struct Screen *wb_screen;

		if((wb_screen=LockPubScreen(NULL)))
		{
			struct Rectangle rect;
/*
			rect.MinX=wb_screen->LeftEdge;
			rect.MinY=wb_screen->TopEdge;
			rect.MaxX=wb_screen->LeftEdge+wb_screen->Width;
			rect.MaxY=wb_screen->TopEdge+wb_screen->Height;
*/
			rect.MinX=50;
			rect.MinY=50;
			rect.MaxX=640;
			rect.MaxY=400;

			ilbm_Write("screen.ilbm",&wb_screen->RastPort,
						&rect,wb_screen->ViewPort.ColorMap,
						NULL,0);
			UnlockPubScreen(NULL,wb_screen);
		}
		else
			status=RETURN_ERROR;
		close_libs();
	}
	else
		status=RETURN_ERROR;

	return status;
}
