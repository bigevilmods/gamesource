// (c) Electronic Arts. All Rights Reserved.

#if defined(_MSC_VER) && _MSC_VER>=1400
#define ENABLE_OMP 1
#include <omp.h>
#endif

#define WANTBZIP2 1

const char *HUFF_history[] =
{
"------------------------------------------------------------------",
"                                                                  ",
"                     HUFF ME Compressor v6.50                     ",
"                                                                  ",
"                     by FrANK G. Barchard, EAC                    ",
"                                                                  ",
"                    Main program - Jun 29, 2006                   ",
"                                                                  ",
"------------------------------------------------------------------",
"                                                                  ",
"  See huff.doc for pack types                                     ",
"                                                                  ",
"------------------------------------------------------------------",
"                                                                  ",
" Version Date   SE History:                                       ",
" ------- ----   -- --------                                       ",
"    1.20<890224 FB huff packing w/ implied pattern header         ",
"    1.30 890224 FB non-redundent header                           ",
" RL 1.40 890224 FB delta packed header, 20 bit lengths,RL         ",
"    1.50 890226 FB implied repeat clues                           ",
"    1.60 890303 FB multiple files, fast sort                      ",
"    1.62 890310 FB 256 chars, run length inhibiting               ",
"    1.63 890317 FB option to inhibit RL packing                   ",
"    1.70 890502 FB atari st (megamax) compatible                  ",
"    1.71 890502 FB terse help                                     ",
"    1.72 890504 FB atari debugging info                           ",
"    1.73 890504 FB ST dependent code                              ",
"    1.80 890505 FB simple header, delta clues                     ",
"    1.82 890509 FB split clues                                    ",
"    1.90 890511 FB malloc buffer                                  ",
"    2.00 890511 FB combined headers                               ",
"    2.10 890707 FB fix inhibit bug                                ",
"    2.20 890711 FB long files/chainsaw trimming                   ",
"    2.30 890714 FB Lattice/GS conditional defs                    ",
"    2.40 890716 FB long repeats                                   ",
"    2.50 890814 FB inhibit huffman (runlength only)               ",
"    2.51 890815 FB laser C globals prefixes                       ",
"    2.52 890815 FB wildcard name (join,extensions)                ",
"    2.60 890815 FB modular huff for Mac integration               ",
"    2.70 890817 FB interlace                                      ",
" GN 2.80 890818 FB general numerics                               ",
"    2.82 890818 FB amiga SQFREE                                   ",
"    2.90 890822 FB locals/registers/global args                   ",
"    3.00 890823 FB list joining                                   ",
"    3.10 890823 FB parm passing                                   ",
"    3.20 890824 FB batch stats                                    ",
"    3.30 890828 FB no clue on first byte                          ",
"    3.40 890829 FB chainsaw trim branches                         ",
"    3.50 890927 FB pack type lookup/destination dir               ",
" HS 3.60 890928 FB explicit EOF byte/simplified header            ",
"    3.61 890928 FB allow same dest name as source name            ",
"    3.63 890928 FB btree                                          ",
"    3.66 890928 FB signature added to pack type                   ",
"    3.70 891002 FB force clues to be 8 bits codes                 ",
"    3.80 891003 FB extract                                        ",
"    3.86 891003 FB 16 bit clue bytes allowed again                ",
"    3.90 891009 FB explicit EOF byte for btree                    ",
"    3.98 891012 FB additive checksum                              ",
"    4.00 891012 FB delta a file                                   ",
"    4.10 891031 FB unit files (joined)                            ",
"    4.13 891124 FB 80 column source code                          ",
"    4.20 891126 FB unhuff include file                            ",
"    4.30 891127 FB faster btree, 16 bit ints                      ",
"    4.40 891127 FB multinode btree join/defaults file             ",
"    4.50 891206 FB command line expansion                         ",
"    4.51 891207 FB unit file loader/extracter                     ",
"    4.52 891207 FB view unit files                                ",
"    4.60 891219 FB preserved unpack len                           ",
"    4.70 900104 FB view profile                                   ",
"    4.80 900118 FB new unpack dispatcher format                   ",
"    4.90 900214 FB assembler pack module                          ",
"    5.00 900220 FB command line parsing                           ",
"    5.10 900402 FB pack code linked/latenight units               ",
"    5.20 900427 FB hex dump, option parser                        ",
"    5.30 900507 FB 'pure' huff (no global/static variables)       ",
" XE 5.40 901201 FB SPIT, cross machine portability                ",
"    5.41 901220 FB Spit v1.22/SPITsargv,SPITdargv list format     ",
"    5.42 910104 FB Amiga RL bug fixes                             ",
"    5.43 910205 FB Load Adjust byte part of standard header       ",
"    5.44 910401 FB two column usage/auto opts environment var     ",
"    5.46 910404 FB btree bug fix (bestsize=0)                     ",
"    5.47 910423 FB unhuff module                                  ",
"    5.48 910522 FB btree fixed for ibm/c unhuff btree for ibm     ",
"    5.49 910529 FB aztec style options list                       ",
"    5.50 910612 FB runpacki.c include module                      ",
"    5.51 910618 FB gn5 n0(n+2)1                                   ",
"    5.52 910621 FB deltai.c include module                        ",
"    5.53 910625 FB extract option/link module                     ",
"    5.54 910627 FB delta huff                                     ",
"    5.55 910628 FB print packing type as packing/unpacking        ",
"    5.56 910703 FB view packed files                              ",
"    5.57 910708 FB view lumpy2/unpack dh/autoselect packtype      ",
"    5.58 910714 FB c0fb lumpy file header id                      ",
"    5.59 910714 FB verify unpacked length (self diagnosis)        ",
"    5.60 910715 FB unhuff EOF 16bit clue bug fix/extract units    ",
"    5.61 910716 FB lumpy simple filenames/kill filenames/free file",
"    5.62 910723 FB spit lumpy                                     ",
"    5.63 910802 FB ibm SQBlockMove/unknown unpack size of huff    ",
"    5.64 910807 FB lumpy single&large files,huff view lumpy files ",
"    5.65 910815 FB batch option for lumpy                         ",
"    5.66 910826 FB extract nameless lumpy files/lumpy&not wildcard",
"    5.68 910826 FB type mask/disable storing as option            ",
"    5.69 911002 FB extract single unit                            ",
"    5.70 911007 FB unlump/unhuff tiny model on ibm                ",
"    5.71 911009 FB expression for bestpack decission              ",
"    5.72 911021 FB var for overlap to prevent overflows           ",
"    5.73 911022 FB zero suppress btree/lumpy fileerror handling   ",
"    5.74 911023 FB suppress 0..31 for -bz pack/inverse node assign",
"    5.75 911024 FB lumpy naming using spit rules                  ",
"    5.76 911103 FB storeid FB3 format (6bfb)                      ",
"    5.77 911104 FB ibm -o3 shorthand/-o0 all                      ",
"    5.78 911106 FB rl inhibit removed/-o tg16 size/68k nonoverlap ",
"    5.79 911113 FB lumpy groups/unhufflen                         ",
"    5.80 911122 FB spitopts removed from azm.h                    ",
"    5.81 911125 FB spit 1.90/azm expression cleanup               ",
"    5.82 911125 FB more memory efficient bestpack                 ",
"    5.83 911201 FB 1 item trees in huff                           ",
"    5.84 911203 FB chainsaw hanging bug fixed (chainsaw IV)       ",
"    5.85 911213 FB acceleration delta huff/lumpy view single lumps",
"    5.87 911217 FB -t type letters (nxurwbzqsad)                  ",
"    5.88 911226 FB unpack runlength packed files (C and 68000)    ",
"    5.89 911227 FB runlength as part of bestpack                  ",
"    5.90 911231 FB fb4 brief format headers:affects all formats   ",
"    5.91 920119 FB writebits moved to huffpack for speed          ",
"    5.92 920203 FB better type checking for composite packing     ",
"    5.93 920326 FB updated spit to insert q                       ",
"    5.94 920501 FB spit 2.0 interface to allow MS overlay for spit",
"    5.95 920611 FB btree explicite EOF using [clue][0]            ",
"    5.96 920730 FB btree use words for source in adjcount         ",
"    5.97 920914 FB lumpy upgraded to spit 2.0                     ",
"    5.98 920922 FB unhuff 2.0 PC library format huffman(PC only)  ",
"    5.99 920922 FB -tx to try all pack types                      ",
"    6.00 920923 FB -v context sensitive                           ",
"    6.01 921004 FB huffman/btree (stuff) as auto packtype         ",
"    6.02 921011 FB huff header codes sorted                       ",
"    6.03 921105 FB copyright -c option to write copyright at start",
"    6.04 921115 FB view and extract copyright files               ",
"    6.05 921221 FB defaults for IBM include nopac,huff & dhuff    ",
"    6.06 930528 FB unpack delta and acceleration huff             ",
"    6.07 930615 FB pack/unpack reference pack (refpack)           ",
"    6.08 930618 FB btreerefhuff pack                              ",
"    6.09 930621 FB huff warn >1024 overlap, ref w/huffnstuff lit  ",
"    6.10 930812 FB 68000 assembler D5 and A5 freed for MPW/Sega   ",
"    6.11 930921 FB btree clear count optimized                    ",
"    6.12 931015 FB refpack by default (fast enough now)           ",
"    6.13 931022 FB refpack EXtreme (optimized) updated            ",
"    6.14 940112 FB fast mac fileio                                ",
"    6.15 940708 FB huff best module for use in other tools        ",
"    6.16 941227 FB lumpy use ref codex                            ",
"    6.17 950102 FB simple terminology/lumpy compression off       ",
" CX 6.18 950306 FB refpack codex                                  ",
"    6.19 950308 FB compression names changed to match uncompress  ",
"    6.20 950620 FB lzh codex                                      ",
"    6.21 960725 FB ansi protos                                    ",
"    6.22 961105 FB codex *opt api change                          ",
"    6.23 970117 FB codex for huf/btr/ref decode                   ",
"    6.24 990809 LC FILE->GSTREAM; removed runpack and delta opts  ",
"    6.25 990810 LC general housecleaning stuff; added bakname     ",
"    6.26 000131 FB use exe name unhuff to default -x              ",
" ME 6.27 000331 FB view return error count for uncompressed files ",
"    6.28 000424 FB -time                                          ",
"    6.29 000515 FB refpack dont pack last 8 bytes                 ",
"    6.30 000627 FB lzss codec added (-tl)                         ",
"    6.31 000701 FB bzip2 codec added (-tw)                        ",
"    6.32 000705 FB hide obsolete options doc                      ",
"    6.33 001022 FB leapcodes renamed for sun linker               ",
"    6.34 001031 FB view percent rounding consistant               ",
"    6.35 001214 FB -time for encode too                           ",
"    6.36 010426 FB 32 bit size fields (fb6 header)                ",
"    6.37 010511 FB -time total time                               ",
"    6.38 020712 JF all codecs working again                       ",
"    6.39 020716 FB percentage calc overflow fix                   ",
"    6.40 020717 FB SPIT_tempname                                  ",
"    6.41 020723 FB undo backup if not huffed                      ",
"    6.42 020814 FB -tz zip added, -o & -c removed                 ",
"    6.43 030606 FB -tw bzip2 removed                              ",
"    6.44 031026 FB removed backup logic (buggy/not so useful)     ",
"    6.45 040613 FB extract uncompressed files                     ",
" LT 6.46 041106 FB verbosity fixes                                ",
"    6.47 041222 FB more verbosity fixes                           ",
"    6.48 060629 FB -tn do a copy 32k at time for large files      ",
"    6.50 080704 FB multicore omp                                  ",
"                                                                  ",
"------------------------------------------------------------------",
"                                                                  ",
"  HUFF VERSIONS 020814                                            ",
"                                                                  ",
"    Unpack     Pack      Latest   Library   Most Common:          ",
"    ------     ----      ------   -------   -----------           ",
"    PC         PC/Linux  6.46     6.45      6.45                  ",
"    PS2        PC/Linux  6.42     6.41      6.45                  ",
"    XBox       PC/Linux  6.42     6.41      6.45                  ",
"    Gamecube   PC/Linux  6.42     6.41      6.45                  ",
"    OSX        OSX       6.42     6.41      6.41                  ",
"    Solaris    Solaris   6.41     6.41      6.41                  ",
"    SGI        SGI       6.41     6.41      6.41                  ",
"    PSX        PC/SGI    6.27     6.27      6.20                  ",
"    N64        PC/SGI    6.27     6.27      6.26                  ",
"    Saturn     PC        6.21     6.21      6.20                  ",
"    3DO        Mac       6.15     6.13      6.13                  ",
"    Macintosh  Macintosh 6.15     4.31      5.68/4.31/3.42        ",
"    Sega       Mac/PC    6.13     6.10      5.48                  ",
"    SNES       Mac/PC    6.13     5.48      5.48                  ",
"    TG16       PC/Mac    6.13     5.48      5.48                  ",
"    NES        PC/Mac    6.13     5.31      5.31                  ",
"    Amiga      Amiga     5.76     5.57      5.57/5.41/4.50/3.50   ",
"    Atari ST   Atari ST  5.52     5.52      5.45                  ",
"    Apple][gs  Apple][gs 4.20     2.27      2.27                  ",
"    C64        Amiga     1.40     1.31      1.31                  ",
"                                                                  ",
"  HUFF CLASSES                                                    ",
"                                                                  ",
"  RL runlength                                                    ",
"     integrated run length that performs better in both           ",
"     reduction and unpacking speed.                               ",
"                                                                  ",
"  GN general numerics                                             ",
"     a numerical encoding format that packs and                   ",
"     simplifies the header and removes numerical field            ",
"     size restictions and machine storage dependencies            ",
"                                                                  ",
"  HS high speed                                                   ",
"     enhancements to compliment the high speed unhuff             ",
"     and btree algorithms                                         ",
"                                                                  ",
"  XE cross environment/development                                ",
"     machine, compiler, and application portability of huff code  ",
"     and data                                                     ",
"                                                                  ",
"  CX codex API                                                    ",
"     uses codex API for compression and decompression modules     ",
"                                                                  ",
"  ME Millenium Edition scripting                                  ",
"     returns error count if you view uncompressed files           ",
"     for better scripting                                         ",
"                                                                  ",
"  LT Loadtimes                                                    ",
"     huff on bigfiles for solid compression                       ",
"     overlap to save memory                                       ",
"                                                                  ",
"------------------------------------------------------------------",
"\0"
};

