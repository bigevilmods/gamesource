// (c) 1994-2007 Electronic Arts Inc.

#include <string.h>
#include "gimex.h"
#include "rlagimex.h"

/****************************************************************/
/*  Export Functions                                            */
/****************************************************************/

/*  encode - run length encodes pixel data for one channel
 *                 returns size of encoded data
 */

static int encode(unsigned char *c_in,unsigned char *c_out,int width)
{
    int     len;
    int     ct;

    len = 0;

    while (width > 0)
    {
        /* don't run-length encode unless we have > 2 equal values in a row */

        if ((width > 2) && (c_in[0] == c_in[1]) && (c_in[1] == c_in[2]))
        {
            for (ct=2; ct < width; ct++)
            {
                if (c_in[ct] != c_in[ct-1])
                    break;
                if (ct >= 127)
                    break;
            }

            /* write out count */
            *c_out++ = (unsigned char)(ct-1);
            len++;

            /* write out value */
            *c_out++ = *c_in;
            len++;
            width -= ct;
            c_in += ct;
        }
        else
        {
            for (ct=1; ct < width; ct++)
            {
                /* don't run-length encode unless we have > 2 equal values in a row */
                if ((width-ct > 2) && (c_in[ct] == c_in[ct+1]) && (c_in[ct+1] == c_in[ct+2]))
                    break;
                if (ct >= 127)
                    break;
            }

            /* write out count */
            *c_out++ = (unsigned char)(256 - ct);
            len++;

            /* copy string of pixels */
            for (; ct-- > 0; len++, width--)
                *c_out++ = *c_in++;
        }
    }
    return(len);
}


static int RLAwriteline(GSTREAM *g, unsigned char *linebase, unsigned char *buf, int width)
{
    ARGB *s;
    int x;
    int len;
    int ok;
    unsigned char *red, *green, *blue, *matte;
    char lenbuf[4];

    red    = buf+width*2;
    green  = buf+width*3;
    blue   = buf+width*4;
    matte  = buf+width*5;

    /* make planar */

    s = (ARGB *) linebase;
    for (x=0; x<width; ++x)
    {
        matte[x] = s->a;
        red[x]   = s->r;
        green[x] = s->g;
        blue[x]  = s->b;
        ++s;
    }

    /* write red scanline */

    len = encode(red, buf, width);
    gputm(lenbuf,len,2);
    ok =  gwrite(g,lenbuf,2);
    gwrite(g,buf,len);

    /* write green scanline */

    len = encode(green, buf, width);
    gputm(lenbuf,len,2);
    ok = gwrite(g,lenbuf,2) && ok;
    gwrite(g,buf,len);

    /* write blue scanline */

    len = encode(blue, buf, width);
    gputm(lenbuf,len,2);
    ok = gwrite(g,lenbuf,2) && ok;
    gwrite(g,buf,len);

    /* write matte scanline */

    len = encode(matte, buf, width);
    gputm(lenbuf,len,2);
    ok = gwrite(g,lenbuf,2) && ok;
    gwrite(g,buf,len);

    return(ok);
}


int GCALL RLA_wopen(GINSTANCE **gxptr, GSTREAM *g, const char *pathname, int numframes)
{
    GINSTANCE *gx;
    int ok=0;

    gx = (GINSTANCE *) galloc(sizeof(GINSTANCE));
    if (gx)
    {
        memset(gx, 0, sizeof(GINSTANCE));
        gx->gstream = g;
        gx->framenum = 0;
        *gxptr = gx;
        ok = 1;
    }
    return(ok);
}

int GCALL RLA_write(GINSTANCE *gx, const GINFO *ginfo, char *source, int rowbytes)
{
    int width;
    int height;
    int y;
    int ok=0;
    char *offsettbl=0;
    char *buf=0;
    char *linebase;
    RLAHeaderStruct rlaheader;

    if (ginfo->bpp>8)
    {
        width = ginfo->width;
        height = ginfo->height;

        /* RLA FILE HEADER structure 740 bytes */

        memset(&rlaheader,0,sizeof(rlaheader));

        RLAPUTFIELD(rlaheader.window.right, width - 1);
        RLAPUTFIELD(rlaheader.window.left, 0);
        RLAPUTFIELD(rlaheader.window.top, height - 1);
        RLAPUTFIELD(rlaheader.window.bottom, 0);
        RLAPUTFIELD(rlaheader.active_window.right, width - 1);
        RLAPUTFIELD(rlaheader.active_window.left, 0);
        RLAPUTFIELD(rlaheader.active_window.top, height - 1);
        RLAPUTFIELD(rlaheader.active_window.bottom, 0);

        RLAPUTFIELD(rlaheader.frame, 1);
        RLAPUTFIELD(rlaheader.storage_type, 0);
        RLAPUTFIELD(rlaheader.num_chan, 3);
        RLAPUTFIELD(rlaheader.num_matte, 1);
        RLAPUTFIELD(rlaheader.num_aux, 0);
        RLAPUTFIELD(rlaheader.revision, 0);

        strcpy(rlaheader.gamma, "2.2");
        strcpy(rlaheader.red_pri, "0.670 0.330");
        strcpy(rlaheader.green_pri, "0.210 0.710");
        strcpy(rlaheader.blue_pri, "0.140 0.080");
        strcpy(rlaheader.white_pt, "0.310 0.316");
        RLAPUTFIELD(rlaheader.job_num , 0);
        strcpy(rlaheader.name, "unknown");
        if (ginfo->comment[0])
            strncpy(rlaheader.desc, ginfo->comment,128-1);
        else
            strcpy(rlaheader.desc, "Gimex Exported Image");
        /*gethostname(rlaheader.machine, 32); */
        strcpy(rlaheader.machine, "Machine Unknown");
        strcpy(rlaheader.user, "User Unknown");

        /* fill in file date - varies with OS */

    /*    second = time(&secondbuf); */
    /*    d_str = ctime(&second); */
    /*    strncpy(rlaheader.date, &d_str[4], 12); */
    /*    strncpy(&rlaheader.date[12], &d_str[19], 5); */
    /*    rlaheader.date[17] = '\0'; */

        strcpy(rlaheader.date, "Aug 11 13:27 1994");

        strcpy(rlaheader.aspect, "user defined");
        strcpy(rlaheader.chan, "rgb");

        if (gwrite(gx->gstream, (char *) &rlaheader, 740))
        {

            /* allocate scanline offset table */

            offsettbl = (char *) galloc(height*4);
            if (offsettbl)
            {
                memset(offsettbl,255,height*4);

                /* write offset table as place holder */

                if (gwrite(gx->gstream, (char *) offsettbl, height*4))
                {
                    buf = (char *) galloc(width*6);
                    if (buf)
                        ok = 1;
                }
            }
        }
        if (ok)
        {
            for (y=0; y<height && ok; ++y)
            {
                linebase = source + rowbytes*(height-1-y);

                /* record location in file */

                gputm(offsettbl+y*4, (unsigned int)gtell(gx->gstream), 4);

                ok = RLAwriteline(gx->gstream, (unsigned char *)linebase, (unsigned char *)buf, ginfo->width);
            }

            /* seek back to offset table, and write values */

            gseek(gx->gstream, 740L);
            ok = gwrite(gx->gstream, offsettbl, height*4);

        }
        if (buf) gfree(buf);
        if (offsettbl) gfree(offsettbl);
    }
    return(ok);
}

int GCALL RLA_wclose(GINSTANCE *gx)
{
    int ok=0;

    if (gx)
        ok = gfree((void *) gx);

    return(ok);
}


