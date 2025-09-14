// (c) 1994-2007 Electronic Arts Inc.

#include "oldshpwrite.h"

/****************************************************************/
/*  Export Functions                                            */
/****************************************************************/

int GCALL OLDSHP_wopen(GINSTANCE **gxptr, GSTREAM *g, const char *pathname, int numframes)
{
    int            TotalSize;
    OLDShapeListPtr    oLdgshape=0L;
    GINSTANCE       *gx;
    int ok=0;

    TotalSize= ((sizeof(OLDgshapeInfo)*(numframes))+sizeof(int));

    oLdgshape=(OLDShapeListPtr) galloc(TotalSize);
    if (oLdgshape)
    {
        memset(oLdgshape,0,TotalSize);
        oLdgshape->NuoLdmshape = (int) numframes;
        gx = (GINSTANCE *) galloc(sizeof(GINSTANCE));
        if (gx)
        {
            memset(gx,0,sizeof(GINSTANCE));
            gx->frames=numframes;
            gx->signature=OLDSHPSIGNATURE;
            gx->size=sizeof(GINSTANCE);
            gx->framenum=0;
            gx->gstream=g;
            gx->gref=(char*)oLdgshape;
            *gxptr=gx;
            ok = 1;
        }
    }
    return(ok);
}


int GCALL OLDSHP_write(GINSTANCE *gx, const GINFO *ginfo, char *source, int rowbytes){
    OLDShapeListPtr oLdgshape=0L;
    int ok=0;

#if OLDSHPMAKECODE==3
    if (ginfo->bpp<=8)
#endif
    {
        if (gx->framenum < gx->frames)
        {
            oLdgshape=(OLDShapeListPtr)gx->gref;
            ok = OLDSHPWAddShape(oLdgshape,ginfo,source,rowbytes,gx->framenum,OLDSHPMAKECODE%TYPEOLDMAX,OLDSHPMAKECODE/TYPEOLDMAX);
            ++gx->framenum;
        }
    }
    return(ok);
}


int GCALL OLDSHP_wclose(GINSTANCE *gx)
{
    OLDShapeListPtr    oLdgshape=0L;
    int             ReturnValue=0;

    if (gx)
    {
        oLdgshape=(OLDShapeListPtr)gx->gref;
        oLdgshape->NuoLdmshape = gx->framenum;
        ReturnValue=OLDSHPWWriteShapeFile(gx->gstream,oLdgshape,OLDSHPMAKECODE%TYPEOLDMAX,OLDSHPMAKECODE/TYPEOLDMAX,OLDSHPMULTIFRAME,OLDSHPSIGNATURE, OLDSHPDATAALIGNMENT, OLDSHPCLUTDATAALIGNMENT, OLDSHPALIGNMENT, OLDSHPMIPMAPDATAALIGNMENT, OLDSHPPACKEDDATAALIGNMENT);
        OLDSHPWFreeShapes(oLdgshape);
        gfree((void*)gx);
    }
    return(ReturnValue);
}

/* cleanup macros */

#undef OLDSHP_about
#undef OLDSHP_is
#undef OLDSHP_open
#undef OLDSHP_info
#undef OLDSHP_read
#undef OLDSHP_close
#undef OLDSHP_wopen
#undef OLDSHP_write
#undef OLDSHP_wclose

#undef OLDSHPSIGNATURE
#undef OLDFNTSIGNATURE
#undef OLDSHPMULTIFRAME
#undef OLDSHPEXTENSION0
#undef OLDSHPEXTENSION1
#undef OLDSHPEXTENSION2
#undef OLDSHPEXTENSION3
#undef OLDSHPMACTYPE0
#undef OLDSHPMACTYPE1
#undef OLDSHPMACTYPE2
#undef OLDSHPMACTYPE3
#undef OLDSHPSHORTTYPE
#undef OLDSHPWORDTYPE
#undef OLDSHPLONGTYPE
#undef OLDSHPALIGNMENT
#undef OLDSHPDATAALIGNMENT
#undef OLDSHPMIPMAPDATAALIGNMENT
#undef OLDSHPCLUTDATAALIGNMENT
#undef OLDSHPPACKEDDATAALIGNMENT

#undef OLDggetx
#undef OLDgputx
#undef OLDSHPGETFIELD
#undef OLDSHPPUTFIELD
#undef OLDSHPALIGN
#undef OLDSHPMIPMAPALIGN

#undef OLDSHPMAKECODE

