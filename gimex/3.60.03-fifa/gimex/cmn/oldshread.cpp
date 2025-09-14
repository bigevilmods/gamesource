// (c) 1994-2007 Electronic Arts Inc.

#include "oldshpread.h"

// for getenv
#include <stdlib.h>

/****************************************************************/
/*  Information Functions                                       */
/****************************************************************/

#if OLDSHPMAKECODE==3
#define DOES32 0
#else
#define DOES32 1
#endif

GABOUT * GCALL OLDSHP_about(void)
{
    GABOUT *info;

    info = (GABOUT *) galloc(sizeof(GABOUT));
    if (info)
    {
        memset(info,0,sizeof(GABOUT));
        info->signature         = GMAKEID('S','H','P',OLDSHPSIGNATURE&255);
        info->size              = sizeof(GABOUT);
        info->version           = GIMEX_VERSION;
        info->canimport         = 1;    /* supports importing */
        info->canexport         = 1;    /* supports exporting */
        info->importpacked      = 3;    /* supports importing packed */
        info->exportpacked      = 3;    /* supports exporting packed */
        info->import8           = 1;    /* supports importing 8 bpp indexed */
        info->export8           = 1;    /* supports exporting 8 bpp indexed */
        info->import32          = DOES32; /* supports importing 32 bpp direct rgb */
        info->export32          = DOES32; /* supports exporting 32 bpp direct rgb */
        info->multiframe        = OLDSHPMULTIFRAME; /* supports multiple frames */
        info->multifile         = 0;    /* format requires additional files or resource fork */
        info->multisize         = 1;    /* supports each frame different size */
        info->framebuffer       = 1;    /* module requires memory to buffer entire frame */
        info->external          = 0;    /* module uses external resources */
        info->usesfile          = 1;    /* module is file based */
        info->mipmap            = 1;    /* module supports mipmaps */
        info->maxframename      = 512-8;/* max frame name character 512 - 1 null -1 (-1) -4 shapeid - 1 null */
        info->defaultquality    = 75;   /* default pack quality for MG */
        info->firstextension    = 1;    /* use first extension for filenames */
#if OLDSHPMAKECODE/10==STREAMINOLDGSHAPE
        info->movie           = 1;      /* movie file format */
#endif

        info->mactype[0]      = OLDSHPMACTYPE0; /* mac file system types used */
        info->mactype[1]      = OLDSHPMACTYPE1;
        info->mactype[2]      = OLDSHPMACTYPE2;
        info->mactype[3]      = OLDSHPMACTYPE3;
		
		bool removeext=false;
		if (getenv("GIMEX_DEPRECIATEOLDSHAPE") || getenv("GIMEX_DISABLEOLDSHAPE") || getenv("GIMEX_DOS"))
			removeext = true;

		if (removeext)
		{
			strcpy(info->extensions[0], OLDSHPEXTENSION1);     /* extensions */
	        strcpy(info->extensions[1], OLDSHPEXTENSION2);
		    strcpy(info->extensions[2], OLDSHPEXTENSION3);
		}
		else
		{
	        strcpy(info->extensions[0], OLDSHPEXTENSION0);     /* extensions */
		    strcpy(info->extensions[1], OLDSHPEXTENSION1);
	        strcpy(info->extensions[2], OLDSHPEXTENSION2);
		    strcpy(info->extensions[3], OLDSHPEXTENSION3);
		}
        strcpy(info->authorstr,     "F Barchard");      /* authors */
        sprintf(info->versionstr,   "%.2f",OLDSHP_VERSION/100.0f); /* version # */
        strcpy(info->shorttypestr,  OLDSHPSHORTTYPE);      /* type */
        strcpy(info->wordtypestr,   OLDSHPWORDTYPE);       /* wordtype */
        strcpy(info->longtypestr,   OLDSHPLONGTYPE );      /* longtype */
    }
    return(info);
}

#undef DOES32