#include "spit/spit.h"
#include "spit/stdgimex.h"

#include "huff.h"
#include "huffu.h"
#include "huffview.h"
#include "huffbest.h"

#if defined(_MSC_VER)
#pragma warning(push,1)
#include <windows.h>
#pragma warning(pop)
#endif

#define TOOL_NAME "huff"
const char TextVersion[]="6.50 (Viv) 2008.7.10";
#define TOOL_VERSION 650
#define TOOL_NAME "huff"

static double TIMEload;
static double TIMEcodec;
static double TIMEsave;

/****************************************************************/
/*  Defines                                                     */
/****************************************************************/

#define CMDPAC      1
#define CMDUNPAC    2
#define CMDVIEW     3

#define MAXHEADER       ((int)128)

#define TYPESALL ((1<<PACKTYPES)-1)
#define TYPESDEFAULT (1<<REFPAC)

static char ExtrList[80];
static char BestExp[PACKTYPES][128];

int  OPTpacktype;
int  OPTcmask;

static int  OPTcmd;
static int  OPTview;
static int  OPTbundle;

/****************************************************************/
/*  the Time                                                    */
/*  - returns the current time                                  */
/****************************************************************/

static int TheTime(void)
{
    return((int)clock());
}

/****************************************************************/
/*  UnPack a File                                               */
/****************************************************************/

