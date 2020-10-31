/**
 * \file   du/du.c
 * \brief  Amiga implementation of the equal-named UNIX-tool
 * \author Dietmar Muscholik <d.muscholik@t-online.de>
 * \date   2017-05-21
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
#include <pragmas/dos_pragmas.h>

#define PATH_MAX 256

int du(char *name,LONG *tot_bytes,LONG *tot_blocks)
{
	int status=0;
	BPTR lock;

/*
	printf("du(%s)\n",name);
*/
	*tot_bytes=0;
	*tot_blocks=0;

	lock=Lock(name,ACCESS_READ);
	if(lock)
	{
		struct FileInfoBlock *info;

		info=AllocDosObject(DOS_FIB,NULL);
		if(info)
		{
			if(Examine(lock,info))
				while(ExNext(lock,info))
				{
					if(info->fib_DirEntryType > 0)
					{
						LONG bytes,blocks;
						char buffer[PATH_MAX];
						size_t n;

						/*printf("dir %s\n",info->fib_FileName);*/
						strcpy(buffer,name);
						n=strlen(buffer);
						if(n>0 && buffer[n-1]!=':' && buffer[n-1]!='/')
							strcat(buffer,"/");
						strcat(buffer,info->fib_FileName);
						if(!du(buffer,&bytes,&blocks))
						{
							*tot_bytes+=bytes;
							*tot_blocks+=blocks;
						}
					}
					else
					{
						/*printf("file %s\n",info->fib_FileName);*/
						*tot_bytes+=info->fib_Size;
						*tot_blocks+=info->fib_NumBlocks;
					}
				}
			else
				status=-1;

			FreeDosObject(DOS_FIB,info);
		}
		else
			status=-1;

		UnLock(lock);
	}
	else
		status=-1;

	return status;
}

int main(int argc,char **argv)
{
	int status=RETURN_OK;
	LONG args[]={0};
	struct RDArgs *rdargs;

	rdargs=ReadArgs("DIRS/A/M",args,NULL);
	if(rdargs)
	{
		char **dirs=(char **)args[0];
		size_t n;

		UBYTE buffer[sizeof(struct AnchorPath)+PATH_MAX];
		struct AnchorPath *anchor=(struct AnchorPath *)buffer;

		LONG bytes,blocks;

		memset(buffer,0,sizeof(buffer));
		anchor->ap_Strlen=PATH_MAX;

		for(n=0;dirs[n];n++)
		{
			if(!MatchFirst(dirs[n],anchor))
			{
				do
				{
					if(anchor->ap_Info.fib_DirEntryType < 0)
					{
						bytes=anchor->ap_Info.fib_Size;
						blocks=anchor->ap_Info.fib_NumBlocks;
					}
					else
						du((char *)anchor->ap_Buf,&bytes,&blocks);

					printf("%-32.32s %6d bytes %4d blocks\n",anchor->ap_Buf,bytes,blocks);
				} while(!MatchNext(anchor));
				MatchEnd(anchor);
			}
			else
			{
				fprintf(stderr,"%s: no such file or directory\n",dirs[n]);
				status=RETURN_WARN;
			}
		}
		FreeArgs(rdargs);
	}
	else
		status=RETURN_ERROR;

	return status;
}
