// (c) 1989-2007 Electronic Arts Inc.

/*------------------------------------------------------------------*/
/*                                                                  */
/*                     HUFF XE Compresser v6.10                     */
/*                                                                  */
/*                     by FrANK G. Barchard, EAC                    */
/*                                                                  */
/*                 BTree Pack Module - Aug 12, 1993                 */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/*      *42fb  tree3    huff 5.4     FB3 header (overlap)           */
/*                                                                  */
/*------------------------------------------------------------------*/

/****************************************************************/
/*  Includes                                                    */
/****************************************************************/

#include    "spit.h"
#include    "huff.h"

/****************************************************************/
/*  Btree Variables                                             */
/****************************************************************/

/****************************************************************/
/*  Btree Packer (Replacable)                                   */
/****************************************************************/

void SQadjcount(s,bend,count)
register unsigned char          *s;
unsigned char                   *bend;
FBWORD                          *count;
{
#ifdef __WATCOMC__
    union
    {
        unsigned char           b[4];
        int                     w;
    } i;

#define COUNTADJ(j) i.b[1] = i.b[0]; i.b[0] = *(s+j); ++count[i.w];

    i.w = 0;
    i.b[0] = (unsigned FBWORD) *s++;

#else
    register unsigned FBWORD        i;
#define COUNTADJ(j) i = ((i<<8) | *(s+j));  ++*(count+i);
    i = (unsigned FBWORD) *s++;
#endif
    bend -= 16;

    if (s<bend)
    {
        do
        {
            COUNTADJ(0);
            COUNTADJ(1);
            COUNTADJ(2);
            COUNTADJ(3);

            COUNTADJ(4);
            COUNTADJ(5);
            COUNTADJ(6);
            COUNTADJ(7);

            COUNTADJ(8);
            COUNTADJ(9);
            COUNTADJ(10);
            COUNTADJ(11);

            COUNTADJ(12);
            COUNTADJ(13);
            COUNTADJ(14);
            COUNTADJ(15);
            s += 16;

        } while (s<bend);
    }

    bend += 16;

    if (s<bend)
    {
        do
        {
            COUNTADJ(0);
            ++s;

        } while (s<bend);
    }
}

/* Clear 128 ints (256 words). Done as ints for speed. */

void SQclearcount(tryq, countbuf)
unsigned char       *tryq;
FBWORD              *countbuf;
{
    register int        zero=0;
    int                 i;
    int                 j;
    int             *intptr;

    intptr = (int *) countbuf;
    j = 256;
    do
    {
        if (*tryq)
        {
            *tryq++ = 1;

            i = 8;
            do
            {
                *(intptr+0) = zero;
                *(intptr+1) = zero;
                *(intptr+2) = zero;
                *(intptr+3) = zero;

                *(intptr+4) = zero;
                *(intptr+5) = zero;
                *(intptr+6) = zero;
                *(intptr+7) = zero;

                *(intptr+8) = zero;
                *(intptr+9) = zero;
                *(intptr+10) = zero;
                *(intptr+11) = zero;

                *(intptr+12) = zero;
                *(intptr+13) = zero;
                *(intptr+14) = zero;
                *(intptr+15) = zero;
                intptr += 16;

            } while (--i);
        }
        else
        {
            ++tryq;
            intptr += 128;
        }
    } while (--j);
}

void SQjoinnodes(cluep, rightp, joinp, clue)
unsigned char           *cluep;
unsigned char           *rightp;
unsigned char           *joinp;
unsigned int            clue;
{
    unsigned char           *s;
    unsigned char           *d;
    unsigned char           *bend;
    unsigned int            i;

/* pack file */

    if (SQVbufbase==SQVbuffer)
    {   s = SQVbuffer;
        d = SQVbuf2;
    }
    else
    {   s = SQVbuf2;
        d = SQVbuffer;
    }
    SQVbufbase = d;
    bend = SQVbufend;

    *bend = (unsigned char) clue;
    while (s<=bend)
    {
        while (!cluep[(unsigned int) (*d++ = *s++)])
            ;

        i = (unsigned int) *(s-1);

        if (cluep[i]==1)
        {   if (*s==rightp[i])
            {   *(d-1) = joinp[i];
                ++s;
            }
        }
        else
        {   if (cluep[i]==3)
            {   *(d-1) = (unsigned char) clue;
                *d++ = *(s-1);
            }
            else
                *d++ = *s++;
        }
    }
    SQVbufend = d-2;
}


