// (c) Electronic Arts. All Rights Reserved.
#ifndef __ZIPREAD
#define __ZIPREAD 1

#include <stdio.h>
#include <string.h>
#include "xceedzip/zlib.h"
#include "codex.h"
#include "zipcodex.h"
#include "spitendian.h"


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

size_t GCALL ZIP_size(const void *compresseddata)
{
    size_t len=0;
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

size_t GCALL ZIP_decode(void *dest, const void *compresseddata, size_t *compressedsize)
{
//    ZipInternalHeader *ZIHeader = (ZipInternalHeader *)compresseddata;
    int err;
    z_stream stream;
    size_t inbuflen=0;
    size_t outbuflen = ZIP_size(compresseddata);

    compresseddata = (void *) ((char *) compresseddata + ZIP_INTERNAL_HEADER_SIZE);

    /* Do the decompression */
    {
        memset(&stream,0,sizeof(stream));
        stream.zalloc = (alloc_func)0;
        stream.zfree = (free_func)0;
        stream.opaque = (voidpf)0;

        /* Call with -MAX_WBITS to suppress zlib header!!! */
        err = inflateInit2(&stream, -MAX_WBITS);
//        err = inflateInit2(&stream, -MAX_WBITS64);
        if (err != Z_OK)
        {
            printf("ZIPCODEX - Error in <inflateInit2>\n");
            return 0;
        }

        stream.next_in  = (unsigned char *) compresseddata;
        stream.next_out = (unsigned char *) dest;

        for (;;)
        {
            stream.avail_in  = 32768;
            stream.avail_out = (int)(outbuflen - stream.total_out);
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

