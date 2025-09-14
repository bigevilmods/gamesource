// (c) 1995-2007 Electronic Arts Inc.

#include "packshpi.h"

typedef unsigned char ubyte;
typedef unsigned short uword;
typedef unsigned int ulong;

typedef signed char byte;
typedef signed short word;

/* Usage: */
/* BuildNBitsPackedLine(input,output,length,inbpp,outbpp,workbuffer,keeptransparencyflag) */
/* Returns length of packed data.  If passed an output point of 0, just returns length of */
/* packed data.  workbuffer must be large enough to accomodate 2 bytes per pixel in the line. */

#define PACK_EOL 0
#define PACK_LITERAL 1
#define PACK_TRANSPARENT 2
#define PACK_REPEAT 3       /* packet ids */

#define PACK_TYPE_SIZE 2    /* 2 bits pack type size */
#define PACK_LENGTH_SIZE 6  /* 6 bits count size */

#define MIN_PADDING 32      /* pad to 32 bit multiples */

static int ReadPixel(ubyte *srcData,unsigned int offset,unsigned int length)
{
    int    TempLong;

    if (length==32)
    {
        TempLong=ggetbitfield((char *)srcData,offset,length);
        if (TempLong&0xFF000000) TempLong=celpixel8to5(TempLong)|0x00008000;
        else TempLong=celpixel8to5(TempLong);
        return(TempLong);
    }
    else return(ggetbitfield((char *)srcData,offset,length));
}

static ulong CheckTransparent(ubyte* srcData, ulong inbits, ulong column,int TransColor,int BlackPacked)
{
    if (!BlackPacked) return(0);
    if (ReadPixel(srcData,column*inbits,inbits)==TransColor) return(1);
    else return(0);
}

