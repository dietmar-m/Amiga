#include <string.h>
#include <stdlib.h>
#include <dos/dos.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/graphics_pragmas.h>

#include "ilbm.h"


typedef UBYTE Masking;
#define mskNone 0L
#define mskHasMask 1L
#define mskHasTransparentColor 2L
#define mskLasso 3L

typedef UBYTE Compression;
#define cmpNone 0L
#define cmpByteRun1 1L

typedef struct {
	UWORD w,h;
	WORD x,y;
	UBYTE nPlanes;
	Masking masking;
	Compression compression;
	UBYTE pad1;
	UWORD transparentColor;
	UBYTE xAspect,yAspect;
	WORD pageWidth,pageHeight;
} bmp_header_t;

typedef struct {
	UBYTE r,g,b;
} color_t;


static LONG WriteFileHeader(BPTR fh,int count)
{
	struct {
		ilbm_header_t head;
		char body[4];
	} buf;

	strncpy(buf.head.id,"FORM",4);
	buf.head.size=count;
	strncpy(buf.body,"ILBM",4);

	return Write(fh,&buf,sizeof(buf));
}

static LONG WriteBmpHeader(BPTR fh,struct RastPort *rp,struct Rectangle *rect)
{
	struct {
		ilbm_header_t head;
		bmp_header_t body;
	} buf;

	strncpy(buf.head.id,"BMHD",4);
	buf.head.size=sizeof(buf.body);

	buf.body.w=rect->MaxX-rect->MinX;
	buf.body.h=rect->MaxY-rect->MinY;
	buf.body.x=rect->MinX;
	buf.body.y=rect->MinY;
	buf.body.nPlanes=rp->BitMap->Depth;
	buf.body.masking=mskNone;
	buf.body.compression=cmpNone;
	buf.body.pad1=0;
	buf.body.transparentColor=0;
	buf.body.xAspect=10L;
	buf.body.yAspect=11L;
	buf.body.pageWidth=buf.body.w;
	buf.body.pageHeight=buf.body.h;

	printf("BMHD: w=%d, h=%d\n",buf.body.w,buf.body.h);

	return Write(fh,&buf,sizeof(buf));
}

static LONG WriteColorMap(BPTR fh,struct ColorMap *cm)
{
	LONG count=0;
	int cm_size=cm->Count*3;
	int buf_size;
	UBYTE *buf;

	if(cm_size%2)
		cm_size++;

	printf("CMAP: colors=%d, size=%d\n",(int)cm->Count,cm_size);

	buf_size=cm_size+sizeof(ilbm_header_t);
	buf=malloc(buf_size);
	if(buf)
	{
		LONG c;
		int i;
		UBYTE *p;

		strncpy(((ilbm_header_t *)buf)->id,"CMAP",4);
		((ilbm_header_t *)buf)->size=cm_size;

		for(i=0, p=buf+sizeof(ilbm_header_t); i<cm->Count; i++, p+=3)
		{
			c=GetRGB4(cm,i);
			*(p+0)=(c>>4) & 0x00f0;
			*(p+1)=(c>>0) & 0x00f0;
			*(p+2)=(c<<4) & 0x00f0;
		}
		count=Write(fh,buf,buf_size);
		free(buf);
	}
	else
		count=-1;
	return count;
}

static LONG WriteColorMap32(BPTR fh,struct ColorMap *cm)
{
	LONG count=0;
	int cm_size=cm->Count*3;
	int buf_size;
	UBYTE *buf;

	if(cm_size%2)
		cm_size++;

	printf("CMAP: colors=%d, size=%d\n",(int)cm->Count,cm_size);

	buf_size=cm_size+sizeof(ilbm_header_t);
	buf=malloc(buf_size);
	if(buf)
	{
		ULONG *colors;
		colors=malloc(cm_size*sizeof(ULONG));

		if(colors)
		{
			int i;
			UBYTE *p;
			ULONG *c;

			strncpy(((ilbm_header_t *)buf)->id,"CMAP",4);
			((ilbm_header_t *)buf)->size=cm_size;

			GetRGB32(cm,0,(ULONG)cm->Count,colors);

			for(i=0, p=buf+sizeof(ilbm_header_t), c=colors;
				i<cm->Count; i++, p+=3, c+=3)
			{
				printf("%d: 0x%08x,0x%08x,0x%08x\n",i,*(c+0),*(c+1),*(c+2));
				*(p+0)=*(c+0)>>24;
				*(p+1)=*(c+1)>>24;
				*(p+2)=*(c+2)>>24;
			}

			count=Write(fh,buf,buf_size);
			free(colors);
		}
		else
			count=-1;
		free(buf);
	}
	else
		count=-1;
	return count;
}