static void unpackbest(SPITMEM *infile,char *outname)
{
    int uflen;
    int starttime;
    int timeittook;
    int  timesunpacked;
    SPITMEM outfile;
    const unsigned char *s;
    char statbuf[SPITMAXLINE];
    unsigned char workbuf[1024];

    s = (const unsigned char *)(infile->ptr);

    uflen = SQunhuff(s, workbuf, workbuf, SPITNO);
    if (uflen>0)
    {
        outfile.len = uflen+16;
        outfile.ptr = static_cast<char *>(malloc(outfile.len));

        if (outfile.ptr != NULL)
        {
            outfile.len = uflen;
            if (OPTverbose>=2)
            {
                switch (TypeOFile(s))
                {
                    case NOPAC:     printf("unStoring, ");              break;
                    case BTREEPAC:  printf("unBTreeing, ");             break;
                    case HUFFPAC:   printf("unHuffing, ");              break;
                    case REFPAC:    printf("unRefpacking, ");           break;
#if WANTBZIP2
                    case BZ2PAC:    printf("unBZip2ing, ");             break;
#endif
                    case ZIPPAC:    printf("unZiping, ");               break;
                }
                fflush(stdout);
            }

            /* huff extract self test length */

            if (uflen>2)
            {
                *(outfile.ptr+uflen-1) = (char) (unsigned char) 0xfb;
                *(outfile.ptr+uflen) =   (char) (unsigned char) 0xfb;
                *(outfile.ptr+uflen+1) = (char) (unsigned char) 0xfb;
            }

            starttime = TheTime();
            timeittook = 0L;
            timesunpacked = 0;
            while (timeittook<100)
            {
                TIMEcodec = SPIT_gettime();
                outfile.len = SQunhuff(s, (unsigned char *)outfile.ptr, workbuf, SPITYES);
                TIMEcodec = SPIT_gettime()-TIMEcodec;
                timeittook = TheTime()-starttime;
                ++timesunpacked;
                if (OPTverbose<4)
                    break;
            }

            if (uflen>2)
            {
                if (OPTverbose>=2)
                {
                    if ((*(outfile.ptr+uflen) != (char) (unsigned char) 0xfb) || (*(outfile.ptr+uflen+1) != (char) (unsigned char) 0xfb))
                    {
                        if (*(outfile.ptr+uflen+1) != (char) (unsigned char) 0xfb)
                            printf("WARNING unhuff extracted too many bytes\n");
                        else
                            printf("WARNING unhuff extracted 1 too many bytes\n");
                    }
                    if (*(outfile.ptr+uflen-1) == (char) (unsigned char)  0xfb)
                    {
                        if (OPTverbose>=2)
                            printf("WARNING unhuff may have extracted too few bytes\n");
                    }
                }
            }

            TIMEsave = SPIT_gettime();

            SPIT_savefile(outname, outfile.ptr, outfile.len);

            TIMEsave = SPIT_gettime()-TIMEsave;

            free(outfile.ptr);
            outfile.ptr = NULL;

            /* print some stats */

            if (OPTverbose>=3)
            {
                SPIT_percentstr(statbuf, infile->len, outfile.len);
                if (OPTverbose>=4)
                {
                    printf("%ld->%ld (%s) %ld bps\n", infile->len, outfile.len, statbuf,outfile.len*CLOCKS_PER_SEC*timesunpacked/timeittook);
                }
                else
                {
                    printf("%ld->%ld (%s)\n", infile->len, outfile.len, statbuf);
                }

                if (OPTtime)
                    printf(" Time load %.2fms, decode %.2fms, save %.2fms\n",TIMEload,TIMEcodec,TIMEsave);

                fflush(stdout);
            }
        }
        else
        {
            if (OPTverbose>=1)
                printf("ERROR could not allocate unpack buffer\n");
        }
    }
    else
    {
        if (OPTverbose>=3)
            printf("WARNING file is not compressed.  Saving file as is.\n");
        SPIT_savefile(outname, infile->ptr, infile->len);


        /* print some stats */

        if (OPTverbose>=3)
        {
            SPIT_percentstr(statbuf, infile->len, infile->len);
            printf("%ld->%ld (%s)\n", infile->len, infile->len, statbuf);

            fflush(stdout);
        }
    }
}

