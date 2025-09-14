// (c) 1994-2007 Electronic Arts Inc.

#include <string.h>
#include "gimex.h"
#include "movgimex.h"

/* not supported yet */

/****************************************************************/
/*  Export Functions                                            */
/****************************************************************/

int GCALL MOV_wopen(GINSTANCE **gxptr, GSTREAM *g, const char *pathname, int numframes)
{
    GINSTANCE *gx;
    int ok=0;
    unsigned char movheader[8];
    MOVINFO *mov;

    gx = (GINSTANCE *) galloc(sizeof(GINSTANCE));
    if (gx)
    {
        memset(gx, 0, sizeof(GINSTANCE));
        gx->gstream = g;
        gx->frames = numframes;
        gx->framenum = 0;
        *gxptr = gx;

        gx->gref = galloc(sizeof(MOVINFO));
        if (gx->gref)
        {
            mov = (MOVINFO *) gx->gref;
            memset((void *) mov,0,sizeof(MOVINFO));

            gputm(movheader+0,0,4);
            gputm(movheader+4,GMAKEID('m','d','a','t'),4);
            ok = gwrite(g,movheader,8);
        }
        else
            gfree(gx);
    }
    return(ok);
}

int GCALL MOV_write(GINSTANCE *gx, const GINFO *ginfo, char *source, int rowbytes)
{
    char *linebuf;
    char *lineptr;
    int bpp;
    int bpl;
    int x;
    int y;
    int ok=0;
    ARGB *s;
    MOVINFO *mov;

    mov = (MOVINFO *)gx->gref;
    if (!mov->width)
    {
        mov->width  = ginfo->width;
        mov->height = ginfo->height;
        bpp = 16;
        if (ginfo->originalbpp<=8) bpp = 32;  // map 8 to 32 bpp
        if (ginfo->originalbpp>16) bpp = 24;
        if (ginfo->originalbpp>24) bpp = 32;
        mov->bpp    = bpp;
        mov->fps    = (int)ginfo->fps;
        if (!mov->fps)
        {
            mov->fps = ginfo->subtype;
            if (mov->fps<5 || mov->fps>100)
                mov->fps = 15;
        }
        if (mov->fps<1 || mov->fps>1000)
            mov->fps = 15;
    }

    bpp = mov->bpp;
    bpl = ginfo->width*bpp/8;

    linebuf = (char *) galloc(bpl);
    if (linebuf)
    {
        ok = 1;
        for (y=0; y<ginfo->height && ok; ++y)
        {
            lineptr = linebuf;
            s = (ARGB *) (source+rowbytes*y);

            switch (bpp)
            {
                case 16:
                    for (x=0; x<ginfo->width; ++x)
                    {
                        unsigned int r;
                        unsigned int g;
                        unsigned int b;
                        unsigned int p;

                        r = (s->r*31+128)/255;
                        g = (s->g*31+128)/255;
                        b = (s->b*31+128)/255;
                        p = (r<<10)|(g<<5)|(b<<0);
                        gputm(lineptr,p,2);
                        ++s;
                        lineptr += 2;
                    }
                    break;

                default:
                case 24:
                    for (x=0; x<ginfo->width; ++x)
                    {
                        lineptr[0] = s->r;
                        lineptr[1] = s->g;
                        lineptr[2] = s->b;
                        ++s;
                        lineptr += 3;
                    }
                    break;

                case 32:
					if (ginfo->originalbpp<=8)
					{
						unsigned char *si=(unsigned char *)s;
						for (x=0; x<ginfo->width; ++x)
						{
							lineptr[0] = 255-ginfo->colourtbl[*si].a;	// Quicktime inverts alpha
							lineptr[1] = ginfo->colourtbl[*si].r;
							lineptr[2] = ginfo->colourtbl[*si].g;
							lineptr[3] = ginfo->colourtbl[*si].b;
							++si;
							lineptr += 4;
						}
					}
					else
					{
						for (x=0; x<ginfo->width; ++x)
						{
							lineptr[0] = 255-s->a;  // Quicktime inverts alpha
							lineptr[1] = s->r;
							lineptr[2] = s->g;
							lineptr[3] = s->b;
							++s;
							lineptr += 4;
						}
					}

                    break;
            }

            ok = gwrite(gx->gstream,linebuf,bpl);
        }

        gfree(linebuf);
    }
    ++gx->framenum;

    return(ok);
}


