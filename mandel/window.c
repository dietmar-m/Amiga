#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <graphics/modeid.h>
#include <libraries/gadtools.h>
#include <libraries/asl.h>

#include <pragmas/intuition_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/gadtools_pragmas.h>
#include <pragmas/asl_pragmas.h>

/*#include "mandel.h"*/
#include <ilbm.h>

#include "request.h"
#include "window.h"

/*
static struct ColorSpec colors[]={
	{FILLTEXTPEN+ 0, 0, 0, 0},
	{FILLTEXTPEN+ 1,15,15, 0},
	{FILLTEXTPEN+ 2,15,13, 0},
	{FILLTEXTPEN+ 3,15,11, 0},
	{FILLTEXTPEN+ 4,15, 9, 0},
	{FILLTEXTPEN+ 5,15, 7, 0},
	{FILLTEXTPEN+ 6,15, 5, 0},
	{FILLTEXTPEN+ 7,15, 3, 0},
	{FILLTEXTPEN+ 8,15, 1, 0},
	{FILLTEXTPEN+ 9,13, 0, 0},
	{-1,0,0,0}
};
*/
/*
static ULONG colors32[]={
	(10<<16) + FILLTEXTPEN,
	0,0,0,
	0xf0000000,0xf0000000,0,
	0xf0000000,0xd0000000,0,
	0xf0000000,0xb0000000,0,
	0xf0000000,0x90000000,0,
	0xf0000000,0x70000000,0,
	0xf0000000,0x50000000,0,
	0xf0000000,0x30000000,0,
	0xf0000000,0x10000000,0,
	0xd0000000,0x00000000,0,
	0
};
*/

static struct TagItem screen_tags[]={
	{SA_Width, 0},
	{SA_Height, 0},
	{SA_Depth, 0},
	{SA_Pens, 0},
/*	{SA_Colors, 0},*/
	{SA_Colors32, 0},
	{SA_DisplayID, 0 /*HIRESLACE_KEY */},
	{TAG_DONE,0}
};

static struct TagItem window_tags[]={
	{WA_Left, 0},
	{WA_Top, 0},
	{WA_Width, 0},
	{WA_Height, 0},
	{WA_CustomScreen, 0},
/*	{WA_Title, (ULONG)((CONST_STRPTR)"Mandelbrot")},*/
	{WA_NewLookMenus ,TRUE},
	{WA_CloseGadget,TRUE},
	{WA_DragBar, TRUE},
	{WA_Activate, TRUE},
	{WA_Flags, WFLG_SMART_REFRESH | WFLG_NOCAREREFRESH},
	{WA_IDCMP, IDCMP_CLOSEWINDOW
			| IDCMP_MENUPICK
			| IDCMP_MOUSEBUTTONS
			| IDCMP_MOUSEMOVE
	},
	{TAG_DONE, 0}
};

static struct NewMenu new_menu[]={
	{NM_TITLE, (STRPTR)"Project", NULL, 0, 0, NULL},
	{NM_ITEM,  (STRPTR)"Start",   NULL, 0, 0, NULL},
	{NM_ITEM,  (STRPTR)"Stop",    NULL, NM_ITEMDISABLED, 0, NULL},
	{NM_ITEM,  (STRPTR)"Reset",   NULL, 0, 0, NULL},
	{NM_ITEM,  (STRPTR)"Load",    NULL, 0, 0, NULL},
	{NM_ITEM,  (STRPTR)"Save",    NULL, NM_ITEMDISABLED, 0, NULL},
	{NM_ITEM,  NM_BARLABEL,       NULL, 0, 0, NULL},
	{NM_ITEM,  (STRPTR)"Quit",    NULL, 0, 0, NULL},
	{NM_END,   NULL,              NULL, 0, 0, NULL}
};

static struct TagItem menu_tags[]={
	{GTMN_NewLookMenus, TRUE},
	{TAG_DONE, 0}
};