/*
;
; ABSTRACT
;
; huff - general purpose file packer
;
; SUMMARY
;
; Huff packs and bundles files
;
; EXPRESSION VARIABLES (exp):
;    inlen     length of input file
;    bestlen   length of best packed file so far
;    besttype  type of best packed file so far
;    outlen    length of file packed with current pack type
;    l         alias for outlen
;    a         adjust value (huff pack types only)
;
; huff -options [files]
;    @file     response file, containing a list of options and files
;    =file     specify a output file ie source=dest
;    -nologo   disable banner
;    -t<n>     types of packing to try bitmask
;    -tx       try all methods
;    -tn       try storing
;    -tb       try btree
;    -tq       try huff
;    -tj       try jref
;    -tz       try zip
;    -v        view a file
;    -v0       no console output
;    -v1       prints source file name and errors
;    -v2       status line for each file (default)
;    -v3       misc additional info
;    -w0       don't output file
;    -w1       output to a file (default)
;    -x        unpack files

;
; DESCRIPTION
;
; Huff compresses data with lossless compression methods.  By
; several are tried and the best choosen.  The most common are
; refpack, huffman and btree methods.  You may also specify the
; type (or types) explicitety with the -t option.  unpack
; code is available in the eac library (unpack()/loadpack()) or
; as a codex module.
;
; RETURNS
;
; number of errors
;
; SEE ALSO
;
; codex, unpack, lumpy, Spit
;
; SAMPLE OUTPUT
;
; >huff *.afnt
;
;   Huff XE v5.56 (view) (c) 1992 FrANK Barchard
;   benc.afnt: Reading, Squeezing, Writing, 1964->1156 (41.2%)
;   garnet9.afnt: Reading, Squeezing, Writing, 3876->2612 (32.7%)
;   teeny3.afnt: Reading, Squeezing, Writing, 984->446 (54.7%)
;   teeny4.afnt: Reading, Squeezing, Writing, 1036->493 (52.5%)
;   teeny5.afnt: Reading, Squeezing, Writing, 1088->539 (50.5%)
;   wittle6.afnt: Reading, Squeezing, Writing, 1268->769 (39.4%)
;
; END ABSTRACT
;
*/

