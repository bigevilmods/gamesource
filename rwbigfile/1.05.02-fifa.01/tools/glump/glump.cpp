// (c) 1998-2007 Electronic Arts Inc.

// enable codecs
#define WANTREF 1
#define WANTZIP 0
#define WANTLZMA 1
#define EXTRACTONLY 0

// enable refpack decompression
#define WANTDECOMPRESSION 1
// append to table of contents this many bytes for venom and flags. 0=off, 8=on (number of bytes in debug info)
#define GLUMP_DEBUGINFO 8

const char *SPIT_history[] =
{
#if !EXTRACTONLY
"------------------------------------------------------------------",
"                                                                  ",
"                           Glump v2.82                            ",
"                                                                  ",
"             by Frank Barchard, Pete Veys, Yggy King,             ",
"               Steve Kilthaw, Kaz Okuda, Lydia Chan               ",
"                                                                  ",
"                    Main program - Dec 11, 2006                   ",
"                                                                  ",
"------------------------------------------------------------------",
"                                                                  ",
" Version Date   SE History:                                       ",
" ------- ----   -- --------                                       ",
" 1.00    980727 SK Updated glump to use chimp modules             ",
" 1.13    990101 FB See Source Safe                                ",
" 1.14    990323 KO Make sure to alloc enough space for compression",
" 1.15    990427 KO -killnames actually sets the name to "" now.   ",
" 1.16    990616 LC deleted tmpnam(); use outname.tm1 file instead ",
" 1.17    990816 FB -tolower                                       ",
" 1.18    991117 LC use CODEX_find(), return (char*) optptr        ",
" 1.19    000310 LC -nobak option                                  ",
" 1.20    000324 LC -view -verbose separate, added default actions ",
" 1.21    000508 LC fixed gfree bug, removed LZH from codex        ",
" 1.22    000515 FB refwrite avoids compressing last packet        ",
" 1.23    000517 LC -x now defaults to extracting all like *       ",
" 1.24    000521 FB -v, -help, keepopts                            ",
" 1.25    000526 LC added checking if can fit in 24bit else goto 32",
" 1.26    000529 FB extracting information same as old glump       ",
" 1.27    000530 FB abstract updates                               ",
" 1.28    000602 LC same as 1.25 (24->32) but better!   		   ",
" 1.29    000614 FB use sargv for big file                         ",
" 1.30    000616 FB error return, and spit arg 0 ignore namerule   ",
" 1.31    000616 LC -a output, -v1, -m diff for each arg           ",
" 1.32    000622 FB -v3 show wider filename in view                ",
" 1.33    000622 LC add default vs view all                        ",
" 1.34    000630 LC check to make sure that dest file is valid type",
" 1.35    000701 FB default extensions rewritten                   ",
" 1.36    000704 FB BZIP2                                          ",
" 1.37    000707 LC default align4 fixed, static functions         ",
" 1.38    000707 LC big and viv files refer to different packing   ",
" 1.39    000712 LC can kind of tell diff b/w viv and big          ",
" 1.40    000714 LC backwards compatible with old viv refpack      ",
" 1.41    000817 LC recognize creation of new file from scratch    ",
" 1.42    000914 LC removed some documentation, added -pack        ",
" 1.43    000928 LC -big -pack2 force to use refpack, -pack3 bip2  ",
" 1.44    001003 LC .viv defaults back to refpack                  ",
" 1.45    001006 FB 0744 access for directories on unix            ",
" 1.46    001017 LC fixed recurse for unix (findfile)              ",
" 1.47    001019 LC add shows pack type, fixed -viv bug            ",
" 1.48    001213 LC percentstr and typecasting glen                ",
" 1.49    001219 LC print unextracted, recognize zip & wad files   ",
" 1.50    010108 LC glump docs                                     ",
" 1.51    010206 LC spit -r fixed                                  ",
" 1.52    010308 FB use lm1 for temporary name to avoid tm1 clash  ",
" 1.53    010619 LC sort7 sorts by filename, not path              ",
" 1.54    010628 LC NOCOMP, prints out no compression if -pack0    ",
" 1.55    010813 LC fixed corrupt on extract and +align on filesize",
" 2.00    011009 FB Simple glump add similiar to original glump    ",
" 2.01    011010 FB Copy large files in pieces                     ",
" 2.02    011011 FB -path, -killnames, 64 bit                      ",
" 2.03    011012 FB -chunk, -24, 32K blocks                        ",
" 2.04    011013 FB -blocksize                                     ",
" 2.05    011015 FB renamed lump to glump, codex 2 API             ",
" 2.06    011016 FB check bigfile opened                           ",
" 2.07    011022 FB -big (ignored)                                 ",
" 2.08    011023 FB view                                           ",
" 2.09    011025 FB nobak (ignored)                                ",
" 2.10    011101 FB reduction, -40 started                         ",
" 2.11    020108 FB error return codes                             ",
" 2.12    020120 FB -q1..9 for BZIP2, 0 based on size              ",
" 2.13    020201 FB -nofiles                                       ",
" 2.14    020215 FB maxpack bumpped to 1 GB - zip2 liposuction     ",
" 2.15    020312 FB -40 for BIG5 files, read error message         ",
" 2.16    020407 FB BIG5 venom, offset                             ",
" 2.17    020416 FB -x using view code                             ",
" 2.18    020417 FB BIG5 same as BIGF and put venom after TOC      ",
" 2.19    020418 FB BIG4 (-n32), updated documentation             ",
" 2.20    020419 FB -n24 fill in offset, -n16 alias                ",
" 2.21    020424 FB -raw format, chunk fix, faster align padding   ",
" 2.22    020522 FB -sourcepath to use source pathname (-r fix)    ",
" 2.23    020528 FB -sourcepath disable by default now             ",
" 2.24    020623 FB BZIP2 optional, glump function                 ",
" 2.25    020626 FB refpack also optional                          ",
" 2.26    020720 FB extract refpacked data                         ",
" 2.27    020723 FB calculate alignment in view mode               ",
" 2.28    020807 FB zlib                                           ",
" 2.29    020814 FB hashnames option using %04x of crc16           ",
" 2.30    021010 FB ShpF same as BIG4                              ",
" 2.31    021014 FB Name last for longer names, documentation      ",
" 2.32    030114 FB branched from lumpy                            ",
" 2.33    030326 FB toupper                                        ",
" 2.34    030331 FB -paths3 for folder name removal                ",
" 2.35    030402 FB -hash                                          ",
" 2.36    030406 FB -hashcheck, -namelist                         ",
" 2.37    030606 FB BZIP2 disabled (until legal resolved)          ",
" 2.38    030627 FB shape1 support                                 ",
" 2.39    030707 FB -span to avoid crossing sector boundaries      ",
" 2.40    030813 FB -chop to break up large files into chunks      ",
" 2.41    030823 FB min/max compressed size on add/create          ",
" 2.42    030827 FB show chunkhex name                             ",
" 2.43    030907 FB -return and -time                              ",
" 2.44    030909 FB -returnvenom                                   ",
" 2.45    030909 FB -m to set compression threshhold required      ",
" 2.46    031005 FB -rel to make names relative to CWD             ",
" 2.47    031027 FB BZIP2 (xceed) enabled and extracter written    ",
" 2.48    040201 FB shp1 extract fixed                             ",
" 2.49    040612 FB refpack overlap calc, efficiency rating        ",
" 2.50    040618 FB -failoverlap                                   ",
" 2.51    040625 FB -extract2 for verbatim extract, zero len files ",
" 2.52    040626 FB -failmaxname<n> to fail if filenames too long  ",
" 2.53    040705 FB -faildoublepack to fail if compressing twice   ",
" 2.54    040711 FB title v2 or more                               ",
" 2.55    040719 FB -ignoremissingfile for nhl                     ",
" 2.56    040721 FB -x list of files                               ",
" 2.57    040726 FB -u update smaller files                        ",
" 2.58    040726 FB -u update larger files                         ",
" 2.59    040803 FB -pack total %, spit -r, -namelist list        ",
" 2.60    040810 FB -removeonerror                                 ",
" 2.63    040813 FB align64 by default                             ",
" 2.64    040916 FB treat size -1 as zero                          ",
" 2.65    040930 FB -view pack %, -x dest, -serial, hashbits       ",
" 2.66    041008 FB -pad                                           ",
" 2.67    041022 FB -packthresh, command line doc                  ",
" 2.68    041101 FB -failoverlap show overlap failure in -v6,-sort ",
" 2.69    041101 FB -remove placebo option added                   ",
" 2.70    041108 FB overlap additional check added                 ",
" 2.71    041210 FB -autoalign to align based on size              ",
" 2.72    050128 FB -namelist                                      ",
" 2.73    050301 FB -q0 apply to refpack                           ",
" 2.74    050309 FB zip codec leak fix                             ",
" 2.75    050315 FB LZMA using external tool                       ",
" 2.76    050529 FB return error if memory check fails             ",
" 2.77    050606 FB unix port                                      ",
" 2.78    060406 FB pad with simple zeros for raw padding          ",
" 2.79    060504 FB -paths4 for / to \\                            ",
" 2.80    060608 FB -gap to introduce gaps between files           ",
" 2.81    060817 FB 11 digit numbers for up to 99 GB files         ",
" 2.82    061211 FB -overlapthresh to disable pack based on overlap",
"                                                                  ",
" Format notes                                                     ",
"                                                                  ",
" Big file format                                                  ",
" http://tools.eac.ea.com/gimex/release/formats/viv.htm            ",
"                                                                  ",
" Big head file format                                             ",
" http://tools.eac.ea.com/gimex/release/formats/bh.htm             ",
"                                                                  ",
" BZIP2 documentation, which talks about memory usage/speed        ",
" ftp://sourceware.cygnus.com/pub/LZMA/docs/manual_2.html#SEC6     ",
"                                                                  ",
" LZMA documentation                                               ",
" http://www.7-zip.org/                                            ",
"                                                                  ",
" Tar file format                                                  ",
" http://www.gnu.org/manual/tar/index.html                         ",
"                                                                  ",
" Zip file format                                                  ",
" http://www.pkware.com/support/appnote.html#2                     ",
"                                                                  ",
"------------------------------------------------------------------",
#endif // !EXTRACTONLY
"\0"
};

// fat lady
/* http://ourworld.compuserve.com/homepages/ken_crossman/Fatlady.jpg */

/****************************************************************/
/*  Spit Setup                                                  */
/****************************************************************/
#if defined(_MSC_VER)
#include "windows.h"        // SetConsoleTitle
#endif

#include "../spit/spit.h"
#include "../spit/range.h"
#include "../spit/ranger.h"
#include "../spit/expr.h"
#include "../spit/codex.h"
#include "../spit/stdgimex.h"
#if WANTREF
#include "../spit/refcodex.h"
#endif
#if WANTZIP
#include "../spit/zipcodex.h"
#endif
#if EXTRACTONLY
#define gprintf sizeof
#endif

const char TextVersion[]="2.82b (Baden Powell) 2007.1.30";
#define TOOL_VERSION 282
#define TOOL_NAME "glump"

void replacePathAbs(char *out, char *in, char *path);

int OPTalign;
int OPTautoalign;
int OPTspan;
int OPTpad;
int OPTgap;
#if WANTREF || WANTZIP || WANTLZMA
int OPTpack;
int OPTpackmax;
int OPTpackthresh;
int OPTquality;
#endif
int OPThashnames;
int OPThashcheck;
int OPTnamelist;
int OPTkillnames;
int OPTserial;
int OPTcsv;
int OPTremoveonerror;
int OPToverlapthresh;
int OPTfailoverlap;
int OPTfailmaxname;
int OPTfaildoublepack;
int OPTignoremissingfile;
int OPTnofiles;
int OPTidupper;
int OPTpaths;
int OPTrelative;    // make paths relative
int OPTtolower;
int OPTtoupper;
int OPTsourcepath;
int OPThex;
int OPTblocksize;
int OPTextract;
int OPTupdate;
int OPTview;
int OPTbign=4;  //BIGSUBTYPE_BIGN uses this many bytes for offset (1..8)
int OPTbigsubtype;
int OPTseed=5381;
int OPTchop=0; // chop into chunks of n bytes
int OPTchunkhex=-1; // name the chunk using %4x starting with this number if >=0
char OPTreturn[SPITMAXLINE];

#define BIGSUBTYPE_32 0
#define BIGSUBTYPE_SHP 1
#define BIGSUBTYPE_SHP1 2
#define BIGSUBTYPE_24 3
#define BIGSUBTYPE_BIGN 4
#define BIGSUBTYPE_CHUNK 5
#define BIGSUBTYPE_RAW 6
#define BIGSUBTYPE_HASHN 7  /* BIGN with hashed names */

#define FOURGB (CONST64(1)<<32)
#define SIXTEENMB (CONST64(1)<<24)
#define SIXTYFOURKB (CONST64(1)<<16)

static unsigned int *HashList=NULL;
static int HashMaxCount=0;
static int HashCount=0;

int gerrorcount=0;

/****************************************************************/
/*  REFpack with overlap calculation                            */
/****************************************************************/

#include <string.h>
#include "../spit/codex.h"
#include "../spit/refcodex.h"

struct RefStats
{
	int quality; // passed in to specify speed over compression ratio. 0=fast
	int overlap;
    int countLiterals;
    int countShort;
    int countLong;
    int countVLong;
};

static unsigned int matchlen(unsigned char *s,unsigned char *d, unsigned int maxmatch)
{
    unsigned int current;

    for (current=0; current<maxmatch && *s++==*d++; ++current)
        ;

    return(current);
}

#define HASH(cptr) (int)((((unsigned int)(unsigned char)cptr[0]<<8) | ((unsigned int)(unsigned char)cptr[2])) ^ ((unsigned int)(unsigned char)cptr[1]<<4))