#define MENUITEM_START FULLMENUNUM(0,0,NOSUB)
#define MENUITEM_STOP  FULLMENUNUM(0,1,NOSUB)
#define MENUITEM_RESET FULLMENUNUM(0,2,NOSUB)
#define MENUITEM_LOAD  FULLMENUNUM(0,3,NOSUB)
#define MENUITEM_SAVE  FULLMENUNUM(0,4,NOSUB)
#define MENUITEM_QUIT  FULLMENUNUM(0,6,NOSUB)


static struct Menu *menu_Create(struct Window *window,APTR *vi)
{
	struct Menu *menu;

	if((menu=CreateMenusA(new_menu,NULL)))
	{
		printf("menu=%p\n",menu);
		*vi=GetVisualInfoA(window->WScreen,NULL);
		if(*vi)
		{
			printf("vi=%p\n",*vi);
			if(LayoutMenusA(menu,*vi,menu_tags))
				return menu;
			FreeVisualInfo(*vi);
		}
		FreeMenus(menu);
	}

	return NULL;
}

static ULONG *colors_Create(UWORD count)
{
	ULONG *map;

	map=malloc((count*3+2)*sizeof(ULONG));
	if(map)
	{
		ULONG r=0xff000000;
		ULONG g=0xff000000;
		ULONG b=0x00000000;
		/*int step=512/count;*/
		ULONG step=(512/count)<<24;
		ULONG *p=map;
		UWORD c;


		*p++=(count<<16)+FILLTEXTPEN;
		*p++=0x00000000;
		*p++=0x00000000;
		*p++=0x00000000;
		for(c=1; g>step && c<count; c++)
		{
/*
			*p++=r<<24;
			*p++=g<<24;
			*p++=b<<24;
*/
			*p++=r;
			*p++=g;
			*p++=b;
			g-=step;
		}
		printf("colors_Create(): c=%d\n",(int)c);

		for(g=0x00000000; r>step && c<count; c++)
		{
/*
			*p++=r<<24;
			*p++=g<<24;
			*p++=b<<24;
*/
			*p++=r;
			*p++=g;
			*p++=b;
			r-=step;
		}

		printf("colors_Create(): step=0x%08x, c=%d, count=%d\n",
				step,(int)c,(int)count);

		for(; c<count; *p++=0, c++);
		*p=0;
	}

	return map;
}

struct Window *window_Create(int colors)
{
	struct Window *window=NULL;
	struct Screen *wb_screen;

	if((wb_screen=LockPubScreen(NULL)))
	{
		struct DrawInfo *di;

		if((di=GetScreenDrawInfo(wb_screen)))
		{
			struct Screen *screen;
			int max_colors=(1<<di->dri_Depth)-FILLTEXTPEN;

			printf("colors=%d, max_colors=%d\n",colors,max_colors);

			screen_tags[0].ti_Data=wb_screen->Width;
			screen_tags[1].ti_Data=wb_screen->Height;
			screen_tags[2].ti_Data=di->dri_Depth;
			screen_tags[3].ti_Data=(ULONG)di->dri_Pens;
			/* screen_tags[4].ti_Data=(ULONG)colors; */
			/* screen_tags[4].ti_Data=(ULONG)colors32; */
			/* screen_tags[4].ti_Data=(ULONG)colors_Create((1<<di->dri_Depth)-FILLTEXTPEN);*/
			screen_tags[4].ti_Data=(ULONG)colors_Create((UWORD)
														(colors>max_colors ?
														max_colors : colors));

			screen_tags[5].ti_Data=GetVPModeID(&wb_screen->ViewPort);

			printf("width=%d, height=%d, mode=0x%08x\n",
					wb_screen->Width,wb_screen->Height,screen_tags[5].ti_Data);
			printf("depth=%d\n",di->dri_Depth);

			screen=OpenScreenTagList(NULL,screen_tags);
			FreeScreenDrawInfo(wb_screen,di);
			free((void *)screen_tags[4].ti_Data);

			if(screen)
			{
/*
				int i;

				printf("%d colors\n",screen->ViewPort.ColorMap->Count);
				for(i=0; i<screen->ViewPort.ColorMap->Count; i++)
					printf("color%d=%08x\n",i,GetRGB4(screen->ViewPort.ColorMap,i));
*/
				window_tags[1].ti_Data=screen->BarHeight-screen->BarVBorder+2;
				window_tags[2].ti_Data=screen->Width;
				window_tags[3].ti_Data=screen->Height-window_tags[1].ti_Data;
				window_tags[4].ti_Data=(ULONG)screen;

				window=OpenWindowTagList(NULL,window_tags);
				if(window)
				{
					struct Menu *menu=menu_Create(window,&(APTR)window->UserData);

					printf("menu=%p\n",menu);
					if(!menu)
					{
						CloseWindow(window);
						window=NULL;
					}
					else
						SetMenuStrip(window,menu);
				}
				if(!window)
					CloseScreen(screen);

			}
		}
		UnlockPubScreen(NULL,wb_screen);
	}