/****************************************************************/
/*  SPIT Routines                                               */
/****************************************************************/

static void HUFF_init(void)
{
    OPTcmd = 0;
    OPTview = 0;
    OPTpacktype = BESTPAC;
    OPTbundle = SPITNO;
    OPTverbose = 2;

    ExtrList[0] = '\0';

    OPTcmask = TYPESDEFAULT; /* nopac, btree, huff, dhuff */
}

static char *optionhelp[]=
{
    "EXPRESSION VARIABLES (exp):\n"
    "    inlen     length of input file\n"
    "    bestlen   length of best packed file so far\n"
    "    besttype  type of best packed file so far\n"
    "    outlen    length of file packed with current pack type\n"
    "    l         alias for outlen\n"
    "    a         adjust value (huff pack types only)\n",
    "\n",
    "    @file     response file, containing a list of options and files\n",
    "    =file     specify a output file ie source=dest\n",
    "    -nologo   disable banner\n",
    "    -t<n>     types of packing to try bitmask\n"
    "    -tx       try all methods\n"
    "    -tn       try storing\n"
    "    -tb       try btree\n"
    "    -tq       try huff\n"
    "    -tj       try jref\n"
#if WANTBZIP2
    "    -tw       try bzip2\n",
#endif
    "    -tz       try zip\n",
    "    -v        view a file\n"
    "    -v0       no console output\n"
    "    -v1       prints source file name and errors\n"
    "    -v2       status line for each file (default)\n"
    "    -v3       misc additional info\n",
    "    -w0       don't output file\n"
    "    -w1       output to a file (default)\n",
    "    -x        unpack files\n"
    "\n",
    0,
};


