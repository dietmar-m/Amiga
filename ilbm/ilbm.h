#ifndef ILBM_H
#define ILBM_H

/*#include <graphics/gfx.h>*/
#include <graphics/rastport.h>
#include <graphics/view.h>

typedef struct {
	char id[4];
	LONG size;
} ilbm_header_t;

LONG ilbm_Write(char *name,struct RastPort *rp,
				struct Rectangle *rect,struct ColorMap *cm,
				ULONG mode,
				UBYTE *extra,size_t extra_size);
LONG ilbm_Read(char *name,struct RastPort *rp,
				struct Rectangle *rect,struct ColorMap *cm,
				UBYTE *extra,size_t extra_size);
#endif