static int refcompress(unsigned char *from, int len, unsigned char *dest, int hlen, int maxback, RefStats *refStats)
{
    unsigned int tlen;
    unsigned int tcost;
    unsigned int run;
    unsigned int toffset;
    unsigned int boffset;
    unsigned int blen;
    unsigned int bcost;
    unsigned int mlen;
    unsigned char *tptr;
    unsigned char *cptr;
    unsigned char *to;
    unsigned char *rptr;
    int countLiterals=0;
    int countShort=0;
    int countLong=0;
    int countVLong=0;
    int hash;
    int hoffset;
    int minhoffset;
    int i;
    int *link;
    int *hashtbl;

	int quality = refStats->quality;
	if (quality<1 || quality>8) quality = 8;

	int quick = quality&1;
	int maxbackthresh = 16384<<(((quality-1)/2)&3);

    if ((unsigned int)maxback > (unsigned int)maxbackthresh)
        maxback = maxbackthresh;
	int maxbackmask = maxback-1;
    
	int overlap=hlen;
    to = dest+hlen;
    run = 0;
    cptr = rptr = from;


	hashtbl = (int *) galloc(65536L*sizeof(int));
	if (!hashtbl)
        return(0);
	link = (int *) galloc(maxback*sizeof(int));
	if (!link)
        return(0);

    memset(hashtbl,-1,65536L*sizeof(int));

    len -= 4;
    while (len>=0)
    {
        boffset = 0;
        blen = 2;
        bcost = 2;
		int beff = 0; // decode efficiency rating
        mlen = qmin(len,1028);
        tptr=cptr-1;
        hash = HASH(cptr);
        hoffset = hashtbl[hash];
        minhoffset = qmax(cptr-from-(maxbackmask),0);

        if (hoffset>=minhoffset)
        {
            do
            {
                tptr = from+hoffset;
                if (cptr[blen]==tptr[blen])
                {
                    tlen = matchlen(cptr,tptr,mlen);
                    if (tlen > blen)
                    {
                        toffset = (cptr-1)-tptr;
                        if (toffset<1024 && tlen<=10)       /* two byte int form */
                            tcost = 2;
                        else if (toffset<16384 && tlen<=67) /* three byte int form */
                            tcost = 3;
                        else                                /* four byte very int form */
                            tcost = 4;

                        if (tlen-tcost >= blen-bcost)
                        {
							int teff = tlen;	// efficiency is a function of length and alignment
							if (((int)tptr&1)==0) ++teff;
							if (((int)tptr&3)==0) ++teff;
							if ((tlen-tcost > blen-bcost) || (teff>beff))
							{
	                            blen = tlen;
		                        bcost = tcost;
			                    boffset = toffset;
				                if (blen>=1028) break;
							}
                        }
                    }
                }
            } while ((hoffset = link[hoffset&(maxbackmask)]) >= minhoffset);
        }

        if (bcost>=blen || len<4)
        {
            hoffset = (cptr-from);
            link[hoffset&(maxbackmask)] = hashtbl[hash];
            hashtbl[hash] = hoffset;

            ++run;
            ++cptr;
            --len;

			if ((rptr-from)>(to-dest+overlap))
				overlap = (rptr-from)-(to-dest);
        }
        else
        {
            while (run>3)                   /* literal block of data */
            {
                tlen = qmin(112,run&~3);
                run -= tlen;
                *to++ = (unsigned char) (0xe0+(tlen>>2)-1);
                memcpy(to,rptr,tlen);
                rptr += tlen;
                to += tlen;
                ++countLiterals;
            }
            if (bcost==2)                   /* two byte int form */
            {
                *to++ = (unsigned char) (((boffset>>8)<<5) + ((blen-3)<<2) + run);
                *to++ = (unsigned char) boffset;
                ++countShort;
            }
            else if (bcost==3)              /* three byte int form */
            {
                *to++ = (unsigned char) (0x80 + (blen-4));
                *to++ = (unsigned char) ((run<<6) + (boffset>>8));
                *to++ = (unsigned char) boffset;
                ++countLong;
            }
            else                            /* four byte very int form */
            {
                *to++ = (unsigned char) (0xc0 + ((boffset>>16)<<4) + (((blen-5)>>8)<<2) + run);
                *to++ = (unsigned char) (boffset>>8);
                *to++ = (unsigned char) (boffset);
                *to++ = (unsigned char) (blen-5);
                ++countVLong;
            }
            if (run)
            {
                memcpy(to, rptr, run);
                to += run;
                run = 0;
            }

            if (quick)
            {
                hoffset = (cptr-from);
                link[hoffset&(maxbackmask)] = hashtbl[hash];
                hashtbl[hash] = hoffset;
                cptr += blen;
            }
            else
            {
                for (i=0; i < (int)blen; ++i)
                {
                    hash = HASH(cptr);
                    hoffset = (cptr-from);
                    link[hoffset&(maxbackmask)] = hashtbl[hash];
                    hashtbl[hash] = hoffset;
                    ++cptr;
                }
            }

            rptr = cptr;

			if ((rptr-from)>(to-dest+overlap))
				overlap = (rptr-from)-(to-dest);

            len -= blen;
        }
    }
    len += 4;
    run += len;
    while (run>3)                       /* no match at end, use literal */
    {
        tlen = qmin(112,run&~3);
        run -= tlen;
        *to++ = (unsigned char) (0xe0+(tlen>>2)-1);
        memcpy(to,rptr,tlen);
        rptr += tlen;
        to += tlen;

        ++countLiterals;
		if ((rptr-from)>(to-dest+overlap))
			overlap = (rptr-from)-(to-dest);
    }

    *to++ = (unsigned char) (0xfc+run); /* end of stream command + 0..3 literal */
    if (run)
    {
        memcpy(to,rptr,run);
        to += run;
		rptr += run;
    }

	if ((rptr-from)>(to-dest+overlap))
		overlap = (rptr-from)-(to-dest);

	overlap = (to-dest+overlap)-(rptr-from);
	if (refStats)
	{
		refStats->overlap = overlap;
	    refStats->countLiterals = countLiterals;
	    refStats->countShort = countShort;
	    refStats->countLong = countLong; 
	    refStats->countVLong = countVLong;
	}

	gfree(link);
	gfree(hashtbl);
    return(to-dest);
}


/****************************************************************/
/*  Encode Function                                             */
/****************************************************************/

int GCALL REF_encode(void *compresseddata, const void *source, int sourcesize, int *opts)
{
    int    maxback=131072;
    int    hlen;

	RefStats *refStats = (RefStats *)opts;

    /* simple fb6 header */

    if (sourcesize>0xffffff)  // 32 bit header required
    {
        gputm(compresseddata,   (unsigned int) 0x90fb, 2);
        gputm((char *)compresseddata+2, (unsigned int) sourcesize, 4);
        hlen = 6;
    }
    else
    {
        gputm(compresseddata,   (unsigned int) 0x10fb, 2);
        gputm((char *)compresseddata+2, (unsigned int) sourcesize, 3);
        hlen = 5;
    }
    return refcompress((unsigned char *)source, sourcesize, (unsigned char *)compresseddata, hlen, maxback, refStats);
}

/****************************************************************/
/*  Encode Function                                             */
/****************************************************************/

int LZMA_is(const void *source)
{
	return 0;
}

int LZMA_size(const void *source)
{
	return 0;
}

int LZMA_decode(void *decompresseddata, const void *source)
{
	return 0;
}

int GCALL LZMA_encode(void *compresseddata, const void *source, int sourcesize, int *opts)
{
	GSTREAM *fo = gwopen("tmpin");
	gwrite(fo,source,sourcesize);
	gclose(fo);
#if !defined(__PPU__)
	system("lzma e tmpin tmpout 2>nul");
#endif

	GSTREAM *fi = gopen("tmpout");
	int flen = (int)glen(fi);
	gread(fi,compresseddata,flen);
	gclose(fi);
	return flen;
}
/****************************************************************/
/*  Hash support                                                */
/****************************************************************/

static unsigned int *hashList=NULL;
static int hashMaxCount=0;
static int hashCount=0;
static int hashBits=1;

// returns true if a collision is found

static bool HashCollisionCheck(unsigned int hash)
{
    // add to list.  guarantees we'll find it
    if (hashCount>=hashMaxCount || !hashList)
    {
        hashMaxCount += 1024;

        unsigned int *HashNewList = (unsigned int *)galloc(hashMaxCount*sizeof(unsigned int));
        if (hashList)
        {
            memcpy(HashNewList,hashList,hashCount*sizeof(unsigned int));
            gfree((void *)hashList);
        }
        hashList = HashNewList;
    }

    hashList[hashCount++] = hash;

    int i=0;
	bool collision;

	do
	{
		collision=false;
		int hashBitsmask = (unsigned int)(-1)>>(32-hashBits);
		do
		{
			;
		} while ((hash&hashBitsmask)!=(hashList[i++]&hashBitsmask));

		if (i!=hashCount)   // did not hit end.. collision
		{
			collision = true;
			++hashBits;
		}
	} while (collision && hashBits<33);

    return collision;
}


static void HashCollisionRestore(void)
{
    if (hashList) gfree((void *)hashList);
    hashList = NULL;
    hashMaxCount = 0;
    hashCount = 0;
}

static void HashCollisionInit(void)
{
    HashCollisionRestore();
	hashBits = 1;
}

// DJB2 hash
// http://www.cs.yorku.ca/~oz/hash.html
// NOTE require int to be 32 bits

#pragma warning (disable: 4706) // assignment within conditional expression

static unsigned int hash(const char *str, unsigned int hash = 5381)
{
    const unsigned char *s = (const unsigned char *) str;
    int c;

    while (c = *s++)
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}

// end of hash



int GlumpCompareNames(const char *name1, const char *name2)
{
	return stricmp(name1,name2);
}

bool IsOnList(int &whicharg, const char *fname,char **SPITsargv,int i,int SPITargnum)
{
	bool found=true;
	while (i<SPITargnum)
	{
		if (SPITsargv[i][0]!='-')
		{
			found=false;
			if (!GlumpCompareNames(fname,SPITsargv[i]))
			{
				found = true;
				break;
			}
		}
		++i;
	}
	whicharg = i;
	return found;
}


bool GLUMP_extract(GSTREAM *g, const char *fname, GPOS foffset, int fsize, int OPTextract)
{
    bool ok=false;
    GSTREAM *gw = gwopen(fname);
    if (gw)
    {
        GPOS opos = gtell(g);
        gseek(g,foffset);
        void *buf = galloc(fsize+1);	// 1 byte extra to allow 0 length files and for a sentinel
        if (buf)
        {
			((unsigned char *)buf)[fsize] = (unsigned char) 0xfb;
			if (gread(g,buf,fsize)==fsize)
            {
				if (OPTextract==1)
				{
#if WANTDECOMPRESSION
#if WANTREF
					if (REF_is(buf))
					{
						int decompressedsize = REF_size(buf);
						void *dbuf = galloc(decompressedsize);
						if (dbuf)
						{
							if (REF_decode(dbuf,buf))
							{
								gfree(buf);
								buf = dbuf;
								fsize = decompressedsize;
							}
							else // failed... export the original
							{
								if (OPTverbose>=2)
									gprintf(TOOL_NAME " - DECOMPRESS FAILED. EXPORTING AS IS\n");
								gfree(dbuf);
							}
						}

					}
#endif
#if WANTZIP
					else if (ZIP_is(buf))
					{
						int decompressedsize = ZIP_size(buf);
						void *dbuf = galloc(decompressedsize);
						if (dbuf)
						{
							if (ZIP_decode(dbuf,buf))
							{
								gfree(buf);
								buf = dbuf;
								fsize = decompressedsize;
							}
							else // failed... export the original
							{
								if (OPTverbose>=2)
									gprintf(TOOL_NAME " - DECOMPRESS FAILED. EXPORTING AS IS\n");
								gfree(dbuf);
							}
						}

					}
#endif
#if WANTLZMA
					else if (LZMA_is(buf))
					{
						int decompressedsize = LZMA_size(buf);
						void *dbuf = galloc(decompressedsize);
						if (dbuf)
						{
							if (LZMA_decode(dbuf,buf))
							{
								gfree(buf);
								buf = dbuf;
								fsize = decompressedsize;
							}
							else // failed... export the original
							{
								if (OPTverbose>=2)
									gprintf(TOOL_NAME " - DECOMPRESS FAILED. EXPORTING AS IS\n");
								gfree(dbuf);
							}
						}

					}
#endif
#endif
				}

                if (gwrite(gw,buf,fsize)==fsize)
                    ok = true;
            }
            gseek(g,opos);
            gfree(buf);
        }
        gclose(gw);
    }
    return ok;
}

void *GLUMP_compress(const char *fname, const void *inbuf, int *inoutsize)
{
	int fsize = *inoutsize;
	int sizePackBuffer=(int)((double)(int)(fsize)*1.2)+1024;
	void *packbuf=galloc(sizePackBuffer);
	int compressedsize=0;

#if WANTLZMA
	if (OPTpack==2)
	{
		int opt=OPTquality;
		if (!opt)
			opt = gmax(4,gmin(9,(int)((fsize+99999)/100000)));
		compressedsize = LZMA_encode(packbuf,inbuf,(int)fsize,&opt);
	}
	else
#endif
#if WANTZIP
	if (OPTpack==4)
	{
		compressedsize = ZIP_encode(packbuf,inbuf,(int)fsize);
	}
	else
#endif
	if (OPTpack==1)
	{
		if (REF_is(inbuf))
		{
			compressedsize = (int)fsize;

			memcpy(packbuf,inbuf,compressedsize);

			if (OPTfaildoublepack)
			{
				if (OPTverbose)
					gprintf(TOOL_NAME " - ERROR attempted to compress a file that is already compressed. File '%s'\n",fname);
				++gerrorcount;
			}
			else if (OPTverbose>=2)
			{
				gprintf(TOOL_NAME " - WARNING attempted to compress a file that is already compressed. File '%s'\n",fname);
			}
		}
		else
		{
			RefStats refStats;

			refStats.quality = gmax(0,gmin(100,OPTquality));

			compressedsize = REF_encode(packbuf,inbuf,(int)fsize,(int *)&refStats);
		}
	}
	else
		gprintf(TOOL_NAME " - ERROR calling compress without pack type set on '%s'\n",fname);

	*inoutsize = compressedsize;
	return packbuf;
}

void *GLUMP_load(const char *fname, int *size)
{
	void *buf = NULL;
	GSTREAM *g = gopen(fname);
	if (g)
	{
		int fsize = (int) glen(g);
		bool ok=false;
		buf = galloc(fsize+1);	// 1 byte extra to allow 0 length files and for a sentinel
		if (buf)
		{
			((unsigned char *)buf)[fsize] = (unsigned char) 0xfb;
			if (gread(g,buf,fsize)==fsize)
			{
				ok = true;
				if (OPTpack)
				{
					void *newBuf = GLUMP_compress(fname,buf,&fsize);
					gfree(buf);
					buf = newBuf;
				}
			}
			if (ok)
				*size = fsize;
			else
			{
				gfree(buf);
				buf = NULL;
			}
		}
	}
	return buf;	
}

int GLUMP_update(GSTREAM *g, const char *fname, GPOS foffset, int fsize, int OPTextract)
{
	int newSize=0;
	void *buf = GLUMP_load(fname,&newSize);

	if (newSize==fsize)
	{
		gseek(g,foffset);
		gwrite(g,buf,newSize);
		if (OPTverbose>=4)
			printf("%s %d (same size)\n",fname,newSize,fsize);
	}
	else if (newSize<fsize)
	{
		gseek(g,foffset);
		gwrite(g,buf,newSize);
		if (OPTverbose>=4)
			printf("%s %d <- %d (got smaller)\n",fname,newSize,fsize);
	}
	if (newSize>fsize)
	{
		if (OPTverbose)
			printf("glump - ERROR %s %d <- %d (larger).  Can not increase size.\n",fname,newSize,fsize);
		++gerrorcount;
		newSize = 0;
	}

	gfree(buf);
	return newSize;
}

static int GLUMP_zeropad(GSTREAM *g,int padbytes)
{
    int ok=1;
	static bool firstPad=true;
    while (padbytes>=8 && ok)
    {
        char buf[8]={0};
        ok = gwrite(g,buf,8);
        padbytes -= 8;
    }
    while (padbytes-- && ok)
    {
        char buf[1]={0};
        ok = gwrite(g,buf,1);
    }
    return ok;
}

// note (uint)-pos % align should work, but compiler does not like it

static int GLUMP_alignpad(INT64 pos, int OPTalign)
{
    int padbytes = OPTalign-(int)(pos % OPTalign);
    if (padbytes==OPTalign)
        padbytes = 0;
    return padbytes;
}

static void GLUMP_settitle(char *title)
{
#if defined(_MSC_VER)
    if (OPTverbose)
        SetConsoleTitle(title);
#endif
}


/* percentage for compression */
// usage:
//    char reduction[8];
//    GLUMP_percentstr(reduction, unpacked, packed);

double GLUMP_CompressionRatio(GPOS olen, GPOS nlen)
{
    double reduction=100.0;

    if (olen < nlen)
    {
        reduction = -GLUMP_CompressionRatio(nlen,olen);
    }
    else
    {
        if (nlen && olen)
        {
            reduction = 100.0-(double)nlen*100.0/(double)olen;
        }
    }
    return reduction;
}

