// (c) 2002-2007 Electronic Arts Inc.
#include <stdio.h>
#include <string.h>
#include "codex.h"
#include "dxtcodex.h"

void OLDSHPRDeleteShape(OLDShapeListPtr theShape);
int OLDSHPRGetOldshpbpp(OLDSHPHeaderStruct *theShape,int type);
int OLDSHPRclutcolours(OLDClutHeaderStruct *theClut,int *startptr,int bpp,int type);
int OLDSHPRReadShapeFile(GSTREAM *g,OLDShapeListPtr *theShape,int multi,int type,unsigned int signature, unsigned int oldfntsignature, int oldshpmipmapdataalignment);
int OLDSHPRGetShapeHotSpot(OLDShapeListPtr theShape,int IndexFrame,GINFO *ginfo,int type);
void OLDSHPRGetShapeAnim(OLDShapeListPtr theShape,int IndexFrame,GINFO *ginfo,int type);
void OLDSHPRGetShapeName(OLDShapeListPtr theShape,int IndexFrame,GINFO *ginfo,int type);
void OLDSHPRGetShapeComment(OLDShapeListPtr theShape,int IndexFrame,GINFO *ginfo,int type);
void OLDSHPRGetShapeTpage(OLDShapeListPtr theShape,int IndexFrame,GINFO *ginfo,int type);
void OLDSHPRGetShapeMetal(OLDShapeListPtr theShape,int IndexFrame,GINFO *ginfo,int type);
int OLDSHPRReadBitmap(OLDSHPHeaderStruct *HeaderPict,char *MemPtr,int MemRowBytes,int type);
bool OLDSHPRisvalidtype(unsigned char *buf,int type);


int OLDSSF_numchunks(GSTREAM *g, unsigned int oldshpid, unsigned int oldfntid, int maxcount);

