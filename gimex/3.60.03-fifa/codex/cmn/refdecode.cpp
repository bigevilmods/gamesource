// (c) 1995-2007 Electronic Arts Inc.

// 1.0 122 MB/s
// 1.1 if do/while
// 1.2 4 byte

//#define USEC 1
#ifndef __REFREAD
#define __REFREAD 1

#include <string.h>
#include "codex.h"
#include "refcodex.h"

/****************************************************************/
/*  Information Functions                                       */
/****************************************************************/

/* check for reasonable header: */
/* 10fb header */

bool GCALL REF_is(const void *compresseddata)
{
    bool ok=false;
    int packtype=ggetm(compresseddata,2);

    if (packtype==0x10fb
     || packtype==0x11fb
     || packtype==0x90fb
     || packtype==0x91fb)
        ok = true;

    return(ok);
}

/****************************************************************/
/*  Decode Functions                                            */
/****************************************************************/

int GCALL REF_size(const void *compresseddata)
{
    int len=0;
    int packtype=ggetm(compresseddata,2);
    int ssize=(packtype&0x8000)?4:3;

    if (packtype&0x100)     /* 11fb */
    {
        len = ggetm((char *)compresseddata+2+ssize,ssize);
    }
    else                    /* 10fb */
    {
        len = ggetm((char *)compresseddata+2,ssize);
    }

    return(len);
}

#if defined(__PPU__) && !defined(__SNC__) && !defined(USEC)

// copy long, sloppy with overlap

static inline unsigned char *copylongn(unsigned char *d, unsigned char *s,int64_t run)
{
    register uint64_t si = (int64_t) s;
    register uint64_t di = (int64_t) d;
    register uint64_t temp;

    if ((di-si)>=4)
    {
        do
        {
            __asm__ ("lwz %0,0(%1)" : "=r"(temp): "r"(si));
            __asm__ ("stw %0,0(%1)" :: "r"(temp), "r"(di));
            si += 4;
            di += 4;
       
        } while ((run-=4)>0);

        return (unsigned char *)di+run;
    }

    do
    {
        __asm__ ("lbz %0,0(%1)" :"=r"(temp):  "r"(si));
        ++si;
        __asm__ ("stb %0,0(%1)" :: "r"(temp), "r"(di));
        ++di;
    } while (--run);
    return (unsigned char *)di;
}

// copy 1 to 3, sloppy with no overlap

static inline unsigned char *copyn(unsigned char *d, unsigned char *s,int64_t run)
{

    register uint64_t si = (int64_t) s;
    register uint64_t di = (int64_t) d;
    register uint64_t temp;

    __asm__ ("lwz %0,0(%1)" : "=r"(temp): "r"(si));
    __asm__ ("stw %0,0(%1)" :: "r"(temp), "r"(di));

    return d+run;
}

// copy long literal with no overlap, and must be clean (not sloppy)

static inline unsigned char *copylitn(unsigned char *d, unsigned char *s,int64_t run)
{
    register uint64_t si = (int64_t) s;
    register uint64_t di = (int64_t) d;
    register uint64_t temp=0;

    if (run>=4)
    {
        do
        {
            __asm__ ("lwz %0,0(%1)" : "=r"(temp): "r"(si));
            __asm__ ("stw %0,0(%1)" :: "r"(temp), "r"(di));
            si += 4;
            di += 4;
       
        } while ((run-=4)>=4);
    }

    if (run)
        do
        {
            __asm__ ("lbz %0,0(%1)" :"=r"(temp):  "r"(si));
            ++si;
            __asm__ ("stb %0,0(%1)" :: "r"(temp), "r"(di));
            ++di;
        } while (--run);
    return (unsigned char *)di;
}

#elif (defined(__PPU__) && defined(__SNC__)) || defined(USEC)

static inline unsigned char *copylongn(unsigned char *d, unsigned char *s,int run)
{
    if ((d-s)>=4)
    {
        do
        {
            uint32_t v = *(uint32_t *) s;
            *((uint32_t *) d) = v;

            s += 4;
            d += 4;
       
        } while ((run-=4)>0);

        return (unsigned char *)d+run;
    }

    do
    {
        uint8_t v = *(uint8_t *) s;
        *((uint8_t *) d) = v;

        s += 1;
        d += 1;
    } while (--run);

    return (unsigned char *)d;
}

