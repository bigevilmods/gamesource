// (c) 2004-2007 Electronic Arts Inc.
#ifndef __ZIPWRITE
#define __ZIPWRITE 1

#include <stdio.h>
#include <string.h>
#include "zlib.h"
#include "zutil.h"
#include "codex.h"
#include "zipcodex.h"

/****************************************************************/
/*  Encode Function                                             */
/****************************************************************/

int GCALL ZIP_encode(void *compresseddata, const void *source, int sourcesize, int *opts)
{
    ZipInternalHeader *ZIHeader = (ZipInternalHeader *)compresseddata;
    z_stream stream;
    unsigned int outbuflen;
    int level = Z_DEFAULT_COMPRESSION;
    int err;

    /* Clear totals in z_stream structure */
    stream.total_out = 0;
    stream.total_in  = 0;

    /* If source is NULL then just build a header */
    if (source && sourcesize != 0)
    {
        /* Set the compression level */
        if (opts)
            if (opts[0] >= 0 || opts[0] <= 9)
                level = opts[0];

        /* Skip the internal header for now */
        compresseddata = (void *)((char *)compresseddata + ZIP_INTERNAL_HEADER_SIZE);
        outbuflen = (unsigned int)(sourcesize);

        if (level != 0)
        {
            stream.zalloc = (alloc_func)0;
            stream.zfree  = (free_func)0;
            stream.opaque = (voidpf)0;

            err = deflateInit2(&stream, level, Z_DEFLATED, -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY);
            if (err != Z_OK)
            {
                printf("ZIPCODEX - Error in <deflateInit2> <%i>\n", err);
                return 0;
            }

            stream.next_in  = (Bytef *)source;
            stream.next_out = (Byte *)compresseddata;

            while (stream.total_in != (uLong)sourcesize &&
                    stream.total_out < outbuflen)
            {
                stream.avail_in = qmin(524288, sourcesize - stream.total_in);
                if (sourcesize - stream.total_in < 524288)
                    stream.avail_out = outbuflen - stream.total_out;
                else
                    stream.avail_out = qmin(524288, outbuflen - stream.total_out);
                err = deflate(&stream, Z_NO_FLUSH);
                if (err != Z_OK)
                {
                    /* If the buffer is too small then return outbuflen bigger than sourcesize */
                    if (err == Z_BUF_ERROR)
                        return sourcesize + ZIP_INTERNAL_HEADER_SIZE + 1;
                    printf("ZIPCODEX - Error in <deflate> <%i>\n", err);
                    return 0;
                }
            }

            for (;;)
            {
                stream.avail_in =  qmin(524288, sourcesize  - stream.total_in);
                if (sourcesize - stream.total_in < 524288)
                    stream.avail_out = outbuflen - stream.total_out;
                else
                    stream.avail_out = qmin(524288, outbuflen - stream.total_out);
                err = deflate(&stream, Z_FINISH);
                if (err == Z_STREAM_END) break;
                if (err != Z_OK)
                {
                    /* If the buffer is too small then return outbuflen bigger than sourcesize */
                    if (err == Z_BUF_ERROR)
                        return sourcesize + ZIP_INTERNAL_HEADER_SIZE + 1;
                    printf("ZIPCODEX - Error in <deflateEnd> <%i>\n", err);
                    return 0;
                }
            }
            deflateEnd(&stream);
        }
        else
        {
            stream.total_out = outbuflen;
            memcpy(compresseddata, source, sourcesize);
        }
    }
    else if (source && sourcesize == 0)
    {
        printf("ZIPCODEX - Input buffer length is 0\n");
        return 0;
    }
    else if (!source && sourcesize != 0)
    {
        printf("ZIPCODEX - Input buffer length is not 0, but input buffer is NULL\n");
        return 0;
    }

    /* Write the codex header */
    gputm(ZIHeader->signature, ZIP_INTERNAL_SIG, 2);
    gputm(ZIHeader->uncompressedsize, stream.total_in, 4);

    return (int)(stream.total_out+ZIP_INTERNAL_HEADER_SIZE);
}

#endif

