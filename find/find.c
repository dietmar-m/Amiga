/**
 * \file   find/find.c
 * \brief  Amiga implementation of the equal-named UNIX-tool
 * \author Dietmar Muscholik <d.muscholik@t-online.de>
 * \date   2016-08-29
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
#include <stdlib.h>
#include <string.h>

#include <dos/dos.h>
#include <pragmas/dos_pragmas.h>

#include "find_rev.h"

#define PATH_MAX 256

typedef enum {MATCH_NONE, MATCH_DIR, MATCH_FILE} match_type_t;

typedef struct
{
	match_type_t type;
	STRPTR name;
	struct DateTime *older;
	struct DateTime *newer;
	STRPTR exec;
} cond_t;


enum {ARGS_DIRS, ARGS_TYPE, ARGS_NAME, ARGS_OLDER, ARGS_NEWER, ARGS_EXEC};


char *verstag=VERSTAG;

int find(char *dir,cond_t *cond)
{
	int status=0;
	BPTR lock;

/*	printf("find(%s)\n",dir);*/

/*
	lock=Lock((CONST_STRPTR)dir,ACCESS_READ);
*/
/*
	printf("cond->name=%s\n",cond->name?"NOT NULL":"NULL");
*/
	lock=Lock(dir,ACCESS_READ);
	if(lock)
	{
		struct FileInfoBlock *info;

		info=AllocDosObject(DOS_FIB,NULL);
		if(info)
		{
			if(Examine(lock,info))
			{
				BOOL match;
				char path[PATH_MAX];
				size_t n;

				while(ExNext(lock,info))
				{
					match=TRUE;

					/*printf("%s\t",info->fib_FileName);*/
					if(cond->type!=MATCH_NONE)
					{
						if(cond->type==MATCH_DIR && info->fib_DirEntryType<0)
							match=FALSE;
						if(cond->type==MATCH_FILE && info->fib_DirEntryType>0)
							match=FALSE;
					}

					/*printf("match=%s\t",match?"TRUE":"FALSE");*/
					if(match && cond->name)
					{
						/*printf("cond->name=%s",cond->name);*/
						if(!MatchPattern((CONST_STRPTR)cond->name,
										(STRPTR)info->fib_FileName))
							match=FALSE;
					}
					/*printf("match=%s\n",match?"TRUE":"FALSE");*/

					if(match && cond->older)
					{
						if(CompareDates(cond->older,&info->fib_Date) > 0)
							match=FALSE;
					}

					if(match && cond->newer)
					{
						if(CompareDates(cond->newer,&info->fib_Date) <= 0)
							match=FALSE;
					}

					if(match || info->fib_DirEntryType>0)
					{
						strcpy(path,dir);
						n=strlen(path);
						if(n>0 && path[n-1]!=':' && path[n-1]!='/')
							strcat(path,"/");
						strcat(path,info->fib_FileName);

						if(match)
						{
							if(cond->exec)
							{
								char buffer[PATH_MAX];
								char *p;

								p=strstr((const char *)cond->exec,"[]");
								if(p)
								{
									size_t n;

									n=p-cond->exec;
									strncpy(buffer,(const char *)cond->exec,n);
									buffer[n]=0;
									strcat(buffer,path);
									strcat(buffer,p+2);
								}
								else
								{
									strcpy(buffer,(const char *)cond->exec);
									strcat(buffer," ");
									strcat(buffer,path);
								}
								/*printf("%s\n",buffer); */
								Execute(buffer,(BPTR)NULL,(BPTR)NULL);
							}
							else
								printf("%s\n",path);
						}

						if(info->fib_DirEntryType>0)
							find(path,cond);
					}
				}
			}
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


BOOL parse_DateTime(struct DateTime *dt,char *s)
{
	char *p;

	DateStamp(&dt->dat_Stamp);

	p=strchr(s,',');
	if(!p)
		p=strchr(s,' ');

	if(p)
	{
/*		printf("datetime: %s\n",s);*/
		dt->dat_StrDate=(UBYTE *)s;
		*p=0;
		dt->dat_StrTime=(UBYTE *)p+1;
	}
	else
	{
/*
		if(strchr(s,'-'))
		{
			printf("date: %s\n",s);
			dt->dat_StrDate=(UBYTE *)s;
			dt->dat_StrTime=NULL;
		}
		else if(strchr(s,':'))
		{
			printf("time: %s\n",s);
			dt->dat_StrDate=NULL;
			dt->dat_StrTime=(UBYTE *)s;
		}
		else
			return FALSE;
*/
		if(strchr(s,':'))
		{
/*			printf("time: %s\n",s);*/
			dt->dat_StrDate=NULL;
			dt->dat_StrTime=(UBYTE *)s;
		}
		else
		{
/*			printf("date: %s\n",s);*/
			dt->dat_StrDate=(UBYTE *)s;
			dt->dat_StrTime=NULL;
		}
	}

	dt->dat_Format=FORMAT_DOS;

	if(StrToDate(dt))
		return TRUE;

	dt->dat_Format=FORMAT_INT;

	return StrToDate(dt);
}


int main(int argc,char **argv)
{
	int status=RETURN_OK;
	LONG args[]={0,0,0,0,0,0};
	struct RDArgs *rdargs;

	rdargs=ReadArgs("DIRS/A/M,TYPE/K,NAME/K,OLDER/K,NEWER/K,EXEC/K",
					args,NULL);
	if(rdargs)
	{
		cond_t cond/*={MATCH_NONE, NULL}*/;
		struct DateTime older,newer;

		cond.type=MATCH_NONE;
		cond.name=NULL;
		cond.older=NULL;
		cond.newer=NULL;
		cond.exec=NULL;

/*		printf("exec=%s\n",args[ARGS_EXEC] ? (char *)args[ARGS_EXEC] : "NULL");*/
/*
		printf("type=%s, name=%s\n",args[ARGS_TYPE]?"NOT NULL":"NULL",args[ARGS_NAME]?"NOT NULL":"NULL");
		printf("cond.name=%s\n",cond.name?"NOT NULL":"NULL");
*/
		if(args[ARGS_TYPE])
		{
			switch(*(char *)args[ARGS_TYPE])
			{
			case 'd':
				cond.type=MATCH_DIR;
				break;
			case 'f':
				cond.type=MATCH_FILE;
				break;
			default:
				status=RETURN_ERROR;
			}
		}
/*
		printf("cond.name=%s\n",cond.name?"NOT NULL":"NULL");
*/
		if(status==RETURN_OK && args[ARGS_NAME])
		{
			cond.name=malloc(PATH_MAX);
			if(!cond.name || ParsePattern((CONST_STRPTR)args[ARGS_NAME],cond.name,PATH_MAX)<0)
				status=RETURN_ERROR;
/*
			printf("pattern=%s\n",cond.name);
*/
		}
/*
		printf("cond.name=%s\n",cond.name?"NOT NULL":"NULL");
*/
		if(status==RETURN_OK && args[ARGS_OLDER])
		{
			if(parse_DateTime(&older,(char *)args[ARGS_OLDER]))
				cond.older=&older;
			else
				status=RETURN_ERROR;
		}

		if(status==RETURN_OK && args[ARGS_NEWER])
		{
			if(parse_DateTime(&newer,(char *)args[ARGS_NEWER]))
				cond.newer=&newer;
			else
				status=RETURN_ERROR;
		}

		if(status==RETURN_OK && args[ARGS_EXEC])
			cond.exec=(STRPTR)args[ARGS_EXEC];

		if(status==RETURN_OK)
		{
			char **dirs=(char **)args[ARGS_DIRS];
			size_t n;
			UBYTE buffer[sizeof(struct AnchorPath)+PATH_MAX];
			struct AnchorPath *anchor=(struct AnchorPath *)buffer;

			memset(buffer,0,sizeof(buffer));
			anchor->ap_Strlen=PATH_MAX;

			/*printf("ok\n");*/
			for(n=0;dirs[n];n++)
			{
				if(!MatchFirst(dirs[n],anchor))
				{
					do
					{
						find((char *)anchor->ap_Buf,&cond);
					} while(!MatchNext(anchor));
					MatchEnd(anchor);
				}
				else
				{
					fprintf(stderr,"%s: no such file or directory\n",dirs[n]);
					status=RETURN_WARN;
				}
			}
		}
		if(cond.name)
			free(cond.name);
		FreeArgs(rdargs);
	}
	else
		status=RETURN_ERROR;

	return status;
}