static LONG WriteBody(BPTR fh,struct RastPort *rp,struct Rectangle *rect)
{
	LONG count;
	UBYTE *buf;
	int width,bytes;

	width=rect->MaxX-rect->MinX;
	if(width & 0x07)
		bytes=(width >> 3) + 1;
	else
		bytes=width >> 3;
	if(bytes & 0x01)
		bytes++;

	printf("BytesPerRow=%d, width=%d\n",bytes,width);

	buf=malloc(bytes);
	if(buf)
	{
		ilbm_header_t head;
		struct BitMap *bm=rp->BitMap;

		/* TODO */
		strncpy(head.id,"BODY",4);
		head.size=(rect->MaxY-rect->MinY)*bytes*bm->Depth;

		printf("depth=%d\n",bm->Depth);
		printf("BytesPerRow=%d\n",bm->BytesPerRow);

		count=Write(fh,&head,sizeof(head));
		if(count>0)
		{
			int skip,shift;
			/*UWORD mask=*/
			int row,plane;
			UBYTE *p;
			LONG n;

			skip=rect->MinX >> 3;
			shift=rect->MinX & 0x07;
			printf("skip=%d, shift=%d\n",skip,shift);
			for(row=rect->MinY; count>0 && row<rect->MaxY; row++)
				for(plane=0; count>0 && plane<bm->Depth; plane++)
				{
					p=bm->Planes[plane]+row*bm->BytesPerRow+skip;
					for(n=0; n<bytes; n++)
					{
						buf[n]=*(p+n) << shift;
						buf[n]|=*(p+n+1) >> (8-shift);
					}
/*					n=Write(fh,bm->Planes[plane]+row*bm->BytesPerRow,bytes);*/
					n=Write(fh,buf,bytes);
					if(n>0)
						count+=n;
					else
						count=-1;
				}
		}
		free(buf);
	}
	else
		count=-1;

	return count;
}

static LONG WriteBodyRtg(BPTR fh,struct RastPort *rp,struct Rectangle *rect)
{
	LONG count;
	struct BitMap *bm=rp->BitMap;
	int bytes=rect->MaxX-rect->MinX;
	ilbm_header_t head;
	int row;
	int n;

	if(bytes > bm->BytesPerRow)
		bytes=bm->BytesPerRow;
	if(bytes & 0x01)
		bytes++;

	strncpy(head.id,"BODY",4);
	head.size=(rect->MaxY-rect->MinY)*bytes;
	count=Write(fh,&head,sizeof(head));

	printf("plane0=%p, bytes=%d\n",bm->Planes[0],bytes);

	for(row=rect->MinY; count>0 && row<rect->MaxY; row++)
	{
		/*p=bm->Planes[0]+row*bm->BytesPerRow+rect->MinX;*/
		n=Write(fh,bm->Planes[0]+row*bm->BytesPerRow+rect->MinX,bytes);
		if(n>0)
			count+=n;
		else
			count=-1;
	}

	return count;
}

LONG ilbm_Write(char *name,struct RastPort *rp,
				struct Rectangle *rect,struct ColorMap *cm,
				ULONG mode,
				UBYTE *extra,size_t extra_size)
{
	LONG count=0;
	BPTR fh;

	fh=Open(name,MODE_NEWFILE);
	if(fh)
	{
		LONG n;

		n=WriteFileHeader(fh,0);
		if(n>0)
			count+=n;
		else
			count=-1;

		if(count>0)
		{
			n=WriteBmpHeader(fh,rp,rect);
			if(n>0)
				count+=n;
			else
				count=-1;
		}
#if 1
		if(count>0)
		{
			/* n=WriteColorMap(fh,cm);*/
			n=WriteColorMap32(fh,cm);
			if(n>0)
				count+=n;
			else
				count=-1;
		}
#endif

		if(count>0)
		{
			struct {
				ilbm_header_t head;
				ULONG mode;
			} buf;

			strncpy(buf.head.id,"CAMG",4);
			buf.head.size=sizeof(mode);
			buf.mode=mode;
			n=Write(fh,&buf,sizeof(buf));
			if(n>0)
				count+=n;
			else
				count=-1;

		}

		if(count>0)
		{
			/* n=WriteBody(fh,rp,rect);*/
			n=WriteBodyRtg(fh,rp,rect);
			if(n>0)
				count+=n;
			else
				count=-1;
		}

		if(count>0 && extra)
		{
			if(extra_size%2)
				extra_size++;
			n=Write(fh,extra,extra_size);
			if(n>0)
				count+=n;
			else
				count=-1;
		}

		if(count>0)
		{
			if(Seek(fh,0,OFFSET_BEGINNING)>=0)
				n=WriteFileHeader(fh,count);
			else
				count=-1;
			if(n>0)
				count+=n;
			else
				count=-1;
		}

		Close(fh);
		if(count<0)
			DeleteFile(name);
	}
	else
		count=-1;

	return count;
}