/* percentage calculation status */

char *GLUMP_percentstr(char *buf, GPOS olen, GPOS nlen)
{
    double reduction=100.0;

    if (olen < nlen)
    {
        *buf = '-';
        GLUMP_percentstr(buf+1, nlen, olen);
    }
    else
    {
        if (nlen && olen)
        {
            reduction = 100.0-(double)nlen*100.0/(double)olen;
        }
        sprintf(buf, "%.1f%%", reduction);
    }
    return(buf);
}

/* get motorola memory up to 40 bits */

static __inline UINT64 ggetm64(const void *src, int bytes)
{
    if (bytes==1)
        return (UINT64) *(const unsigned char *) src;
    else if (bytes==2)
        return (((UINT64) *(const unsigned char *) src)<<8)  | (((UINT64) *((const unsigned char *) src+1)));
    else if (bytes==3)
        return (((UINT64) *(const unsigned char *) src)<<16) | (((UINT64) *((const unsigned char *) src+1))<<8) | (((UINT64) *((const unsigned char *) src+2)));
    else if (bytes==4)
        return (((UINT64) *(const unsigned char *) src)<<24) | (((UINT64) *((const unsigned char *) src+1))<<16) | (((UINT64) *((const unsigned char *) src+2))<<8) | (((UINT64) *((const unsigned char *) src+3)));
    else if (bytes==5)
        return (((UINT64) *(const unsigned char *) src)<<32) | (((UINT64) *((const unsigned char *) src+1))<<24) | (((UINT64) *((const unsigned char *) src+2))<<16) | (((UINT64) *((const unsigned char *) src+3))<<8) | (((UINT64) *((const unsigned char *) src+4)));
    return 0;
}

/* put motorolla memory */

static __inline void gputm64(void *dst, UINT64 data, int bytes)
{
    if (bytes==1)
    {
        ((unsigned char *) dst)[0] = (unsigned char) data;
    }
    else if (bytes==2)
    {
        ((unsigned char *) dst)[0] = (unsigned char) (data>>8);
        ((unsigned char *) dst)[1] = (unsigned char) data;
    }
    else if (bytes==3)
    {
        ((unsigned char *) dst)[0] = (unsigned char) (data>>16);
        ((unsigned char *) dst)[1] = (unsigned char) (data>>8);
        ((unsigned char *) dst)[2] = (unsigned char) data;
    }
    else if (bytes==4)
    {
        ((unsigned char *) dst)[0] = (unsigned char) (data>>24);
        ((unsigned char *) dst)[1] = (unsigned char) (data>>16);
        ((unsigned char *) dst)[2] = (unsigned char) (data>>8);
        ((unsigned char *) dst)[3] = (unsigned char) data;
    }
    else if (bytes==5)
    {
        ((unsigned char *) dst)[0] = (unsigned char) (data>>32);
        ((unsigned char *) dst)[1] = (unsigned char) (data>>24);
        ((unsigned char *) dst)[2] = (unsigned char) (data>>16);
        ((unsigned char *) dst)[3] = (unsigned char) (data>>8);
        ((unsigned char *) dst)[4] = (unsigned char) data;
    }
}



/*
 * abs2rel: convert an absolute path name into relative.
 *
 *  i)  pathname    absolute path
 *  i)  folder      base directory (must be absolute path)
 *  o)  result      result buffer
 *  i)  size        size of result buffer
 *  r)      != NULL: relative path
 *          == NULL: error
 */

#if defined(_MSC_VER)
static const char Slash = '\\';
#else
static const char Slash = '/';
#endif

static inline bool isseperator(char c)
{
    return c=='/' || c=='\\';
}

// compare two characters slash and case independent
// true if different, false if same

static inline bool chrcmp(char c1, char c2)
{
    if (tolower(c1)-tolower(c2))
        if (isseperator(c1)!=isseperator(c2))
            return true;
    return false;
}

static char *abs2rel(const char *pathname, const char *folder, char *result, const size_t size)
{
    const char *p=pathname;
    const char *f=folder;

    // find common folder
    const char *branch = f-1;   // +1 added later
    for (;*p && !chrcmp(*p,*f); ++p, ++f)
        if (isseperator(*p))
            branch = f;
    if ((!*p && isseperator(*f)) || (!*f && isseperator(*p)))
        branch = f;

    p = pathname+(branch-folder)+1; // remember source branch point as well

    // go up tree with ../ to branch point

    char *r = result;
    if (branch>=folder)
    {
        for (;*branch; ++branch)
        {
            if (isseperator(*branch) && branch[1])
            {
                if (r>=(result+size-3)) // out of string space
                    return NULL;
                *r++ = '.';
                *r++ = '.';
                *r++ = Slash;

            }
        }
    }
    *r = '\0';
    if (r>=(result+size-strlen(p))) // out of string space
        return NULL;
    strcpy(r,p);
    return result;
}






/* RETURNS THE FILE NAME OF A PATH */

static void StripPath(char *dest, const char *src)
{
    char buf[1024];
    char *p;

    strcpy(buf, src);
    p = buf + strlen(buf);
    while (*p != '\\' && *p != '/' && p != buf)
        --p;
    if (p == buf)
        strcpy(dest, buf);
    else
        strcpy(dest, p+1);
    return;
}

static bool PathSame(const char *name1,const char *name2)
{
    bool isSame=false;
    char dir1[SPITMAXLINE];
    char dir2[SPITMAXLINE];
    SPIT_getpath(dir1,name1);
    SPIT_getpath(dir2,name2);
    if (!stricmp(dir1,dir2))
        isSame = true;
    return isSame;
}


/* Fix up the name based on command line options -killnames and -p0 */
// hashname replaces name with crc32 as 8 chars

// for OPTpaths==3 (common directory replacement)
static char PrevDstName[SPITMAXLINE]={0,};

static void LUMP_processname(char *dstname, const char *srcname, int OPTbigsubtype, int &OPTchunkhex)
{
    if (OPTbigsubtype==BIGSUBTYPE_CHUNK)
        sprintf(dstname,"%-4.4s",srcname);
    else
        strcpy(dstname, srcname);

    if (OPTchunkhex>=0)
    {
        sprintf(dstname,"%04x",OPTchunkhex);
        if (!OPTchop)
            ++OPTchunkhex;
    }

// pass in CurrentDirectory for efficiency if this is too slow
#if defined(_MSC_VER)
    if (OPTrelative)
    {
        char CurrentDirectory[SPITMAXLINE];
        GetCurrentDirectory((DWORD)sizeof(CurrentDirectory), CurrentDirectory);

        char RelativeName[SPITMAXLINE];

        if (abs2rel(dstname,CurrentDirectory,RelativeName,sizeof(RelativeName)))
            strcpy(dstname,RelativeName);
    }
#endif


    if (OPTpaths==0)
        StripPath(dstname, dstname);

    if (OPTpaths==2 || OPTpaths==3)
    {
        for (int i=0; dstname[i]; ++i)
            if (dstname[i]=='\\')
                dstname[i] = '/';
    }
    if (OPTpaths==4)
    {
        for (int i=0; dstname[i]; ++i)
            if (dstname[i]=='/')
                dstname[i] = '\\';
    }

    if (OPTtolower)
        strlwr(dstname);
    if (OPTtoupper)
        strupr(dstname);

    if (OPTidupper && OPTbigsubtype==BIGSUBTYPE_CHUNK)
        strupr(dstname);

    if (OPTpaths==3)
    {
        if (PathSame(PrevDstName,dstname))
        {
            char TempDstName[SPITMAXLINE];
            StripPath(TempDstName,dstname);
            sprintf(dstname,":%s",TempDstName);
        }
        else
            strcpy(PrevDstName,dstname);
    }

    if (OPThashnames)
    {
        unsigned int hashid = hash(dstname, OPTseed);
        sprintf(dstname,"%08x",hashid);
    }

    if (OPTkillnames)
        dstname[0] = '\0';
}


#if 0
/*
;
; ABSTRACT
;
; glump - bigfile client tool for the command line<html img border="0" src="glump.gif" align="right" width="128" height="188">
;
; SUMMARY
;     glump -options bigfile.viv [files]
; 
;     -options       one command and multiple options.  semi-order sensitive
;     bigfile.viv    name of bigfile
;     [files]        list of input files.  optional =name.out for each.
;     @file.rsp      use response file to specify files
; 
; COMMANDS:
;     -new           create new bigfile. (aka -n) (default) (ignored)
;     -add           add files (aka -a) (ignored)
;     -update        update files from bigfile (aka -u)
;     -view          view file (aka -v) (default if only one file given)
;     -extract       extract files from bigfile (aka -x)
;     -x2            extract without decompression
;     -?<topic>      help for specified topic. Also -help, ?, -h
; 
; FORMATS:
;     -big           output big file (aka -viv) (default)
;     -16            16 bit big file (aka -n16)
;     -24            24 bit big file (aka -n24)
;     -32            32 bit big file (aka -n32)
;     -40            40 bit big file (aka -n40)
;     -o24           old 24 bit big file
;     -o32           old 32 bit big file
;     -shp           shape style big file (same as 32 but with ShpF signature)
;     -shp1          old shape style big file (same as old fsh)
;     -chunk         chunk based big file
;     -raw           raw file concatenation
; 
; ENCODE:
;     -pack<n>       compression methods to try (0=none, 1=refpack, 4=zip)
;     -packthresh<n> compress specified amount, or disable compression
;     -overlapthresh<n> disable compression if refpack overlap exceeds the specified value
;     -packmax<n>    don't attempt to compress files this size or larger (1GB)
;     -q<n>          quality for compression 1..9.  1=fast, 9=best (default)\n"
;     -span<n>       align enough to avoid spanning N byte boundary. ie sectors
;     -pad<n>        pad end of bigfile with zeros to specified alignment(ie 2048)
;     -gap<n>        pad between files in bigfile with zeros
;     -seed<n>       seed value for name hash (default 5381)
;     -chop<n>       chop large files into chunks of specified size
;     -chunkhex<n>   name the chunk using %4x starting with this number if >=0
;     -paths0        strip directories from paths (aka -p0)
;     -paths1        keep directories from paths (default)
;     -paths2        convert \ to / in paths
;     -paths3        common tree replacement with :
;     -paths4        convert / to \ in paths
;     -rel           make paths relative to current directory
;     -sourcepath    use source name, not destination name (default off)
;     -killnames     kill names in the bigfile (aka -k)
;     -serial        embed a mastering serial number
;     -hash          big with hash names (a big file format)
;     -hashnames     replace string names with DJB2 hash of name
; 
; OPTIONS:
;     @file          response file, containing a list of options and files
;     =file          specify a output file ie source=dest.
;     -b0            disable banner
;     -v0            no console output
;     -v1            no output except fatal errors
;     -v2            terse output (default). 1 line summary
;     -v3            verbose output. small file level summary
;     -v4            diagnostic output. small file level diagnostics
;     -namelist      output filename list and nothing else. (aka -viewnames) Combines with -x -v
;     -sort1         sort by pathname with awareness of numerals in names
;     -sort3         sort by pathname alphabetically
;     -sort5         sort by destination name
;     -sort6         sort by destination extension
;     -sort          aliases to -sort1
;     -w0            do not write output file
;     -r             recurse subdirectories to find files
;     -r%ENV%        traverse environment variable ENV to find files
;     -time          show execution time
;     -hex           show output in hexadecimal values
;     -blocksize<n>  copy in blocks this size (32768 default)
;     -removeonerror remove bigfile if an error occurs
;     -failoverlap<n>  fail if refpack overlap is more than specified amount
;     -failmaxname<n>  fail if max file name length is more than specified amount
;     -faildoublepack<n> fail if attempting to compress a file already compressed
;     -ignoremissingfile ignore read errors on files not found
;     -hashcheck     just check for hash name collisions. Don't build bigfile
;     -nofiles       kill files and just do header
;     -idupper       change chunk id's names to upper case
;     -tolower       change names to lower case
;     -toupper       change names to upper case
;     -nobak         creates no backup file (ignored)
;     -time          Display timing diagnostic
;     -return<expr>  return expression (default errorcount). See VARIABLES
; 
; VARIABLES:
;     errorcount, TOOL_VERSION, SPIT_VERSION, chunksizemax, chunksizemin,
;     filecount, totalsize, totaloriginalsize, headersize, bigtype, align,
;     time, venom, overlap, namesizemax, namesizemin
; 
; WILDCARD SYNTAX:
;     *  zero of more of any characters
;     ?  any character (exactly 1 character)
;     #  numerical value of any number of digits
;     ## 2 digit numerical value, zero padded, for output
; 
; RESPONSE FILE:
;     %1    first argument of response file.  Passing example @file.rsp(bob)
;     %var% environment variable
;
; DESCRIPTION
;
; Glump joins many files into a single file.
; The destination bigfile is the first filename in the command line.
; Options can be in any order, but if specified more than once, apply
; left to right, allowing options (ie -pack) to preceed each small file.
;
; Glump can import/export several file formats:
;  viv      big file format
;  bh       big head format
;  fsh      Shape File format.  Also xsh & ssh
;  fss      Streaming Shape format (chunk)
;  raw      raw data
;  mad/mpc  Movie formats
;  ffn      Font files
;
; RETURNS
;
; number of errors
;
; SEE ALSO
;
; .viv, bighead, huff, chimp, Spit
;
; SAMPLE OUTPUT
;
; >glump -x player.viv -namelist >list.rsp
;
; >glump -pack1 -align64 -sort player2.viv @list.rsp
; player2.viv: 17260 files, 164485646 <- 304406829 bytes 46.0%, 543081 align bytes, 2 overlap, sizes 168..141309, name sizes 6..49
;
; >glump player2.viv
; player2.viv: 17260 files, 164485646 bytes <- 304406829 46.0%, align 64, sizes 168..141309, name sizes 6..49
;
; END ABSTRACT
;
*/
#endif

/****************************************************************/
/*  SPIT Routines                                               */
/****************************************************************/

void SPIT_reset(void)
{
	OPTstartsortat    = 1;
    OPTstartrecurseat = 1;
    OPTkeepopts       = 1;
    OPTskiprules      = 1; // dont apply rules on first arg
    OPTpathrule       = 0; // keep path as is

#if WANTREF || WANTZIP || WANTLZMA
    OPTpackmax        = 1*1024*1024*1024;
	OPTpackthresh     = -1024; // 1024 is how much overlap we normally allow for.  If compression grows by more than this, runtime may have issues
    OPTblocksize      = 32*1024;
    OPTquality        = 0; // auto based on size
#endif
    OPTalign          = 64;  // 64 is optimal for ps2 dma & amd/p4 cahc
    OPTspan           = 0;
    OPTpad            = 0;   // pad at end of bigfile
    OPTgap            = 0;   // gap between files
    OPTkillnames      = 0;
	OPTserial		  = 0;
	OPTremoveonerror  = 0;
    OPToverlapthresh  = 0;
    OPTfailoverlap    = 0;
    OPTfailmaxname    = 0;
	OPTfaildoublepack = 0;
	OPTignoremissingfile = 0;
	OPThashnames      = 0;
    OPThashcheck      = 0;
    OPTnamelist       = 0;
    OPTnofiles        = 0;
    OPTidupper        = 0;
    OPTpaths          = 1; // keeppaths
    OPTrelative       = 0; // option to make paths relative off
    OPTbigsubtype     = BIGSUBTYPE_BIGN;
    OPTtolower        = 0;
    OPTtoupper        = 0;
    OPTsourcepath     = 0; // use source or destination pathname
    OPThex            = 0;
    OPTseed           = 5381;
    OPTchop           = 0;
    OPTchunkhex       = -1; // name the chunk using %4x starting with this number if >=0
    strcpy(OPTreturn,"");

#if EXTRACTONLY
	OPTverbose		  = 0;
#endif

    return;
}

