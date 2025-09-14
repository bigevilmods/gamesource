// (c) 1995-2004 Electronic Arts Inc.

// version 0 - simple reference
// version 1 - short hash, quick2
// version 2 - quick3
// version 3 - quick3 renamed to 2, copylong added
// version 4 - int safe
// version 5 - quick 2 needs no link

#ifndef __REFWRITE
#define __REFWRITE 1

#ifndef QUICK
#define QUICK 0
#endif

// for alloca
#if defined(_MSC_VER) || defined(__R5900)
#include <malloc.h>
#elif defined(__PPU__)
#include <alloca.h>
#include <stdlib.h>
#else
#include <stdlib.h>
#endif

// for memset
#include <string.h>

// gint_t is at least 16 bits (SPU), and can be 64 bits
#ifdef __PPU__
#include <stdint.h>
#define gint_t uint64_t
#else
#define gint_t unsigned int
#endif

#if QUICK
// compresses faster
#define MAXMAXBACK 16384
#else
// compresses better
#define MAXMAXBACK 131072
#endif

/****************************************************************/
/*  Internal Functions                                          */
/****************************************************************/

static gint_t matchlen(unsigned char *s,unsigned char *d, gint_t maxmatch)
{
    gint_t current=0;

    while (maxmatch>=4)
    {
        if (s[0]!=d[0]) return current;
        if (s[1]!=d[1]) return current+1;
        if (s[2]!=d[2]) return current+2;
        if (s[3]!=d[3]) return current+3;
        s += 4;
        d += 4;
        current += 4;
        maxmatch -= 4;
    }

    if (maxmatch<1 || s[0]!=d[0]) return current;
    if (maxmatch<2 || s[1]!=d[1]) return current+1;
    if (maxmatch<3 || s[2]!=d[2]) return current+2;
    if (maxmatch<4 || s[3]!=d[3]) return current+3;

    return current+4;
}

#if 0

// simple version
static inline gint_t matchlen(unsigned char *s,unsigned char *d, gint_t maxmatch)
{
    gint_t current=0;
    for (; current<maxmatch && *s++==*d++; ++current) ;
    return(current);
}

#endif

// short saves memory.  16k for realtime performance
#ifdef __SPU__
#define int_hash_t unsigned short
#else
#define int_hash_t unsigned int
#endif

// DJB2 hash
#define HASHSIZE 16384
static inline int_hash_t HASH(const unsigned char *s)
{
    int_hash_t hash  = (int_hash_t) (s[0]*1089 + s[1]*33+s[2]);

    return hash&(HASHSIZE-1);
}

//#define HASH(cptr) (int)((((unsigned int)(unsigned char)cptr[0]<<8) | ((unsigned int)(unsigned char)cptr[2])) ^ ((unsigned int)(unsigned char)cptr[1]<<4))

#if defined(_MSC_VER)
#define uint32_t unsigned int
#define uint8_t unsigned char
#endif

#if 0
inline unsigned char *copylongn(unsigned char *d, unsigned char *s,unsigned int run)
{
    if (run>=4)
    {
        do
        {
            uint32_t v = *(uint32_t *) s;
            s += 4;
            *((uint32_t *) d) = v;

            d += 4;
            run -= 4;
        } while (run>=4);
    }

    if (run)
        do
        {
            uint8_t v = *(uint8_t *) s;
            run -= 1;
            *((uint8_t *) d) = v;

            s += 1;
            d += 1;
        } while (run);

    return (unsigned char *)d;
}
#endif

inline unsigned char *copylongn(unsigned char *d, unsigned char *s,unsigned int run)
{
    memcpy(d,s,run);
    return d+run;
}

