// (c) 1994-2007 Electronic Arts Inc.

#include <string.h>
#include "gimex.h"
#include "simgimex.h"

extern int alloccount;

/****************************************************************/
/*  Internal Functions                                          */
/****************************************************************/


int gputc(GSTREAM *g, int v)
{
    unsigned char c[4];

    c[0] = (unsigned char) v;
    return(gwrite(g,&c,1));
}

GSTREAM *DKW_pictureWriteFileHeader(GSTREAM * file, DK_Picture *pic,
                   DK_channelPacket **pck)
{
    char                chained;
    DK_channelPacket    *ptr;
    int ok;

    if ((*pck) == NULL)
    {
        (*pck) = (DK_channelPacket *) galloc(sizeof(DK_channelPacket));
        (*pck)->size     = 8;
        (*pck)->type     =  DK_UNSIGNED_INTEGER |  DK_MIXED_RUN_LENGTH;
        (*pck)->channels =  (char) (DK_RED_CHANNEL |  DK_GREEN_CHANNEL |  DK_BLUE_CHANNEL);
        if (pic->alpha == NULL)
            (*pck)->next = NULL;
        else
        {
            (*pck)->next = (DK_channelPacket *) galloc(sizeof(DK_channelPacket));
            (*pck)->next->size     = 8;
            (*pck)->next->type     = DK_UNSIGNED_INTEGER |  DK_MIXED_RUN_LENGTH;
            (*pck)->next->channels = DK_ALPHA_CHANNEL;
            (*pck)->next->next     = NULL;
        }
    }

    ok = gwrite(file,(const void *) pic, 4+4+80+4+2+2+4+2+2); /* read PIC header */

    for (ptr = *pck; ptr != NULL; ptr = ptr->next)
    {
        chained = (ptr->next != NULL);
        gputc(file,chained);
        gputc(file,ptr->size);
        gputc(file,ptr->type);
        gputc(file,ptr->channels);
    }

    if (!ok)
        file = 0;

    return(file);
}

void DKW_channelPacketDispose(DK_channelPacket **pck)
{
    DK_channelPacket    *ptr, *nxt;

    for (ptr = *pck; ptr != NULL; ptr = nxt)
    {
        nxt = ptr->next;
        gfree(ptr);
    }
}

DK_Picture *DKW_pictureAllocate(void)
{
    DK_Picture *pic;

    pic = (DK_Picture *) galloc(sizeof(DK_Picture));
    if (pic)
    {
        memset(pic,0,sizeof(DK_Picture));
        SIMPUTFIELD(pic->width,0);
        SIMPUTFIELD(pic->height,0);
        SIMPUTFIELD(pic->ratio,0x3f800000);   /* 1.0 in float */
        SIMPUTFIELD(pic->fields,DK_NO_FIELD);
        pic->red    =  pic->ptrRed   = NULL;
        pic->green  =  pic->ptrGreen = NULL;
        pic->blue   =  pic->ptrBlue  = NULL;
        pic->alpha  =  pic->ptrAlpha = NULL;
    }
    return(pic);
}

DK_Boolean DKW_pictureDispose(DK_Picture **pic)
{
    if (*pic)
    {
        if ((*pic)->red)    gfree((*pic)->red);
        if ((*pic)->green)  gfree((*pic)->green);
        if ((*pic)->blue)   gfree((*pic)->blue);
        if ((*pic)->alpha)  gfree((*pic)->alpha);

        gfree(*pic);
    }

    return(TRUE);
}

static void DK_channelsWriteRaw(GSTREAM *file, int nbPix, int nbChnl, char **chnl)
{
    int   i, j;

    for (i = 0; i < nbPix; i++)
        for (j = 0; j < nbChnl; j++)
            gputc(file,chnl[j][i]);
}

static void DK_channelsWritePure(GSTREAM *file, int nbPix, int nbChnl, char **chnl)
{
    unsigned char   col[4];
    int             i, k, count;
    DK_Boolean      same;

    count = 0;
    col[0] = col[1] = col[2] = col[3] = 0;
    for (k = 0; k < nbPix; k++)
    {
        if (count == 0)
        {
            for (i = 0; i < nbChnl; i++)
                col[i] = chnl[i][k];
            count = 1;
        }
        else
        {
            same = TRUE;
            for (i = 0; i < nbChnl; i++)
                if (col[i] != chnl[i][k])
                    same = FALSE;

            if (same)
                count++;

            if (!same || (count == 255))
            {
                gputc(file,(unsigned char) count);
                for (i = 0; i < nbChnl; i++)
                    gputc(file,(unsigned char) col[i]);
                count = 0;
            }
        }
    }

    if (count != 0)
    {
        gputc(file,(unsigned char) count);
        for (i = 0; i < nbChnl; i++)
            gputc(file,(unsigned char) col[i]);
    }
}