	return window;
}

int window_Delete(struct Window *window)
{
	struct Menu *menu=window->MenuStrip;
	struct Screen *screen=window->WScreen;
	APTR vi=window->UserData;

	ClearMenuStrip(window);
	if(menu)
		FreeMenus(menu);
	CloseWindow(window);
	if(vi)
		FreeVisualInfo(vi);
	CloseScreen(screen);

	return 0;
}

static void window_Clear(struct Window *window)
{
	SetAPen(window->RPort,FILLTEXTPEN+0);
	RectFill(window->RPort,
			(LONG)window->BorderLeft,
			(LONG)window->BorderTop,
			(LONG)window->Width-window->BorderRight-1,
			(LONG)window->Height-window->BorderBottom-1);
}


const char *screen_title="Processing...%s";
const char *window_title="%lf%cj%lf - %lf%cj%lf";

typedef enum {
	STATE_NONE,
	STATE_PROCESSING,
	STATE_STOPPED,
	STATE_DONE
} state_t;

state_t window_ChangeState(struct Window *win,state_t state)
{
	static char title[32];

	switch(state)
	{
	case STATE_NONE:
		break;

	case STATE_PROCESSING:
		OffMenu(win,MENUITEM_START);
		OnMenu(win,MENUITEM_STOP);
		sprintf(title,screen_title,"");
		SetWindowTitles(win,(char *)-1,title);
		break;

	case STATE_STOPPED:
		OnMenu(win,MENUITEM_START);
		OffMenu(win,MENUITEM_STOP);
		sprintf(title,screen_title,"stopped");
		SetWindowTitles(win,(char *)-1,title);
		break;

	case STATE_DONE:
		OnMenu(win,MENUITEM_START);
		OffMenu(win,MENUITEM_STOP);
		OnMenu(win,MENUITEM_SAVE);
		sprintf(title,screen_title,"done");
		SetWindowTitles(win,(char *)-1,title);
		break;

	default:
		break;
	}
	return state;
}

static void window_DrawRect(struct RastPort *rp,Point *p1,Point *p2)
{
	Move(rp,(LONG)p1->x,(LONG)p1->y);
	Draw(rp,(LONG)p2->x,(LONG)p1->y);
	Draw(rp,(LONG)p2->x,(LONG)p2->y);
	Draw(rp,(LONG)p1->x,(LONG)p2->y);
	Draw(rp,(LONG)p1->x,(LONG)p1->y);
}

static BOOL window_Select(struct Window *win,Point *p1,Point *p2)
{
	BOOL ok=FALSE;
	BOOL done;
	struct IntuiMessage *msg;
	
	ReportMouse1(win,TRUE);
	SetDrMd(win->RPort,COMPLEMENT);
	window_DrawRect(win->RPort,p1,p2);

	for(done=FALSE; !done;)
	{
		WaitPort(win->UserPort);
		msg=(struct IntuiMessage *)GetMsg(win->UserPort);
		switch(msg->Class)
		{
		case IDCMP_MOUSEMOVE:
			window_DrawRect(win->RPort,p1,p2);
			p2->x=msg->MouseX;
			p2->y=msg->MouseY;
			window_DrawRect(win->RPort,p1,p2);
			break;
		case IDCMP_MOUSEBUTTONS:
			if(msg->Code==SELECTUP &&
				(p2->x-p1->x > 4 || p1->x-p2->x > 4) &&
				(p2->y-p1->y > 4 || p1->y-p2->y > 4))
				ok=TRUE;
			done=TRUE;
			break;
		default:
			done=TRUE;
		}
		ReplyMsg((struct Message *)msg);
	}

	window_DrawRect(win->RPort,p1,p2);
	SetDrMd(win->RPort,JAM1);
	ReportMouse1(win,FALSE);

	return ok;
}

