// (c) Electronic Arts. All Rights Reserved.

/*------------------------------------------------------------------*/
/*                                                                  */
/*                   HUFF XE Best Compresser v6.14                  */
/*                                                                  */
/*                     by FrANK G. Barchard, EAC                    */
/*                                                                  */
/*                    Main program - Jan 10, 1994                   */
/*                                                                  */
/*                                                                  */
/*------------------------------------------------------------------*/

/****************************************************************/
/*  Spit Setup                                                  */
/****************************************************************/

#define WANTBZIP2 0

#include    "spit/codex.h"
#include    "spit/stdgimex.h"

#include    "spit/refcodex.h"
#include    "spit/btreecodex.h"
#if WANTBZIP2
#include    "bzip2codex.h"
#endif
#include    "spit/zipcodex.h"
#include    "spit/huffcodex.h"
#include    "spit/spit.h"
#include    "huff.h"
#include    "huffu.h"
#include    "huffview.h"
#include    "huffbest.h"

/****************************************************************/
/*  Get Unpacked Length of a File                               */
/****************************************************************/

static int SQunhufflen(const unsigned char *ptr, int len)
{
    int  tof;
    const unsigned char *s;

    tof = 0;
    if (len>2)
    {
        s = (ptr);
        len -= (int)(s-ptr);

        tof = TypeOFile(s);

        if (tof>0 && tof<=NEWPACKTYPES)
        {
            int id = (unsigned int) *s;
            if (id&0x80)
                len = (int) ggetm(s+2, 4);
            else
                len = (int) ggetm(s+2, 3);
        }
    }

    return(len);
}

/****************************************************************/
/*  Pack a File with Specified Pack Type                        */
/*  - note that delta huff destroys the original file           */
/****************************************************************/
static void packfile(struct SPITMEM *infile, struct SPITMEM *outfile, int type)
{
    int ulen;

    ulen = SQunhufflen((const unsigned char *)infile->ptr,infile->len);
    if (!ulen)
        ulen = infile->len;

    switch (type)
    {
        default:
        case NOPAC:
            memcpy(outfile->ptr, infile->ptr, infile->len);
            outfile->len = infile->len;
            break;

        case BTREEPAC:
            outfile->len = BTREE_encode(outfile->ptr, infile->ptr, infile->len);
            break;

        case HUFFPAC:
            outfile->len = HUFF_encode(outfile->ptr, infile->ptr, infile->len);
            break;

        case REFPAC:
            outfile->len = REF_encode(outfile->ptr, infile->ptr, infile->len);
            break;

#if WANTBZIP2
        case BZ2PAC:
        {
            void *t = galloc(infile->len*3/2+8192);
            memcpy(t,infile->ptr, infile->len);
            outfile->len = BZIP2_encode(outfile->ptr, t, infile->len);
            gfree(t);
            break;
        }
#endif

        case ZIPPAC:
            outfile->len = ZIP_encode(outfile->ptr, infile->ptr, infile->len);
            break;
    }
}


/****************************************************************/
/*  Pack a File with Best Pack Type                             */
/****************************************************************/

static struct SPITMEM BESTinfile;
static struct SPITMEM BESToutfile;
static int            BESTbesttype;
static struct SPITMEM BESTtryinfile;
static struct SPITMEM BESTtryoutfile;
static int            BESTtryweight;
static int            BESToutweight;

static void trybest(int trytype)
{
    char                        *tempptr;
    int                        templen;

    if (OPTcmask&(1<<trytype))
    {
        BESTtryinfile.len = BESTinfile.len;
        memcpy(BESTtryinfile.ptr, BESTinfile.ptr, BESTinfile.len);
        packfile(&BESTtryinfile, &BESTtryoutfile, trytype);

        BESTtryweight = BESTtryoutfile.len;

        if (BESTtryweight<BESToutweight)
        {
            tempptr = BESToutfile.ptr;
            templen = BESToutfile.len;
            BESToutfile.ptr = BESTtryoutfile.ptr;
            BESToutfile.len = BESTtryoutfile.len;
            BESTbesttype = trytype;
            BESToutweight = BESTtryweight;
            BESTtryoutfile.ptr = tempptr;
            BESTtryoutfile.len = templen;
        }
    }
}

static int bestpackfile(struct SPITMEM *infile, struct SPITMEM *outfile)
{
    /* default is to store file */

    BESTinfile = *infile;
    BESToutfile = *outfile;

    BESToutfile.len = 0x7fffffffL;
    BESToutweight = 0x7fffffffL;
    BESTbesttype = -1;

    BESTtryinfile.len += (int)SLOPAGE;   /* kludge slopage for btree */
    BESTtryinfile.ptr = static_cast<char *>(malloc(BESTinfile.len));

    if (BESTtryinfile.ptr != NULL)
    {
        BESTtryoutfile.len += (int)SLOPAGE;
        BESTtryoutfile.ptr = static_cast<char *>(malloc(BESTtryoutfile.len));

        if (BESTtryoutfile.ptr != NULL)
        {
            trybest(NOPAC);
            trybest(BTREEPAC);
            trybest(HUFFPAC);
            trybest(REFPAC);
            trybest(BZ2PAC);
            trybest(ZIPPAC);

            free(BESTtryoutfile.ptr);
            BESTtryoutfile.ptr = NULL;
        }
        else
        {
            if (OPTverbose>=1)
                printf("ERROR could not allocate pack buffer\n");
        }
        free(BESTtryinfile.ptr);
        BESTtryinfile.ptr = NULL;
    }
    else
    {
        if (OPTverbose>=1)
            printf("ERROR could not allocate pack buffer\n");
    }

    *outfile = BESToutfile;

    return(BESTbesttype);
}

/****************************************************************/
/*  Pack a File                                                 */
/*  - uses bestpackfile() to find best packing type             */
/*  - if only one type to choose from analysis is skipped       */
/****************************************************************/

int packbest(struct SPITMEM *infile, struct SPITMEM *outfile, unsigned int trytypes)
{
    int         i;
    int         bestpacktype;

    /* first choose the type of packing to use */

    i = 0;
    if (OPTpacktype==BESTPAC)
    {
        bestpacktype = NOPAC;
        i = trytypes;
        while (i)
        {
            if (i&1)
            {
                if (i!=1)
                {
                    bestpacktype = -1;
                }
                break;
            }
            ++bestpacktype;
            i >>= 1;
        }

        i = 0;
        if (bestpacktype==-1)
        {
            if (OPTverbose>=3)
            {
                printf("Analysing, ");
                fflush(stdout);
            }
            bestpacktype = bestpackfile(infile, outfile);
            i = 1;
        }
    }
    else
        bestpacktype = OPTpacktype;

    if (OPTverbose>=2)
    {
        switch (bestpacktype)
        {
            case NOPAC:     printf("Storing, ");                break;
            case BTREEPAC:  printf("Btreeing, ");               break;
            case HUFFPAC:   printf("Huffing, ");                break;
            case REFPAC:    printf("Refpacking, ");             break;
#if WANTBZIP2
            case BZ2PAC:    printf("BZip2ing, ");               break;
#endif
            case ZIPPAC:    printf("Ziping, ");                 break;
            default:        printf("???, ");                    break;
        }
        fflush(stdout);
    }
    if (!i)
    {
        packfile(infile, outfile, bestpacktype);
    }
    return(bestpacktype);
}
