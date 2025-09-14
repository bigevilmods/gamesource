// (c) 1994-2007 Electronic Arts Inc.

#include <string.h>
#include <stdio.h>
#include "gimex.h"
#include "wctgimex.h"
#include "wavelet.h"

/****************************************************************/
/*  Export Functions                                            */
/****************************************************************/

int GCALL WCT_wopen(GINSTANCE **gxptr, GSTREAM *g, const char *pathname, int numframes)
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

/*
00000: 12001001 00010001 14001e00 1e001400  ................
00010: c9700000 00000000 46548bff 42494c5a  Ip......FT..BILZ
00020: 00800100 c9700000 0f000000 09000000  ....Ip..........
*/

int GCALL WCT_write(GINSTANCE *gx, const GINFO *ginfo, char *source, int rowbytes)
{
    WCTHeaderStruct wctheader;
    int x;
    int y;
    int ok=0;
    int m_nYQuant;
    int m_nPrQuant;
    int m_nPbQuant;
    int m_nAlphaQuant;
    float fYQuant;
    float fPrQuant;
    float fPbQuant;
    float fAlphaQuant;
    char *source24;
    char *source8;
    char *dest24;
    char *dest8;
    unsigned int rgbsize=0;
    unsigned int alphasize=0;
    ARGB *s;
   	CWavelet * pWavelet;

    source24 = (char *) galloc(ginfo->width*3*ginfo->height);
    if (source24)
    {
        dest24 = (char *) galloc(ginfo->width*3*ginfo->height*2+8192);
        if (dest24)
        {
            source8 = (char *) galloc(ginfo->width*ginfo->height);
            if (source8)
            {
                dest8 = (char *) galloc(ginfo->width*ginfo->height*2+8192);
                if (dest8)
                {
                    pWavelet = CreateWavelet();
                    if (pWavelet)
                    {
                        char *s1;

                        s1 = source24;
                        for (y=0; y<ginfo->height; ++y)
                        {
                            s = (ARGB *) (source+rowbytes*y);

                            for (x=0; x<ginfo->width; ++x)
                            {
                                s1[2] = s->r;
                                s1[1] = s->g;
                                s1[0] = s->b;

                                s1 += 3;
                                ++s;
                            }
                        }

                        m_nYQuant = m_nAlphaQuant = gmax(0,gmin(100,100-ginfo->quality));
                        m_nPrQuant = m_nPbQuant =   gmax(0,gmin(100,m_nYQuant*120/100));

                        fYQuant     = 1.0f + 50.0f * (float)m_nYQuant     / 100.0f;
                        fPrQuant    = 1.0f + 25.0f * (float)m_nPrQuant    / 100.0f;
                        fPbQuant    = 1.0f + 25.0f * (float)m_nPbQuant    / 100.0f;
                        fAlphaQuant = 1.0f + 100.0f * (float)m_nAlphaQuant / 100.0f;

                    	pWavelet->Compress(ginfo->width,ginfo->height, true, (const unsigned char *) source24, (unsigned char *)dest24, rgbsize, fYQuant, fPbQuant, fPrQuant);

                        if (ginfo->originalbpp>24)
                        {
                            s1 = source8;
                            for (y=0; y<ginfo->height; ++y)
                            {
                                s = (ARGB *) (source+rowbytes*y);

                                for (x=0; x<ginfo->width; ++x)
                                {
                                    s1[0] = s->a;
                                    s1 += 1;
                                    ++s;
                                }
                            }
                        	pWavelet->Compress(ginfo->width,ginfo->height, false, (const unsigned char *) source8, (unsigned char *)dest8, alphasize, fAlphaQuant, fAlphaQuant, fAlphaQuant);
                        }

                    	WCTPUTFIELD(wctheader.version,0x12);
                        WCTPUTFIELD(wctheader.flags,0);
                        WCTPUTFIELD(wctheader.bpp,ginfo->originalbpp);
                        WCTPUTFIELD(wctheader.mipmaps,1);

                        WCTPUTFIELD(wctheader.width,ginfo->width);
                        WCTPUTFIELD(wctheader.height,ginfo->height);
                        WCTPUTFIELD(wctheader.yquant,m_nYQuant);
                        WCTPUTFIELD(wctheader.prquant,m_nPrQuant);
                        WCTPUTFIELD(wctheader.pbquant,m_nPbQuant);
                        WCTPUTFIELD(wctheader.alphaquant,m_nAlphaQuant);

                        WCTPUTFIELD(wctheader.rgbsize,rgbsize);
                        WCTPUTFIELD(wctheader.alphasize,alphasize);

                        WCTPUTFIELD(wctheader.blue,0);
                        WCTPUTFIELD(wctheader.green,0);
                        WCTPUTFIELD(wctheader.red,0);
                        WCTPUTFIELD(wctheader.alpha,0);

                        ok = gwrite(gx->gstream,&wctheader,28);

                        ok = gwrite(gx->gstream,dest24,rgbsize);

                        if (alphasize)
                            ok = gwrite(gx->gstream,dest8,alphasize);

                       	pWavelet->Release();
                    }
                    gfree(dest8);
                }
                gfree(source8);
            }
            gfree(dest24);
        }
        gfree(source24);
    }
    ++gx->framenum;
    return(ok);
}

int GCALL WCT_wclose(GINSTANCE *gx)
{
    int ok=0;

    if (gx)
        ok = gfree((void *) gx);

    return(ok);
}

