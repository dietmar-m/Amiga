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