char *optionhelp[]=
{
#if !EXTRACTONLY
    "USAGE:\n"
    "    glump -options bigfile.viv [files]\n"
	"\n"
    "    -options       one command and multiple options.  semi-order sensitive\n"
	"    bigfile.viv    name of bigfile\n"
	"    [files]        list of input files.  optional =name.out for each.\n" 
	"    @file.rsp      use response file to specify files\n" 
	"\n",
    "COMMANDS:\n"
    "    -new           create new bigfile. (aka -n) (default) (ignored)\n"
    "    -add           add files (aka -a) (ignored)\n"
    "    -update        update files from bigfile (aka -u)\n"
    "    -view          view file (aka -v) (default if only one file given)\n"
    "    -extract       extract files from bigfile (aka -x).  -x2 to not decompress\n"
    "    -?<topic>      help for specified topic. Also -help, ?, -h\n"
	"\n",
	"FORMATS:\n"
    "    -big           output big file (aka -viv) (default)\n"
    "    -16            16 bit big file (aka -n16)\n"
    "    -24            24 bit big file (aka -n24)\n"
    "    -32            32 bit big file (aka -n32)\n"
    "    -40            40 bit big file (aka -n40)\n"
    "    -o24           old 24 bit big file\n"
    "    -o32           old 32 bit big file\n"
    "    -shp           shape style big file (same as 32 but with ShpF signature)\n"
    "    -shp1          old shape style big file (same as old fsh)\n"
    "    -chunk         chunk based big file\n"
    "    -raw           raw file concatenation\n"
	"\n",
    "ENCODE:\n"
#if WANTREF || WANTZIP || WANTLZMA
#if WANTREF && WANTZIP && WANTLZMA
    "    -pack<n>       compression methods to try (0=none, 1=refpack, 2=lzma, 4=zip)\n"
#elif WANTREF && WANTZIP
    "    -pack<n>       compression methods to try (0=none, 1=refpack, 4=zip)\n"
#endif
#if WANTREF || WANTLZMA
    "    -q<n>          quality for compression 1..9.  1=fast, 9=best (default)\n"
#endif
	"    -packthresh<n> compress specified amount, or disable compression\n"
	"    -overlapthresh<n> disable compression if refpack overlap exceeds the specified value\n"
	"    -packmax<n>    don't attempt to compress files this size or larger (1GB)\n"
#endif
	"    -align<n>      align big files to N bytes (aka -l)\n"
    "    -span<n>       align enough to avoid spanning N byte boundary. ie sectors\n"
	"    -pad<n>        pad at end of bigfile with zeros to specified alignment(ie 2048)\n"
	"    -gap<n>        pad between files in bigfile with zeros with specified number of zeros(ie 16)\n"
    "    -seed<n>       seed value for name hash (default 5381)\n"
    "    -chop<n>       chop large files into chunks of specified size\n"
    "    -chunkhex<n>   name the chunk using %4x starting with this number if >=0\n"
    "    -nofiles       kill files and just do header\n"
    "    -paths0        strip directories from paths (aka -p0)\n"
    "    -paths1        keep directories from paths (default)\n"
    "    -paths2        convert \\ to / in paths\n"
    "    -paths3        common tree replacement with :\n"
    "    -paths4        convert / to \\ in paths\n"
    "    -rel           make paths relative to current directory\n"
    "    -sourcepath    use source name, not destination name (default off)\n"
    "    -killnames     kill names in the bigfile (aka -k)\n"
    "    -serial        embed a mastering serial number\n"
    "    -hash          big with hash names (a big file format)\n"
	"    -hashnames     replace string names with DJB2 hash of name\n"
	"\n",
    "\nOPTIONS:\n"
	"    @file          response file, containing a list of options and files\n"
    "    =file          specify a output file ie source=dest.\n"
    "                   %d in <dest> uses frame index.  %s uses framename\n"
    "    -b0            disable banner\n"
    "    -v0            no console output\n"
    "    -v1            no output except fatal errors\n"
    "    -v2            terse output (default). 1 line summary\n"
    "    -v3            verbose output. small file level summary\n"
    "    -v4            diagnostic output. small file level diagnostics\n"
    "    -namelist      output filename list and nothing else. (aka -viewnames) Combines with -x -v\n"
    "    -sort1         sort by pathname with awareness of numerals in names\n"
    "    -sort3         sort by pathname alphabetically\n"
    "    -sort5         sort by destination name\n"
    "    -sort6         sort by destination extension\n"
    "    -sort          aliases to -sort1\n"
	"    -w0            do not write output file\n"
    "    -r             recurse subdirectories to find files\n"
    "    -r%ENV%        traverse environment variable ENV to find files\n"
    "    -time          show execution time\n"
    "    -hex           show output in hexadecimal values\n"
	"    -blocksize<n>  copy in blocks this size (32768 default)\n"
	"    -removeonerror remove bigfile if an error occurs\n"
	"    -failoverlap<n>  fail if refpack overlap is more than specified amount\n"
	"    -failmaxname<n>  fail if max file name length is more than specified amount\n"
	"    -faildoublepack<n> fail if attempting to compress a file already compressed\n"
	"    -ignoremissingfile ignore read errors on files not found\n"
    "    -hashcheck     just check for hash name collisions. Don't build bigfile\n"
    "    -idupper       change chunk id's names to upper case\n"
    "    -tolower       change names to lower case\n"
    "    -toupper       change names to upper case\n"
    "    -nobak         creates no backup file (ignored)\n"
    "    -time          Display timing diagnostic\n"
    "    -return<expr>  return expression (default errorcount). See VARIABLES\n"
#if 0
    "\n",
    "    old options\n",
    "    -d -define     print #defines for bigfile\n",
    "    -expand        expand other zip/wad/big files and convert entries\n",
    "    -m -mincompress<n> compress if can reduce by <n> percent\n",
    "    -remove        remove listed files from bigfile\n",
    "    -wad           output UK wad file (refpack)\n",
    "    -zip           output zip file\n",
    "    -nobak         creates no backup file\n",
#endif
    "\n",
    "VARIABLES:\n"
    "    errorcount, TOOL_VERSION, SPIT_VERSION, chunksizemax, chunksizemin,\n"
    "    filecount, totalsize, totaloriginalsize, headersize, bigtype, align,\n"
    "    time, venom, overlap, namesizemax, namesizemin,\n"
    "\n",
    "WILDCARD SYNTAX:\n"
    "    *  zero of more of any characters\n"
    "    ?  any character (exactly 1 character)\n"
    "    #  numerical value of any number of digits\n"
    "    ## 2 digit numerical value, zero padded, for output\n"
    "\n",
    "RESPONSE FILE:\n"
    "    %1    first argument of response file.  Passing example @file.rsp(bob)\n"
    "    %var% environment variable\n"
    "\n",
#endif // !EXTRACTONLY
    0,
};

void SPIT_usage(const char *prgname, const char *envname, const char *urlname, int utype)
{
#if !EXTRACTONLY
    char **hlp;

    if (!OPTnamelist && ((OPTverbose>=2 && utype==2) || OPTverbose>=4))
        gprintf("Glump v%s by FrANK G. Barchard, EAC\n",TextVersion);

    switch (utype)
    {
        case 0:
        case 1:
            break;

        case 2: gprintf("%s",
	"Bigfile client tool for the command line.  For help, type -?TOPIC\n"
	"\n"
	"HELP TOPICS:\n"
    "    usage       simple command line syntax\n"
    "    commands    list all commands\n"
	"    formats     file format options\n"
	"    encode      options affecting data output\n"
	"\n"
    "    options     miscellenous options\n"
    "    variables   variables for expression parser/options\n"
    "    wildcard    wildcard syntax\n"
	"    response    help on response files\n"
	"    help        help topics\n"
	"\n"
	"    The full user manual is available at http://tools.eac.ea.com/util/glump.htm\n");
            break;

        case 3:
            gprintf("\n");
            gprintf("VERSION: %s v%s, Spit v%.2f\n",SPIT_MACHINE,TextVersion,(float)SPIT_VERSION/100.0f);
            gprintf("SUMMARY: make a big file\n");
            gprintf("         SPIT environment variable is %s\n",envname);
            gprintf("\n");

            hlp = optionhelp;
            while (*hlp)
            {
                gprintf("%s",*hlp);
                ++hlp;
            }
            break;

         case 4:
            gprintf("\n");
            {
                hlp = optionhelp;
                while (*hlp)
                {
                    if (SPIT_strstr(*hlp,envname))
                        gprintf("%s",*hlp);
                    ++hlp;
                }
                gprintf("\n");
            }
            break;

        default:
            break;
    }
#endif // !EXTRACTONLY
}

void BIG_settype(int bits, int subtype, bool forcesubtype)
{
    OPTbign = bits/8;
    if (forcesubtype || (OPTbigsubtype != BIGSUBTYPE_BIGN && OPTbigsubtype != BIGSUBTYPE_HASHN))
        OPTbigsubtype = subtype;
}