static void HUFF_usage(const char *prgname, const char *envname, const char *urlname, int utype)
{
    char **hlp;

    if ((OPTverbose>=2 && utype==2) || OPTverbose>=4)
        printf("Huff LT v%s by FrANK Barchard, EAC\n",TextVersion);
    if (utype)
        printf("USAGE: %s -options [files]\n", prgname);

    switch (utype)
    {
        case 2:
            puts("    -x extract files");
            puts("    -t pack type select          -v verbose");
            puts("    -e analysis expression       -w write file");
            puts("     @ response file              = output file");
          printf("type '%s -h' for more help\n", prgname);
          printf("type '%s -help' for web page (%s)\n", prgname, urlname);
            break;

        case 3:
          printf("VERSION: %s v%s, Spit v%.2f\n",SPIT_MACHINE,TextVersion,(float)SPIT_VERSION/100.0f);
            puts("SUMMARY: Huff packs and bundles files");
          printf("         SPIT environment variable is %s\n",envname);
            puts("");

            hlp = optionhelp;
            while (*hlp)
            {
                printf("%s",*hlp);
                ++hlp;
            }
            break;

         case 4:
            puts("");
            {
                hlp = optionhelp;
                while (*hlp)
                {
                    if (SPIT_strstr(*hlp,envname))
                        printf("%s",*hlp);
                    ++hlp;
                }
                puts("");
            }
            break;

        default:
            break;
    }
}

static char *SPIT_getmaskopt(char *optptr,int *optvalptr,char *optname,char *ptypes)
{
    char *t;
    char optchar;
    int  tmask;
    int  cmask;
    int  maxmask;

    cmask = *optvalptr;
    maxmask = (1<<(strlen(ptypes)))-1;

    optchar = *optptr;
    if (optchar==(char)'+' || optchar==(char)'-')
        ++optptr;
    t = strchr(ptypes,(int)(*optptr|(char)' '));
    if (t)
    {
        tmask = 0;
        while (t)
        {
            ++optptr;
            tmask |= 1<<(t-ptypes);
            t = strchr(ptypes,(int)(*optptr|(char)' '));
        }
        if (optchar==(char)'+')
            cmask |= tmask;
        else if (optchar==(char)'-')
            cmask &= ~tmask;
        else
            cmask = tmask;
    }
    else
    {
        if (*optptr=='x')
        {
            cmask = -1;
            ++optptr;
        }
        else
            optptr = SPIT_getopt( &cmask,optptr, optname, 0, TYPESALL);
    }
    *optvalptr = cmask;
    return(optptr);
}