static LONG ReadColorMap(BPTR fh,struct ColorMap *cm,LONG size)
{
	LONG count=0;
	UBYTE *buf;

	buf=malloc(size);
	if(buf)
	{
		LONG n;

		n=Read(fh,buf,size);
		if(n==size)
		{
			count+=n;
		}
		else
			count=-1;
		free(buf);
	}
	else
		count=-1;

	return count;
}

#if 0
static LONG ReadBody(BPTR fh,bmp_header_t *head,
						struct RastPort *rp,struct Rectangle *rect)
{
	LONG count=0;
	size_t bytes;
	UBYTE *buf;
	size_t b;
	UBYTE mask;

	bytes=head->w/8;
	if(bytes%8)
		bytes++;
	if(bytes%2)
		bytes++;

	b=head->w/8;
	if(b%8)
		b++;
	mask=0xff << (8-(head->w%8));

	printf("%d bytes\n",bytes);
	buf=malloc(bytes);
	if(buf)
	{
		struct BitMap *bm=rp->BitMap;
		WORD w,h;
		WORD x0,row0;
		int shift;
		UWORD row;
		UBYTE plane;
		LONG n;
		UBYTE *p;
		size_t c;

		w=(rect->MaxX-rect->MinX)/8;
		if((rect->MaxX-rect->MinX)%8)
			w++;
		h=rect->MaxY-rect->MinY;
		row0=rect->MinY*bm->BytesPerRow;
		x0=rect->MinX/8;
		shift=rect->MinX%8;

		printf("x0=%d, shift=%d\n",x0,shift);

		for(row=0; count>=0 && row<head->h; row++)
			for(plane=0; count>=0 && plane<head->nPlanes; plane++)
			{
				n=Read(fh,buf,bytes);
				if(n==bytes)
					count+=n;
				else
					count=-1;
				if(row<h)
				{
					p=bm->Planes[plane]+row0+row*bm->BytesPerRow+x0;
					*p&=0xff00 >> shift;
					*p|=buf[0] >> shift;
/*					for(c=1; c<bytes && c<w; c++)*/
					for(c=1; c<b && c<w; c++)
/*					for(c=1; c<b-1 && c<w; c++)*/
					{
						*(p+c)=buf[c-1] << (8-shift);
						*(p+c)|=buf[c] >> shift;
					}
/*
					if(bytes>=w)
					{
						*(p+w)&=0x00ff >> shift;
						*(p+w)|=buf[w-1] << (8-shift);
					}
*/
					if(b>=w)
					{
						*(p+c)&=0x00ff >> shift;
/*						*(p+c)|=buf[c-1] << (8-shift);*/
/*						*(p+c)|=(buf[c-1] & mask) << (8-shift);*/
						*(p+c)|=(buf[c-1] << (8-shift)) & mask;
					}
				}
			}

		free(buf);
	}
	else
		count=-1;

	return count;
}
#endif

static LONG ReadBody(BPTR fh,bmp_header_t *head,
						struct RastPort *rp,struct Rectangle *rect)
{
	LONG count=0;
	UBYTE *buf;
	size_t buf_size;

	buf_size=head->w >> 3;
	if(head->w & 0x07)
		buf_size++;

	if(buf_size & 0x01)
		buf_size++;

