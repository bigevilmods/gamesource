// (c) 1994-2007 Electronic Arts Inc.

#include "shpwrite.h"

/****************************************************************/
/*  Export Functions                                            */
/****************************************************************/

int GCALL SHP_wopen(GINSTANCE **gxptr, GSTREAM *g, const char *pathname, int numframes)
{
    int            TotalSize;
    ShapeListPtr    gShape=0L;
    GINSTANCE       *gx;
    int ok=0;

    TotalSize= ((sizeof(GShapeInfo)*(numframes))+sizeof(int));

    gShape=(ShapeListPtr) galloc(TotalSize);
    if (gShape)
    {
        memset(gShape,0,TotalSize);
        gShape->NumShape = (int) numframes;
        gx = (GINSTANCE *) galloc(sizeof(GINSTANCE));
        if (gx)
        {
            memset(gx,0,sizeof(GINSTANCE));
            gx->frames=numframes;
            gx->signature=SHPSIGNATURE;
            gx->size=sizeof(GINSTANCE);
            gx->framenum=0;
            gx->gstream=g;
            gx->gref=(char*)gShape;
            *gxptr=gx;
            ok = 1;
        }
    }
    return(ok);
}


int GCALL SHP_write(GINSTANCE *gx, const GINFO *ginfo, char *source, int rowbytes)
{
    ShapeListPtr gShape=0L;
    int ok=0;

    if (gx->framenum < gx->frames)
    {
        gShape=(ShapeListPtr)gx->gref;
        ok = SHPWAddShape(gShape,ginfo,source,rowbytes,gx->framenum,SHPMAKECODE%TYPEMAX,SHPMAKECODE/TYPEMAX);
        ++gx->framenum;
    }
    return(ok);
}


int GCALL SHP_wclose(GINSTANCE *gx)
{
    ShapeListPtr    gShape=0L;
    int             ReturnValue=0;

    if (gx)
    {
        gShape=(ShapeListPtr)gx->gref;
        gShape->NumShape = gx->framenum;
        ReturnValue=SHPWWriteShapeFile(gx->gstream,gShape,SHPMAKECODE%TYPEMAX,SHPMAKECODE/TYPEMAX,SHPMULTIFRAME,SHPSIGNATURE, SHPDATAALIGNMENT, SHPCLUTDATAALIGNMENT, SHPALIGNMENT, SHPMIPMAPDATAALIGNMENT, SHPPACKEDDATAALIGNMENT);
        SHPWFreeShapes(gShape);
        gfree((void*)gx);
    }
    return(ReturnValue);
}

/* cleanup macros */

#undef SHP_about
#undef SHP_is
#undef SHP_open
#undef SHP_info
#undef SHP_read
#undef SHP_close
#undef SHP_wopen
#undef SHP_write
#undef SHP_wclose

#undef SHPSIGNATURE
#undef FNTSIGNATURE
#undef SHPMULTIFRAME
#undef SHPEXTENSION0
#undef SHPEXTENSION1
#undef SHPEXTENSION2
#undef SHPEXTENSION3
#undef SHPMACTYPE0
#undef SHPMACTYPE1
#undef SHPMACTYPE2
#undef SHPMACTYPE3
#undef SHPSHORTTYPE
#undef SHPWORDTYPE
#undef SHPLONGTYPE
#undef SHPALIGNMENT
#undef SHPDATAALIGNMENT
#undef SHPMIPMAPDATAALIGNMENT
#undef SHPCLUTDATAALIGNMENT
#undef SHPPACKEDDATAALIGNMENT

#undef ggetx
#undef gputx
#undef SHPGETFIELD
#undef SHPPUTFIELD
#undef SHPALIGN
#undef SHPMIPMAPALIGN

#undef SHPMAKECODE