static char *HUFF_opt(const char *optptr)
{
    char *s;
    int  optstate;

    switch (*optptr|(char)' ')
    {
        case 't':
            if (!strnicmp(optptr,"time",4))
            {
                return(SPIT_getopt( &OPTtime,optptr+4, "enable timing diagnostics", 0, 1));
            }

            if (!OPTcmd) OPTcmd = CMDPAC;
            return(SPIT_getmaskopt((char *) optptr+1, &OPTcmask,  "packtype mask", "nbqjwz"));

        case 'e':
            optstate = 0;
            s = SPIT_getopt( &optstate,optptr+1, "expression number", 0, PACKTYPES-1);
            s = SPIT_nextfield(s);
            return(SPIT_getstropt( BestExp[optstate],s, "expression string", "", SPITNO, 128));

        case 'v':
            if (!(isdigit(*(optptr+1))))    /* -v<n> verbose */
                OPTview = 1;                /* -v view */
            return((char *) 0);

        case 'x':
            optstate = 0;
            s = SPIT_getopt( &optstate,optptr+1, "extract", 0, 2);
            OPTcmd = CMDUNPAC;

            if (!stricmp(OPTextrule,"-q"))      /* only if user hasn't changed ext */
                strcpy(OPTextrule,"-z");

            s = SPIT_getstropt( ExtrList,s, "extract name", ",", SPITYES, 80);
            return(s);

        default: ;
    }
    return((char *) 0);
}


bool HUFF_copyfile(const char *filenamein, const char *filenameout)
{
#if defined(_MSC_VER_ALTERNATIVE)
    // as of june 2006, copyfile takes 46 vs gimex gread/write takes 39
    int Err = CopyFile(filenamein, filenameout, FALSE); // Copy file to system directory, overwrite file if already present
    return Err==0;
#else
    size_t blockchop = 32768;
    void *inbuf = galloc(blockchop);
    GSTREAM *gin = gopen(filenamein);
    GSTREAM *gout = gwopen(filenameout);
    GPOS fsizeleft=glen(gin);
    bool ok = true;

    while (fsizeleft && ok)
    {
        size_t nextsize=(size_t) gmin(fsizeleft,blockchop);
        ok = gread(gin,inbuf,nextsize)==nextsize;
        if (ok)
        {
            ok = gwrite(gout,inbuf,nextsize)==nextsize;
        }
        if (ok)
            fsizeleft -= nextsize;
    }

    gfree(inbuf);
    gclose(gin);
    gclose(gout);
    return fsizeleft==0;
#endif
}