char *SPIT_opt(const char* optptr)
{
    char *d=NULL;
    switch (tolower(*optptr))
    {
        case '1':
            if (!strnicmp(optptr, "16", 2))
            {
                BIG_settype(16,BIGSUBTYPE_BIGN,false);
                d = (char*)(optptr+2);
            }
            break;

        case '2':
            if (!strnicmp(optptr, "24", 2))
            {
                BIG_settype(24,BIGSUBTYPE_BIGN,false);
                d = (char*)(optptr+2);
            }
            break;

        case '3':
            if (!strnicmp(optptr, "32", 2))
            {
                BIG_settype(32,BIGSUBTYPE_BIGN,false);
                d = (char*)(optptr+2);
            }
            break;

        case '4':
            if (!strnicmp(optptr, "40", 2))
            {
                BIG_settype(40,BIGSUBTYPE_BIGN,false);
                d = (char*)(optptr+2);
            }
            break;

        case 'a':
        {
            int OPTaddfile=1;

            if (!strnicmp(optptr, "add", 3))
            {
                d = (char*)(optptr+3);
            }
            else if (!strnicmp(optptr, "align", 5))
            {
                d = SPIT_getopt( &OPTalign,optptr+5, "alignment", 0, 65536);
                if (OPTalign==0) OPTalign = 1;
            }
            else
            {
                d = SPIT_getopt( &OPTaddfile,optptr+1, "add", 0, 1);
                if (OPTaddfile==0 && OPTverbose)
                    gprintf(TOOL_NAME " -add0 not supported\n");
            }
            break;
        }

        case 'b':
            if (!strnicmp(optptr, "big", 3))
            {
                int OPTbig=0;
                d = SPIT_getopt( &OPTbig,optptr+3, "big file format", 0, 1);
                if (OPTbig)
                    OPTbigsubtype = BIGSUBTYPE_32;
                else
                    OPTbigsubtype = BIGSUBTYPE_CHUNK;
            }
            else if (!strnicmp(optptr, "blocksize", 9))
                d = SPIT_getopt( &OPTblocksize,optptr+9, "copy blocksize", 1, 0x7fffffff);
            break;

        case 'c':
            if (!strnicmp(optptr, "chunkhex", 8))
            {
                d = SPIT_getopt( &OPTchunkhex,optptr+8, "chunk use hex names", -1, 0x7fffffff);
            }
            else if (!strnicmp(optptr, "chunk", 5))
            {
                int OPTchunk=0;
                d = SPIT_getopt( &OPTchunk,optptr+5, "chunk file format", 0, 1);
                if (OPTchunk)
                    OPTbigsubtype = BIGSUBTYPE_CHUNK;
                else
                    OPTbigsubtype = BIGSUBTYPE_BIGN;
            }
            else if (!strnicmp(optptr, "chop", 4))
            {
                d = SPIT_getopt( &OPTchop,optptr+4, "chop into chunks", 0, 0x7fffffff);
            }
            break;

        case 'e':
            if (!strnicmp(optptr, "extract", 7))
                d = SPIT_getopt( &OPTextract,optptr+7, "extract", 0, 2);
            break;

        case 'f':
            if (!strnicmp(optptr, "failoverlap", 11))
                d = SPIT_getopt( &OPTfailoverlap,optptr+11, "fail if overlap exceeded", 0, 16384);
            else if (!strnicmp(optptr, "failmaxname", 11))
                d = SPIT_getopt( &OPTfailmaxname,optptr+11, "fail if maximum name length exceeded", 0, 1024);
            else if (!strnicmp(optptr, "faildoublepack", 14))
                d = SPIT_getopt( &OPTfaildoublepack,optptr+14, "fail if compressing a file that is already compressed", 0, 1);
            break;

        case 'g':
            if (!strnicmp(optptr, "gap", 3))
            {
                d = SPIT_getopt( &OPTgap,optptr+3, "gap size", 0, 65536);
            }
			break;

		case 'h':
            if (!strnicmp(optptr, "hashnames", 9))
                d = SPIT_getopt( &OPThashnames,optptr+9, "hash names", 0, 1);
            else if (!strnicmp(optptr, "hashcheck", 9))
                d = SPIT_getopt( &OPThashcheck,optptr+9, "hash check", 0, 1);
            else if (!strnicmp(optptr, "hex", 3))
                d = SPIT_getopt( &OPThex,optptr+3, "show hex values", 0, 1);

            else if (!strnicmp(optptr, "hash", 4))
            {
                int OPThash=0;
                d = SPIT_getopt( &OPThash,optptr+4, "hash file format", 0, 1);
                if (OPThash)
                    OPTbigsubtype = BIGSUBTYPE_HASHN;
                else
                    OPTbigsubtype = BIGSUBTYPE_BIGN;
            }
            else if (!strnicmp(optptr, "h16", 3))
            {
                BIG_settype(16,BIGSUBTYPE_HASHN,true);
                d = (char*)(optptr+3);
            }
            else if (!strnicmp(optptr, "h24", 3))
            {
                BIG_settype(24,BIGSUBTYPE_HASHN,true);
                d = (char*)(optptr+3);
            }
            else if (!strnicmp(optptr, "h32", 3))
            {
                BIG_settype(32,BIGSUBTYPE_HASHN,true);
                d = (char*)(optptr+3);
            }
            else if (!strnicmp(optptr, "h40", 3))
            {
                BIG_settype(40,BIGSUBTYPE_HASHN,true);
                d = (char*)(optptr+3);
            }

            break;

        case 'i':
            if (!strnicmp(optptr, "idupper", 7))
                d = SPIT_getopt( &OPTidupper,optptr+7, "chunk id uppercase", 0, 1);
            else if (!strnicmp(optptr, "ignoremissingfile", 17))
                d = SPIT_getopt( &OPTignoremissingfile,optptr+17, "ignore missing files", 0, 1);
            break;

        case 'k':
            if (!strnicmp(optptr, "killnames", 9))
                d = SPIT_getopt( &OPTkillnames,optptr+9, "kill names", 0, 1);
            else
                d = SPIT_getopt( &OPTkillnames,optptr+1, "kill names", 0, 1);
            break;

        case 'l':
            d = SPIT_getopt( &OPTalign,optptr+1, "align", 0, 65536);
            if (OPTalign==0) OPTalign = 1;
            break;

        case 'n':
			if (!strnicmp(optptr, "namelist", 8))
            {
                d = SPIT_getopt( &OPTnamelist,optptr+8, "print name list", 0, 1);
            }
			else if (!strnicmp(optptr, "nobak", 5))
            {
                int OPTnoback=1;
                d = SPIT_getopt( &OPTnoback,optptr+5, "no backup file", 0, 1);
                if (OPTnoback==0 && OPTverbose)
                    gprintf(TOOL_NAME " -nobak0 not supported\n");
            }
            else if (!strnicmp(optptr, "nofiles", 7))
                d = SPIT_getopt( &OPTnofiles,optptr+7, "no files", 0, 1);
            else if (!strnicmp(optptr, "new", 3))
            {
                int OPTnewfile=1;
                d = SPIT_getopt( &OPTnewfile,optptr+3, "create new", 0, 1);
                if (OPTnewfile==0 && OPTverbose)
                    gprintf(TOOL_NAME " -new0 not supported\n");
            }
            else if (!strnicmp(optptr, "n16", 3))
            {
                BIG_settype(16,BIGSUBTYPE_BIGN,true);
                d = (char*)(optptr+3);
            }
            else if (!strnicmp(optptr, "n24", 3))
            {
                BIG_settype(24,BIGSUBTYPE_BIGN,true);
                d = (char*)(optptr+3);
            }
            else if (!strnicmp(optptr, "n32", 3))
            {
                BIG_settype(32,BIGSUBTYPE_BIGN,true);
                d = (char*)(optptr+3);
            }
            else if (!strnicmp(optptr, "n40", 3))
            {
                BIG_settype(40,BIGSUBTYPE_BIGN,true);
                d = (char*)(optptr+3);
            }
            else
            {
                int OPTnewfile=1;
                d = SPIT_getopt( &OPTnewfile,optptr+1, "create new", 0, 1);
                if (OPTnewfile==0 && OPTverbose)
                    gprintf(TOOL_NAME " -n0 not supported\n");
            }
            break;

        case 'o':
            if (!strnicmp(optptr, "overlapthresh", 13))
                d = SPIT_getopt( &OPToverlapthresh,optptr+13, "turn off compression if overlap exceeded", 0, 16384);
            else if (!strnicmp(optptr, "o24", 3))
            {
                BIG_settype(24,BIGSUBTYPE_24,true);
                d = (char*)(optptr+3);
            }
            else if (!strnicmp(optptr, "o32", 3))
            {
                BIG_settype(32,BIGSUBTYPE_32,true);
                d = (char*)(optptr+3);
            }
            break;

        case 'p':
            if (!strnicmp(optptr, "pad", 3))
            {
                d = SPIT_getopt( &OPTpad,optptr+3, "pad alignment", 0, 65536);
            }
            else if (!strnicmp(optptr, "paths", 5))
                d = SPIT_getopt( &OPTpaths,optptr+5, "paths", 0, 4);
#if WANTREF || WANTZIP || WANTLZMA
            else if (!strnicmp(optptr, "packmax", 7))
                d = SPIT_getopt( &OPTpackmax,optptr+7, "maximum size file to compress", 0, 0x7fffffff);
            else if (!strnicmp(optptr,"packthresh",10))
                d = SPIT_getopt( &OPTpackthresh,optptr+10, "pack threshhold", -1000000, 1000000);
            else if (!strnicmp(optptr,"pack",4))
                d = SPIT_getopt2( &OPTpack,optptr+4, "pack", 0, 4, 1);
#endif
            else
                d = SPIT_getopt( &OPTpaths,optptr+1, "paths", 0, 4);
            break;

#if WANTREF || WANTLZMA
	    case 'q':
            d = SPIT_getopt( &OPTquality,optptr+1, "quality", 0, 9);
            break;
#endif

        case 'r':
            if (!strnicmp(optptr, "rel", 3))
                d = SPIT_getopt( &OPTrelative,optptr+3, "relative", 0, 1);
            else if (!strnicmp(optptr,"return",6))
                d = SPIT_getfield( OPTreturn,optptr+6,1,0);
            else if (!strnicmp(optptr, "removeonerror", 13))
                d = SPIT_getopt( &OPTremoveonerror,optptr+13, "remove bigfile on error", 0, 1);
            else if (!strnicmp(optptr, "raw", 3))
            {
                OPTbigsubtype = BIGSUBTYPE_RAW;
                d = (char*)(optptr+3);
            }
            else if (!strnicmp(optptr, "remove", 6))
			{
                d = (char*)(optptr+6);
				if (OPTverbose>=1)
				{
					gprintf(TOOL_NAME " - ERROR REMOVE OPTION NOT SUPPORTED\n");
					exit(1); // exit quickly before bigfile gets damaged
				}
			}
            break;

        case 's':
            if (!strnicmp(optptr, "sourcepath", 10))
            {
                d = SPIT_getopt( &OPTsourcepath,optptr+10, "use source pathname", 0, 1);
            }
            else if (!strnicmp(optptr, "shp", 3))
            {
                int OPTshp=0;
                d = SPIT_getopt2( &OPTshp,optptr+3, "shape file format", 0, 2, 2);
                if (OPTshp==2)
                    OPTbigsubtype = BIGSUBTYPE_SHP;
                else if (OPTshp==1)
                    OPTbigsubtype = BIGSUBTYPE_SHP1;
                else
                    OPTbigsubtype = BIGSUBTYPE_BIGN;
            }
            else if (!strnicmp(optptr, "seed", 4))
            {
                d = SPIT_getopt( &OPTseed,optptr+4, "seed for hash", 0, 65536);
            }
            else if (!strnicmp(optptr, "span", 4))
            {
                d = SPIT_getopt( &OPTspan,optptr+4, "span alignment", 0, 65536);
            }
            else if (!strnicmp(optptr, "serial", 6))
			{
                d = SPIT_getopt( &OPTserial,optptr+6, "serialize", 0, 1);
			}
            break;

        case 't':
            if (!strnicmp(optptr, "tolower", 7))
            {
                d = SPIT_getopt( &OPTtolower,optptr+7, "name to lower case", 0, 1);
            }
            else if (!strnicmp(optptr, "toupper", 7))
            {
                d = SPIT_getopt( &OPTtoupper,optptr+7, "name to upper case", 0, 1);
            }
            break;

		case 'u':
            if (!strnicmp(optptr, "update", 6))
                d = SPIT_getopt( &OPTupdate,optptr+6, "update", 0, 1);
            else
                d = SPIT_getopt( &OPTupdate,optptr+1, "update", 0, 1);
			break;

        case 'v':
            if (!strnicmp(optptr, "viv", 3))
            {
                d = (char*)(optptr+3);
            }
            else if (!strnicmp(optptr, "viewnames", 9))
            {
                d = SPIT_getopt( &OPTnamelist,optptr+9, "print a name list", 0, 1);
            }
            else if (!strnicmp(optptr, "view", 4))
            {
                d = SPIT_getopt( &OPTview,optptr+4, "view", 0, 1);
            }
            break;

        case 'x':
            d = SPIT_getopt( &OPTextract,optptr+1, "extract", 0, 2);
            break;


        default:
            break;
    }
    return d;
}