/* find 48 most common nodes */

unsigned int    SQfindbest(countptr,tryq,bestn,bestval,ratio)
FBWORD                      *countptr;
unsigned char               *tryq;
unsigned int                *bestn;
unsigned int                *bestval;
int                         ratio;
{
    unsigned int                i;
    unsigned int                i1;
    unsigned int                i2;
    unsigned int                bestsize;

    unsigned int                i3;
    unsigned int                l;

    bestsize = 1;
    i = 3;
    l = 0;
    for (i2=0;i2<256;++i2)
    {
        if (tryq[i2])
        {   for (i1=0;i1<256;++i1)
            {
                if (*countptr++>i)
                {
                    if (tryq[i1])
                    {   i = *(countptr-1);
                        i3=bestsize;
                        while (bestval[i3-1]<i)
                        {
                            bestn[i3] = bestn[i3-1];
                            bestval[i3] = bestval[i3-1];
                            --i3;
                        }
                        bestn[i3] = l+i1;
                        bestval[i3] = i;
                        if (bestsize<48)
                            ++bestsize;
                        while (bestval[bestsize-1]<(bestval[1]/ratio))
                            --bestsize;
                        if (bestsize<48)
                            i = bestval[1]/ratio;
                        else
                            i = bestval[bestsize-1];
                    }
                }
            }
        }
        else
            countptr += 256;
        l += 256;
    }
    return(bestsize);
}