#define PATH_MAX 256

static BOOL window_Load(struct Window *win,mandel_t *mandel)
{
#if 0
	struct Rectangle rect;
	struct {
		ilbm_header_t head;
		char body[80];
	} extra;

	rect.MinX=win->LeftEdge+win->BorderLeft;
	rect.MinY=win->TopEdge+win->BorderTop;
	rect.MaxX=win->LeftEdge+win->Width-win->BorderRight;
	rect.MaxY=win->TopEdge+win->Height-win->BorderBottom;

	extra.head.size=0;

	if(ilbm_Read("mandel.ilbm",
				win->RPort,
				&rect,
				win->WScreen->ViewPort.ColorMap,
				(UBYTE *)&extra,sizeof(extra)) > 0)
	{
		if(extra.head.size>0)
		{
			printf("MAND=%s\n",extra.body);
			printf("size=%d\n",extra.head.size);
			sscanf(extra.body,"min.r=%lf,min.i=%lf,max.r=%lf,max.i=%lf",
					&mandel->min.r,&mandel->min.i,
					&mandel->max.r,&mandel->max.i);
			printf("min.r=%lf,min.i=%lf,max.r=%lf,max.i=%lf\n",
					mandel->min.r,mandel->min.i,
					mandel->max.r,mandel->max.i);

		}

		return TRUE;
	}
	return FALSE;
#endif
	BOOL ok=TRUE;
	char path[PATH_MAX];

	if(request_File(win,(BOOL)FALSE,path,sizeof(path)))
	{
		struct Rectangle rect;
		struct {
			ilbm_header_t head;
			char body[80];
		} extra;
		struct TagItem ptr_tags[]={
			{WA_BusyPointer,0},
			{TAG_DONE,0}
		};

		rect.MinX=win->LeftEdge+win->BorderLeft;
		rect.MinY=win->TopEdge+win->BorderTop;
		rect.MaxX=win->LeftEdge+win->Width-win->BorderRight;
		rect.MaxY=win->TopEdge+win->Height-win->BorderBottom;

		extra.head.size=0;

		ptr_tags[0].ti_Data=TRUE;
		SetWindowPointerA(win,ptr_tags);
		if(ilbm_Read(path,win->RPort,
						&rect,win->WScreen->ViewPort.ColorMap,
						(UBYTE *)&extra,sizeof(extra)) > 0)
		{
			if(extra.head.size>0)
			{
				printf("MAND=%s\n",extra.body);
				printf("size=%d\n",extra.head.size);
				sscanf(extra.body,"min.r=%lf,min.i=%lf,max.r=%lf,max.i=%lf",
						&mandel->min.r,&mandel->min.i,
						&mandel->max.r,&mandel->max.i);
				printf("min.r=%lf,min.i=%lf,max.r=%lf,max.i=%lf\n",
						mandel->min.r,mandel->min.i,
						mandel->max.r,mandel->max.i);
			}
		}
		else
			ok=FALSE;
		ptr_tags[0].ti_Data=FALSE;
		SetWindowPointerA(win,ptr_tags);
	}
	else
		ok=FALSE;

	return ok;
}