static int BuildNBitsPackedLine(ubyte *srcData,ubyte *dstLine,uword loopCnt,uword inbits, uword outbits, ubyte *countbuf,int TransColor,int BlackPacked)
{

    ulong inoff,outoff,pixel,trans,column,listindex=0,tempoff,offset,nextlineoff;
    uword packetsize,packettype;
    int myLoop;
    int i;

/* Step 1:  Generate a list of counts and types for the line of data.  Format as follows:  */
/* word 1 = string type (0=end,1=literal run,2=packed transparent run,3=packed run) */
/* word 2 = string count */

    inoff = column = 0;
    myLoop = loopCnt;

    while (myLoop)
    {
        /* If at least 2 more bytes and they are equal, then replicate */

        pixel = ReadPixel(srcData,inoff,inbits);        /* get current pixel */
        trans = CheckTransparent(srcData,inbits,column,TransColor,BlackPacked);

        if (trans)  /* 1 transparent pixel */
        {
            for (i=1; i< 64 && (int)i<myLoop; i++)
            {
                if (!CheckTransparent(srcData,inbits,column+i,TransColor,BlackPacked))
                    break;
            }

            countbuf[listindex++] = PACK_TRANSPARENT;

            countbuf[listindex++] = (unsigned char)i;

            column+=i;
            inoff += inbits*i;  /* point to next pixel after transparent */
            myLoop -= i;        /* decrement count by 1 */

        }
        else if (myLoop >= 2  &&  pixel == (ulong)ReadPixel(srcData,inoff+inbits,inbits) && !CheckTransparent(srcData,inbits,column+1,TransColor,BlackPacked))
        {
            tempoff = inoff+(inbits*2);
            for (i=2; i< 64 && (int)i<myLoop; i++)
            {
                if ((ulong)ReadPixel(srcData,tempoff,inbits) != pixel || CheckTransparent(srcData,inbits,column+i,TransColor,BlackPacked))
                    break;

                tempoff+=inbits;
            }


        /* i now holds length of run */


            countbuf[listindex++] = PACK_REPEAT;

            countbuf[listindex++] = (unsigned char)i;

            inoff += i*inbits;  /* point to next pixel after run */
            column+=i;
            myLoop -= i;        /* decrement count by number of pixels in run */

        }
        else  /* Count literally till 2 byte run or transparency found */
        {
            tempoff = inoff+inbits;
            for (i=1; i< 64 && (int)i<myLoop; i++)
            {
                if (CheckTransparent(srcData,inbits,column+i,TransColor,BlackPacked))   /* check if pixel is transparent */
                    break;

                pixel = ReadPixel(srcData,tempoff,inbits);

                /*if (pixel == transparentCol) break; */
                if ( pixel == (ulong)ReadPixel(srcData,tempoff+inbits,inbits) && pixel == (ulong)ReadPixel(srcData,tempoff+(inbits<<1),inbits))
                    break;

                tempoff+=inbits;
            }

        /* i now holds length of run */

            countbuf[listindex++] = PACK_LITERAL;

            countbuf[listindex++] = (unsigned char)i;

            column+=i;
            inoff += i*inbits;  /* point to next pixel after run */
            myLoop -= i;        /* decrement count by number of pixels in run */

        }

    }

    countbuf[listindex++] = PACK_EOL;       /* Terminate list */
    countbuf[listindex++] = PACK_EOL;



/* Step 2:  Munge the list to optimize by merging short runs into literal packs and stuff  */


/* To Do!!! */

/* For now, I'll just strip off the last packed transparent packets (if any) */

    for (i=listindex-4; i >= 0; i-=2)
    {
        if (countbuf[i] != PACK_TRANSPARENT)
            break;

        countbuf[i] = PACK_EOL;
        countbuf[i+1] = PACK_EOL;   /* end packing early if rest of line is transparent */
    }

/* Step 3:  Using the list, generate a packed cel */


    inoff = outoff = listindex = 0;


/* First, let's save space for a next line pointer */

    nextlineoff = outoff;

    if (outbits < 8)
        outoff += 8;
    else
        outoff += 16;       /* sizes of headers for various bpp (8 bits for 1,2,4 and 6, 16 bits for 8 and 16) */

    while ((packettype = countbuf[listindex++]) != PACK_EOL)
    {
        packetsize = countbuf[listindex++];

        /* Generate a packet header */

        if (dstLine)
            gsetbitfield((char *)dstLine,outoff,PACK_TYPE_SIZE,packettype);

        outoff += PACK_TYPE_SIZE;

        /* Generate a packet count */

        if (dstLine)
            gsetbitfield((char *)dstLine,outoff,PACK_LENGTH_SIZE,packetsize-1);

        outoff += PACK_LENGTH_SIZE;

        switch(packettype)
        {
            case PACK_LITERAL:

                for (i=0; i<packetsize; i++)    /* Copy all the pixels */
                {
                    if (dstLine)
                        gsetbitfield((char *)dstLine,outoff,outbits,ReadPixel(srcData,inoff,inbits));

                    outoff += outbits;
                    inoff += inbits;
                }
                break;

            case PACK_TRANSPARENT:

                inoff += packetsize*inbits;     /* Skip the transparent pixels */

                break;

            case PACK_REPEAT:

                if (dstLine)
                    gsetbitfield((char *)dstLine,outoff,outbits,ReadPixel(srcData,inoff,inbits));   /* Copy the repeated pixel */

                outoff += outbits;
                inoff += packetsize*inbits;     /* Skip the repeated pixels */

                break;
        }

    }

    /* End the list */

    if (dstLine)
        gsetbitfield((char *)dstLine,outoff,PACK_TYPE_SIZE,PACK_EOL);

    outoff += PACK_TYPE_SIZE;

    if (outoff < (MIN_PADDING*2))   /* Pad to at least 2 dwords of data */
    {
        if (dstLine)
            gsetbitfield((char *)dstLine,outoff,(MIN_PADDING*2)-outoff,0);  /* pad with 0's */

        outoff = MIN_PADDING*2;
    }


    if (outoff % MIN_PADDING)   /* Pad to fill last dword of output stream */
    {
        if (dstLine)
            gsetbitfield((char *)dstLine,outoff,(MIN_PADDING-(outoff % MIN_PADDING)),0);    /* pad with 0's */

        outoff += (MIN_PADDING-(outoff % MIN_PADDING));
    }

    /* Now, set the next line pointer */

    offset = (outoff/MIN_PADDING) - 2;

    if (outbits < 8)
    {
        if (dstLine)
            gsetbitfield((char *)dstLine,nextlineoff,8,offset);
    }
    else
    {
        if (dstLine)
            gsetbitfield((char *)dstLine,nextlineoff,16,offset);
    }

    return (outoff/8);  /* return size in bytes */

}

int packshapedata(char *s, int srcRowByte, char *d, int w, int h, int sbits, int bpp,int TransColor,int BlackPacked)
{
    int line;
    int height0;
    int step;
    int workWidth;
    int sizeLine;
    int sizeData;
    char *spacebuf;
    char *sourceLine;
    char *destLine;

    height0 = h;
    step = 1;
    workWidth = w;
    sizeData = 0;
    sourceLine = s;
    destLine = d;

    spacebuf=(char*)galloc(4096);
    for (line=0; line < height0;line+=step)
    {
        sizeLine = BuildNBitsPackedLine((ubyte *)sourceLine,(ubyte *)destLine,(uword)workWidth,(uword)sbits,(uword)bpp,(ubyte *)spacebuf,TransColor,BlackPacked);
        sizeData += sizeLine;
        if (d)
            destLine += sizeLine;
        sourceLine += srcRowByte;
    }
    gfree(spacebuf);
    return(sizeData);
}