void SQtreepack(dest,verbose, opt, passes, multimax, quick, zerosuppress)
struct MemStruct        *dest;
unsigned int            verbose, opt, passes, multimax, quick;
int                     zerosuppress;
{

    register unsigned char  *bend;
    register unsigned char  *ptr1;
    register unsigned char  *workbuf;
    register FBWORD         *count;
    register unsigned int   i;
    register unsigned int   i1;

    unsigned char           lastbyte;
    int                     joinnode, leftnode, rightnode;
    int                     ratio;
    unsigned int            hlen;
    unsigned int            domore;
    unsigned int            cost, save;
    unsigned int            tcost, tsave;

    unsigned int            freeptr;
    unsigned int            count2[CODES];
    unsigned char           tryq[CODES];
    unsigned char           freeq[CODES];

    unsigned int            bestsize;
    unsigned char           bestjoin[CODES];
    unsigned int            bestn[CODES];
    unsigned int            bestval[CODES];

    unsigned int            bt_size;
    unsigned int            bt_node[CODES];
    unsigned int            bt_left[CODES];
    unsigned int            bt_right[CODES];

    int                 btbuf1size;
    int                 btbuf2size;

    btbuf1size = 65536L*sizeof(FBWORD);
    btbuf2size = SQVulen+(int)SLOPAGE;

    workbuf =   (unsigned char *) FBALLOC(btbuf1size);
    if (!workbuf)
        quit_cleanup("Insufficient memory for work buffer");

    SQVbuf2 =   (unsigned char *) FBALLOC(btbuf2size);
    if (!SQVbuf2)
        quit_cleanup("Insufficient memory for source buffer");

    count = (FBWORD *) workbuf;

    SQVbufbase = SQVbuffer;
    SQVbufend = SQVbufptr;
    lastbyte = *SQVbufend;

    if (quick)  ratio = quick;
    else        ratio = 2;

/*** count file ***/

    for (i=0; i<CODES; ++i)
        count2[i] = 0;

    i1 = 0;
    bend = SQVbufptr;
    ptr1 = SQVbuffer;
    while (ptr1<bend)
    {   i = (unsigned int) *ptr1++;
        i1 = i1 + i;
        ++count2[i];
    }
    SQVcsum = i1;


/* init clue array */

    for (i=0;i<CODES;++i)
        SQVclueq[i] = 0;

/* get a clue byte (forced) */

    for(i=0; i<CODES; ++i)
    {   freeq[i] = 1;
        if (count2[i]>3)
            tryq[i] = 1;
        else
            tryq[i] = 0;
    }

/* don't use 0 for clue or node (so that [clue][0] is reserved for EOF) */

    count2[0] = BIGNUM;     /* don't use for clue or node */

/* asciiz btree suppress packing of 0..31 */

    if (zerosuppress)
    {
        for (i=0; i<32; ++i)
        {
            count2[i] = BIGNUM;     /* don't use for nodes */
            tryq[i] = 0;            /* don't try to pack */
            freeq[i] = 0;           /* don't try to pack */
        }
    }


/* sort codes with least used codes first */
/* primary key is count, secondary key is higher code */

/* registers vars usage
    i  - code
    i1 - done flag/swap temp
*/

    for (i=0;i<CODES;++i)
        SQVsortptr[i] = i;

    i1=1;
    while (i1)
    {
        i1 = 0;
        for (i=1; i<CODES; ++i)
        {
            if (count2[SQVsortptr[i]]<count2[SQVsortptr[i-1]] || ((count2[SQVsortptr[i]]==count2[SQVsortptr[i-1]]) && (SQVsortptr[i]>SQVsortptr[i-1])))
            {
                i1 = SQVsortptr[i];
                SQVsortptr[i] = SQVsortptr[i-1];
                SQVsortptr[i-1] = i1;
                i1 = 1;
            }
        }
    }

    freeptr = 0;
    i = SQVsortptr[freeptr++];
    SQVclue = (unsigned char) i;
    freeq[i] = 0;
    tryq[i] = 0;
    SQVclueq[i] = 3;

    if (count2[i])
        SQjoinnodes(SQVclueq, SQVrightl, SQVjoinl, SQVclue);

    SQVclueq[i] = 2;

/* init best array */

    bestval[0] = (unsigned int) -1;


/* count file (pass 2) */

    bt_size = 0;
    domore = passes;
    while (domore)
    {

/* clear count array */

        SQclearcount(tryq,count);

/* do an adjacency count */

        ptr1 = SQVbufbase;
        bend = SQVbufend;

        SQadjcount(ptr1, bend, count);

/* find most common nodes */

        bestsize = SQfindbest(count,tryq,bestn,bestval,ratio);

/* decide which nodes should be joined to what */

        domore = 0;
        if (bestsize>1)
        {
            tcost = tsave = 0;
            i = i1 = domore = 1;
            while (domore)
            {
                leftnode = (bestn[i]>>8)&255;
                rightnode = bestn[i]&255;

                if ((tryq[leftnode]==1) && (tryq[rightnode]==1))
                {
                    domore = 0;
                    while ((!freeq[SQVsortptr[freeptr]]) && (freeptr<CODES))
                        ++freeptr;

                    if (freeptr<CODES)
                    {
                        joinnode = SQVsortptr[freeptr];

                        cost = 3+count2[joinnode];
                        save = bestval[i];

                        if (cost<save)
                        {
                            tcost += cost;
                            tsave += save;

                            bestjoin[i1] = joinnode;
                            bestn[i1] = bestn[i];
                            ++i1;

                            freeq[joinnode] = 0;
                            tryq[joinnode] = 2;
                            SQVclueq[joinnode] = 3;

                            freeq[leftnode] = 0;
                            tryq[leftnode] = 2;
                            SQVclueq[leftnode] = 1;
                            SQVrightl[leftnode] = rightnode;
                            SQVjoinl[leftnode] = joinnode;

                            freeq[rightnode] = 0;
                            tryq[rightnode] = 2;

                            bt_node[bt_size] = joinnode;
                            bt_left[bt_size] = leftnode;
                            bt_right[bt_size] = rightnode;
                            ++bt_size;

                            if (i1<=multimax)
                                domore = 1;
                        }
                    }
                }

                ++i;
                if (i>=bestsize)
                    domore = 0;
            }

            bestsize = i1;

            if (bestsize>1)
            {
                /* multijoin nodes */

                SQjoinnodes(SQVclueq, SQVrightl, SQVjoinl, SQVclue);

                /* restore clue table */

                for(i=1;i<bestsize;++i)
                {   leftnode = (bestn[i]>>8)&255;
                    joinnode = bestjoin[i];
                    SQVclueq[leftnode] = SQVclueq[joinnode] = 0;
                }

                domore = --passes;
            }

        }

    }
    SQVbufptr = SQVbufend;

/* write header */

    SQwritebits(dest,(unsigned int) SQVclue, 8);    /* clue byte */
    SQwritebits(dest,(unsigned int) bt_size, 8);    /* tree size */

    for (i=0; i<bt_size;++i)
    {   SQwritebits(dest,(unsigned int) bt_node[i], 8);
        SQwritebits(dest,(unsigned int) bt_left[i], 8);
        SQwritebits(dest,(unsigned int) bt_right[i], 8);
    }

    hlen = SQVplen;

/*** write packed file ***/

/* registers vars usage
    i  - current byte
    i1 - previous byte
    i2 - rep
    i3 - rep2
*/

    ptr1 = SQVbufbase;
    bend = SQVbufend;

    while (ptr1<bend)
        SQwritebits(dest,(unsigned int) *ptr1++, 8);

    SQwritebits(dest,(unsigned int) SQVclue, 8);
    SQwritebits(dest,(unsigned int) 0, 8);

    SQwritebits(dest,0L,7); /* flush bits */

    FBFREE(SQVbuf2);
    FBFREE(workbuf);


/* file stats */

#ifdef NEVER
    SQps(verbose,opt,SQVplen,SQVulen,SQVladr,0L,hlen,1,0,SQVcsum,0);
#endif

    SQVnewadr = SQVladr;
}