static BOOL window_Save(struct Window *win,mandel_t *mandel)
{
#if 0
	BOOL ok=FALSE;
	struct TagItem req_tags[]={
		{ASLFR_Window,0},
		{ASLFR_SleepWindow,TRUE},
		{ASLFR_DoSaveMode,TRUE},
		{TAG_DONE,0}
	};
	struct FileRequester *request;

	req_tags[0].ti_Data=(LONG)win;
	request=AllocAslRequest(ASL_FileRequest,req_tags);
	if(request)
	{
		if(AslRequest(request,NULL) && *request->fr_File)
		{
			char path[PATH_MAX];
			size_t n;
			struct Rectangle rect;
			struct TagItem ptr_tags[]={
				{WA_BusyPointer,0},
				{TAG_DONE,0}
			};
			struct {
				ilbm_header_t head;
				char body[80];
			} extra;

			strcpy(path,request->fr_Drawer);
			n=strlen(path);
			if(n && path[n-1]!=':' && path[n-1]!='/')
				strcat(path,"/");
			strcat(path,request->fr_File);
			printf("%s\n",path);
/*			printf("%s/%s\n",request->fr_Drawer,request->fr_File);*/
/*			ok=TRUE;*/

			rect.MinX=win->LeftEdge+win->BorderLeft;
			rect.MinY=win->TopEdge+win->BorderTop;
			rect.MaxX=win->LeftEdge+win->Width-win->BorderRight;
			rect.MaxY=win->TopEdge+win->Height-win->BorderBottom;

			sprintf(extra.body,
					"min.r=%lf,min.i=%lf,max.r=%lf,max.i=%lf,i_max=%d",
					/* "%lf,%lf,%lf,%lf,%d", */
					/* "%lf %lf %lf %lf %d", */
					mandel->min.r,mandel->min.i,
					mandel->max.r,mandel->max.i,
					mandel->i_max);
			strncpy(extra.head.id,"MAND",4);
			extra.head.size=strlen(extra.body)+1;
			if(extra.head.size%2)
				extra.head.size++;

			ptr_tags[0].ti_Data=TRUE;
			SetWindowPointerA(win,ptr_tags);
			if(ilbm_Write(path,win->RPort,
				&rect,win->WScreen->ViewPort.ColorMap,
				(UBYTE *)&extra,extra.head.size+sizeof(extra.head)) > 0)
				ok=TRUE;
			ptr_tags[0].ti_Data=FALSE;
			SetWindowPointerA(win,ptr_tags);
		}
		FreeAslRequest(request);
	}
	return ok;
#endif
	BOOL ok=TRUE;
	char path[PATH_MAX];

	if(request_File(win,(BOOL)TRUE,path,sizeof(path)))
	{
		struct Rectangle rect;
		struct TagItem ptr_tags[]={
			{WA_BusyPointer,0},
			{TAG_DONE,0}
		};
		struct {
			ilbm_header_t head;
			char body[80];
		} extra;

		rect.MinX=win->LeftEdge+win->BorderLeft;
		rect.MinY=win->TopEdge+win->BorderTop;
		rect.MaxX=win->LeftEdge+win->Width-win->BorderRight;
		rect.MaxY=win->TopEdge+win->Height-win->BorderBottom;

		sprintf(extra.body,
				"min.r=%lf,min.i=%lf,max.r=%lf,max.i=%lf,i_max=%d",
				mandel->min.r,mandel->min.i,
				mandel->max.r,mandel->max.i,
				mandel->i_max);
		strncpy(extra.head.id,"MAND",4);
		extra.head.size=strlen(extra.body)+1;
		if(extra.head.size%2)
			extra.head.size++;

		ptr_tags[0].ti_Data=TRUE;
		SetWindowPointerA(win,ptr_tags);
		if(ilbm_Write(path,win->RPort,
			&rect,win->WScreen->ViewPort.ColorMap,
			GetVPModeID(&win->WScreen->ViewPort),
			(UBYTE *)&extra,extra.head.size+sizeof(extra.head)) < 0)
			ok=FALSE;
		ptr_tags[0].ti_Data=FALSE;
		SetWindowPointerA(win,ptr_tags);
	}
	else
		ok=FALSE;

	return ok;
}