int GCALL OLDSHP_is(GSTREAM *g)
{
    unsigned char oldshpheader[16];
    int ok=0;

    gseek(g,(GPOS)0);
    if (gread(g,oldshpheader,16) == 16)
    {

#if OLDSHPMAKECODE==TYPEOLDVSH
        int shapefilelen;
        int num;
        int flen;

        flen = (int)glen(g);
        shapefilelen = ggeti(oldshpheader,4);
        num          = ggeti(oldshpheader+4,2);
        if (num>0
         && flen>=(6+num*8+num*16)
         && (unsigned int) (shapefilelen-flen+16) < (unsigned int) 32
         && num<=1500)
        {
            ok = 50;
            if (shapefilelen==flen
             && num<=64)
                ok = 100;
        }

#else

#if OLDSHPMULTIFRAME
        if (OLDSHPSIGNATURE && ggetm(oldshpheader,4)==OLDSHPSIGNATURE)
        {
            ok = 100;
        }
        else if (OLDFNTSIGNATURE && ggetm(oldshpheader,4)==OLDFNTSIGNATURE)
        {
            ok = 98; // lower than font or streaming shape
        }

// if chunk based, skip over audio chunks
#if OLDSHPMAKECODE/10==STREAMINOLDGSHAPE
        else
        {
            gseek(g,(GPOS)0);
            if (OLDSSF_numchunks(g,OLDSHPSIGNATURE,OLDFNTSIGNATURE,4)) // max 4 chunks before we give up
                ok = 99; // so we can tell it had audio
            else
            {
                int next;
                int flen;
                unsigned char nextheader[16];

                flen = (int)glen(g);
                next = OLDggetx(oldshpheader+1,3);
                if ((unsigned int)next < (unsigned int) flen)
                {
                    gseek(g,next);
                    if (gread(g,nextheader,8) == 8)
                    {
						int validity = OLDSHPRisvalidtype(oldshpheader,OLDSHPMAKECODE);
						if (validity
						 && OLDSHPRisvalidtype(nextheader,OLDSHPMAKECODE)
						 && ggeti(oldshpheader+4,2)  // width
						 && ggeti(oldshpheader+6,2)) // height
						{
							ok = 10+validity;
						}
                    }
				}
            }
        }
#endif

#else

        {
            int next;
            int flen;
            unsigned char nextheader[16];

            if (OLDSHPSIGNATURE && ggetm(oldshpheader,4)==OLDSHPSIGNATURE)
            {
                ok = 95;
            }
            else
            {
                flen = (int)glen(g);
                next = OLDggetx(oldshpheader+1,3);
                if ((unsigned int)next < (unsigned int) flen)
                {
                    gseek(g,next);
                    if (gread(g,nextheader,8) == 8)
                    {
                        if (OLDSHPMAKECODE && OLDSHPRisvalidtype(oldshpheader,OLDSHPMAKECODE)
                         && OLDSHPRisvalidtype(nextheader,OLDSHPMAKECODE))
                        {
                            ok = 80;
                        }
                    }
                }
            }
        }
#endif
#endif

    }
    return(ok);
}

/****************************************************************/
/*  Import Functions                                            */
/****************************************************************/

int GCALL OLDSHP_open(GINSTANCE **gx, GSTREAM *g, const char *pathname,bool framecountflag)
{
    GINSTANCE    *Result=0L;
    OLDShapeListPtr oLdgshape=0L;
    int frames=0;

    if (OLDSHPRReadShapeFile(g,&oLdgshape,OLDSHPMULTIFRAME,OLDSHPMAKECODE,OLDSHPSIGNATURE,OLDFNTSIGNATURE,OLDSHPMIPMAPDATAALIGNMENT))
    {
        Result=(GINSTANCE*)galloc(sizeof(GINSTANCE));
        if (Result)
        {
            frames=oLdgshape->NuoLdmshape;
            memset(Result,0,sizeof(GINSTANCE));
            Result->frames=frames;
            Result->signature=OLDSHPSIGNATURE;
            Result->size=sizeof(GINSTANCE);
            Result->gstream=g;
            Result->gref=(char *)oLdgshape;
        }
        *gx=Result;
    }
    return(frames);
}

#define SIGNEXTEND(x) (int)(short)(x)