static void DK_channelsWriteMixed(GSTREAM *file, int nbPix, int nbChnl, char **chnl)
{
    int              same, seq_same=0;
    register int    i, j, k;
    unsigned char    pixel[128][4], col[4];
    unsigned int     count;
    char             buf[4];

    count = 0;
    col[0] = col[1] = col[2] = col[3] = 0;
    for (k = 0; k < nbPix; k++)
    {
        for (i = 0; i < nbChnl; i++)
            col[i] = chnl[i][k];

        if (count == 0)
        {
            pixel[0][0] = col[0];
            pixel[0][1] = col[1];
            pixel[0][2] = col[2];
            pixel[0][3] = col[3];
            count ++;
        }
        else if (count == 1)
        {
            seq_same = (col[0] == pixel[0][0]);
            seq_same &= (col[1] == pixel[0][1]);
            seq_same &= (col[2] == pixel[0][2]);
            seq_same &= (col[3] == pixel[0][3]);
            if (!seq_same)
            {
                pixel[count][0] = col[0];
                pixel[count][1] = col[1];
                pixel[count][2] = col[2];
                pixel[count][3] = col[3];
            }
            count ++;
        }
        else if (count > 1)
        {
            if (seq_same)
            {
                same  = (col[0] == pixel[0][0]);
                same &= (col[1] == pixel[0][1]);
                same &= (col[2] == pixel[0][2]);
                same &= (col[3] == pixel[0][3]);
            }
            else
            {
                same  = (col[0] == pixel[count-1][0]);
                same &= (col[1] == pixel[count-1][1]);
                same &= (col[2] == pixel[count-1][2]);
                same &= (col[3] == pixel[count-1][3]);
            }

            if (same ^ seq_same)
            {
                /* sequence is break */

                if (!seq_same)
                {
                    /* put (count - 1) different item */

                    gputc(file,(unsigned char) (count - 2));
                    for (i = 0; i < (int)count-1; i++)
                        for (j = 0; j < nbChnl; j++)
                            gputc(file,pixel[i][j]);
                    pixel[0][0] = pixel[1][0] = col[0];
                    pixel[0][1] = pixel[1][1] = col[1];
                    pixel[0][2] = pixel[1][2] = col[2];
                    pixel[0][3] = pixel[1][3] = col[3];
                    count = 2;
                    seq_same = 1;
                }
                else
                {
                    /* put count similar item */

                    if (count < 128)
                        gputc(file,(unsigned char) (count + 127));
                    else
                    {
                        gputc(file,128);
                        gputm(buf,count,2);
                        gwrite(file,(const void *) buf, 2);
                    }
                    for (j = 0; j < nbChnl; j++)
                        gputc(file,pixel[0][j]);

                    pixel[0][0] = col[0];
                    pixel[0][1] = col[1];
                    pixel[0][2] = col[2];
                    pixel[0][3] = col[3];
                    count = 1;
                }
            }
            else
            {
                if (!same)
                {
                    pixel[count][0] = col[0];
                    pixel[count][1] = col[1];
                    pixel[count][2] = col[2];
                    pixel[count][3] = col[3];
                }
                count ++;
                if ((count == 128) && !seq_same)
                {
                    /* put (128) different item */
                    gputc(file,127);
                    for (i = 0; i < (int)count; i++)
                        for (j = 0; j < nbChnl; j++)
                            gputc(file,pixel[i][j]);
                    count = 0;
                }
                if ((count == 65535) && seq_same)
                {
                    gputc(file,128);
                    gputm(buf,count,2);
                    gwrite(file,(const void *) buf, 2);
                    for (j = 0; j < nbChnl; j++)
                        gputc(file,pixel[0][j]);
                    count = 0;
                }
            }
        }
    }
    if (count)
    {
        if ((count == 1) || (!seq_same))
        {
            /* put (count) different item */

            gputc(file,(unsigned char) (count - 1));
            for (i = 0; i < (int)count; i++)
                for (j = 0; j < nbChnl; j++)
                    gputc(file,pixel[i][j]);
        }
        else
        {
            if (count < 128)
                gputc(file,(unsigned char) (count + 127));
            else
            {
                gputc(file,128);
                gputm(buf,count,2);
                gwrite(file,(const void *) buf, 2);
            }
            for (j = 0; j < nbChnl; j++)
                gputc(file,pixel[0][j]);
        }
    }
}