int glumpmain(int argc, char *argv[])
{
    int more;
    int i;
    int errorcount=0;
    int returncode=0;
    INT64 chunksizemax=0;
    INT64 chunksizemin=0xffffffff;
    int namesizemax=0;
    int namesizemin=0x7fffffff;
	int filecount = 0;
    double totaltime;
    GPOS totaloriginalsize;
    GPOS totalsize=0;
    int headersize = 0;
    int infovenom=0;

	RefStats totalRefStats = { 0, };

    double encodetime=0.0;

    SPIT_init(SPIT_reset, SPIT_usage, SPIT_opt, SPIT_history);

    EXPR *expr = EXPR_open();

#if defined(_MSC_VER)
    if (OPTtime)
        SetThreadPriority(0, THREAD_PRIORITY_ABOVE_NORMAL);
#endif

    totaltime = SPIT_gettime();
    do
    {
        more = SPIT_parse(argc, argv, TOOL_NAME, TOOL_VERSION);

        totaloriginalsize = 0;
        if (!SPIThelpgiven && SPITargnum>0)
        {
            int bigarg = -1;
            filecount = 0;
            headersize = 0;
            char *headerbuf;
            char *headerptr = NULL;
            GPOS offset=0;

/****************************************************************/
/*  PASS1: prepass thru arguments to calculate header size and  */
/*  file count. no File IO (for efficiency)                     */
/*  - determines if user is doing add or view/extract           */
/****************************************************************/

            HashCollisionInit(); // hash collision table
            int collisionCount=0;
            for (i=0; i<SPITargnum; ++i)
            {
                bool isopt = false;
                if (SPITsargv[i][0]=='-' || SPITsargv[i][0]=='+' || (SPITsargv[i][0]==OPTdot && SPITsargv[i][1]!='.' && SPITsargv[i][1]!='/' && SPITsargv[i][1]!='\\'))
                    isopt = Zop(SPITsargv[i]+1)?true:false;
                else if (SPITsargv[i][0]=='=')
                    isopt = Zop(SPITsargv[i])?true:false;

                if (!isopt)
                {
                    if (bigarg<0)
                        bigarg = i;
                    else
                    {
                        char outfilename[SPITMAXLINE];

                        LUMP_processname(outfilename,OPTsourcepath?SPITsargv[i]:SPITdargv[i],OPTbigsubtype,OPTchunkhex);
                        switch (OPTbigsubtype)
                        {
                            case BIGSUBTYPE_BIGN:  headersize += OPTbign+gmin(4,OPTbign)+strlen(outfilename)+1; break;
                            case BIGSUBTYPE_HASHN: headersize += OPTbign+gmin(4,OPTbign)+4; break;
                            case BIGSUBTYPE_32:    headersize += 4+4+strlen(outfilename)+1; break;
                            case BIGSUBTYPE_SHP:   headersize += 4+4+strlen(outfilename)+1; break;
                            case BIGSUBTYPE_SHP1:  headersize += 8; break;
                            case BIGSUBTYPE_24:    headersize += 3+3+strlen(outfilename)+1; break;
                            case BIGSUBTYPE_CHUNK: break;
                            case BIGSUBTYPE_RAW:   break;
                        }
                        ++filecount;

                        if (OPThashcheck || OPThashnames || OPTbigsubtype==BIGSUBTYPE_HASHN)
                        {
                            if (HashCollisionCheck(hash(outfilename)))
                            {
                                gprintf(TOOL_NAME " - ERROR Hash collision on %s.\n",outfilename);
                                ++collisionCount;
                            }
                        }
                    }
                }
            }

            HashCollisionRestore();
            if (collisionCount || OPThashcheck)
            {
                gprintf(TOOL_NAME " - %d collisions, %d files\n",collisionCount,filecount);

                if (OPThashcheck)   // all done
                    continue;
            }

            // if you run glump with no args without a path on the tool, and found in the path, not the current folder, 1 arg will be glump.viv.  bug

            if (bigarg == -1)
            {
                if (OPTverbose>=6)
                    gprintf(TOOL_NAME " - bigarg %d, filecount %d, SPITargnum %d\n",bigarg,filecount,SPITargnum);
                if (OPTverbose>=2)
                    gprintf(TOOL_NAME " - OUTPUT FILE NOT SPECIFIED");
                ++errorcount;
                goto bigerror;
            }


/****************************************************************/
/*  VIEW/EXTRACT                                                */
/****************************************************************/

            if (OPTview || OPTnamelist || OPTextract || OPTupdate || !filecount) // if files specified, we are creating/adding a bigfile. Otherwise its a view/extract operation
            {
				for (i=0; i<SPITargnum; ++i)
				{
					bool isopt = false;
					if (SPITsargv[i][0]=='-' || SPITsargv[i][0]=='+' || (SPITsargv[i][0]==OPTdot && SPITsargv[i][1]!='.' && SPITsargv[i][1]!='/' && SPITsargv[i][1]!='\\'))
						isopt = Zop(SPITsargv[i]+1)?true:false;
					else if (SPITsargv[i][0]=='=')
						isopt = Zop(SPITsargv[i])?true:false;

	                if (!isopt)
	                {
                        char outfilename[SPITMAXLINE];
                        LUMP_processname(outfilename,OPTsourcepath?SPITsargv[i]:SPITdargv[i],OPTbigsubtype,OPTchunkhex);

                        // add on the extension if no extension is given
                        {
                            char *ext = strrchr(outfilename,'.');
                            if (!ext)
                            {
                                SPIT_changeext(outfilename, outfilename, "viv");
                            }
                        }

                        int chunkoffsetbits=0;
                        chunksizemax=0;
                        chunksizemin=0xffffffff;
						namesizemax=0;
		                namesizemin=0x7fffffff;

                        if (OPTverbose>=3 && !OPTnamelist)
                        {
                            if (OPTextract==2)
                                gprintf("Extracting asis %s",outfilename);
                            else if (OPTextract)
                                gprintf("Extracting %s",outfilename);
                            else if (OPTupdate)
                                gprintf("Updating %s",outfilename);
							else
                                gprintf("Viewing %s",outfilename);
                        }

                        GSTREAM *g;
						if (OPTupdate)
						{
							g = grwopen(outfilename);	// open for read write access if updating 
						}
						else
						{
							g = gopen(outfilename);
						}
						if (!g)
						{
							if (OPTverbose)
								gprintf(TOOL_NAME " - ERROR can not open bigfile %s for view/extract\n",outfilename);
							++errorcount;
						}
                        else
                        {
                            char tempbuf[32];
                            totalsize=0;
                            gread(g,tempbuf,32);
                            if (ggetm(tempbuf+0,4)==GMAKEID('B','I','G','F'))
                            {
                                OPTbigsubtype=BIGSUBTYPE_32;
                                filecount =  ggetm(tempbuf+8,4);
                                headersize = ggetm(tempbuf+12,4);
                                if (OPTverbose>=3 && !OPTnamelist)
                                    gprintf(", old 32 bit");
                            }
                            else if (ggetm(tempbuf+0,4)==GMAKEID('S','h','p','F') 
                                  || ggetm(tempbuf+0,4)==GMAKEID('S','h','p','X')
                                  || ggetm(tempbuf+0,4)==GMAKEID('S','h','p','S'))
                            {
                                OPTbigsubtype=BIGSUBTYPE_SHP;
                                filecount =  ggetm(tempbuf+8,4);
                                headersize = ggetm(tempbuf+12,4);
                                if (OPTverbose>=3 && !OPTnamelist)
                                    gprintf(", shape file");
                            }
                            else if (ggetm(tempbuf+0,4)==GMAKEID('S','H','P','I')
                                  || ggetm(tempbuf+0,4)==GMAKEID('S','H','P','X')
                                  || ggetm(tempbuf+0,4)==GMAKEID('S','H','P','S')
                                  || ggetm(tempbuf+0,4)==GMAKEID('G','S','T','S'))
                            {
                                OPTbigsubtype=BIGSUBTYPE_SHP1;
                                filecount =  ggeti(tempbuf+8,4);
                                headersize = 16+filecount*8;
                                if (OPTverbose>=3 && !OPTnamelist)
                                    gprintf(", old shape file");
                            }

                            else if (ggetm(tempbuf+0,2)==0xc0fb)
                            {
                                OPTbigsubtype=BIGSUBTYPE_24;
                                headersize = ggetm(tempbuf+2,2)+4;
                                filecount =  ggetm(tempbuf+4,2);
                                if (OPTverbose>=3 && !OPTnamelist)
                                    gprintf(", old 24 bit");
                            }
                            else if ((ggetm(tempbuf+0,4)&0xffffff00)==GMAKEID('B','I','G',0)
                                  && isdigit(tempbuf[3]))
                            {
                                OPTbigsubtype=BIGSUBTYPE_BIGN;
                                OPTbign = tempbuf[3]-'0';
                                filecount =  ggetm(tempbuf+8,4);
                                headersize = ggetm(tempbuf+12,4);
                                if (OPTverbose>=3 && !OPTnamelist)
                                    gprintf(", %d bit",OPTbign*8);
                            }
                            else if ((ggetm(tempbuf+0,4)&0xffffff00)==GMAKEID('B','i','g',0)
                                  && isdigit(tempbuf[3]))
                            {
                                OPTbigsubtype=BIGSUBTYPE_HASHN;
                                OPTbign = tempbuf[3]-'0';
                                filecount =  ggetm(tempbuf+8,4);
                                headersize = ggetm(tempbuf+12,4);
                                if (OPTverbose>=3 && !OPTnamelist)
                                    gprintf(", hash %d bit",OPTbign*8);
                            }
                            else
                            {
                                OPTbigsubtype=BIGSUBTYPE_CHUNK;
                            }

                            if (OPTbigsubtype==BIGSUBTYPE_BIGN || OPTbigsubtype==BIGSUBTYPE_HASHN || OPTbigsubtype==BIGSUBTYPE_32 || OPTbigsubtype==BIGSUBTYPE_SHP || OPTbigsubtype==BIGSUBTYPE_SHP1 || OPTbigsubtype==BIGSUBTYPE_24)
                            {
                                int filenum=0;

                                headerbuf = (char *) galloc(headersize);
                                if (headerbuf)
                                {
                                    infovenom=0;
                                    int infoflags=1; // assume compressed

                                    gseek(g,0);
                                    gread(g,headerbuf,headersize);

                                    char *venomptr=headerbuf+headersize-GLUMP_DEBUGINFO;
                                    if (isalpha(venomptr[0]) && isdigit(venomptr[1]) && isdigit(venomptr[2]) && isdigit(venomptr[3]))
                                    {
                                        infovenom=(venomptr[1]-'0')*100 + (venomptr[2]-'0')*10 + (venomptr[3]-'0');
                                        infoflags=ggetm(venomptr+4,4);
                                        if (OPTverbose>=3 && !OPTnamelist)
                                        {
                                            gprintf(", version %d",infovenom);
                                            if (infoflags&2)
                                                gprintf(", sorted");
                                            if (infoflags&4)
                                                gprintf(", seed %d",(infoflags>>16));
                                            if (infoflags&8)
                                                gprintf(", islower");
                                        }
                                    }

                                    if (OPTverbose>=3 && !OPTnamelist)
                                    {
                                        gprintf("\nIndex      Offset       Size     Packed  Ratio  File\n");
                                    }

                                    if (OPTbigsubtype==BIGSUBTYPE_BIGN)
                                        headerptr = headerbuf+16;
                                    else if (OPTbigsubtype==BIGSUBTYPE_HASHN)
                                        headerptr = headerbuf+16;
                                    else if (OPTbigsubtype==BIGSUBTYPE_32)
                                        headerptr = headerbuf+16;
                                    else if (OPTbigsubtype==BIGSUBTYPE_SHP)
                                        headerptr = headerbuf+16;
                                    else if (OPTbigsubtype==BIGSUBTYPE_SHP1)
                                        headerptr = headerbuf+16;
                                    else
                                        headerptr = headerbuf+6;
                                    totaloriginalsize = 0;
                                    totalsize = 0;

                                    while (filenum<filecount)
                                    {
                                        GPOS foffset;
                                        GPOS fsize;
                                        GPOS originalsize;
                                        char tempname[16];

                                        char *fname;
                                        if (OPTbigsubtype==BIGSUBTYPE_BIGN)
                                        {
                                            foffset = ggetm64(headerptr+0,OPTbign);
                                            fsize   = ggetm64(headerptr+OPTbign,gmin(4,OPTbign));
                                            fname = headerptr+OPTbign+gmin(4,OPTbign);
                                        }
                                        else if (OPTbigsubtype==BIGSUBTYPE_HASHN)
                                        {
                                            foffset = ggetm64(headerptr+0,OPTbign);
                                            fsize   = ggetm64(headerptr+OPTbign,gmin(4,OPTbign));
                                            int fhash = (int)ggetm64(headerptr+OPTbign+gmin(4,OPTbign),4);
                                            sprintf(tempname,"%08x",fhash);
                                            fname = tempname;
                                        }
                                        else if (OPTbigsubtype==BIGSUBTYPE_32
                                              || OPTbigsubtype==BIGSUBTYPE_SHP)
                                        {
                                            foffset = ggetm64(headerptr+0,4);
                                            fsize   = ggetm64(headerptr+4,4);
                                            fname = headerptr+8;
                                        }
                                        else if (OPTbigsubtype==BIGSUBTYPE_SHP1)
                                        {
                                            foffset = (GPOS) ggeti(headerptr+4,4);
                                            int fnextoffset = ggeti(headerbuf+4,4);
                                            if (filenum<(filecount-1))
                                                fnextoffset = ggeti(headerptr+12,4);
                                            fsize   = fnextoffset-foffset;
											tempname[4] = '\0';
                                            sprintf(tempname,"%-4.4s",headerptr);
                                            fname = tempname;
                                        }
                                        else
                                        {
                                            foffset = ggetm64(headerptr+0,3);
                                            fsize   = ggetm64(headerptr+3,3);
                                            fname = headerptr+6;
                                        }
										if (fsize>0x7fffffff)	// make it negative
											fsize -= 0x100000000LL;

                                        // size/offset stats
                                        chunkoffsetbits |= foffset;
                                        if (fsize<chunksizemin)
                                            chunksizemin = fsize;
                                        if (fsize>chunksizemax)
                                            chunksizemax = fsize;
										int nsize = strlen(fname);
										if (nsize<namesizemin)
											namesizemin = nsize;
										if (nsize>namesizemax)
											namesizemax = nsize;
										if (OPTfailmaxname && nsize>OPTfailmaxname)
										{
											if (OPTverbose)
												gprintf(TOOL_NAME " - ERROR max name size of %d exceeded by file %s with name size of %d\n",OPTfailmaxname,fname,nsize);
											++errorcount;
										}

                                        originalsize = fsize;

                                        if (originalsize && gseek(g,foffset))
										{
											// note if last file is less than 12 bytes, this will read off end of file
                                            if (gread(g,tempbuf,12)==12)	// determine original size
											{
                                                if (REF_is(tempbuf))
													originalsize = REF_size(tempbuf);
#if WANTLZMA
												else if (LZMA_is(tempbuf))
													originalsize = LZMA_size(tempbuf);
#endif
#if WANTZIP
                                                else if (ZIP_is(tempbuf))
													originalsize = ZIP_size(tempbuf);
#endif
											}
										}
                                        totaloriginalsize += originalsize;
                                        totalsize += fsize;

										int dargv=0;
										bool foundName = IsOnList(dargv, fname,SPITsargv,i+1,SPITargnum);
										if (foundName)
										{
											const char *outname = fname;
											char outbuf[SPITMAXLINE];
											if	(dargv<SPITargnum)
												outname = SPITdargv[dargv];
											else if (OPTpathrule)
											{
												replacePathAbs(outbuf, fname, OPTpathrule);
												outname = outbuf;
											}

											if (OPTnamelist)
											{
												gprintf("%s\n",fname);
											}
											else if (OPTverbose>=3)
											{
												char reductionbuf[8];
												if (OPThex) gprintf("%5x%12" FORMAT64 "x%11" FORMAT64 "x",filenum,foffset,originalsize);
												else        gprintf("%5d%12" FORMAT64 "d%11" FORMAT64 "d",filenum,foffset,originalsize);

												if (OPThex) gprintf("%11" FORMAT64 "x%7s",fsize,GLUMP_percentstr(reductionbuf, originalsize, fsize));
												else        gprintf("%11" FORMAT64 "d%7s",fsize,GLUMP_percentstr(reductionbuf, originalsize, fsize));
												
												if (OPTverbose>=4)
													gprintf("  %s\n",outname);
												else
													gprintf("  %s\n",fname);
											}

											if (OPTextract)
											{
												GLUMP_extract(g,outname,foffset,(int)fsize, OPTextract);
											}

											if (OPTupdate)
											{
												int newSize = GLUMP_update(g,fname,foffset,(int)fsize, OPTextract);
												if (newSize)
												{
													fsize = newSize;

			                                        if (OPTbigsubtype==BIGSUBTYPE_BIGN)
													{
				                                        gputm(headerptr+OPTbign,newSize,gmin(4,OPTbign));
													}
												}
											}

											if (OPTverbose>=3)
											{
												char titlebuf[SPITMAXLINE];

												if (OPThex) sprintf(titlebuf,"%s - GLUMP %s %s %" FORMAT64 "x %" FORMAT64 "x",outfilename,OPTextract?"Extracting":OPTupdate?"Updating":"Viewing",fname,foffset,fsize);
												else        sprintf(titlebuf,"%s - GLUMP %s %s %" FORMAT64 "d %" FORMAT64 "d",outfilename,OPTextract?"Extracting":OPTupdate?"Updating":"Viewing",fname,foffset,fsize);

												GLUMP_settitle(titlebuf);
											}
										}

                                        if (OPTbigsubtype==BIGSUBTYPE_HASHN)
                                            headerptr += 12;
                                        else if (OPTbigsubtype==BIGSUBTYPE_SHP1)
                                            headerptr += 8;
                                        else
                                            headerptr = fname+strlen(fname)+1;
                                        ++filenum;
                                    }

									if (OPTupdate)
									{
										gseek(g,0);
	                                    gwrite(g,headerbuf,headersize);
									}

                                    gfree(headerbuf);
                                }

                                if (OPTverbose>=2 && !OPTnamelist)
                                {
                                    int chunkalignment=1;

                                    while (chunkoffsetbits && !(chunkoffsetbits&1) && chunkalignment<(256*1024))
                                    {
                                        chunkoffsetbits >>= 1;
                                        chunkalignment <<= 1;
                                    }
                                    if (totaloriginalsize!=totalsize)
                                    {
										char reductionbuf[8];
                                        gprintf("%s: %d files, %" FORMAT64 "d bytes",outfilename,filecount, glen(g));
                                        gprintf(" <- %" FORMAT64 "d %s",totaloriginalsize, GLUMP_percentstr(reductionbuf, totaloriginalsize, glen(g)));
                                    }
                                    else
                                    {
                                        gprintf("%s: %d files, %" FORMAT64 "d bytes",outfilename,filecount, glen(g));
                                    }
                                    gprintf(", align %d, sizes %" FORMAT64 "d..%" FORMAT64 "d, name sizes %d..%d\n",chunkalignment,chunksizemin,chunksizemax,namesizemin,namesizemax);
                                    OPTalign = chunkalignment; // for -return
                                }
                                totalsize = glen(g);
                            }
                            gclose(g);
                        }
						break;
                    }
                }
            }

/****************************************************************/
/*  CREATE/ADD                                                  */
/****************************************************************/

#if !EXTRACTONLY
            else
            {
			    bool ok;
	            GSTREAM *g;
	            char bigfileout[SPITMAXLINE];

                int TotalPad=0;
                chunksizemax=0;
                chunksizemin=0x7fffffff;
                namesizemax=0;
                namesizemin=0x7fffffff;

                strcpy(bigfileout,SPITdargv[bigarg]);

                // add on the extension if no extension is given
                {
                    char *ext = strrchr(bigfileout,'.');
                    if (!ext)
                    {
                        SPIT_changeext(bigfileout, bigfileout, "viv");
                    }
                }

                infovenom = TOOL_VERSION; // for return
                if (OPTbigsubtype==BIGSUBTYPE_24 || ((OPTbigsubtype==BIGSUBTYPE_BIGN||OPTbigsubtype==BIGSUBTYPE_HASHN) && OPTbign==2))
                {
                    if (filecount>=65536)
                    {
                        if (OPTverbose)
                            gprintf(TOOL_NAME " - ERROR file count is larger than 64K.  Use -32\n");
                        ++errorcount;
                        goto bigadderror;
                    }
                    if (headersize>=65536-6)
                    {
                        if (OPTverbose)
                            gprintf(TOOL_NAME " - ERROR header is larger than 64K.  Use -32\n");
                        ++errorcount;
                        goto bigadderror;
                    }
                }

                switch (OPTbigsubtype)
                {
                    case BIGSUBTYPE_BIGN:  headersize += 16; break;
                    case BIGSUBTYPE_HASHN: headersize += 16; break;
                    case BIGSUBTYPE_32:    headersize += 16; break;
                    case BIGSUBTYPE_SHP:   headersize += 16; break;
                    case BIGSUBTYPE_SHP1:  headersize += 16; break;
                    case BIGSUBTYPE_24:    headersize += 6;  break;
                    case BIGSUBTYPE_CHUNK: break;
                    case BIGSUBTYPE_RAW:   break;
                }

#if GLUMP_DEBUGINFO
                if (OPTbigsubtype==BIGSUBTYPE_BIGN
                 || OPTbigsubtype==BIGSUBTYPE_HASHN
                 || OPTbigsubtype==BIGSUBTYPE_32
                 || OPTbigsubtype==BIGSUBTYPE_SHP
                 || OPTbigsubtype==BIGSUBTYPE_SHP1
                 || OPTbigsubtype==BIGSUBTYPE_24)
                    headersize += GLUMP_DEBUGINFO;
#endif
				if (OPTverbose>=2)
				{
					char titlebuf[SPITMAXLINE];
					sprintf(titlebuf, "%s - GLUMP Creating",bigfileout);
					GLUMP_settitle(titlebuf);
				}

                if (OPTverbose>=3 && !OPTnamelist)
                {
                    gprintf("Creating %s, ",bigfileout);
                    switch (OPTbigsubtype)
                    {
                        case BIGSUBTYPE_BIGN:  gprintf("%d bit",OPTbign*8); break;
                        case BIGSUBTYPE_HASHN: gprintf("hash %d bit",OPTbign*8); break;
                        case BIGSUBTYPE_32:    gprintf("old 32 bit"); break;
                        case BIGSUBTYPE_SHP:   gprintf("Shape File"); break;
                        case BIGSUBTYPE_SHP1:  gprintf("Old Shape File"); break;
                        case BIGSUBTYPE_24:    gprintf("old 24 bit"); break;
                        case BIGSUBTYPE_CHUNK: gprintf("Chunk"); break;
                        case BIGSUBTYPE_RAW:   gprintf("Raw"); break;
                    }
                    gprintf(", align %d",OPTalign);
                    if (OPTspan)
                        gprintf(", span %d",OPTspan);
                    if (OPTpad)
                        gprintf(", pad %d",OPTpad);
                    if (OPTgap)
                        gprintf(", gap %d",OPTgap);
					if (OPTchop)
                        gprintf(", chop %d",OPTchop);
                    if (OPTchunkhex>=0)
                        gprintf(", chunkhex %d",OPTchunkhex);
#if WANTREF || WANTZIP || WANTLZMA
                    if (OPTpack)
                    {
                        if (OPTpack==1)
                            gprintf(", Refpack\n");
#if WANTLZMA
                        else if (OPTpack==2)
                            gprintf(", LZMA\n");
#endif
#if WANTZIP
                        else
                            gprintf(", Zip\n");
#endif
                    }
                    else
#endif
                        gprintf("\n");
				}

                if (OPTverbose>=3 && !OPTnamelist)
                {
                    if (OPTbigsubtype==BIGSUBTYPE_HASHN)
                        gprintf("Index      Offset       Size     Packed  Ratio  Hash      File\n");
                    else
                        gprintf("Index      Offset       Size     Packed  Ratio  File\n");
                }

                if (headersize)
                {
                    headerbuf=(char *)galloc(headersize);
                    memset(headerbuf,0,headersize);
                }
                else
                    headerbuf = (char *) NULL;

                switch (OPTbigsubtype)
                {
                    case BIGSUBTYPE_BIGN:  headerptr=headerbuf+16; break;
                    case BIGSUBTYPE_HASHN: headerptr=headerbuf+16; break;
                    case BIGSUBTYPE_32:    headerptr=headerbuf+16; break;
                    case BIGSUBTYPE_SHP:   headerptr=headerbuf+16; break;
                    case BIGSUBTYPE_SHP1:  headerptr=headerbuf+16; break;
                    case BIGSUBTYPE_24:    headerptr=headerbuf+6;  break;
                    case BIGSUBTYPE_CHUNK: headerptr=headerbuf;    break;
                    case BIGSUBTYPE_RAW:   headerptr=headerbuf;    break;
                }

                g = gwopen(bigfileout);
                if (!g)
                {
                    if (OPTverbose)
                        gprintf(TOOL_NAME " - ERROR unable to open bigfile %s\nCould be a directory, missing directory, read only, or permission issue\n",bigfileout);

                    gfree(headerbuf);
                    ++errorcount;
                    goto bigadderror;
                }
                gwrite(g,headerbuf,headersize);

				if (OPTserial)
				{
					const char *serial = "---\r\nxa37dd45ffe100bfffcc9753aabac325f07cb3fa231144fe2e33ae4783feead2b8a73ff021fac326df0ef9753ab9cdf6573ddff0312fab0b0ff39779eaff312x\r\n---";

	                gwrite(g,serial,strlen(serial));
				}
				
				/****************************************************************/
                /*  PASS2: copy files into bigfile                              */
                /****************************************************************/

                filecount = 0;
                for (i=0; i<SPITargnum; ++i)
                {
					bool isopt = false;
					if (SPITsargv[i][0]=='-' || SPITsargv[i][0]=='+' || (SPITsargv[i][0]==OPTdot && SPITsargv[i][1]!='.' && SPITsargv[i][1]!='/' && SPITsargv[i][1]!='\\'))
						isopt = Zop(SPITsargv[i]+1)?true:false;
					else if (SPITsargv[i][0]=='=')
						isopt = Zop(SPITsargv[i])?true:false;

                    if (!isopt && i!=bigarg)
                    {
                        GPOS fsize;
                        GPOS originalsize;
                        int padbytes = 0;
                        void *inbuf = NULL;
                        char outfilename[SPITMAXLINE];
                        LUMP_processname(outfilename,OPTsourcepath?SPITsargv[i]:SPITdargv[i],OPTbigsubtype,OPTchunkhex);

                        // show title asap so we can see what is being added before we do the add

                        if (OPTverbose>=3)
                        {
                            char titlebuf[SPITMAXLINE];

                            sprintf(titlebuf,"%s - GLUMP Adding %s",bigfileout,outfilename);

                            GLUMP_settitle(titlebuf);
                        }

                        // copy content (failure is not fatal)

                        GSTREAM *gin = gopen(SPITsargv[i]);
                        if (!gin)
                        {
                            fsize = originalsize = offset = 0;
//                            ok = 0;
							if (OPTignoremissingfile)
                            {
                                if (OPTverbose>=4)
                                    gprintf(TOOL_NAME " - WARNING READING %s\n",SPITsargv[i]);
                            }
							else
                            {
                                if (OPTverbose)
                                    gprintf(TOOL_NAME " - ERROR READING %s\n",SPITsargv[i]);
                                ++errorcount;
                            }
                        }
                        else
                        {
                            fsize = originalsize = glen(gin);


                            if (OPTnofiles)
                                offset = 0;
                            else
                            {
								bool compressedOkay=false;
                                // compress file

#if WANTREF || WANTZIP || WANTLZMA
                                if (OPTpack && fsize<(GPOS)OPTpackmax)
                                {
                                    int compressedsize;
                                    inbuf = galloc((int)fsize+1);	// +1 to avoid galloc 0 warning
                                    gread(gin,inbuf,(int)fsize);
									gseek(gin,0);	// incase we need to reread it for uncompressed add
                                    int sizePackBuffer=(int)((double)(int)(fsize)*1.2)+1024;
                                    void *packbuf=galloc(sizePackBuffer);
                                    encodetime -= SPIT_gettime();
#if WANTLZMA
                                    if (OPTpack==2)
                                    {
                                        int opt=OPTquality;
                                        if (!opt)
                                            opt = gmax(4,gmin(9,(int)((fsize+99999)/100000)));
                                        compressedsize = LZMA_encode(packbuf,inbuf,(int)fsize,&opt);
										compressedOkay = true;
                                    }
                                    else
#endif
#if WANTZIP
                                    if (OPTpack==4)
                                    {
                                        compressedsize = ZIP_encode(packbuf,inbuf,(int)fsize);
										compressedOkay = true;
                                    }
                                    else
#endif
									{
										if (REF_is(inbuf))
										{
											compressedsize = (int)fsize;

											memcpy(packbuf,inbuf,compressedsize);
											compressedOkay = true;

											if (OPTfaildoublepack)
											{
												if (OPTverbose)
													gprintf(TOOL_NAME " - ERROR attempted to compress a file that is already compressed. File '%s'\n",SPITsargv[i]);
												++errorcount;
												compressedOkay = false;
											}
											else if (OPTverbose>=2)
											{
												gprintf(TOOL_NAME " - WARNING attempted to compress a file that is already compressed. File '%s'\n",SPITsargv[i]);
											}
										}
										else
										{
											RefStats refStats;

											refStats.quality = gmax(0,gmin(100,OPTquality));

											compressedsize = REF_encode(packbuf,inbuf,(int)fsize,(int *)&refStats);

											if (OPTverbose>=6)
												gprintf("REFSTATS overlap %d, literals %d, short %d, long %d, vlong %d\n",refStats.overlap,refStats.countLiterals,refStats.countShort,refStats.countLong,refStats.countVLong);

											compressedOkay = true;
											if (OPTfailoverlap && refStats.overlap>OPTfailoverlap)
											{
												if (OPTverbose)
													gprintf(TOOL_NAME " - ERROR overlap of %d exceeded by file %s with overlap of %d\n",OPTfailoverlap,SPITsargv[i],refStats.overlap);
												++errorcount;
												compressedOkay = false;
											}
											if (OPToverlapthresh && refStats.overlap>OPToverlapthresh)
											{
												if (OPTverbose>=4)
													gprintf(TOOL_NAME " - WARNING attempted overlap of %d exceeded by file %s with overlap of %d.  Refpack disabled.\n",OPTfailoverlap,SPITsargv[i],refStats.overlap);
												compressedOkay = false;
											}

											if (compressedOkay)
											{
												if (refStats.overlap>totalRefStats.overlap)
													totalRefStats.overlap = refStats.overlap;
												totalRefStats.countLiterals += refStats.countLiterals;
												totalRefStats.countShort += refStats.countShort;
												totalRefStats.countLong += refStats.countLong;
												totalRefStats.countVLong += refStats.countVLong;
											}
										}
									}
                                    encodetime += SPIT_gettime();

									// throws away compression if it came out larger by threshhold

                                    if (!(compressedsize<(int)(fsize-OPTpackthresh) && (compressedsize<sizePackBuffer)))
										compressedOkay = false;

									if (!(OPTpack && fsize<(GPOS)OPTpackmax))
										compressedOkay = false;

									if (compressedOkay)
									{
                                        gfree(inbuf);
                                        inbuf = packbuf;
                                        originalsize = fsize;
                                        fsize = compressedsize;
                                    }
                                    else
									{
										if (OPTverbose>=6)
											gprintf(TOOL_NAME " - WARNING disabled compression on file  %s\n",SPITsargv[i]);
                                        gfree(inbuf);
                                        gfree(packbuf);
									}
                                }
								else
									compressedOkay = false;


#endif


                                // align start of next file
                                if (OPTbigsubtype==BIGSUBTYPE_BIGN
                                 || OPTbigsubtype==BIGSUBTYPE_HASHN
                                 || OPTbigsubtype==BIGSUBTYPE_32
                                 || OPTbigsubtype==BIGSUBTYPE_SHP
                                 || OPTbigsubtype==BIGSUBTYPE_SHP1
                                 || OPTbigsubtype==BIGSUBTYPE_24
                                 || OPTbigsubtype==BIGSUBTYPE_RAW)
                                {
                                    padbytes = GLUMP_alignpad(gtell(g), OPTalign);
                                    if (padbytes)
                                        GLUMP_zeropad(g,padbytes);
                                    TotalPad += padbytes;

                                    if (OPTspan)
                                    {
                                        padbytes = GLUMP_alignpad(gtell(g), OPTspan);
                                        GPOS spos = gtell(g);
                                        GPOS epos = gtell(g)+fsize;
                                        if (((epos+padbytes)/OPTspan-(spos+padbytes)/OPTspan) < (epos/OPTspan-spos/OPTspan))
                                        {
                                            GLUMP_zeropad(g,padbytes);
                                            if (OPTverbose>=6)
                                                gprintf("spanning additional %d bytes.\n",padbytes);
                                            TotalPad += padbytes;
                                        }
                                    }
                                }

                                offset = gtell(g);



								
								
								if (OPTpack && fsize<(GPOS)OPTpackmax && compressedOkay)
                                {
                                    if (OPTchop)
                                    {
                                        GPOS sizetogo=fsize;
                                        while (sizetogo>0)
                                        {
                                            char chunkheader[8];
                                            GPOS writesize = gmin(sizetogo,OPTchop);

                                            padbytes = GLUMP_alignpad(writesize+8, OPTalign);
                                            TotalPad += padbytes;
                                            if (OPTchunkhex>=0)
                                            {
                                                sprintf(chunkheader,"%04x",OPTchunkhex);
                                                ++OPTchunkhex;
                                            }
                                            else
                                                sprintf(chunkheader,"%-4.4s",outfilename);
                                            gputi(chunkheader+4,8+(int)writesize+padbytes,4);
                                            gwrite(g,chunkheader,8);
                                            gwrite(g,inbuf,(int)writesize);
                                            sizetogo -= writesize;

                                            // align size of chunk
                                            if (padbytes)
                                            {
                                                GLUMP_zeropad(g,padbytes);
                                            }
                                        }

                                    }
                                    else
                                    {
                                        if (OPTbigsubtype==BIGSUBTYPE_CHUNK)
                                        {
                                            char chunkheader[8];
                                            padbytes = GLUMP_alignpad(fsize+8, OPTalign);
                                            TotalPad += padbytes;
                                            sprintf(chunkheader,"%-4.4s",outfilename);
                                            gputi(chunkheader+4,8+(int)fsize+padbytes,4);
                                            gwrite(g,chunkheader,8);
                                        }

                                        gwrite(g,inbuf,(int)fsize);
                                    }
                                    gfree(inbuf);
                                }

                                // ADD uncompressed files

                                else
                                {
                                    int blockchop = OPTchop?OPTchop:OPTblocksize;
                                    inbuf = galloc(blockchop);
                                    GPOS fsizeleft=fsize;
                                    ok = true;
                                    bool ChunkHeader=(OPTbigsubtype==BIGSUBTYPE_CHUNK||OPTchop)?true:false;

                                    while (fsizeleft && ok)
                                    {
                                        int nextsize=(int) gmin(fsizeleft,blockchop);
                                        ok = gread(gin,inbuf,nextsize)==nextsize;
                                        if (ok)
                                        {
                                            padbytes = 0;
                                            if (ChunkHeader)
                                            {
                                                char chunkheader[8];

                                                int chunksize=(OPTchop)?8+nextsize:8+(int)fsize;
                                                if (fsizeleft==nextsize||OPTchop)
                                                    padbytes = GLUMP_alignpad(chunksize, OPTalign);
                                                chunksize += padbytes;
                                                TotalPad += padbytes;
                                                if (OPTchunkhex>=0)
                                                {
                                                    sprintf(chunkheader,"%04x",OPTchunkhex);
                                                    ++OPTchunkhex;
                                                }
                                                else
                                                    sprintf(chunkheader,"%-4.4s",outfilename);

                                                gputi(chunkheader+4,chunksize,4);
                                                gwrite(g,chunkheader,8);

                                                ChunkHeader=OPTchop?true:false;
                                            }
                                            ok = gwrite(g,inbuf,nextsize)==nextsize;

                                            // align size of chunk
                                            if (OPTbigsubtype==BIGSUBTYPE_CHUNK && padbytes)
                                            {
                                                GLUMP_zeropad(g,padbytes);
                                            }


                                        }
                                        if (ok)
                                            fsizeleft -= nextsize;
                                    }
                                    gfree(inbuf);

                                    if (!ok)
                                    {
                                        if (OPTverbose)
                                            gprintf(TOOL_NAME " - ERROR LUMPING %s\n",SPITsargv[i]);
                                        ++errorcount;
                                    }
                                }
                            }

                            gclose(gin);
                        }

                        // add gap padding to end of each file
                        if (OPTbigsubtype==BIGSUBTYPE_BIGN
						 || OPTbigsubtype==BIGSUBTYPE_HASHN
						 || OPTbigsubtype==BIGSUBTYPE_32
						 || OPTbigsubtype==BIGSUBTYPE_SHP
						 || OPTbigsubtype==BIGSUBTYPE_SHP1
						 || OPTbigsubtype==BIGSUBTYPE_24
						 || OPTbigsubtype==BIGSUBTYPE_RAW)
						{
							GLUMP_zeropad(g,OPTgap);
							TotalPad += OPTgap;
						}


                        // ADD update header
                        {
                            if (OPTbigsubtype==BIGSUBTYPE_BIGN)
                            {
                                //OPTbign
                                gputm64(headerptr, offset, OPTbign);
                                gputm(headerptr+OPTbign, (int) fsize, gmin(4,OPTbign));
                                strcpy(headerptr+OPTbign+gmin(4,OPTbign), outfilename);
                                headerptr += OPTbign+gmin(4,OPTbign)+strlen(outfilename)+1;

                            }
                            else if (OPTbigsubtype==BIGSUBTYPE_HASHN)
                            {
                                // hashed names
                                gputm64(headerptr, offset, OPTbign);
                                gputm(headerptr+OPTbign, (int) fsize, gmin(4,OPTbign));
                                gputm(headerptr+OPTbign+gmin(4,OPTbign), hash(outfilename,OPTseed), 4);
                                headerptr += OPTbign+gmin(4,OPTbign)+4;
                            }
                            else if (OPTbigsubtype==BIGSUBTYPE_32 || OPTbigsubtype==BIGSUBTYPE_SHP)
                            {
                                gputm(headerptr+0, (int) offset, 4);
                                gputm(headerptr+4, (int) fsize, 4);
                                strcpy(headerptr+8, outfilename);
                                headerptr += 4+4+strlen(outfilename)+1;
                            }
                            else if (OPTbigsubtype==BIGSUBTYPE_SHP1)
                            {
                                strncpy(headerptr+0, outfilename, 4);
                                gputi(headerptr+4, (int) offset, 4);
                                headerptr += 8;
                            }
                            else if (OPTbigsubtype==BIGSUBTYPE_24)
                            {
                                gputm(headerptr+0, (int) offset, 3);
                                gputm(headerptr+3, (int) fsize, 3);
                                strcpy(headerptr+6, outfilename);
                                headerptr += 3+3+strlen(outfilename)+1;
                            }

                            if (OPTbign>=4 && (fsize >= FOURGB))
                            {
                                if (OPTverbose)
                                    gprintf(TOOL_NAME " - ERROR small file %s is larger than 4 GB\n",outfilename);
                                ++errorcount;
                            }
                            if (OPTbign==3 && (fsize >= SIXTEENMB))
                            {
                                if (OPTverbose)
                                    gprintf(TOOL_NAME " - ERROR small file %s is larger than 16 MB\n",outfilename);
                                ++errorcount;
                            }
                            if (OPTbign==2 && (fsize >= SIXTYFOURKB))
                            {
                                if (OPTverbose)
                                    gprintf(TOOL_NAME " - ERROR small file %s is larger than 64 KB\n",outfilename);
                                ++errorcount;
                            }

                            // no headers for BIGSUBTYPE_CHUNK or BIGSUBTYPE_RAW

                            totaloriginalsize += originalsize;

                            if (fsize<chunksizemin)
                                chunksizemin = fsize;
                            if (fsize>chunksizemax)
                                chunksizemax = fsize;
							int nsize = strlen(outfilename);
							if (nsize<namesizemin)
								namesizemin = nsize;
							if (nsize>namesizemax)
								namesizemax = nsize;
							if (OPTfailmaxname && nsize>OPTfailmaxname)
							{
								if (OPTverbose)
									gprintf(TOOL_NAME " - ERROR max name size of %d exceeded by file %s with name size of %d\n",OPTfailmaxname,outfilename,nsize);
								++errorcount;
							}

                            if (OPTnamelist)
                            {
                                gprintf("%s\n",outfilename);
                            }
                            else if (OPTverbose>=3)
                            {
                                char reductionbuf[8];
                                if (OPThex) gprintf("%5x%12" FORMAT64 "x%11" FORMAT64 "x",filecount,offset,originalsize);
                                else        gprintf("%5d%12" FORMAT64 "d%11" FORMAT64 "d",filecount,offset,originalsize);

                                if (OPThex) gprintf("%11" FORMAT64 "x%7s",fsize,GLUMP_percentstr(reductionbuf, originalsize, fsize));
                                else        gprintf("%11" FORMAT64 "d%7s",fsize,GLUMP_percentstr(reductionbuf, originalsize, fsize));

                                if (OPTbigsubtype==BIGSUBTYPE_HASHN)
                                    gprintf("  %08x",hash(outfilename,OPTseed));
                                gprintf("  %s",outfilename);
                                if (OPTchop && OPTchunkhex>=0)
                                    gprintf(" %04x",OPTchunkhex-1);
                                gprintf("\n",outfilename);
                            }

                            ++filecount;
                        }
                    }
                }


                /****************************************************************/
                /*  Pad end of file                                             */
                /****************************************************************/

				if (OPTpad)
				{
					int padbytes = GLUMP_alignpad(gtell(g), OPTpad);
					GLUMP_zeropad(g,padbytes);
					if (OPTverbose>=6)
						gprintf("padding additional %d bytes.\n",padbytes);
					TotalPad += padbytes;
				}

                /****************************************************************/
                /*  Backpatch header                                            */
                /****************************************************************/

                // Check for file overflows
                if ((OPTbigsubtype==BIGSUBTYPE_32) || (OPTbigsubtype==BIGSUBTYPE_SHP) || (OPTbigsubtype==BIGSUBTYPE_SHP1) ||((OPTbigsubtype==BIGSUBTYPE_BIGN || OPTbigsubtype==BIGSUBTYPE_HASHN) && OPTbign==4))
                {
                    // warning on marginal 64 bit files
                    if (offset < FOURGB // last offset is within 4 GB
                     && glen(g) >= FOURGB // file size is more 4 GB
                     && OPTverbose)
                    {
                        gprintf(TOOL_NAME " - warning file is larger than 4 GB, but offset table is within 4 GB\n");
                    }
                    // error on files beyond 32 bits
                    else if (offset >= FOURGB) // last offset beyond 4 GB
                    {
                        if (OPTverbose)
                            gprintf(TOOL_NAME " - ERROR file is larger than 4 GB\n");
                        ++errorcount;
                    }
                }
                else if (OPTbigsubtype==BIGSUBTYPE_24 || ((OPTbigsubtype==BIGSUBTYPE_BIGN || OPTbigsubtype==BIGSUBTYPE_HASHN) && OPTbign==3))
                {
                    // error on files beyond 24 bits
                    if (offset >= SIXTEENMB) // last offset beyond 16 MB
                    {
                        if (OPTverbose)
                            gprintf(TOOL_NAME " - ERROR file is larger than 16 MB\n");
                        ++errorcount;
                    }
                }
                else if ((OPTbigsubtype==BIGSUBTYPE_BIGN || OPTbigsubtype==BIGSUBTYPE_HASHN) && OPTbign==2)
                {
                    // error on files beyond 24 bits
                    if (offset >= SIXTYFOURKB) // last offset beyond 64 KB
                    {
                        if (OPTverbose)
                            gprintf(TOOL_NAME " - ERROR file is larger than 64 KB\n");
                        ++errorcount;
                    }
                }

                offset = glen(g);

                if (OPTbigsubtype==BIGSUBTYPE_BIGN)
                {
                    gputm(headerbuf+0, GMAKEID('B','I','G','0')+OPTbign, 4);
                    gputi(headerbuf+4, (int) offset, 4);
                    gputm(headerbuf+8, filecount, 4);
                    gputm(headerbuf+12, headersize, 4);
                }
                else if (OPTbigsubtype==BIGSUBTYPE_HASHN)
                {
                    gputm(headerbuf+0, GMAKEID('B','i','g','0')+OPTbign, 4);
                    gputi(headerbuf+4, (int) offset, 4);
                    gputm(headerbuf+8, filecount, 4);
                    gputm(headerbuf+12, headersize, 4);
                }
                else if (OPTbigsubtype==BIGSUBTYPE_32)
                {
                    gputm(headerbuf+0, GMAKEID('B','I','G','F'), 4);
                    gputi(headerbuf+4, (int) offset, 4);
                    gputm(headerbuf+8, filecount, 4);
                    gputm(headerbuf+12, headersize, 4);
                }
                else if (OPTbigsubtype==BIGSUBTYPE_SHP)
                {
                    gputm(headerbuf+0, GMAKEID('S','h','p','F'), 4);
                    gputi(headerbuf+4, (int) offset, 4);
                    gputm(headerbuf+8, filecount, 4);
                    gputm(headerbuf+12, headersize, 4);
                }
                else if (OPTbigsubtype==BIGSUBTYPE_SHP1)
                {
                    gputm(headerbuf+0, GMAKEID('S','H','P','I'), 4);
                    gputi(headerbuf+4, (int) offset, 4);
                    gputi(headerbuf+8, filecount, 4);
                    gputm(headerbuf+12,GMAKEID(
                                'L',
                                (TOOL_VERSION/100)%10+'0',
                                (TOOL_VERSION/10)%10+'0',
                                (TOOL_VERSION)%10+'0'),4);
                }
                else if (OPTbigsubtype==BIGSUBTYPE_24)
                {
                    gputm(headerbuf+0, 0xc0fb, 2);
                    gputm(headerbuf+2, headersize-4, 2);
                    gputm(headerbuf+4, filecount, 2);
                }

                // after table of contents, tag on venom

#if GLUMP_DEBUGINFO
                if (headersize)
                {
                    int flags = 0;
#if WANTREF || WANTZIP || WANTLZMA
                    flags |= (OPTpack    ? 1:0);

#endif
                    flags |= (OPTsorttype? 2:0);
                    flags |= (OPTbigsubtype==BIGSUBTYPE_HASHN?4:0);
                    flags |= (OPTseed      <<16);

                    gputm(headerbuf+headersize-GLUMP_DEBUGINFO+0,GMAKEID(
                                'L',
                                (TOOL_VERSION/100)%10+'0',
                                (TOOL_VERSION/10)%10+'0',
                                (TOOL_VERSION)%10+'0'),4);

                    gputm(headerbuf+headersize-GLUMP_DEBUGINFO+4,flags,4);
                }
#endif

                if (headersize)
                {
                    gseek(g,0);
                    gwrite(g,headerbuf,headersize);
                    gfree(headerbuf);
                }
                gclose(g);

                totalsize = offset;
                if (OPTverbose>=2 && !OPTnamelist)
                {
					if (OPTverbose>=2)
					{
						char titlebuf[SPITMAXLINE];
						sprintf(titlebuf, "%s - GLUMP %d files, %" FORMAT64 "d bytes done",bigfileout,filecount, totalsize);
						GLUMP_settitle(titlebuf);
					}

#if WANTREF || WANTZIP || WANTLZMA
					char reductionbuf[8];
                    if (OPTpack)
	                    gprintf("%s: %d files, %" FORMAT64 "d <- %" FORMAT64 "d bytes %s",bigfileout,filecount, totalsize, totaloriginalsize,GLUMP_percentstr(reductionbuf, totaloriginalsize, totalsize));
					else
#endif
	                    gprintf("%s: %d files, %" FORMAT64 "d bytes",bigfileout,filecount, totalsize);

                    gprintf(", %d align bytes",TotalPad);
					if (OPTpack==1)
						gprintf(", %d overlap",totalRefStats.overlap);

                    if (OPThashcheck || OPThashnames || OPTbigsubtype==BIGSUBTYPE_HASHN)
                    {
						gprintf(", hashbits %d",hashBits);
					}

					gprintf(", sizes %" FORMAT64 "d..%" FORMAT64 "d, name sizes %d..%d\n",chunksizemin,chunksizemax,namesizemin,namesizemax);

	                if (OPTverbose>=5 && !OPTnamelist && OPTpack==1)
		            {
						gprintf("REFSTATS overlap %d, literals %d, short %d, long %d, vlong %d\n",totalRefStats.overlap,totalRefStats.countLiterals,totalRefStats.countShort,totalRefStats.countLong,totalRefStats.countVLong);
					}
                }

                if (OPTchop)
                    returncode = (int) chunksizemax;

bigadderror:
				errorcount += gerrorcount;
				gerrorcount = 0;

				if (errorcount && OPTremoveonerror)
				{
					if (OPTverbose>=4)
						gprintf("%s removed\n",bigfileout);
                    remove(bigfileout);
				}
            }
#endif // !EXTRACTONLY
        }
    } while (more>0);

bigerror:

    totaltime = SPIT_gettime()-totaltime;

	errorcount += gerrorcount;
	gerrorcount = 0;

	returncode = errorcount;
    if (OPTreturn[0])
    {
//      EXPR *expr = EXPR_open();

        RANGER *r = RANGER_open(expr,OPTreturn,0.0,1000000.0);  // min/max mostly ignored

        EXPR_set(expr,"errorcount",   errorcount);
        EXPR_set(expr,"TOOL_VERSION", TOOL_VERSION);
        EXPR_set(expr,"SPIT_VERSION", SPIT_VERSION);

        EXPR_set(expr,"chunksizemax", (double) chunksizemax);
        EXPR_set(expr,"chunksizemin", (double) chunksizemin);
        EXPR_set(expr,"namesizemax",  (double) namesizemax);
        EXPR_set(expr,"namesizemin",  (double) namesizemin);
		EXPR_set(expr,"filecount",    filecount);
        EXPR_set(expr,"totalsize",    (double) totalsize);
        EXPR_set(expr,"totaloriginalsize",    (double) totaloriginalsize);
        EXPR_set(expr,"headersize",   headersize);
        EXPR_set(expr,"bigtype",      OPTbigsubtype);
        EXPR_set(expr,"align",        OPTalign);
        EXPR_set(expr,"time",         totaltime);
        EXPR_set(expr,"venom",        infovenom);
        EXPR_set(expr,"overlap",      totalRefStats.overlap);

        returncode = (int) RANGER_closest(r, (double) returncode, RANGER_ROUND_CLOSEST);

//        EXPR_close(expr);
        RANGER_close(r);
    }

//    if (OPTverbose==1)
//        puts("");

    if (OPTtime)
    {
#if WANTREF || WANTZIP || WANTLZMA
        if (OPTpack)
        {
            if (encodetime>=60000.0)
                gprintf("TIME " TOOL_NAME " encodetime %.2f ms (%.2f seconds) (%.2f minutes)\n",encodetime,encodetime/1000.0,encodetime/60000.0);
            else if (encodetime>=1000.0)
                gprintf("TIME " TOOL_NAME " encodetime %.2f ms (%.2f seconds)\n",encodetime,encodetime/1000.0);
            else
                gprintf("TIME " TOOL_NAME " encodetime %.2f ms\n",encodetime);
        }
#endif
        if (totaltime>=3600000.0)
            printf("TIME " TOOL_NAME " %.2f ms (%.2f seconds) (%.2f minutes) (%.2f hours) ",totaltime,totaltime/1000.0,totaltime/60000.0,totaltime/3600000.0);
        else if (totaltime>=60000.0)
            printf("TIME " TOOL_NAME " %.2f ms (%.2f seconds) (%.2f minutes) ",totaltime,totaltime/1000.0,totaltime/60000.0);
        else if (totaltime>=1000.0)
            printf("TIME " TOOL_NAME " %.2f ms (%.2f seconds) ",totaltime,totaltime/1000.0);
        else
            printf("TIME " TOOL_NAME " %.2f ms ",totaltime);
    }

    EXPR_close(expr);

    SPIT_restore();

#if !defined(GLUMPLIB)
    if (galloccount)
    {
        if (OPTverbose)
            gprintf("galloccount = %i\n", galloccount);
        ++errorcount;
    }
    if (!OPTreturn[0])
		returncode = errorcount;
#endif
    totaltime = SPIT_gettime()-totaltime;

    return returncode;
}