static inline unsigned char *copyn(unsigned char *d, unsigned char *s,int run)
{
    uint32_t v = *(uint32_t *) s;
    *((uint32_t *) d) = v;
    return d+run;
}


static inline unsigned char *copylitn(unsigned char *d, unsigned char *s,int run)
{
    if (run>=4)
    {
        do
        {
            uint32_t v = *(uint32_t *) s;
            *((uint32_t *) d) = v;
            s += 4;
            d += 4;
        } while ((run-=4)>=4);
    }

    if (run)
        do
        {
            uint8_t v = *(uint8_t *) s;
            *((uint8_t *) d) = v;

            s += 1;
            d += 1;
        } while (--run);
    return d;
}


#else

static inline unsigned char *copyn(unsigned char *d, unsigned char *s,int run)
{
    do
    {
        *d++ = *s++;
    } while (--run);
    return d;
}

#define copylongn copyn
#define copylitn copyn
#endif



int GCALL REF_decode(void *dest, const void *compresseddata, int *compressedsize)
{
    unsigned char *s;
    unsigned char *ref;
    unsigned char *d;
    unsigned char first;
    unsigned char second;
    unsigned char third;
    unsigned char forth;
    unsigned int  run;
    unsigned int  type;
    int          ulen;

    s = (unsigned char *) compresseddata;
    d = (unsigned char *) dest;
    ulen = 0L;

    if (s)
    {
        type = *s++;
        type = (type<<8) + *s++;

        if (type&0x8000) /* 4 byte size field */
        {
            if (type&0x100)                       /* skip ulen */
                s += 4;

            ulen = *s++;
            ulen = (ulen<<8) + *s++;
            ulen = (ulen<<8) + *s++;
            ulen = (ulen<<8) + *s++;
        }
        else
        {
            if (type&0x100)                       /* skip ulen */
                s += 3;

            ulen = *s++;
            ulen = (ulen<<8) + *s++;
            ulen = (ulen<<8) + *s++;
        }

        for (;;)
        {
            first = *s;
            if (!(first&0x80))          /* short form */
            {
                do
                {
                    second = s[1];
                    s += 2;
                    run = first&3;

                    if (run)
                    {
                        d = copyn(d,s,run);
                        s += run;
                    }

                    ref = d-1 - (((first&0x60)<<3) + second);
                    run = ((first&0x1c)>>2)+3;

                    d = copylongn(d,ref,run);

                    first = *s;
                } while (!(first&0x80));
            }
            if (!(first&0x40))          /* long form */
            {
                second = s[1];
                third = s[2];
                s += 3;
                run = second>>6;
                if (run)
                {
                    d = copyn(d,s,run);
                    s += run;
                }

                ref = d-1 - (((second&0x3f)<<8) + third);

                run = (first&0x3f)+4;

                d = copylongn(d,ref,run);

                continue;
            }
            if (!(first&0x20))          /* very long form */
            {
                second = s[1];
                third = s[2];
                forth = s[3];
                s += 4;
                run = first&3;
                if (run)
                {
                    d = copyn(d,s,run);
                    s += run;
                }

                ref = d-1 - (((first&0x10)>>4<<16) +  (second<<8) + third);

                run = ((first&0x0c)>>2<<8) + forth + 5;

                d = copylongn(d,ref,run);
                continue;
            }
            ++s;
            run = ((first&0x1f)<<2)+4;  /* literal */
            if (run<=112)
            {
                d = copylitn(d,s,run);
                s += run;
                continue;
            }
            run = first&3;              /* eof (+0..3 literal) */
            if (run)
            {
                copylitn(d,s,run);
                s += run;
            }
            break;
        }
    }
    if (compressedsize)
        *compressedsize = (int)((char *)s-(char *)compresseddata);
    return(ulen);
}

#endif