DK_Boolean DKW_pictureWriteScanline(GSTREAM *file, DK_Picture *pic,  DK_channelPacket *pck)
{
    DK_channelPacket    *ptr;
    char                *col[4];
    int                 nbcol;

    for (ptr = pck; ptr != NULL; ptr = ptr->next)
    {
        nbcol = 0;
        if (ptr->channels & DK_RED_CHANNEL)     col[ nbcol++ ] = (char *) pic->ptrRed;
        if (ptr->channels & DK_GREEN_CHANNEL)   col[ nbcol++ ] = (char *) pic->ptrGreen;
        if (ptr->channels & DK_BLUE_CHANNEL)    col[ nbcol++ ] = (char *) pic->ptrBlue;
        if (ptr->channels & DK_ALPHA_CHANNEL)   col[ nbcol++ ] = (char *) pic->ptrAlpha;

        switch(ptr->type & 0x0f)
        {
            case DK_UNCOMPRESSED:
                DK_channelsWriteRaw(file, SIMGETFIELD(pic->width), nbcol, col);
                break;
            case  DK_PURE_RUN_LENGTH:
                DK_channelsWritePure(file, SIMGETFIELD(pic->width), nbcol, col);
                break;
            case  DK_MIXED_RUN_LENGTH:
                DK_channelsWriteMixed(file, SIMGETFIELD(pic->width), nbcol, col);
                break;
        }

        if (ptr->channels & DK_RED_CHANNEL)     pic->ptrRed   += SIMGETFIELD(pic->width);
        if (ptr->channels & DK_GREEN_CHANNEL)   pic->ptrGreen += SIMGETFIELD(pic->width);
        if (ptr->channels & DK_BLUE_CHANNEL)    pic->ptrBlue  += SIMGETFIELD(pic->width);
        if (ptr->channels & DK_ALPHA_CHANNEL)   pic->ptrAlpha += SIMGETFIELD(pic->width);
    }
    return(TRUE);
}


DK_Boolean DKW_pictureWriteFile(GSTREAM *file,
              DK_Picture *pic,
               DK_channelPacket *fmt   /* if != NULL then format used */
            )
{
    DK_channelPacket    *pck = fmt;
    int      i;

    if ((file = DKW_pictureWriteFileHeader(file, pic, &pck)) != NULL)
    {
        pic->ptrRed   = pic->red;
        pic->ptrGreen = pic->green;
        pic->ptrBlue  = pic->blue;
        pic->ptrAlpha = pic->alpha;
        for (i = 0; i < (int)SIMGETFIELD(pic->height); ++i)
            DKW_pictureWriteScanline(file, pic, pck);
/*        gclose(file); */
    }

    if (fmt == NULL)
        DKW_channelPacketDispose(&pck);
    return(TRUE);
}



/****************************************************************/
/*  Export Functions                                            */
/****************************************************************/

int GCALL SIM_wopen(GINSTANCE **gxptr, GSTREAM *g, const char *pathname, int numframes)
{
    GINSTANCE *gx;
    int ok=0;

    gx = (GINSTANCE *) galloc(sizeof(GINSTANCE));
    if (gx)
    {
        memset(gx, 0, sizeof(GINSTANCE));
        gx->gstream = g;
        *gxptr = gx;
        ok = 1;
    }
    return(ok);
}

int GCALL SIM_write(GINSTANCE *gx, const GINFO *ginfo, char *source, int rowbytes)
{
    int i;
    int x;
    int y;
    int ok=0;
    int planebytes;
    ARGB *s;
    DK_Picture *pic;

    if (ginfo->bpp>8)
    {
        pic = DKW_pictureAllocate();
        if (pic)
        {
            SIMPUTFIELD(pic->ID,0x5380f634);        /* magic number */
            SIMPUTFIELD(pic->version,0x4027ae14);   /* version number (float) */
            if (ginfo->comment[0])
                strncpy(pic->comment,ginfo->comment,80-1);
            else
                strcpy(pic->comment,"Done with Soft Image Gimex Module by Frank Barchard");
            SIMPUTFIELD(pic->type,GMAKEID('P','I','C','T'));
            SIMPUTFIELD(pic->width,ginfo->width);
            SIMPUTFIELD(pic->height,ginfo->height);
            SIMPUTFIELD(pic->ratio,0x3f800000);   /* 1.0 in float */
            SIMPUTFIELD(pic->fields,DK_FULL_FRAME);

            planebytes = (ginfo->width * ginfo->height)*5/4+8192; /* slopage for packing */
            pic->ptrRed   = pic->red   = (unsigned char *) galloc(planebytes);
            pic->ptrGreen = pic->green = (unsigned char *) galloc(planebytes);
            pic->ptrBlue  = pic->blue  = (unsigned char *) galloc(planebytes);
            if (ginfo->alphabits)
                pic->ptrAlpha = pic->alpha = (unsigned char *) galloc(planebytes);

            if (pic->red && pic->green && pic->blue)
            {
                i = 0;
                for (y=0; y<ginfo->height; ++y)
                {
                    s = (ARGB *) (source+rowbytes*y);
                    for (x=0; x<ginfo->width; ++x)
                    {
                        if (pic->alpha)  pic->alpha[i] = s->a;
                        pic->red[i]   = s->r;
                        pic->green[i] = s->g;
                        pic->blue[i]  = s->b;
                        ++s;
                        ++i;
                    }
                }

                ok = DKW_pictureWriteFile(gx->gstream,pic,(DK_channelPacket *)0);
            }

            DKW_pictureDispose(&pic);
        }
    }

    return(ok);
}

int GCALL SIM_wclose(GINSTANCE *gx)
{
    int ok=0;

    if (gx)
        ok = gfree((void *) gx);

    return(ok);
}