#if defined(GLUMPLIB)

extern int OPTlibnohardspace;

int glump(const char *commandline)
{
    int argc=1;
    bool moreargs=true;

    const char *cmd=commandline;
    do
    {
        char buf[SPITMAXLINE];
        cmd = SPIT_getfield(buf,cmd,0,1);
        if (buf[0])
        {
            ++argc;
            cmd = SPIT_nextfield(cmd);
        }
        else
            moreargs = false;
    } while (moreargs);

    char **argv = (char **)galloc(sizeof(char *)*argc);
    memset(argv,0,sizeof(char *)*argc);

    argv[0] = "glumplib";
    argc = 1;
    cmd=commandline;
    moreargs=true;
    do
    {
        char buf[SPITMAXLINE];
        cmd = SPIT_getfield(buf,cmd,0,1);
        if (buf[0])
        {
            argv[argc] = (char *) galloc(strlen(buf)+1);
            strcpy(argv[argc],buf);

            ++argc;
            cmd = SPIT_nextfield(cmd);
        }
        else
            moreargs = false;
    } while (moreargs);

    OPTlibnohardspace = 1;             // tell it not to use hardspaces since I have only one argv with all options in it.
    int returncode = glumpmain(argc,argv);

    if (argv)
    {
        for (int i=1; i<argc; ++i)
            gfree(argv[i]);
        gfree(argv);
    }

    if (galloccount)
    {
        if (OPTverbose)
            gprintf("galloccount = %i\n", galloccount);
        ++returncode;
    }

    return returncode;
}
#else

int main(int argc,char **argv)
{
    return(glumpmain(argc, argv));
}

#endif
