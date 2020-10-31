/**
 * \file   iconpos/iconpos.c
 * \brief  read and write the position of an icon
 * \author Dietmar Muscholik <d.muscholik@t-online.de>
 * \date   2016-06-03
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
#include <dos/rdargs.h>
#include <pragmas/dos_pragmas.h>

#include <pragmas/exec_pragmas.h>
/*
#include <workbench/icon.h>
*/
/*
#include <pragmas/icon_pragmas.h>
*/
#include <workbench/workbench.h>
#include <clib/icon_protos.h>


enum {
	ARG_NAME,
	ARG_ICON_X,
	ARG_ICON_Y,
	ARG_DRAWER_X,
	ARG_DRAWER_Y,
	ARG_DRAWER_W,
	ARG_DRAWER_H
};

#define TEMPLATE "FILE/A,ICON_X=IX/K/N,ICON_Y=IY/K/N,"\
					"DRAWER_X=DX/K/N,DRAWER_Y=DY/K/N,"\
					"DRAWER_WIDTH=DW/K/N,DRAWER_HEIGHT=DH/K/N"

struct Library *IconBase=NULL;

int main(int argc,char **argv)
{
	int status=RETURN_OK;
	struct RDArgs *rdargs;
	LONG args[7]={0,0,0,0,0,0,0};

	rdargs=ReadArgs(TEMPLATE,args,NULL);
	if(!rdargs)
		return RETURN_ERROR;

	/*printf("%s\n",args[0]);*/
	IconBase=OpenLibrary("icon.library",0);
	if(IconBase)
	{
		struct DiskObject *object;
		/*printf("success, so far\n");*/

		object=GetDiskObject((CONST_STRPTR)args[ARG_NAME]);
		if(object)
		{
			printf("icon_x=%d, icon_y=%d\n",
					object->do_CurrentX,object->do_CurrentY);
			if(object->do_Type==WBDRAWER)
			{
				printf("drawer_x=%d, drawer_y=%d\n",
						object->do_DrawerData->dd_NewWindow.LeftEdge,
						object->do_DrawerData->dd_NewWindow.TopEdge);
				printf("drawer_w=%d, drawer_h=%d\n",
						object->do_DrawerData->dd_NewWindow.Width,
						object->do_DrawerData->dd_NewWindow.Height);
			}

			if(args[ARG_ICON_X] || args[ARG_ICON_Y] ||
				object->do_Type==WBDRAWER &&
				(args[ARG_DRAWER_X] || args[ARG_DRAWER_Y] ||
				args[ARG_DRAWER_W] || args[ARG_DRAWER_H]))
			{
				if(args[ARG_ICON_X])
					object->do_CurrentX=*(int *)args[ARG_ICON_X];
				if(args[ARG_ICON_Y])
					object->do_CurrentY=*(int *)args[ARG_ICON_Y];
				if(object->do_CurrentX<0)
					object->do_CurrentX=NO_ICON_POSITION;
				if(object->do_CurrentY<0)
					object->do_CurrentY=NO_ICON_POSITION;

				if(object->do_Type==WBDRAWER)
				{
					if(args[ARG_DRAWER_X])
						object->do_DrawerData->dd_NewWindow.LeftEdge=
							*(int *)args[ARG_DRAWER_X];
					if(args[ARG_DRAWER_Y])
						object->do_DrawerData->dd_NewWindow.TopEdge=
							*(int *)args[ARG_DRAWER_X];
					if(args[ARG_DRAWER_W])
						object->do_DrawerData->dd_NewWindow.Width=
							*(int *)args[ARG_DRAWER_W];
					if(args[ARG_DRAWER_H])
						object->do_DrawerData->dd_NewWindow.Height=
							*(int *)args[ARG_DRAWER_H];
				}
				PutDiskObject((CONST_STRPTR)args[ARG_NAME],object);
			}
			FreeDiskObject(object);
		}
		else
			status=RETURN_ERROR;

		CloseLibrary(IconBase);
	}
	else
		status=RETURN_ERROR;

	FreeArgs(rdargs);

	return status;
}