void btreepackfile(infile, outfile, ulen, zerosuppress, verbose)
struct MemStruct            *infile;
struct MemStruct            *outfile;
int                     ulen;
int                         zerosuppress;
int                         verbose;
{
    register unsigned int   i;

    unsigned int            opt;
    unsigned int            chainsaw;
    unsigned int            passes;
    unsigned int            multimax;
    unsigned int            quick;
    unsigned int            loadadr;
    unsigned int            destadr;
    unsigned int            destoffset;

/* set defaults */

    loadadr = destadr = destoffset = 0;
    quick = 0;
    SQVpackbits = 0;

    SQVdumpamt = SQVworkpattern = 0L;
    SQVladr = 0L;
    opt = 57;
    chainsaw = 15;
    passes = 256;
    multimax = 32;
    SQVmasks[0] = 0;
    for (i=1;i<17;++i)
        SQVmasks[i] = (SQVmasks[i-1] << 1) + 1;

/* initialize huffman vars */

    SQinithuff();

/* read in a source file */

    SQVbuffer = (unsigned char *) (infile->ptr);
    SQVflen = infile->len;

    SQVulen = SQVflen;
    SQVbufptr = SQVbuffer + SQVflen;

/* get c64 loadadr */

    if (loadadr || destadr)
    {   SQVflen = SQVulen = SQVulen - 2;

        SQVladr = (unsigned int) *SQVbuffer++;
        SQVladr = SQVladr | (unsigned int) *SQVbuffer++ * 256;
    }

/* pack a file */

    outfile->ptr = outfile->ptr;
    outfile->len = 0L;

    SQVpackbits = 0;
    SQVworkpattern = 0L;
    SQVplen = 0L;

    opt = opt & ~32;

    if (loadadr || destoffset)
        SQwritebits(outfile,(unsigned int) 0, 16);

/* write standard header stuff (type/signature/ulen/adjust) */

    if (destadr)
        SQwritebits(outfile,(unsigned int) SQVladr, 16);

    /* simple fb4 header */

    if (ulen==infile->len)
    {
        SQwritebits(outfile,(unsigned int) 0x46fb, 16);
        SQwritebits(outfile,(unsigned int) infile->len, 24);
    }

    /* composite fb4 header */

    else
    {
        SQwritebits(outfile,(unsigned int) 0x47fb, 16);
        SQwritebits(outfile,(unsigned int) ulen, 24);
        SQwritebits(outfile,(unsigned int) infile->len, 24);
    }


    SQtreepack(outfile,verbose, opt, passes, multimax, quick, zerosuppress);

}