	buf=malloc(buf_size);
	if(buf)
	{
		struct BitMap *bm=rp->BitMap;
		WORD w,h;
		WORD x0,y0;
		UBYTE shift;
		UBYTE mask;
		UWORD row;
		UBYTE plane;
		LONG n;
		UBYTE *p;


/*		CHANGE ME */
/*
		rect->MinX=8;
		rect->MaxX=rect->MinX+322;
		rect->MaxY=rect->MinY+200;
*/

		w=rect->MaxX-rect->MinX;
		if(w>head->w)
			w=head->w;
		if(w & 0x07)
			w=(w >> 3) + 1;
		else
			w>>=3;

		h=rect->MaxY-rect->MinY;
		x0=rect->MinX >> 3;
		y0=rect->MinY*bm->BytesPerRow;
		shift=rect->MinX & 0x07;
		mask=0xff << (8 - (head->w & 0x07));
		printf("buf_size=%d, w=%d, shift=%d, mask=0x%02x\n",
				buf_size,w,shift,(int)mask);

		for(row=0; count>=0 && row<head->h; row++)
			for(plane=0; count>=0 && plane<head->nPlanes; plane++)
			{
				n=Read(fh,buf,buf_size);
				if(n==buf_size)
					count+=n;
				else
					count=-1;

				if(n>0 && row<h)
				{
					p=bm->Planes[plane]+y0+row*bm->BytesPerRow+x0;
					p[0]&=0xff00 >> shift;
					p[0]|=buf[0] >> shift;
					for(n=1; n<w; n++)
					{
						p[n]=buf[n-1] << (8-shift);
						p[n]|=buf[n] >> shift;
					}

					p[n]&=(0x00ff >> shift);
/*					if(!row)
						printf("n=%d, 0x%02x ",n,(int)p[n]); */
					p[n]|=((buf[n-1] << (8-shift)) & mask);
/*					if(!row)
						printf("0x%02x\n",(int)p[n]); */
				}
			}

		free(buf);
	}
	else
		count=-1;

	return count;
}


LONG ilbm_Read(char *name,struct RastPort *rp,
				struct Rectangle *rect,struct ColorMap *cm,
				UBYTE *extra,size_t extra_size)
{
	LONG count=0;
	BPTR fh;

	printf("file: %s\n",name);
	fh=Open(name,MODE_OLDFILE);
	if(fh)
	{
		struct {
			ilbm_header_t head;
			char body[4];
		} buf;
		LONG n;
		bmp_header_t bmp_header;

		n=Read(fh,&buf,sizeof(buf));
		printf("type=%c%c%c%c, size=%d\n",
				buf.head.id[0],
				buf.head.id[1],
				buf.head.id[2],
				buf.head.id[3],
				buf.head.size);

		printf("file open: count=%d type=%c%c%c%c\n",
				count,buf.body[0],buf.body[1],buf.body[2],buf.body[3]);

		if(n==sizeof(buf) &&
			!strncmp(buf.head.id,"FORM",4) ||
			!strncmp(buf.body,"ILBM",4))
			count+=n;
		else
			count=-1;

		while(count>0)
		{
			n=Read(fh,&buf,sizeof(buf.head));
			if(n==sizeof(buf.head))
			{
				printf("type=%c%c%c%c, size=%d\n",
						buf.head.id[0],
						buf.head.id[1],
						buf.head.id[2],
						buf.head.id[3],
						buf.head.size);
				count+=n;
				if(!strncmp(buf.head.id,"BMHD",4))
				{
					printf("BMHD\n");
					n=Read(fh,&bmp_header,sizeof(bmp_header));
					printf("width=%d, height=%d\n",
							(int)bmp_header.w,(int)bmp_header.h);
					if(n==sizeof(bmp_header))
						count+=n;
					else
						count=-1;
				}
				else if(!strncmp(buf.head.id,"CMAP",4))
				{
					printf("CMAP\n");
					n=ReadColorMap(fh,cm,buf.head.size);
					if(n>0)
						count+=n;
					else
						count=-1;
				}
				else if(!strncmp(buf.head.id,"BODY",4))
				{
					printf("BODY\n");
					n=ReadBody(fh,&bmp_header,rp,rect);
					if(n>0)
						count+=n;
					else
						count=-1;
				}
				else
				{
					printf("unknown\n");
					if(extra)
					{
						size_t c;

						/* CHANGE ME */
						/*buf.head.size++;*/

						memcpy(extra,&buf.head,sizeof(buf.head));
						c=extra_size-sizeof(buf.head)<buf.head.size?
							extra_size:buf.head.size;
						n=Read(fh,extra+sizeof(buf.head),c);
						if(n==c)
						{
							c-=buf.head.size;
							if(c>0 && Seek(fh,c,OFFSET_CURRENT)<0)
								count=-1;
							else
								count+=buf.head.size;
						}
						else
							count=-1;
					}
					else if(Seek(fh,buf.head.size,OFFSET_CURRENT)>0)
						count+=buf.head.size;
					else
						count=-1;
				}
			}
			else if(n<0)
				count=-1;
			else
				break;
		}

		Close(fh);
	}
	else
		count=-1;

	printf("count=%d\n",count);

	return count;
}