int refcompress(unsigned char *from, int len, unsigned char *dest, int maxback, int quick, void *hashbuf)
{
    gint_t tlen;
    gint_t tcost;
    unsigned char *tptr;
    unsigned char *cptr;
    unsigned char *rptr;
    int_hash_t hash;
    int_hash_t hoffset;
    int_hash_t minhoffset;
    int i;

    unsigned char *d = dest;
    if (len>0xffffff)  // 32 bit header required
    {
        d[0] = (unsigned char) 0x90;
        d[1] = (unsigned char) 0xfb;
        d[2] = (unsigned char) ((len>>24)&255);
        d[3] = (unsigned char) ((len>>16)&255);
        d[4] = (unsigned char) ((len>> 8)&255);
        d[5] = (unsigned char) ((len    )&255);
        d += 6;
    }
    else
    {
        d[0] = (unsigned char) 0x10;
        d[1] = (unsigned char) 0xfb;
        d[2] = (unsigned char) ((len>>16)&255);
        d[3] = (unsigned char) ((len>> 8)&255);
        d[4] = (unsigned char) ((len    )&255);
        d += 5;
    }

    int_hash_t *hashtbl= (int_hash_t *) hashbuf;
    memset(hashtbl,0,HASHSIZE*sizeof(int_hash_t));
    int_hash_t *link = hashtbl+HASHSIZE;
    if (quick>=2) link = 0;

    int maxbackmask = maxback-1;
    if (quick>=2 && maxback>16383) maxback = maxbackmask = 16383;

    gint_t run = 0;
    cptr = rptr = from;

    len -= 4;
    while (len>=0)
    {
        unsigned int boffset = 0;
        gint_t blen = 2;
        gint_t bcost = 2;

        gint_t mlen = 1028;
        if (len<1028) mlen = (gint_t)len;

        if (quick>=2 && mlen>67) mlen = 67;

        tptr=cptr-1;
        hash = HASH(cptr);
        hoffset = hashtbl[hash];
        minhoffset = (int_hash_t) (((int)((cptr-from)-maxbackmask)<1) ?1: (int)((cptr-from)-maxbackmask));

        if (hoffset>=minhoffset)
        {
            do
            {
                tptr = from+hoffset;
                if (cptr[blen]==tptr[blen])
                {
                    tlen = matchlen(cptr,tptr,mlen);

                    unsigned int toffset = (unsigned int)((cptr-1)-tptr);
                    if (toffset<1024 && tlen<=10)       /* two byte short form */
                        tcost = 2;
                    else if (toffset<16384 && tlen<=67) /* three byte long form */
                        tcost = 3;
                    else                                /* four byte very long form */
                        tcost = 4;

                    if (tlen-tcost+4 > blen-bcost+4)
                    {
                        blen = tlen;
                        bcost = tcost;
                        boffset = toffset;
                        if (blen>=1028) break;
                    }
                }
                if (quick>=2) hoffset = 0;
                else hoffset = link[hoffset&maxbackmask];
            } while (hoffset >= minhoffset);
        }

        if (bcost>=blen || len<4)
        {
            hoffset = (int_hash_t)(cptr-from);

            if (quick!=2)
                link[hoffset&maxbackmask] = hashtbl[hash];
            hashtbl[hash] = hoffset;

            ++run;
            ++cptr;
            --len;
        }
        else
        {
            while (run>3)                   /* literal block of data */
            {
                tlen = run&~3;
                if (tlen>112) tlen=112;
                run -= tlen;
                d[0] = (unsigned char) (0xe0+(tlen>>2)-1);
                copylongn(d+1,rptr,tlen);
                rptr += tlen;
                d += 1+tlen;
            }
            if (bcost==2)                   /* two byte int form */
            {
                d[0] = (unsigned char) (((boffset>>8)<<5) + ((blen-3)<<2) + run);
                d[1] = (unsigned char) boffset;
                d += 2;
            }
            else if (bcost==3)              /* three byte int form */
            {
                d[0] = (unsigned char) (0x80 + (blen-4));
                d[1] = (unsigned char) ((run<<6) + (boffset>>8));
                d[2] = (unsigned char) boffset;
                d += 3;
            }
            else                            /* four byte very int form */
            {
                d[0] = (unsigned char) (0xc0 + ((boffset>>16)<<4) + (((blen-5)>>8)<<2) + run);
                d[1] = (unsigned char) (boffset>>8);
                d[2] = (unsigned char) (boffset);
                d[3] = (unsigned char) (blen-5);
                d += 4;
            }
            if (run)
            {
                copylongn(d, rptr, run);
                d += run;
                run = 0;
            }

            if (quick)
            {
                if (quick>=2)
                {
                    hoffset = (int_hash_t)(cptr-from);
//                    link[hoffset&maxbackmask] = 0;
                    hashtbl[hash] = hoffset;
                }
                else // if (quick==1)
                {
                    hoffset = (int_hash_t)(cptr-from);
                    link[hoffset&maxbackmask] = hashtbl[hash];
                    hashtbl[hash] = hoffset;
                }
                cptr += blen;
            }
            else
            {
                for (i=0; i < (int)blen; ++i)
                {
                    hash = HASH(cptr);
                    hoffset = (int_hash_t)(cptr-from);
                    link[hoffset&maxbackmask] = hashtbl[hash];
                    hashtbl[hash] = hoffset;
                    ++cptr;
                }
            }

            rptr = cptr;
            len -= blen;
        }
    }
    len += 4;
    run += len;
    while (run>3)                       /* no match at end, use literal */
    {
        tlen = run&~3;
        if (tlen>112) tlen=112;   // minus 4 to allow last run to be literal
        run -= tlen;
        d[0] = (unsigned char) (0xe0+(tlen>>2)-1);
        copylongn(d+1,rptr,tlen);
        rptr += tlen;
        d += 1+tlen;
    }

    d[0] = (unsigned char) (0xfc+run); /* end of stream command + 0..3 literal */
    d += 1;
    if (run)
    {
        copylongn(d,rptr,run);
        d += run;
    }

    return (int)(d-dest);
}

/****************************************************************/
/*  Encode Function                                             */
/****************************************************************/

// codex API

#if !defined(GCALL)
#if defined(_MSC_VER) && !defined(_XBOX)
#define GCALL __stdcall
#else
#define GCALL
#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif
int GCALL REF_encode(void *compresseddata, const void *source, int sourcesize, int *opts=0);
#ifdef __cplusplus
}
#endif

int GCALL REF_encode(void *compresseddata, const void *source, int sourcesize, int *opts)
{
    int maxback=MAXMAXBACK;
    int quick=QUICK;

    if (opts)
    {
        maxback = opts[0];
        quick =   opts[1];
    }

    const size_t hashsize = ((quick>=2) ? (HASHSIZE)*sizeof(int_hash_t) : (HASHSIZE+MAXMAXBACK)*sizeof(int_hash_t));  // 524352 bytes maximum, 65536 for spu; quick2 doesnt need link. 32768 for spu (shorts) or 65536 for others (ints)

	void * hashtbl = malloc(hashsize); // use alloca if you can, but requires a stack of 64K at least.

    int plen = refcompress((unsigned char *)source, sourcesize, (unsigned char *)compresseddata, maxback, quick, hashtbl);

	free(hashtbl);

    return(plen);
}

#endif

