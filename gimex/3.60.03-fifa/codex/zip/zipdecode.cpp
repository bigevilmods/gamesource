// (c) 2004-2007 Electronic Arts Inc.
#ifndef __ZIPREAD
#define __ZIPREAD 1

#include <stdio.h>
#include <string.h>
#include "zlib.h"
#include "zutil.h"
#include "codex.h"
#include "zipcodex.h"


/****************************************************************/
/*  Information Functions                                       */
/****************************************************************/

/* check for reasonable header: */
/* 12fb header */

bool GCALL ZIP_is(const void *compresseddata)
{
    bool ok=false;
    int packtype=ggetm(compresseddata,2);

    if (packtype==0x12fb
     || packtype==0x13fb
     || packtype==0x92fb
     || packtype==0x93fb)
        ok = true;

    return(ok);
}

/****************************************************************/
/*  Decode Functions                                            */
/****************************************************************/

int GCALL ZIP_size(const void *compresseddata)
{
    int len=0;
    int packtype=ggetm(compresseddata,2);
    int ssize=(packtype&0x8000)?4:3;

    if (packtype&0x100)     /* 13fb */
    {
        len = ggetm((char *)compresseddata+2+ssize,ssize);
    }
    else                    /* 12fb */
    {
        len = ggetm((char *)compresseddata+2,ssize);
    }

    return(len);
}

int GCALL ZIP_decode(void *dest, const void *compresseddata, int *compressedsize)
{
//    ZipInternalHeader *ZIHeader = (ZipInternalHeader *)compresseddata;
    int err;
    z_stream stream;
    int inbuflen=0;
    int outbuflen = ZIP_size(compresseddata);

    compresseddata = (void *) ((char *) compresseddata + ZIP_INTERNAL_HEADER_SIZE);

    /* Do the decompression */
    {
        memset(&stream,0,sizeof(stream));
        stream.zalloc = (alloc_func)0;
        stream.zfree = (free_func)0;
        stream.opaque = (voidpf)0;

        /* Call with -MAX_WBITS to suppress zlib header!!! */
        err = inflateInit2(&stream, -MAX_WBITS);
        if (err != Z_OK)
        {
            printf("ZIPCODEX - Error in <inflateInit2>\n");
            return 0;
        }

        stream.next_in  = (unsigned char *) compresseddata;
        stream.next_out = (unsigned char *) dest;

        for (;;)
        {
            stream.avail_in  = 2048;
            stream.avail_out = (outbuflen - stream.total_out);
            if (stream.avail_out<=0)
                break;

            err = inflate(&stream, Z_NO_FLUSH);
            if (err == Z_STREAM_END) break;
            if (err != Z_OK)
            {
                printf("ZIPCODEX - Error in <inflate>\n");
                return 0;
            }
        }
        inbuflen = stream.total_in;

        err = inflateEnd(&stream);
        if (err != Z_OK)
        {
            printf("ZIPCODEX - Error in <inflateEnd>\n");
            return 0;
        }
    }

    if (compressedsize)
        *compressedsize = inbuflen;

    return (int)(stream.total_out);
}

#endif

