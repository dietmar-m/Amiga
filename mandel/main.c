/**
 * \file   mandel/main.c
 * \brief  main() function for Mandelbrot program
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
#include <stdio.h>
#include <ctype.h>

#include <dos/dos.h>
#include <graphics/gfxbase.h>
#include <libraries/asl.h>
#include <workbench/icon.h>
#include <workbench/startup.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/icon_pragmas.h>

#include "Mandel_rev.h"
/*#include "mandel.h"*/
#include "window.h"

#define INTUITIONNAME "intuition.library"
#define INTUITIONREV 0
#define GRAPHICSREV 0
#define GADTOOLSNAME "gadtools.library"
#define GADTOOLSREV 0
#define ASLREV 0
#define ICONREV 0


char *verstag=VERSTAG;

struct Library *IntuitionBase=NULL;
struct Library *GfxBase=NULL;
struct Library *GadToolsBase=NULL;
struct Library *AslBase=NULL;
struct Library *IconBase=NULL;


int open_libs()
{
	if(!(IntuitionBase=OpenLibrary(INTUITIONNAME,INTUITIONREV)))
	{
		fprintf(stderr,"failed to open %s\n",INTUITIONNAME);
		return -1;
	}
	if(!(GfxBase=OpenLibrary(GRAPHICSNAME,GRAPHICSREV)))
	{
		fprintf(stderr,"failed to open %s\n",GRAPHICSNAME);
		return -1;
	}
	if(!(GadToolsBase=OpenLibrary(GADTOOLSNAME,GADTOOLSREV)))
	{
		fprintf(stderr,"failed to open %s\n",GADTOOLSNAME);
		return -1;
	}
	if(!(AslBase=OpenLibrary(AslName,ASLREV)))
	{
		fprintf(stderr,"failed to open %s\n",AslName);
		return -1;
	}
	return 0;
}

int close_libs()
{
	if(IconBase)
		CloseLibrary(IconBase);
	if(AslBase)
		CloseLibrary(AslBase);
	if(GadToolsBase)
		CloseLibrary(GadToolsBase);
	if(GfxBase)
		CloseLibrary(GfxBase);
	if(IntuitionBase)
		CloseLibrary(IntuitionBase);
	return 0;
}

int main(int argc,char **argv)
{
	int status=RETURN_OK;
	mandel_t man={
					{MANDEL_DEFAULT_MIN_RE,MANDEL_DEFAULT_MIN_IM},
					{MANDEL_DEFAULT_MAX_RE,MANDEL_DEFAULT_MAX_IM},
					MANDEL_DEFAULT_ITER_MAX
				};
#if 0
	LONG args[]={0};
	struct RDArgs *rdargs;

	rdargs=ReadArgs("IMAX/K/N",args,NULL);
	if(rdargs)
	{
		if(args[0])
			man.i_max=*(int *)args[0];
		printf("i_max=%d\n",man.i_max);
		if(!open_libs())
		{
			struct Window *window;

			if((window=window_Create()))
			{
				window_Process(window,&man);
				window_Delete(window);
			}
			else
				status=RETURN_ERROR;
			close_libs();
		}
		else
			status=RETURN_ERROR;
		FreeArgs(rdargs);
	}
	else
		status=RETURN_ERROR;
#endif
	if(!open_libs())
	{
		if(argc)
		{
			LONG args[]={0};
			struct RDArgs *rdargs;

			rdargs=ReadArgs("IMAX/K/N",args,NULL);
			if(rdargs)
			{
				if(args[0])
					man.i_max=*(int *)args[0];
				FreeArgs(rdargs);
			}
			else
				status=RETURN_ERROR;
		}
		else
		{
			IconBase=OpenLibrary(ICONNAME,ICONREV);
			if(IconBase)
			{
				struct WBStartup *msg=(struct WBStartup *)argv;
				if(msg->sm_NumArgs>0)
				{
					struct DiskObject *obj;
					obj=GetDiskObject(msg->sm_ArgList[0].wa_Name);
					if(obj)
					{
						char *imax;
						imax=(UBYTE *)FindToolType((CONST_STRPTR *)obj->do_ToolTypes,
												(CONST_STRPTR)"IMAX");
						if(imax && isdigit((int)*imax))
							man.i_max=atoi(imax);
						printf("imax=%d\n",man.i_max);
						FreeDiskObject(obj);
					}
				}
			}
			else
				status=RETURN_ERROR;
		}

		if(status==RETURN_OK)
		{
			struct Window *window;

			if((window=window_Create(man.i_max+1)))
			{
				window_Process(window,&man);
				window_Delete(window);
			}
			else
				status=RETURN_ERROR;
		}
		close_libs();
	}
	else
		status=RETURN_ERROR;

	return status;
}