int main(int argc, char **argv)
{
    int more;
    int i;
    int fileerrors=0;
    SPITMEM infile;
    SPITMEM outfile;
    char statbuf[SPITMAXLINE];
    double totaltime;

#if ENABLE_OMP
    if (getenv("OMPDISABLE"))   // old way
        omp_set_num_threads(1);
    char *omp = getenv("OMPNUMTHREADS");
    if (omp)
    {
        int ompnum = atoi(omp);
        omp_set_num_threads(ompnum);
    }
#endif

    totaltime = SPIT_gettime();
    SPIT_init(HUFF_init, HUFF_usage, HUFF_opt, HUFF_history);

    do
    {
        more = SPIT_parse(argc, argv, TOOL_NAME, TOOL_VERSION);

        if (!OPTcmd && argc>1 && SPITargnum>=1)
        {
            SPIT_convertname(statbuf,argv[0]);
            SPIT_changeext(statbuf,statbuf,"=");

            if (!stricmp(statbuf,"unhuff"))
                OPTcmd = CMDUNPAC;
        }
        if (OPTview && !OPTcmd)
        {
            if (SPITargnum>=1)
            {
                OPTcmd = CMDVIEW;
            }
        }

        if (!SPIThelpgiven)
        {
            switch (OPTcmd)
            {
                /* pack files */

                case 0:
                case CMDPAC:

                    for (i=0; i<SPITargnum; ++i)
                    {
                        if (OPTcmask==(1<<NOPAC))
                        {
                            TIMEload = SPIT_gettime();
                            HUFF_copyfile(SPITsargv[i],SPITdargv[i]);
                            TIMEload = SPIT_gettime()-TIMEload;
                            if (OPTverbose>=2 && OPTtime)
                                printf(" Time copy %.2fms\n",TIMEload);
                        }
                        else
                        {
                            TIMEload = SPIT_gettime();

                            if (SPIT_loadfilev(SPITsargv[i], &infile.ptr, &infile.len, OPTverbose))
                            {
                                TIMEload = SPIT_gettime()-TIMEload;

                                outfile.len = static_cast<size_t>(infile.len*1.2) + SLOPAGE;
                                outfile.ptr = static_cast<char *>(malloc(outfile.len));

                                if (outfile.ptr != NULL)
                                {
                                    TIMEcodec = SPIT_gettime();
                                    packbest(&infile, &outfile,OPTcmask);
                                    TIMEcodec = SPIT_gettime()-TIMEcodec;

                                    TIMEsave  = SPIT_gettime();

                                    if (OPTwrite)
                                        SPIT_savefile(SPITdargv[i], outfile.ptr, outfile.len);

                                    TIMEsave  = SPIT_gettime()-TIMEsave;

                                    free(outfile.ptr);
                                    outfile.ptr = NULL;

                                    /* print some stats */

                                    if (OPTverbose>=2)
                                    {
                                        SPIT_percentstr(statbuf, infile.len, outfile.len);
                                        printf("%ld->%ld (%s)\n", infile.len, outfile.len, statbuf);

                                        fflush(stdout);
                                    }
                                    if (OPTverbose>=2 && OPTtime)
                                        printf(" Time load %.2fms, encode %.2fms, save %.2fms\n",TIMEload,TIMEcodec,TIMEsave);
                                }
                                else
                                {
                                    if (OPTverbose>=1)
                                        printf("ERROR could not allocate pack buffer\n");
                                    ++fileerrors;
                                }
                                free(infile.ptr);
                                infile.ptr = NULL;
                            }
                            else
                                ++fileerrors;
                        }
                    }
                    break;

                /* unpack files */

                case CMDUNPAC:

                    for (i=0; i<SPITargnum; ++i)
                    {
                        TIMEload = SPIT_gettime();

                        if (SPIT_loadfilev(SPITsargv[i], &infile.ptr, &infile.len, 0))
                        {
                            TIMEload = SPIT_gettime()-TIMEload;

                            unpackbest(&infile, SPITdargv[i]);

                            free(infile.ptr);
                            infile.ptr = NULL;
                        }
                        else
                            ++fileerrors;
                    }
                    break;

                /* view a file */

                case CMDVIEW:

                    StartViewFile();

                    for (i=0; i<SPITargnum; ++i)
                    {
                        TIMEload = SPIT_gettime();
                        if (SPIT_loadfilev(SPITsargv[i], &infile.ptr, &infile.len, 0))
                        {
                            TIMEload = SPIT_gettime()-TIMEload;

                            /* view single file */

                            if (!ViewFile("", SPITsargv[i], &infile, 0, 0L))
                                ++fileerrors;

                            free(infile.ptr);
                            infile.ptr = NULL;
                        }
                        else
                        {
                            if (OPTverbose>=1)
                                printf("ERROR could not read file <%s>\n", SPITsargv[i]);
                            ++fileerrors;
                        }
                    }

                    StopViewFile();

                    break;
            }
        }
    } while (more>0);

    totaltime = SPIT_gettime()-totaltime;

    if (OPTtime)
    {
        if (totaltime>=60000.0)
            printf("TIME " TOOL_NAME " totaltime %.2f ms (%.2f seconds) (%.2f minutes)\n",totaltime,totaltime/1000.0,totaltime/60000.0);
        else if (totaltime>=1000.0)
            printf("TIME " TOOL_NAME " totaltime %.2f ms (%.2f seconds)\n",totaltime,totaltime/1000.0);
        else
            printf("TIME " TOOL_NAME " totaltime %.2f ms\n",totaltime);
    }


    return(fileerrors);
}

#if defined(_MSC_VER) && !defined(DEBUG)
#pragma warning(push,1)
#include <windows.h>
#pragma warning(pop)

int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpszCmdLine, int cmdShow)
{
    char *argv[2];
    argv[0] = "huff";
    argv[1] = lpszCmdLine;
    return main(2,argv);
}
#endif

