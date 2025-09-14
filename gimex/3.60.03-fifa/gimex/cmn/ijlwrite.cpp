// (c) 1994-2007 Electronic Arts Inc.

#include <string.h>
#include "windows.h"
#include "gimex.h"
#include "ijlgimex.h"
#include "ijl.h"
#include "jpggimex.h"

int IJL_loaddll(void);
extern IJLERR (WINAPI *ijlInitPtr)( JPEG_CORE_PROPERTIES* jcprops );
extern IJLERR (WINAPI *ijlReadPtr)( JPEG_CORE_PROPERTIES* jcprops, IJLIOTYPE iotype );
extern IJLERR (WINAPI *ijlWritePtr)( JPEG_CORE_PROPERTIES* jcprops, IJLIOTYPE iotype );
extern IJLERR (WINAPI *ijlFreePtr)( JPEG_CORE_PROPERTIES* jcprops );

/****************************************************************/
/*  Export Functions                                            */
/****************************************************************/

int GCALL IJL_wopen(GINSTANCE **gxptr, GSTREAM *g, const char *pathname, int numframes)
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

int GCALL IJL_write(GINSTANCE *gx, const GINFO *ginfo, char *source, int rowbytes)
{
    int quality = ginfo->packed?ginfo->quality:100;
    char *framebuf;
    char *jpegbuf;
    int jpegsize;
    char *lineptr;
    int x;
    int y;
    ARGB *s;
    int ok=0;

    if (ginfo->bpp>8)
    {
        if (IJL_loaddll())
        {
            framebuf = (char *)galloc(ginfo->width*ginfo->height*3);

            if (framebuf)
            {
                lineptr = framebuf;
                for (y=0; y<ginfo->height; ++y)
                {
                    s = (ARGB *) (source+rowbytes*y);

                    for (x=0; x<ginfo->width; ++x)
                    {
                        lineptr[0] = s->b;
                        lineptr[1] = s->g;
                        lineptr[2] = s->r;
                        ++s;
                        lineptr += 3;
                    }
                }

                jpegsize = (int)(ginfo->width*ginfo->height*3*1.10f+1024);   // guess
                jpegbuf = (char *)galloc(jpegsize);

                if (jpegbuf)
                {
                    JPEG_CORE_PROPERTIES image;

                    strcpy(jpegbuf,"jpeg buffer");
                    memset( &image, 0, sizeof( JPEG_CORE_PROPERTIES ) );

                    if ( ijlInitPtr( &image ) == IJL_OK )
                    {
                        image.DIBBytes = (unsigned char *)framebuf;
                        image.DIBWidth    = ginfo->width;
                        image.DIBHeight   = ginfo->height;
                        image.DIBPadBytes = 0;
                        image.DIBChannels = 3;
                        image.DIBColor    = IJL_BGR;
                        image.cconversion_reqd = TRUE;

                        image.JPGBytes = (unsigned char *)jpegbuf;
                        image.JPGWidth    = ginfo->width;
                        image.JPGHeight   = ginfo->height;
                        image.JPGSizeBytes = jpegsize;
                        image.jquality    =  quality;
                        image.JPGChannels    = 3;
                        image.JPGColor       = IJL_YCBCR;
                        if (ginfo->quality<75)
                            image.JPGSubsampling = IJL_411;  // note photoshop does the same
                        else
                            image.JPGSubsampling = IJL_422;
                        image.JPGThumbWidth  = 0;
                        image.JPGThumbHeight = 0;
//                    	image.jprops.dcttype = IJL_AAN; // high speed/low accuracy

                        if( ijlWritePtr( &image, IJL_JBUFF_WRITEWHOLEIMAGE) == IJL_OK )
                        {
                            ok = gwrite(gx->gstream,jpegbuf,image.JPGSizeBytes);
                            if (ok)
                                ok = 100;
                        }

                        if( ijlFreePtr( &image ) != IJL_OK )
                        {
                            ok = 0;
                        }
                    }
                }
                gfree(jpegbuf);
            }
            gfree(framebuf);
        }
        else
        {
            ok = JPG_write(gx, ginfo, source, rowbytes);
        }
    }

    return(ok);
}

int GCALL IJL_wclose(GINSTANCE *gx)
{
    int ok=0;

    if (gx)
        ok = gfree((void *) gx);

    return(ok);
}



