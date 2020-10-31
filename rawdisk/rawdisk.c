/**
 * \file   rawdisk/rawdisk.c
 * \brief  read and write adf files
 * \author Dietmar Muscholik <d.muscholik@t-online.de>
 * \date   2016-09-06
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
#include <string.h>

#include <ctype.h>

#include <dos/dos.h>
#include <dos/rdargs.h>

#include <pragmas/dos_pragmas.h>
#include <pragmas/exec_pragmas.h>

#include <clib/alib_protos.h>
#include <devices/trackdisk.h>

#include "rawdisk_rev.h"


/*typedef enum {MODE_NONE,MODE_READ,MODE_WRITE} mode_t;*/
enum {MODE_NONE,MODE_READ,MODE_WRITE};

char *verstag=VERSTAG;


static struct IOStdReq *open_device(char *device,ULONG unit)
{
	struct IORequest *request;
	struct MsgPort *port;

	port=CreatePort(NULL,0);
	if(port)
	{
		request=CreateExtIO(port,sizeof(struct IOExtTD));
		if(request)
		{
			if(!OpenDevice(device,unit,request,0))
				return (struct IOStdReq *)request;
			DeleteExtIO(request);
		}
		DeletePort(port);
	}
	return NULL;
}


static int close_device(struct IOStdReq *request)
{
	DeletePort(request->io_Message.mn_ReplyPort);
	CloseDevice((struct IORequest *)request);
	DeleteExtIO((struct IORequest *)request);
	return 0;
}


/*static int raw_disk(mode_t mode,struct IOExtTD *request,BPTR file,*/
static int raw_disk(int mode,struct IOExtTD *request,BPTR file,
					APTR track_buff,ULONG track_size)
{
	int err=0;
	ULONG track_count;
	ULONG count;
	char msg[80];

	request->iotd_Req.io_Flags=0;

	request->iotd_Req.io_Command=TD_GETNUMTRACKS;
	if(DoIO((struct IORequest *)request))
		return (int)request->iotd_Req.io_Error;
	track_count=request->iotd_Req.io_Actual;

	request->iotd_Req.io_Command=TD_CHANGENUM;
	if(DoIO((struct IORequest *)request))
		return (int)request->iotd_Req.io_Error;
	request->iotd_Count=request->iotd_Req.io_Actual;

	/* do the actual work */
	if(mode==MODE_WRITE)
	{
		request->iotd_SecLabel=0;
		request->iotd_Req.io_Data=track_buff;
		request->iotd_Req.io_Length=track_size;
		for(count=0; !err && count<track_count; count++)
		{
			sprintf(msg,"formatting track %3d\r",count);
			Write(Output(),msg,strlen(msg));
			request->iotd_Req.io_Offset=count*track_size;
			request->iotd_Req.io_Command=ETD_FORMAT;
			err=DoIO((struct IORequest *)request);
			if(err)
				break;

			if(Read(file,track_buff,track_size)<0)
				err=-1;
			if(err)
				break;

			sprintf(msg,"writing track    %3d\r",count);
			Write(Output(),msg,strlen(msg));
			request->iotd_Req.io_Command=ETD_WRITE;
			err=DoIO((struct IORequest *)request);
		}

		if(!err)
		{
			request->iotd_Req.io_Command=ETD_UPDATE;
			err=DoIO((struct IORequest *)request);
		}
	}
	else
	{
		request->iotd_Req.io_Command=ETD_READ;
		request->iotd_Req.io_Data=track_buff;
		request->iotd_Req.io_Length=track_size;
		for(count=0; !err && count<track_count; count++)
		{
			sprintf(msg,"reading track    %3d\r",count);
			Write(Output(),msg,strlen(msg));
			request->iotd_Req.io_Offset=count*track_size;
			err=DoIO((struct IORequest *)request);
			if(err)
				break;

			if(Write(file,request->iotd_Req.io_Data,request->iotd_Req.io_Actual)<0)
				err=-1;
		}
	}
	Write(Output(),"\n",1);

	request->iotd_Req.io_Command=TD_MOTOR;
	request->iotd_Req.io_Length=0;
	DoIO((struct IORequest *)request);

	return err;
}


static ULONG get_unit(char *name)
{
	if(!strcmp(name,"df0:") || !strcmp(name,"0"))
		return 0;
	if(!strcmp(name,"df1:") || !strcmp(name,"1"))
		return 1;
	if(!strcmp(name,"df2:") || !strcmp(name,"2"))
		return 2;
	if(!strcmp(name,"df3:") || !strcmp(name,"3"))
		return 3;

	return ~0;
}

enum {ARG_FROM,ARG_TO};


int main(int argc,char **argv)
{
	int status=RETURN_OK;
	struct RDArgs *rdargs;
	LONG args[]={0,0};

	rdargs=ReadArgs("FROM/A,TO/A",args,NULL);
	if(rdargs)
	{
/*		mode_t mode=MODE_NONE;*/
		int mode=MODE_NONE;
		ULONG unit;
		BPTR file;

		unit=get_unit((char *)args[ARG_FROM]);
		if(unit!=~0)
		{
			mode=MODE_READ;
			file=Open((char *)args[ARG_TO],MODE_NEWFILE);
		}
		else
		{
			unit=get_unit((char *)args[ARG_TO]);
			if(unit!=~0)
			{
				mode=MODE_WRITE;
				file=Open((char *)args[ARG_FROM],MODE_OLDFILE);
			}
		}

		if(mode!=MODE_NONE)
		{
			struct IOStdReq *request;

			request=open_device(TD_NAME,unit);
			if(request)
			{
				int err;
				APTR track_buff;

				track_buff=AllocMem(NUMSECS*TD_SECTOR,MEMF_CHIP);
				if(track_buff)
				{
					err=raw_disk(mode,(struct IOExtTD *)request,file,
									track_buff,NUMSECS*TD_SECTOR);
					printf("raw_disk() returned %d\n",err);
					FreeMem(track_buff,NUMSECS*TD_SECTOR);
				}
				else
					status=RETURN_ERROR;
				close_device(request);
			}
			else
				status=RETURN_ERROR;
			Close(file);
		}
		else
		{
			fprintf(stderr,"no device\n");
			status=RETURN_ERROR;
		}

		FreeArgs(rdargs);
	}
	else
		status=RETURN_ERROR;

	return status;
}