int window_Process(struct Window *window,mandel_t *man)
{
	char buffer[80];
	BOOL done;
	struct IntuiMessage *msg;
	ULONG class,code;
	WORD mx,my;
	state_t state=STATE_NONE;
	int row_max=window->Height-window->BorderTop-window->BorderBottom-1;
	int row=0;
	Point p1,p2;

	window_Clear(window);
	sprintf(buffer,window_title,
			man->min.r, man->min.i<0 ? '-' : '+',fabs(man->min.i),
			man->max.r, man->max.i<0 ? '-' : '+',fabs(man->max.i));
	SetWindowTitles(window,buffer,(char *)-1);

	for(done=FALSE; !done;)
	{
		if(state!=STATE_PROCESSING)
			WaitPort(window->UserPort);
		msg=(struct IntuiMessage *)GetMsg(window->UserPort);
		if(msg)
		{
			class=msg->Class;
			code=msg->Code;
			mx=msg->MouseX;
			my=msg->MouseY;
			ReplyMsg((struct Message *)msg);

			switch(class)
			{
			case IDCMP_MENUPICK:
				printf("IDCMP_MENUPICK\n");
				printf("code=%08x\n",code);
				switch(code)
				{
				case MENUITEM_START:
					printf("START\n");
					if(state==STATE_DONE)
						window_Clear(window);
					state=window_ChangeState(window,(state_t)STATE_PROCESSING);
					break;

				case MENUITEM_STOP:
					printf("STOP\n");
					state=window_ChangeState(window,(state_t)STATE_STOPPED);
					break;

				case MENUITEM_RESET:
					printf("RESET\n");
					man->min.r=MANDEL_DEFAULT_MIN_RE;
					man->min.i=MANDEL_DEFAULT_MIN_IM;
					man->max.r=MANDEL_DEFAULT_MAX_RE;
					man->max.i=MANDEL_DEFAULT_MAX_IM;
					sprintf(buffer,window_title,
							man->min.r, man->min.i<0 ? '-' : '+',fabs(man->min.i),
							man->max.r, man->max.i<0 ? '-' : '+',fabs(man->max.i));
					SetWindowTitles(window,buffer,(char *)-1);
					window_Clear(window);
					row=0;
					break;

				case MENUITEM_LOAD:
					printf("LOAD\n");
					if(window_Load(window,man))
					{
						sprintf(buffer,window_title,
								man->min.r, man->min.i<0 ? '-' : '+',fabs(man->min.i),
								man->max.r, man->max.i<0 ? '-' : '+',fabs(man->max.i));
						SetWindowTitles(window,buffer,(char *)-1);
						state=window_ChangeState(window,(state_t)STATE_DONE);
						row=0;
					}
					break;

				case MENUITEM_SAVE:
					printf("SAVE\n");
					if(window_Save(window,man))
						OffMenu(window,MENUITEM_SAVE);
					break;

				case MENUITEM_QUIT:
					printf("QUIT\n");
					done=TRUE;
					break;
				}
				break;

			case IDCMP_CLOSEWINDOW:
				printf("IDCMP_CLOSEWINDOW\n");
				done=TRUE;
				break;

			case IDCMP_MOUSEBUTTONS:
				printf("code=%08x\n",code);
				printf("mouse: %d,%d\n",mx,my);
				if(code==SELECTDOWN && 
					(state==STATE_STOPPED || state==STATE_DONE))
				{
					p1.x=p2.x=mx;
					p1.y=p2.y=my;
					if(window_Select(window,&p1,&p2))
					{
						int w=window->Width
								-window->BorderLeft
								-window->BorderRight;
						int h=window->Height
								-window->BorderTop
								-window->BorderBottom;
						double dr=man->max.r-man->min.r;
						double di=man->max.i-man->min.i;

						man->max.r=man->min.r+dr*(p2.x-window->BorderLeft)/w;
						man->max.i=man->min.i+di*(p2.y-window->BorderTop)/h;

						man->min.r+=dr*(p1.x-window->BorderLeft)/w;
						man->min.i+=di*(p1.y-window->BorderTop)/h;

						sprintf(buffer,window_title,
								man->min.r, man->min.i<0 ? '-' : '+',fabs(man->min.i),
								man->max.r, man->max.i<0 ? '-' : '+',fabs(man->max.i));
						SetWindowTitles(window,buffer,(char *)-1);
						window_Clear(window);
						state=window_ChangeState(window,(state_t)STATE_PROCESSING);
						row=0;
					}
				}
				break;

			default:
				break;
			}
		}

		if(state==STATE_PROCESSING)
		{
			mandel_calcrow(window,man,row++);
			if(row == row_max)
			{
				state=window_ChangeState(window,(state_t)STATE_DONE);
				row=0;
			}
		}
	}

	return 0;
}
