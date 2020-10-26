/**
 * \file   mandel/request.c
 * \brief  file requester for Mandelbrot program
 * \author Dietmar Muscholik <d.muscholik@t-online.de>
 * \date   2016-07-16
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
#include <string.h>

#include <libraries/asl.h>
#include <pragmas/asl_pragmas.h>

#include "request.h"

BOOL request_File(struct Window *win,BOOL savemode,
					char *path,size_t path_size)
{
	BOOL ok=TRUE;
	struct TagItem req_tags[]={
		{ASLFR_Window,0},
		{ASLFR_DoSaveMode,0},
		{ASLFR_SleepWindow,TRUE},
		{TAG_DONE,0}
	};
	struct FileRequester *request;

	req_tags[0].ti_Data=(LONG)win;
	req_tags[1].ti_Data=(LONG)savemode;
	request=AllocAslRequest(ASL_FileRequest,req_tags);
	if(request)
	{
		if(AslRequest(request,NULL) && *request->fr_File)
		{
			size_t n;

			strcpy(path,(const char *)request->fr_Drawer);
			n=strlen(path);
			if(n && path[n-1]!=':' && path[n-1]!='/')
				strcat(path,"/");
			strcat(path,(const char *)request->fr_File);
		}
		else
			ok=FALSE;
		FreeAslRequest(request);
	}
	else
		ok=FALSE;
	return ok;
}