int GCALL MOV_wclose(GINSTANCE *gx)
{
    int ok=0;
    int mdatlen;
    MOVINFO *mov;
    int width;
    int height;
    int bpp;
    int fps;
    int duration;
    GPOS framesize;
    int i;
    unsigned char movheader[8];
    MOVMVHDStruct mvhd;
    MOVTKHDStruct tkhd;
    MOVELSTStruct elst;
    MOVMDHDStruct mdhd;
    MOVHDLRStruct hdlr;
    MOVVMHDStruct vmhd;
    MOVDHDLRStruct dhdlr;
    MOVDINFStruct dinf;
    MOVSTSDStruct stsd;
    MOVSTTSStruct stts;
    MOVSTSCStruct stsc;
    MOVSTSZStruct stsz;
    MOVSTCOStruct stco;
    MOVUDTAStruct udta;

    if (gx)
    {
        mov = (MOVINFO *)gx->gref;
        if (mov)
        {
            width  = mov->width;
            height = mov->height;
            bpp    = mov->bpp;
            fps    = mov->fps;
            if (!fps) fps = 1;
            duration = (gx->framenum*600+fps/2)/fps;

            /* mdat chunk */
            mdatlen = (int)gtell(gx->gstream);
            gseek(gx->gstream,(GPOS)0);
            gputm(movheader,mdatlen,4);
            ok = gwrite(gx->gstream,movheader,4);
            mdatlen = gseek(gx->gstream,mdatlen);

            /* moov chunk */
            gputm(movheader+0,8+sizeof(mvhd)+8+sizeof(tkhd)+8+sizeof(elst)+8+sizeof(mdhd)+sizeof(hdlr)+8+sizeof(vmhd)+sizeof(dhdlr)+sizeof(dinf)+8+sizeof(stsd)+sizeof(stts)+sizeof(stsc)+sizeof(stsz)+sizeof(stco)+(gx->framenum-1)*4+sizeof(udta),4);
            gputm(movheader+4,GMAKEID('m','o','o','v'),4);
            ok = gwrite(gx->gstream,movheader,8);

                /* mvhd chunk */
                memset(&mvhd,0,sizeof(mvhd));
                MOVPUTFIELD(mvhd.Size,sizeof(mvhd));
                MOVPUTFIELD(mvhd.Type,GMAKEID('m','v','h','d'));
                MOVPUTFIELD(mvhd.Version,0);
                MOVPUTFIELD(mvhd.Flags,0);
                MOVPUTFIELD(mvhd.CreationTime,0xabbd979d);
                MOVPUTFIELD(mvhd.ModificationTime,0xabbd979d);
                MOVPUTFIELD(mvhd.TimeScale,600);
                MOVPUTFIELD(mvhd.Duration,duration);
                MOVPUTFIELD(mvhd.PreferredRate,0x00010000);
                MOVPUTFIELD(mvhd.PreferredVolume,0x00ff);
                MOVPUTFIELD(mvhd.Matrix[0],0x00010000);
                MOVPUTFIELD(mvhd.Matrix[1],0x00000000);
                MOVPUTFIELD(mvhd.Matrix[2],0x00000000);
                MOVPUTFIELD(mvhd.Matrix[3],0x00000000);
                MOVPUTFIELD(mvhd.Matrix[4],0x00010000);
                MOVPUTFIELD(mvhd.Matrix[5],0x00000000);
                MOVPUTFIELD(mvhd.Matrix[6],0x00000000);
                MOVPUTFIELD(mvhd.Matrix[7],0x00000000);
                MOVPUTFIELD(mvhd.Matrix[8],0x40000000);
                MOVPUTFIELD(mvhd.PreviewTime,0);
                MOVPUTFIELD(mvhd.PreviewDuration,0);
                MOVPUTFIELD(mvhd.PosterTime,0);
                MOVPUTFIELD(mvhd.SelectionTime,0);
                MOVPUTFIELD(mvhd.SelectionDuration,duration);
                MOVPUTFIELD(mvhd.CurrentTime,0);
                MOVPUTFIELD(mvhd.NextTrackID,2);
                gwrite(gx->gstream,&mvhd,sizeof(mvhd));

                /* trak chunk */
                gputm(movheader+0,8+sizeof(tkhd)+8+sizeof(elst)+8+sizeof(mdhd)+sizeof(hdlr)+8+sizeof(vmhd)+sizeof(dhdlr)+sizeof(dinf)+8+sizeof(stsd)+sizeof(stts)+sizeof(stsc)+sizeof(stsz)+sizeof(stco)+(gx->framenum-1)*4,4);
                gputm(movheader+4,GMAKEID('t','r','a','k'),4);
                ok = gwrite(gx->gstream,movheader,8);

                    /* tkhd chunk */
                    memset(&tkhd,0,sizeof(tkhd));
                    MOVPUTFIELD(tkhd.Size,sizeof(tkhd));
                    MOVPUTFIELD(tkhd.Type,GMAKEID('t','k','h','d'));
                    MOVPUTFIELD(tkhd.Version,0);
                    MOVPUTFIELD(tkhd.Flags,15); /* enable track,playback, preview & poster */
                    MOVPUTFIELD(tkhd.CreationTime,0xabbd979d);
                    MOVPUTFIELD(tkhd.ModificationTime,0xabbd979d);
                    MOVPUTFIELD(tkhd.TrackID,1);
                    MOVPUTFIELD(tkhd.Duration,duration);
                    MOVPUTFIELD(tkhd.Layer,0);
                    MOVPUTFIELD(tkhd.AlternateGroup,0);
                    MOVPUTFIELD(tkhd.Volume,0);
                    MOVPUTFIELD(tkhd.Matrix[0],0x00010000);
                    MOVPUTFIELD(tkhd.Matrix[1],0x00000000);
                    MOVPUTFIELD(tkhd.Matrix[2],0x00000000);
                    MOVPUTFIELD(tkhd.Matrix[3],0x00000000);
                    MOVPUTFIELD(tkhd.Matrix[4],0x00010000);
                    MOVPUTFIELD(tkhd.Matrix[5],0x00000000);
                    MOVPUTFIELD(tkhd.Matrix[6],0x00000000);
                    MOVPUTFIELD(tkhd.Matrix[7],0x00000000);
                    MOVPUTFIELD(tkhd.Matrix[8],0x40000000);
                    MOVPUTFIELD(tkhd.TrackWidth,width<<16);
                    MOVPUTFIELD(tkhd.TrackHeight,height<<16);
                    gwrite(gx->gstream,&tkhd,sizeof(tkhd));

                    /* edts chunk */
                    gputm(movheader+0,8+sizeof(elst),4);
                    gputm(movheader+4,GMAKEID('e','d','t','s'),4);
                    ok = gwrite(gx->gstream,movheader,8);

                        /* elst chunk */
                        memset(&elst,0,sizeof(elst));
                        MOVPUTFIELD(elst.Size,sizeof(elst));
                        MOVPUTFIELD(elst.Type,GMAKEID('e','l','s','t'));
                        MOVPUTFIELD(elst.Version,0);
                        MOVPUTFIELD(elst.Flags,0);
                        MOVPUTFIELD(elst.NumEntries,1);
                        MOVPUTFIELD(elst.TrackDuration,duration);
                        MOVPUTFIELD(elst.MediaTime,0);
                        MOVPUTFIELD(elst.MediaRate,0x00010000);
                        gwrite(gx->gstream,&elst,sizeof(elst));

                    /* mdia chunk */
                    gputm(movheader+0,8+sizeof(mdhd)+sizeof(hdlr)+8+sizeof(vmhd)+sizeof(dhdlr)+sizeof(dinf)+8+sizeof(stsd)+sizeof(stts)+sizeof(stsc)+sizeof(stsz)+sizeof(stco)+(gx->framenum-1)*4,4);
                    gputm(movheader+4,GMAKEID('m','d','i','a'),4);
                    ok = gwrite(gx->gstream,movheader,8);

                        /* mdhd chunk */
                        memset(&mdhd,0,sizeof(mdhd));
                        MOVPUTFIELD(mdhd.Size,sizeof(mdhd));
                        MOVPUTFIELD(mdhd.Type,GMAKEID('m','d','h','d'));
                        MOVPUTFIELD(mdhd.Version,0);
                        MOVPUTFIELD(mdhd.Flags,0);
                        MOVPUTFIELD(mdhd.CreationTime,0xabbd979d);
                        MOVPUTFIELD(mdhd.ModificationTime,0xabbd979d);
                        MOVPUTFIELD(mdhd.TimeScale,fps); /* 15 FPS */
                        MOVPUTFIELD(mdhd.Duration,gx->framenum);
                        MOVPUTFIELD(mdhd.Language,0);
                        MOVPUTFIELD(mdhd.Quality,0);
                        gwrite(gx->gstream,&mdhd,sizeof(mdhd));

                        /* hdlr chunk */
                        memset(&hdlr,0,sizeof(hdlr));
                        MOVPUTFIELD(hdlr.Size,sizeof(hdlr));
                        MOVPUTFIELD(hdlr.Type,GMAKEID('h','d','l','r'));
                        MOVPUTFIELD(hdlr.Version,0);
                        MOVPUTFIELD(hdlr.Flags,0);
                        MOVPUTFIELD(hdlr.ComponentType,GMAKEID('m','h','l','r'));
                        MOVPUTFIELD(hdlr.ComponentSubType,GMAKEID('v','i','d','e'));
                        MOVPUTFIELD(hdlr.ComponentManufacturer,GMAKEID('a','p','p','l'));
                        MOVPUTFIELD(hdlr.ComponentFlags,0x40000000);
                        MOVPUTFIELD(hdlr.ComponentFlagsMask,0x00020023);
                        MOVPUTFIELD(hdlr.ComponentName[0],25);
                        strncpy(hdlr.ComponentName+1,"Apple Video Media Handler",25);
                        gwrite(gx->gstream,&hdlr,sizeof(hdlr));

                        /* minf chunk */
                        gputm(movheader+0,8+sizeof(vmhd)+sizeof(dhdlr)+sizeof(dinf)+8+sizeof(stsd)+sizeof(stts)+sizeof(stsc)+sizeof(stsz)+sizeof(stco)+(gx->framenum-1)*4,4);
                        gputm(movheader+4,GMAKEID('m','i','n','f'),4);
                        ok = gwrite(gx->gstream,movheader,8);

                            /* vmhd chunk */
                            memset(&vmhd,0,sizeof(vmhd));
                            MOVPUTFIELD(vmhd.Size,sizeof(vmhd));
                            MOVPUTFIELD(vmhd.Type,GMAKEID('v','m','h','d'));
                            MOVPUTFIELD(vmhd.Version,0);
                            MOVPUTFIELD(vmhd.Flags,1);
                            MOVPUTFIELD(vmhd.GraphicsMode,0x0040);
                            gputm(vmhd.Opcolor+0,0x8000,2);
                            gputm(vmhd.Opcolor+2,0x8000,2);
                            gputm(vmhd.Opcolor+4,0x8000,2);
                            gwrite(gx->gstream,&vmhd,sizeof(vmhd));

                            /* dhdlr chunk */
                            memset(&dhdlr,0,sizeof(dhdlr));
                            MOVPUTFIELD(dhdlr.Size,sizeof(dhdlr));
                            MOVPUTFIELD(dhdlr.Type,GMAKEID('h','d','l','r'));
                            MOVPUTFIELD(dhdlr.Version,0);
                            MOVPUTFIELD(dhdlr.Flags,0);
                            MOVPUTFIELD(dhdlr.ComponentType,GMAKEID('d','h','l','r'));
                            MOVPUTFIELD(dhdlr.ComponentSubType,GMAKEID('a','l','i','s'));
                            MOVPUTFIELD(dhdlr.ComponentManufacturer,GMAKEID('a','p','p','l'));
                            MOVPUTFIELD(dhdlr.ComponentFlags,0x40000001);
                            MOVPUTFIELD(dhdlr.ComponentFlagsMask,0x00020026);
                            MOVPUTFIELD(dhdlr.ComponentName[0],24);
                            strncpy(dhdlr.ComponentName+1,"Apple Alias Data Handler",24);
                            gwrite(gx->gstream,&dhdlr,sizeof(dhdlr));

                            /* dinf chunk */
                            memset(&dinf,0,sizeof(dinf));
                            MOVPUTFIELD(dinf.Size,sizeof(dinf));
                            MOVPUTFIELD(dinf.Type,GMAKEID('d','i','n','f'));
                                MOVPUTFIELD(dinf.DREF.Size,sizeof(dinf.DREF));
                                MOVPUTFIELD(dinf.DREF.Type,GMAKEID('d','r','e','f'));
                                MOVPUTFIELD(dinf.DREF.Version,0);
                                MOVPUTFIELD(dinf.DREF.Flags,0);
                                MOVPUTFIELD(dinf.DREF.NumberOfEntries,1);
                                    MOVPUTFIELD(dinf.DREF.ALIS.Size,sizeof(dinf.DREF.ALIS));
                                    MOVPUTFIELD(dinf.DREF.ALIS.Type,GMAKEID('a','l','i','s'));
                                    MOVPUTFIELD(dinf.DREF.ALIS.Version,0);
                                    MOVPUTFIELD(dinf.DREF.ALIS.Flags,1);
                            gwrite(gx->gstream,&dinf,sizeof(dinf));

                            /* stbl chunk */
                            gputm(movheader+0,8+sizeof(stsd)+sizeof(stts)+sizeof(stsc)+sizeof(stsz)+sizeof(stco)+(gx->framenum-1)*4,4);
                            gputm(movheader+4,GMAKEID('s','t','b','l'),4);
                            ok = gwrite(gx->gstream,movheader,8);

                                /* stsd chunk */
                                memset(&stsd,0,sizeof(stsd));
                                MOVPUTFIELD(stsd.Size,sizeof(stsd));
                                MOVPUTFIELD(stsd.Type,GMAKEID('s','t','s','d'));
                                MOVPUTFIELD(stsd.Version,0);
                                MOVPUTFIELD(stsd.Flags,0);
                                MOVPUTFIELD(stsd.SampleSize,1);
                                MOVPUTFIELD(stsd.STSD.IdSize,sizeof(stsd.STSD));
                                MOVPUTFIELD(stsd.STSD.CodecType,GMAKEID('r','a','w',' '));
                                MOVPUTFIELD(stsd.STSD.Reserved1,0);
                                MOVPUTFIELD(stsd.STSD.Reserved2,0);
                                MOVPUTFIELD(stsd.STSD.DataRefIndex,1);
                                MOVPUTFIELD(stsd.STSD.Version,1);
                                MOVPUTFIELD(stsd.STSD.RevisionLevel,1);
                                MOVPUTFIELD(stsd.STSD.Vendor,GMAKEID('a','p','p','l'));
                                MOVPUTFIELD(stsd.STSD.TemporalQuality,0);
                                MOVPUTFIELD(stsd.STSD.SpatialQuality,0x3ff);
                                MOVPUTFIELD(stsd.STSD.Width,width);
                                MOVPUTFIELD(stsd.STSD.Height,height);
                                MOVPUTFIELD(stsd.STSD.HRes,(int)(72.0*0x10000));
                                MOVPUTFIELD(stsd.STSD.VRes,(int)(72.0*0x10000));
                                MOVPUTFIELD(stsd.STSD.DataSize,0);
                                MOVPUTFIELD(stsd.STSD.FrameCount,1);    /*? gx->frames? */
                                stsd.STSD.Name[0] = (char)strlen("None");
                                strcpy(stsd.STSD.Name+1,   "None");
                                MOVPUTFIELD(stsd.STSD.Depth,bpp);
                                MOVPUTFIELD(stsd.STSD.ClutID,0xffff);
                                gwrite(gx->gstream,&stsd,sizeof(stsd));

                                /* stts chunk */
                                memset(&stts,0,sizeof(stts));
                                MOVPUTFIELD(stts.Size,sizeof(stts));
                                MOVPUTFIELD(stts.Type,GMAKEID('s','t','t','s'));
                                MOVPUTFIELD(stts.Version,0);
                                MOVPUTFIELD(stts.Flags,0);
                                MOVPUTFIELD(stts.NumberOfEntries,1);
                                MOVPUTFIELD(stts.SampleCount,gx->framenum);
                                MOVPUTFIELD(stts.SampleDuration,1);
                                gwrite(gx->gstream,&stts,sizeof(stts));

                                /* stsc chunk */
                                memset(&stsc,0,sizeof(stsc));
                                MOVPUTFIELD(stsc.Size,sizeof(stsc));
                                MOVPUTFIELD(stsc.Type,GMAKEID('s','t','s','c'));
                                MOVPUTFIELD(stsc.Version,0);
                                MOVPUTFIELD(stsc.Flags,0);
                                MOVPUTFIELD(stsc.NumberOfEntries,1);
                                MOVPUTFIELD(stsc.Table[0].FirstChunk,1);
                                MOVPUTFIELD(stsc.Table[0].SamplesPerChunk,1);
                                MOVPUTFIELD(stsc.Table[0].SampleDescription,1);
                                gwrite(gx->gstream,&stsc,sizeof(stsc));

                                /* stsz chunk */
                                memset(&stsz,0,sizeof(stsz));
                                MOVPUTFIELD(stsz.Size,sizeof(stsz));
                                MOVPUTFIELD(stsz.Type,GMAKEID('s','t','s','z'));
                                MOVPUTFIELD(stsz.Version,0);
                                MOVPUTFIELD(stsz.Flags,0);
                                MOVPUTFIELD(stsz.SampleSize,width*height*bpp/8);
                                MOVPUTFIELD(stsz.NumberOfEntries,gx->framenum);
                                gwrite(gx->gstream,&stsz,sizeof(stsz));

                                /* stco chunk */
                                memset(&stco,0,sizeof(stco));
                                MOVPUTFIELD(stco.Size,sizeof(stco)+(gx->framenum-1)*4);
                                MOVPUTFIELD(stco.Type,GMAKEID('s','t','c','o'));
                                MOVPUTFIELD(stco.Version,0);
                                MOVPUTFIELD(stco.Flags,0);
                                MOVPUTFIELD(stco.NumberOfEntries,gx->framenum);
                                gwrite(gx->gstream,&stco,sizeof(stco)-4);

                                framesize = width*height*bpp/8;
                                for (i=0; i<gx->framenum; ++i)
                                {
                                    gputm(movheader,8+i*(unsigned int)framesize,4);
                                    ok = gwrite(gx->gstream,movheader,4);
                                }

                /* udta chunk */
                memset(&udta,0,sizeof(udta));
                MOVPUTFIELD(udta.Size,sizeof(udta));
                MOVPUTFIELD(udta.Type,GMAKEID('u','d','t','a'));
                MOVPUTFIELD(udta.NumberOfEntries,12);
                MOVPUTFIELD(udta.ID,GMAKEID('W','L','O','C'));
                MOVPUTFIELD(udta.Field1,0x00320014);
                MOVPUTFIELD(udta.Field2,0);
                gwrite(gx->gstream,&udta,sizeof(udta));

            gfree(gx->gref);
        }

        ok = gfree((void *) gx);
    }
    return(ok);
}


