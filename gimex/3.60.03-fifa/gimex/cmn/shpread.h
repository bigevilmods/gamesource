// (c) 2002-2007 Electronic Arts Inc.
#include <stdio.h>
#include <string.h>
#include "codex.h"
#include "dxtcodex.h"

void SHPRDeleteShape(ShapeListPtr theShape);
int SHPRGetShpbpp(SHPHeaderStruct *theShape,int type);
int SHPRclutcolours(CLUTHeaderStruct *theClut,int *startptr,int bpp,int type);
int SHPRReadShapeFile(GSTREAM *g,ShapeListPtr *theShape,int multi,int type,unsigned int signature, unsigned int fntsignature, int shpmipmapdataalignment);
int SHPRGetShapeHotSpot(ShapeListPtr theShape,int IndexFrame,GINFO *ginfo,int type);
void SHPRGetShapeAnim(ShapeListPtr theShape,int IndexFrame,GINFO *ginfo,int type);
void SHPRGetShapeName(ShapeListPtr theShape,int IndexFrame,GINFO *ginfo,int type);
void SHPRGetShapeComment(ShapeListPtr theShape,int IndexFrame,GINFO *ginfo,int type);
void SHPRGetShapeTpage(ShapeListPtr theShape,int IndexFrame,GINFO *ginfo,int type);
void SHPRGetShapeMetal(ShapeListPtr theShape,int IndexFrame,GINFO *ginfo,int type);
void SHPRGetShapeDataBin(ShapeListPtr theShape,int IndexFrame,GINFO *ginfo,int type);
int SHPRReadBitmap(SHPHeaderStruct *HeaderPict,char *MemPtr,int MemRowBytes,int type);
bool SHPRisvalidtype(unsigned char *buf,int type);


int SSF_numchunks(GSTREAM *g, unsigned int shpid, unsigned int fntid, int maxcount);