GINFO * GCALL OLDSHP_info(GINSTANCE *gx, int framenum)
{
    int             bpp;
    int             width;
    int             height;
    int             centerx;
    int             centery;
    int             shapex;
    int             shapey;
    int             clutx;
    int             cluty;
    int             colours=0;
    int             alphabits;
    int             rgbbits;
    int             startcolour;
    int             i;
    int             j;
    int             csm1=0;
    int             type=OLDSHPMAKECODE;
    int             mode=DXT_DXT3_CODE;
    char            *D;
    char            *pal;
    GINFO           *ginfo=0;
    OLDShapeListPtr    oLdgshape;
    OLDgshapeInfo      *oldshpinfo;
    OLDSHPHeaderStruct *HeaderPict=0;
    OLDClutHeaderStruct *theClut;

    oLdgshape=(OLDShapeListPtr)gx->gref;

    if (framenum<oLdgshape->NuoLdmshape)
    {

        oldshpinfo = &oLdgshape->ListShape[framenum];
        HeaderPict=(OLDSHPHeaderStruct *)oldshpinfo->shape_Data;
        D = ((char*)HeaderPict + sizeof(OLDSHPHeaderStruct));
        if (HeaderPict)
        {
            int rotatebits=OLDSHPGETFIELD(HeaderPict->shapex);
            width    = OLDSHPGETFIELD(HeaderPict->width);//&0x0fff;           /* Width of bitmap in pixels */
            height   = OLDSHPGETFIELD(HeaderPict->height);//&0x0fff;          /* Height of bitmap in pixels */
            centerx  = SIGNEXTEND(OLDSHPGETFIELD(HeaderPict->centerx));    /* Center point x */
            centery  = SIGNEXTEND(OLDSHPGETFIELD(HeaderPict->centery));    /* Center point y */
            shapex   = SIGNEXTEND(OLDSHPGETFIELD(HeaderPict->shapex)<<4)>>4; /* shape position x */
            shapey   = SIGNEXTEND(OLDSHPGETFIELD(HeaderPict->shapey)<<4)>>4; /* shape position y */
            if (rotatebits&0x8000) oldshpinfo->subtype |= SUBROTATE;
            if (rotatebits&0x4000) oldshpinfo->subtype |= SUBTRANSPOSE;
            if (rotatebits&0x2000) oldshpinfo->subtype |= SUBSWIZZLE;

            bpp = OLDSHPRGetOldshpbpp(HeaderPict,type);
            if (bpp>8) oldshpinfo->subtype &= ~SUBNOPAL;

            switch (type)
            {
                case TYPEOLDPSH:
                    alphabits = 1;
                    rgbbits = 5;
                    if (bpp==24)
                        rgbbits = 8;
                    break;

                case TYPEOLDASH:
                    alphabits = 1;
                    rgbbits = 5;
                    break;

                case TYPEOLDGSH:
				case TYPEOLDGSS:
                    alphabits = 4;
                    rgbbits = 5;
                    break;

//                case TYPEOLDNSH:
                case TYPEOLDMSH:
				case TYPEOLDMSS:
                    alphabits = 1;
                    rgbbits = 8;
                    if (bpp==16)
                        rgbbits = 5;
                    break;

    	  	case TYPEOLDXSH:
			case TYPEOLDXSS:
                    alphabits = 8;
                    rgbbits = 8;
    				break;

                default:
                case TYPEOLDVSH:
                case TYPEOLDFSH:
				case TYPEOLDFSS:
                    alphabits = 1;
                    rgbbits = 8;
                    break;
            }

            if (framenum < gx->frames)
                ginfo = (GINFO *) galloc(sizeof(GINFOOLDSHP));

            if (ginfo)
            {
                memset(ginfo, 0, sizeof(GINFOOLDSHP));

                /* template for ginfo */

                ginfo->signature        = OLDSHPSIGNATURE;              /* type of file */
                ginfo->size             = sizeof(GINFOOLDSHP);          /* Length of GINFO */
                ginfo->version          = GIMEX_VERSION;
                ginfo->framenum         = framenum;                  /* Current frame num */
                ginfo->width            = width;                     /* Width of bitmap in pixels */
                ginfo->height           = height;                    /* Height of bitmap in pixels */
                ginfo->subtype          = oldshpinfo->subtype;          /* subtype */
                ginfo->packed           = oldshpinfo->packed;           /* packed or not */
                ginfo->framesize        = oldshpinfo->framesize;        /* total data bytes for shape */
                ginfo->quality          = 100;          /* Quality of lossy packing 0..100 */
                ginfo->alphabits        = alphabits;    /* Number of bits in alpha channel */
                ginfo->redbits          = rgbbits;      /* Number of bits in red channel */
                ginfo->greenbits        = rgbbits;      /* Number of bits in grreen channel */
                ginfo->bluebits         = rgbbits;      /* Number of bits in blue channel */
                ginfo->centerx          = centerx;      /* Center Point relative to upper left corner */
                ginfo->centery          = centery;
                ginfo->defaultx         = shapex;       /* shape Coordinate Point */
                ginfo->defaulty         = shapey;
                ginfo->numhotspots      = 0;            /* Number of Hot Spots defined */

#if OLDSHPMAKECODE/10==SHAPEFILE
                {
                    memcpy(ginfo->framename,oLdgshape->ListShape[framenum].shape_ID,4);  /* Null terminated name of frame/image */

                    /* strip trailing spaces (exporter pads with spaces) */
                    {
                        int i;
                        for (i=3; i>=0; --i)
                        {
                            if (ginfo->framename[i]==' ')
                                ginfo->framename[i]=0;
                            else
                                break;
                        }
                    }
                }
#endif
                if(!OLDSHPRGetShapeHotSpot(oLdgshape,framenum,ginfo,type))
                {
                    if(ginfo)
                        gfree(ginfo);
                    return 0;
                }
                OLDSHPRGetShapeName(oLdgshape,framenum,ginfo,type);
                OLDSHPRGetShapeComment(oLdgshape,framenum,ginfo,type);
                OLDSHPRGetShapeAnim(oLdgshape,framenum,ginfo,type);
                OLDSHPRGetShapeTpage(oLdgshape,framenum,ginfo,type);
                OLDSHPRGetShapeMetal(oLdgshape,framenum,ginfo,type);

                /* 16,24 bpp true colour */
                if (bpp>8)
                {
                    ginfo->bpp              = 32;       /* Bits per pixel (8 or 32) */
                    ginfo->originalbpp      = bpp;
                    ginfo->numcolours  = 0;        /* Number of colours in original indexed palette */

                    if (bpp<=15)
                    {
                        if (type==TYPEOLDGSH || type==TYPEOLDGSS)      /* OLDGSH 5a3 */
                        {
                            ginfo->alphabits = 3;
                            ginfo->redbits   = 5;
                            ginfo->greenbits = 5;
                            ginfo->bluebits  = 5;
                        }
                        else
                        {
                            ginfo->alphabits = 1;
                            ginfo->redbits   = 5;
                            ginfo->greenbits = 5;
                            ginfo->bluebits  = 5;
                        }
                    }
                    if(bpp == 12)
                    {
                        ginfo->alphabits = 0;
                        ginfo->redbits   = 2;
                        ginfo->greenbits = 8;
                        ginfo->bluebits  = 2;
                        ginfo->originalbpp = 12;
                    }
                    if (bpp==16)
                    {
//                        if (type==TYPEOLDNSH)      /* OLDNSH */
//                        {
//                            /* 88 intensity/alpha */
//                            ginfo->alphabits = 8;
//                            ginfo->redbits   = 8;
//                            ginfo->greenbits = 8;
//                            ginfo->bluebits  = 8;
//                        }
                        if (type==TYPEOLDFSH || type==TYPEOLDXSH || type==TYPEOLDXSS || type==TYPEOLDFSS)/* OLDFSH/OLDXSH */
                        {
                            if ((HeaderPict->type&0x7f)==0x6d)  /* 4444 */
                            {
                                ginfo->alphabits = 4;
                                ginfo->redbits   = 4;
                                ginfo->greenbits = 4;
                                ginfo->bluebits  = 4;
                            }
                            else if ((HeaderPict->type&0x7f)==0x67)  /* 655 */
                            {
                                ginfo->alphabits = 0;
                                ginfo->redbits   = 6;
                                ginfo->greenbits = 5;
                                ginfo->bluebits  = 5;
                            }
                            else if ((HeaderPict->type&0x7f)==0x65)  /* IA8 */
                            {
                                ginfo->alphabits = 8;
                                ginfo->redbits   = 8;
                                ginfo->greenbits = 0;
                                ginfo->bluebits  = 0;
                            }
                            else if ((HeaderPict->type&0x7f)==0x68)  /* YUY2 */
                            {
                                ginfo->alphabits = 0;
                                ginfo->redbits   = 4;
                                ginfo->greenbits = 8;
                                ginfo->bluebits  = 4;
                            }
                            else
                            {
                                ginfo->alphabits = 0;
                                ginfo->redbits   = 5;
                                ginfo->greenbits = 6;
                                ginfo->bluebits  = 5;
                            }
                        }


                        else if (type==TYPEOLDMSH || type == TYPEOLDMSS)      /* OLDMSH */
                        {
                            if ((HeaderPict->type&0x7f)==0x5a)  /* 4444 */
                            {
                                ginfo->alphabits = 4;
                                ginfo->redbits   = 4;
                                ginfo->greenbits = 4;
                                ginfo->bluebits  = 4;
                            }
                            else
                            {
                                ginfo->alphabits = 0;
                                ginfo->redbits   = 5;
                                ginfo->greenbits = 6;
                                ginfo->bluebits  = 5;
                            }
                        }
                        else if (type==TYPEOLDGSH || type==TYPEOLDGSS)      /* OLDGSH */
                        {
                            if ((HeaderPict->type&0x7f)==0x13)  /* IA8 */
                            {
                                ginfo->alphabits = 8;
                                ginfo->redbits   = 8;
                                ginfo->greenbits = 0;
                                ginfo->bluebits  = 0;
                            }
                            else
                            {
                                ginfo->alphabits = 0;
                                ginfo->redbits   = 5;
                                ginfo->greenbits = 6;
                                ginfo->bluebits  = 5;
                            }
                        }

                    }
                    if (bpp>16)
                    {
                        if ((HeaderPict->type&0x7f)==0x66)  /* 6666 */
                        {
                            ginfo->alphabits = 6;
                            ginfo->redbits   = 6;
                            ginfo->greenbits = 6;
                            ginfo->bluebits  = 6;
                        }
                        else if ((HeaderPict->type&0x7f)==0x6a)  /* 2101010 */
                        {
                            ginfo->alphabits = 2;
                            ginfo->redbits   = 10;
                            ginfo->greenbits = 10;
                            ginfo->bluebits  = 10;
                        }
                        else
                        {
                            int bpc=gmin(8,bpp/3);
                            ginfo->redbits = ginfo->greenbits = ginfo->bluebits = bpc;
                            ginfo->alphabits = bpp-bpc*3;
                        }
                    }
                    mode = ggetm( &((((DXTHEADER *)(D))->code)) ,sizeof((((DXTHEADER *)(D))->code)));

                    if ((bpp==32 && ginfo->packed==3) || (((HeaderPict->type&0x7f) == 0x61) || ((HeaderPict->type&0x7f)==0x62))) // DXT3 = 4565
                    {

                        if(mode == DXT_DXT3_CODE || (HeaderPict->type&0x7f)==0x61)  // DXT3 packtype or DXT3 shapetype
                        {

                            ginfo->alphabits = 4;
                            ginfo->redbits   = 5;
                            ginfo->greenbits = 6;
                            ginfo->bluebits  = 5;
                        }
                        else
                        {
                            ginfo->alphabits = 3;
                            ginfo->redbits   = 5;
                            ginfo->greenbits = 6;
                            ginfo->bluebits  = 5;
                        }

                    }										// OLDGSH dxt shape type               regular  dxt1 shape type
                    if ((bpp==16 && ginfo->packed==3) || (((HeaderPict->type&0x7f)==0x1e) || ((HeaderPict->type&0x7f)==0x60))) // DXT1 = 1565
                    {
                        ginfo->alphabits = 1;
                        ginfo->redbits   = 5;
                        ginfo->greenbits = 6;
                        ginfo->bluebits  = 5;
                    }
                }
                /* 8 grey or colour, with 16 or 24 bpp palette */
                else
                {
                    if (!bpp)        bpp = 8;

                    ginfo->bpp              = 8;                        /* Bits per pixel (8 or 32) */
                    ginfo->originalbpp      = bpp;
                    theClut= (OLDClutHeaderStruct *) oLdgshape->ListShape[framenum].shape_Clut;
                    colours = (1 << bpp);

                    if (theClut)
                    {
                        clutx = cluty = 0;
                        if (type==TYPEOLDPSH)
                        {
                            clutx = OLDSHPGETFIELD(theClut->shapex);
                            cluty = OLDSHPGETFIELD(theClut->shapey);
                        }

                        colours = OLDSHPRclutcolours(theClut,&startcolour,bpp,type);
                        csm1 = OLDSHPGETFIELD(theClut->shapex)&0x2000;
                        if (type==TYPEOLDSSH && !csm1 && bpp==8)
                            ginfo->subtype |= SUBCSM2;

                        ginfo->numcolours  = colours+startcolour;
                        ((GINFOOLDSHP *)ginfo)->clutx = clutx;
                        ((GINFOOLDSHP *)ginfo)->cluty = cluty;

                        pal = (char *) &theClut->Entries[0];
                        switch (theClut->type)
                        {
                            case 0x22:  /* OLDVSH 24 bpp (6 bit channels) */
                                ginfo->redbits = ginfo->greenbits = ginfo->bluebits = 6;
                                ginfo->alphabits = 0;
                                if (colours==256 && !ggeti(&pal[255*3+0],3))
                                    ginfo->alphabits = 1;

                                for (i=0; i<colours; ++i)
                                {
                                    ginfo->colourtbl[startcolour+i].a = (unsigned char)((i==255 && ginfo->alphabits)?0:255);
                                    ginfo->colourtbl[startcolour+i].r = (unsigned char)((pal[i*3+0]*255+32)/63);
                                    ginfo->colourtbl[startcolour+i].g = (unsigned char)((pal[i*3+1]*255+32)/63);
                                    ginfo->colourtbl[startcolour+i].b = (unsigned char)((pal[i*3+2]*255+32)/63);
                                }
                                break;


                            case 0x24:  /* OLDFSH 24 bpp */
                                ginfo->redbits = ginfo->greenbits = ginfo->bluebits = 8;
                                ginfo->alphabits = 0;
                                if (colours==256 && !ggeti(&pal[255*3+0],3))
                                    ginfo->alphabits = 1;

                                for (i=0; i<colours; ++i)
                                {
                                    ginfo->colourtbl[startcolour+i].a = (unsigned char)((i==255 && ginfo->alphabits)?0:255);
                                    ginfo->colourtbl[startcolour+i].r = pal[i*3+0];
                                    ginfo->colourtbl[startcolour+i].g = pal[i*3+1];
                                    ginfo->colourtbl[startcolour+i].b = pal[i*3+2];
                                }
                                break;

                            case 0x21:  /* OLDSSH 32 bpp */
                            case 0x3b:  /* OLDMSH 32 bpp */
                                ginfo->redbits = ginfo->greenbits = ginfo->bluebits = ginfo->alphabits = 8;

                                for (i=0; i<colours; ++i)
                                {
                                    j = (csm1 && (i&31)>=8 && (i&31)<=23) ? i ^ 0x18 : i;

                                    ginfo->colourtbl[startcolour+i].r = pal[j*4+0];
                                    ginfo->colourtbl[startcolour+i].g = pal[j*4+1];
                                    ginfo->colourtbl[startcolour+i].b = pal[j*4+2];
                                    ginfo->colourtbl[startcolour+i].a = pal[j*4+3];
                                }
                                break;

                            case 0x32:  /* OLDGSH 3555 bpp */
                                ginfo->alphabits = 3;
                                ginfo->redbits =   5;
                                ginfo->greenbits = 5;
                                ginfo->bluebits =  5;
                                for (i=0; i<colours; ++i)
                                {
                                    *(int *)&ginfo->colourtbl[startcolour+i]=OLDGSHRpixel5To8(OLDggetx(pal+i*2,2));
                                }
                                break;

                            case 0x33:  /* OLDGSH 3555 bpp */
                                ginfo->alphabits = 8;
                                ginfo->redbits =   8;
                                ginfo->greenbits = 8;
                                ginfo->bluebits =  8;
                                for (i=0; i<colours; ++i)
                                {
                                    ginfo->colourtbl[startcolour+i].a = pal[i*2+0];
                                    ginfo->colourtbl[startcolour+i].r = pal[i*2+1];
                                    ginfo->colourtbl[startcolour+i].g = pal[i*2+((colours*2+31)&~31)+0];
                                    ginfo->colourtbl[startcolour+i].b = pal[i*2+((colours*2+31)&~31)+1];
                                }
                                break;


                            case 0x2d:  /* OLDFSH 1555 bpp */
                                ginfo->alphabits = 1;
                                ginfo->redbits =   5;
                                ginfo->greenbits = 5;
                                ginfo->bluebits =  5;
                                for (i=0; i<colours; ++i)
                                {
                                    *(int *)&ginfo->colourtbl[startcolour+i]=OLDFSHRpixel5To8(OLDggetx(pal+i*2,2));
                                }
                                break;

                            case 0x20:  /* OLDSSH 1555 bpp */
                            case 0x39:  /* OLDMSH 1555 bpp */
                                ginfo->alphabits = 1;
                                ginfo->redbits =   5;
                                ginfo->greenbits = 5;
                                ginfo->bluebits =  5;
                                for (i=0; i<colours; ++i)
                                {
                                    j = i;
                                    if (csm1)
                                        if ((i&31)>=8 && (i&31)<=23)
                                            j ^= (8^16);

                                    *(int *)&ginfo->colourtbl[startcolour+i]=OLDSSHRpixel5To8(OLDggetx(pal+j*2,2));
                                }
                                break;
                            case 0x30:  /* OLDGSH IA8 clut */
                                ginfo->alphabits = 8;
                                ginfo->redbits =   8;
                                ginfo->greenbits = 0;
                                ginfo->bluebits =  0;
                                for (i=0; i<colours; ++i)
                                {
                                    *(int *)&ginfo->colourtbl[startcolour+i]=OLDGSHRpixelIATo8(OLDggetx(pal+i*2,2));
                                }
                                break;


                            case 0x31:  /* OLDGSH 565 bpp */
                            case 0x29:  /* OLDFSH 565 bpp */
                                ginfo->alphabits = 0;
                                ginfo->redbits =   5;
                                ginfo->greenbits = 6;
                                ginfo->bluebits =  5;
                                for (i=0; i<colours; ++i)
                                {
                                    *(int *)&ginfo->colourtbl[startcolour+i]=OLDFSHRpixel565To8(OLDggetx(pal+i*2,2));
                                }
                                break;

                            case 0x38:  /* OLDMSH 565 bpp */
                                ginfo->alphabits = 0;
                                ginfo->redbits =   5;
                                ginfo->greenbits = 6;
                                ginfo->bluebits =  5;
                                for (i=0; i<colours; ++i)
                                {
                                    *(int *)&ginfo->colourtbl[startcolour+i]=OLDMSHRpixel565To8(OLDggetx(pal+i*2,2));
                                }
                                break;

                            case 0x3a:  /* OLDMSH 4444 */
                                ginfo->alphabits = 4;
                                ginfo->redbits =   4;
                                ginfo->greenbits = 4;
                                ginfo->bluebits =  4;
                                for (i=0; i<colours; ++i)
                                {
                                    *(int *)&ginfo->colourtbl[startcolour+i]=OLDMSHRpixel4444To8(OLDggetx(pal+i*2,2));
                                }
                                break;


                            case 0x2a:  /* OLDXSH/OLDFSH 32 bpp */
                                {
                                    ginfo->alphabits = 8;
                                    ginfo->redbits =   8;
                                    ginfo->greenbits = 8;
                                    ginfo->bluebits =  8;
                                    for (i=0; i<colours; ++i)
                                    {
                                        ginfo->colourtbl[startcolour+i].b = pal[i*4+0];
                                        ginfo->colourtbl[startcolour+i].g = pal[i*4+1];
                                        ginfo->colourtbl[startcolour+i].r = pal[i*4+2];
                                        ginfo->colourtbl[startcolour+i].a = pal[i*4+3];
                                    }
                                }
                                break;

                            case 0x2e:  /* OLDFSH 8555 bpp */
                                ginfo->alphabits = 8;
                                ginfo->redbits =   5;
                                ginfo->greenbits = 5;
                                ginfo->bluebits =  5;
                                for (i=0; i<colours; ++i)
                                {
                                    *(int *)&ginfo->colourtbl[startcolour+i]=OLDFSHRpixel5To8(OLDggetx(pal+i*4,2));
                                    ginfo->colourtbl[startcolour+i].a = pal[i*4+3];
                                }
                                break;

                            case 0x2c:  /* OLDFSH 8565 bpp */
                                ginfo->alphabits = 8;
                                ginfo->redbits =   5;
                                ginfo->greenbits = 6;
                                ginfo->bluebits =  5;
                                for (i=0; i<colours; ++i)
                                {
                                    *(int *)&ginfo->colourtbl[startcolour+i]=OLDFSHRpixel565To8(OLDggetx(pal+i*4,2));
                                    ginfo->colourtbl[startcolour+i].a = pal[i*4+3];
                                }
                                break;

                            case 0x23:  /* OLDPSH 16 bpp */
                                ginfo->alphabits = 1;
                                ginfo->redbits =   5;
                                ginfo->greenbits = 5;
                                ginfo->bluebits =  5;

                                for (i=0; i<colours; ++i)
                                {
                                    *(int *)&ginfo->colourtbl[startcolour+i]=OLDPSHRpixel5To8(OLDggetx(pal+i*2,2));
                                }
                                break;

//                            case 0x27:  /* OLDNSH 16 bpp RGBA */
//                                ginfo->alphabits = 1;
//                                ginfo->redbits =   5;
//                                ginfo->greenbits = 5;
//                                ginfo->bluebits =  5;
//
//                                for (i=0; i<colours; ++i)
//                                {
//                                    *(int *)&ginfo->colourtbl[startcolour+i]=OLDNSHRpixel5To8(OLDggetx(pal+i*2,2));
//                                }
//                                break;

//                            case 0x28:  /* OLDNSH 16 bpp intensity/alpha */
//                                ginfo->alphabits = 8;
//                                ginfo->redbits =   8;
//                                ginfo->greenbits = 8;
//                                ginfo->bluebits =  8;
//
//                                for (i=0; i<colours; ++i)
//                                {
//                                    *(int *)&ginfo->colourtbl[startcolour+i]=OLDNSHRpixelIATo8(OLDggetx(pal+i*2,2));
//                                }
//                                break;

                            default:
                                break;
                        }
                    }
                    else    /* grey scale */
                    {
                        if ((HeaderPict->type&0x7f)==0x10)  /* OLDGSH 4 bpp I4 */
                        {
                            ginfo->alphabits = 0;
                            ginfo->redbits   = 4;
                            ginfo->greenbits = 0;
                            ginfo->bluebits  = 0;
                            ginfo->numcolours  = colours;

                            for (i = 0; i < colours; ++i)
                            {
                                ginfo->colourtbl[i].a = (unsigned char)255;
                                ginfo->colourtbl[i].r = (unsigned char)(i*255/15);
                                ginfo->colourtbl[i].g = (unsigned char)(i*255/15);
                                ginfo->colourtbl[i].b = (unsigned char)(i*255/15);
                            }
                        }
                        else if ((HeaderPict->type&0x7f)==0x11)  /* OLDGSH 8 bpp I8 */
                        {
                            ginfo->alphabits = 0;
                            ginfo->redbits   = 8;
                            ginfo->greenbits = 0;
                            ginfo->bluebits  = 0;
                            ginfo->numcolours  = colours;

                            for (i = 0; i < colours; ++i)
                            {
                                ginfo->colourtbl[i].a = (unsigned char)255;
                                ginfo->colourtbl[i].r = (unsigned char)i;
                                ginfo->colourtbl[i].g = (unsigned char)i;
                                ginfo->colourtbl[i].b = (unsigned char)i;
                            }
                        }
                        else if ((HeaderPict->type&0x7f)==0x12)  /* OLDGSH 8 bpp IA4 */
                        {
                            ginfo->alphabits = 4;
                            ginfo->redbits   = 4;
                            ginfo->greenbits = 0;
                            ginfo->bluebits  = 0;
                            ginfo->numcolours  = colours;

                            for (i = 0; i < colours; ++i)
                            {
                                ginfo->colourtbl[i].a = (unsigned char)((i>>4)*255/15);
                                ginfo->colourtbl[i].r = (unsigned char)((i&15)*255/15);
                                ginfo->colourtbl[i].g = (unsigned char)((i&15)*255/15);
                                ginfo->colourtbl[i].b = (unsigned char)((i&15)*255/15);
                            }
                        }

//                        if ((HeaderPict->type&0x7f)==0x62)       /* OLDNSH 4 bpp IA */
//                        {
//                            ginfo->alphabits = 1;
//                            ginfo->redbits =   3;
//                            ginfo->greenbits = 3;
//                            ginfo->bluebits =  3;
//                            ginfo->numcolours  = colours;
//                            for (i = 0; i < colours; ++i)
//                            {
//                                ginfo->colourtbl[i].a = (unsigned char)((i&1)*255);
//                                ginfo->colourtbl[i].r = (unsigned char)((i>>1)*255/((colours>>1)-1));
//                                ginfo->colourtbl[i].g = (unsigned char)((i>>1)*255/((colours>>1)-1));
//                                ginfo->colourtbl[i].b = (unsigned char)((i>>1)*255/((colours>>1)-1));
//                            }
//                        }
//                        else if ((HeaderPict->type&0x7f)==0x65)  /* OLDNSH 8 bpp IA */
//                        {
//                            ginfo->alphabits = 4;
//                            ginfo->redbits =   4;
//                            ginfo->greenbits = 4;
//                            ginfo->bluebits =  4;
//                            ginfo->numcolours  = colours;
//
//                            for (i = 0; i < colours; ++i)
//                            {
//                                ginfo->colourtbl[i].a = (unsigned char)((i&15)*255/15);
//                                ginfo->colourtbl[i].r = (unsigned char)((i>>4)*255/((colours>>4)-1));
//                                ginfo->colourtbl[i].g = (unsigned char)((i>>4)*255/((colours>>4)-1));
//                                ginfo->colourtbl[i].b = (unsigned char)((i>>4)*255/((colours>>4)-1));
//                            }
//                        }
                        else if ((HeaderPict->type&0x7f)==0x64)  /* OLDXSH Alpha8 */
                        {
                            ginfo->alphabits = 8;
                            ginfo->redbits =   0;
                            ginfo->greenbits = 0;
                            ginfo->bluebits =  0;
                            ginfo->numcolours  = colours;
                            for (i=0; i<colours; ++i)
                            {
                                ginfo->colourtbl[i].b = 0xff;
                                ginfo->colourtbl[i].g = 0xff;
                                ginfo->colourtbl[i].r = 0xff;
                                ginfo->colourtbl[i].a = i;
                            }
                        }
                        else
                        {
                            ginfo->numcolours  = colours;
                            ginfo->alphabits   = 0;
                            if (colours>1)
                            {
                                for (i=0; i<colours; ++i)
                                {
                                    ginfo->colourtbl[i].a = 0xff;
                                    ginfo->colourtbl[i].r = (unsigned char)(i*255/(colours-1));
                                    ginfo->colourtbl[i].g = (unsigned char)(i*255/(colours-1));
                                    ginfo->colourtbl[i].b = (unsigned char)(i*255/(colours-1));
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return(ginfo);
}


int GCALL OLDSHP_read(GINSTANCE *gx, GINFO *ginfo, char *dest, int rowbytes)
{
    OLDShapeListPtr    oLdgshape=0L;
    OLDSHPHeaderStruct *HeaderPict;
    int             framenum;
    int             ok=0;

    oLdgshape=(OLDShapeListPtr)gx->gref;
    framenum=ginfo->framenum;
    if (framenum!=-1)
    {
        HeaderPict=(OLDSHPHeaderStruct*)(oLdgshape->ListShape[framenum].shape_Data);
        if (HeaderPict)
        {
            ok = OLDSHPRReadBitmap(HeaderPict,dest,rowbytes,OLDSHPMAKECODE);
        }
    }
    return(ok);
}


int GCALL OLDSHP_close(GINSTANCE *gx)
{
    OLDShapeListPtr oLdgshape=0L;

    if (gx)
    {
        oLdgshape=(OLDShapeListPtr)gx->gref;
        OLDSHPRDeleteShape(oLdgshape);
        gfree((void*)gx);
    }
    return(1);
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

