// (c) 2005-2008 Electronic Arts Inc.

#include "spit/spit.h"
#include "spit/stdgimex.h"

#define DEBUGHASH 0

#ifdef __PPU__
#include <stdint.h>
#else
#define uint64_t unsigned long long
#define int64_t long long
#endif

// 1.00 2004.6.04 - Initial version based on makepad
// 1.01 2004.6.07 - -k to kill names
// 1.02 2004.6.08 - -n for nonames (8 bytes, no nil)
// 1.03 2004.9.15 - -h for hash format (work in progress)
// 1.04 2004.9.27 - -v just one line output per file, -hash file format, -nowrite
// 1.05 2004.9.28 - -hash12 remove file size field, add sentinel
// 1.06 2004.10.04 - -hash8, compute number of hash bits required
// 1.07 2004.10.12 - hash add full row so hash can be stamped as a sentinel
// 1.08 2004.12.15 - BigHead support old 24 format
// 1.09 2005.05.24 - 90fb support
// 1.10 2005.06.10 - error check if file does not exist
// 1.11 2006.08.31 - error check if output file does not write (read only)
// 1.12 2007.07.09 - hash20 for 64bit hash (for fifa)
// 1.13 2007.07.16 - unsigned sizes (for nhl)
// 1.14 2008.04.22 - aligned packed sizes for fifa (-palign16).  also -ualign


const char TextVersion[]="1.14 (Viv) 2008.7.10";
#define TOOL_VERSION 114
#define TOOL_NAME "BigHead"

int errorCount=0;

#if defined(__unix__) || defined(__APPLE__) || defined(__BEOS__)
#include <ctype.h>
#define strlwr(__s)   { int __i; for (__i=0; (__s)[__i]; ++__i) (__s)[__i]=(char)tolower((__s)[__i]); }
#endif

#if 0
/*
;
; ABSTRACT
;
; bighead - bigfile tool to extract header from bigfile, munge it, and save it to a seperate output file
@BigHead
;
; SUMMARY
;
; BigHead -options <infile.big> <outfile.bh>
;
;  <infile.big> Name of BigFile to extract header from
;  <outfile.bh> Name of BigHead file to save to (optional)
;  -verbose     switch to enable more verbose output (aka -v)
;  -nowrite     disable write of BigHead file (aka -w0)
;  -killnames   switch to disable names in the output file (aka -k)
;  -nonames     switch to disable names entirely (aka -n)
;  -palign      packed file size alignment (default 1)
;  -hash        switch to hash format (options: 8,12,16,20). (-h is hash16)
;
; END ABSTRACT
;
*/
#endif


/****************************************************************/
/*  Hash support                                                */
/****************************************************************/

static uint64_t *hashList=NULL;
static int hashMaxCount=0;
static int hashCount=0;
static int hashBits=1;

// returns true if a collision is found

static bool HashCollisionCheck(uint64_t hash)
{
    // add to list.  guarantees we'll find it
    if (hashCount>=hashMaxCount || !hashList)
    {
        hashMaxCount += 1024;

        uint64_t *HashNewList = (uint64_t *)galloc(hashMaxCount*sizeof(uint64_t));
        if (hashList)
        {
            memcpy(HashNewList,hashList,hashCount*sizeof(uint64_t));
            gfree((void *)hashList);
        }
        hashList = HashNewList;
    }

    hashList[hashCount++] = hash;

    bool collision;

    do
    {
        int i=0;
        collision=false;
        uint64_t hashBitsmask = ((uint64_t)(0xffffffffffffffffULL) >> (64-hashBits));

        do
        {
#if DEBUGHASH
            printf("hash %2d %016I64x %016I64x %016I64x\n",i,hashBitsmask,hash&hashBitsmask,hashList[i]&hashBitsmask);
#else
            ;
#endif
        } while ((hash&hashBitsmask)!=(hashList[i++]&hashBitsmask));

#if DEBUGHASH
        printf("hash exit %u %u\n",i,hashCount);
#endif

        if (i!=hashCount)   // did not hit end.. collision
        {
            collision = true;
            if (hashBits==64)
                break;
            ++hashBits;
        }
    } while (collision);

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
// NOTE require uint64_t to be 64 bits

static uint64_t hash(const char *str, uint64_t hash = 5381)
{
    const unsigned char *s = (const unsigned char *) str;

	for (;;)
	{
	    uint64_t c = (uint64_t) *s++;
		if (!c) break;
	    hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
	}

    return hash;
}

// end of hash



bool IsOld24(const char *bigBuf)
{
    return ggetm(bigBuf,2)==0xc0fb;
}

static int BigEntrySize(const char *bigBuf)
{
    int sizeOfField;
    if (IsOld24(bigBuf)) sizeOfField = 3;   // old 24
    else if (bigBuf[3]=='F') sizeOfField = 4;   // old 32
    else sizeOfField = bigBuf[3]-'0';           // new 24/32
    if (sizeOfField<0 || sizeOfField>4)
        sizeOfField = 4;
    return sizeOfField;
}


static int BigCount(const char *bigBuf)
{
    int fileCount;
    if (IsOld24(bigBuf)) fileCount = ggetm(bigBuf+4,2); // old 24
    else fileCount = ggetm(bigBuf+8,4);
    return fileCount;
}

static int BigHeaderStart(const char *bigBuf)
{
    int sizeOfHeader;
    if (IsOld24(bigBuf)) sizeOfHeader = 6;  // old 24
    else sizeOfHeader = 16;         // new 24/32
    return sizeOfHeader;
}

unsigned int GetDecompressSize(GSTREAM *gIn,unsigned int foffset,unsigned int fsize)
{
    unsigned int msize=0;
    gseek(gIn,foffset);
    char buf[8];
    gread(gIn,buf,8);
    if (ggetm(buf,2)==0x10fb)
        msize = ggetm(buf+2,3);
    else if (ggetm(buf,2)==0x90fb)
        msize = ggetm(buf+2,4);
    return msize;
}

char * HashNames(int &hsize, const char *inName, const char *inBuf, int optHashNames=16, int optPackAlign=1, int optUnPackAlign=1, bool optVerbose=false)
{
    int fileCount = BigCount(inBuf);
    int sizeOfInField = BigEntrySize(inBuf);
    int sizeOfOutField = optHashNames==8?3:sizeOfInField;
    int sizeOfHash = optHashNames==8?2:(optHashNames==20?8:4);
    int sizeOfEntry = (optHashNames==16||optHashNames==20)?sizeOfOutField*3+sizeOfHash:sizeOfOutField*2+sizeOfHash;
    int newSize = 16+(fileCount+1)*sizeOfEntry; // extra row for foffset differencing and hash sentinel
    char *outBuf = (char *)galloc(newSize);
    memset(outBuf,0,newSize);   // zero out sentinel and everything else just incase we miss any fields
    uint64_t totalMSize = 0;
    uint64_t totalFSize = 0;

    // write new header
    memcpy(outBuf,inBuf,16);
    outBuf[3] = (char)(sizeOfInField+'0');  // 3 or 4
    if (optHashNames==20)       memcpy(outBuf,"ViV",3);    // ViV4 signature for 20 byte format
    else if (optHashNames==12)  memcpy(outBuf,"VIV",3);    // VIV4 signature for 12 byte format
    else if (optHashNames==8)   memcpy(outBuf,"viv3",4);   // viv3 signature for 8 byte format
    else                        memcpy(outBuf,"Viv",3);    // Viv4 signature for 16 byte format
    gputi(outBuf+4,newSize,4);      // update file size
    gputm(outBuf+8,fileCount,4);
    gputm(outBuf+12,newSize,4); // update header size

    HashCollisionInit();

    GSTREAM *gIn = gopen(inName);

    // write TOC
    const char *s = inBuf+BigHeaderStart(inBuf);
    char *d = outBuf+16;
    unsigned int foffset=0;

    for (int i=0; i<fileCount; ++i)
    {
        foffset = ggetm(s+0,sizeOfInField);
        unsigned int fsize = ggetm(s+sizeOfInField,sizeOfInField);
        unsigned int msize = GetDecompressSize(gIn,foffset,fsize);

		if (msize) // if compressed, align file size
		{
			fsize = ((fsize+optPackAlign-1)/optPackAlign)*optPackAlign;
			msize = ((msize+optUnPackAlign-1)/optUnPackAlign)*optUnPackAlign;
		}

        totalMSize += msize;    // gather some metrics
        totalFSize += fsize;

        const char *fname = s+sizeOfInField*2;

        uint64_t fhash = hash(fname);

        if (optHashNames==16)
        {
            gputm(d+0, foffset, sizeOfOutField);                // offset
            gputm(d+sizeOfOutField, fsize, sizeOfOutField);     // fsize
            gputm(d+sizeOfOutField*2, msize, sizeOfOutField);   // msize
            gputm(d+sizeOfOutField*3, (unsigned int)fhash, sizeOfHash);       // hash
        }
        else if (optHashNames==20) // aka 64 bit
        {
            gputm(d+0, foffset, sizeOfOutField);                // offset
            gputm(d+sizeOfOutField, fsize, sizeOfOutField);     // fsize
            gputm(d+sizeOfOutField*2, msize, sizeOfOutField);   // msize
            gputm(d+sizeOfOutField*3+0, (unsigned int)  ((fhash>>16)>>16), 4);  // hash hi
            gputm(d+sizeOfOutField*3+4, (unsigned int) (fhash&0xffffffff), 4);  // hash lo
        }
        else if (optHashNames==8)
        {
            const char *sn = s + sizeOfInField*2+1+strlen(s+sizeOfInField*2);
            unsigned int fnextoffset = ggetm(sn+0,sizeOfInField);
            unsigned int fsize2 = fnextoffset - foffset;
            if (i==fileCount-1)
                fsize2 = (fsize+15)&~15;
            if (fsize2&0xf)
            {
                ++errorCount;
                if (optVerbose)
                {
                    printf("Offset not aligned to 16 %s\n", fname);
                }
            }
            if (fsize2>0xffffff || msize>0xffffff)
            {
                ++errorCount;
                if (optVerbose)
                {
                    printf("Size too large for hash8 format on %s\n", fname);
                }
            }

            gputm(d+0, fsize2&0xfffff0 | (msize>>20)&0x0f, 3);  // fsize
            gputm(d+3, ((msize+15)>>4)&0xffff, 2);              // msize
            gputm(d+5, (unsigned int)fhash, 3);                 // hash
        }
        else
        {
            gputm(d+0, foffset, sizeOfOutField);                // offset
            gputm(d+sizeOfOutField, msize, sizeOfOutField);     // msize
            gputm(d+sizeOfOutField*2, (unsigned int)fhash, sizeOfHash);  // hash
        }

        bool collided = HashCollisionCheck(fhash);
        if (collided)
        {
            ++errorCount;
            if (optVerbose)
            {
                printf("Hashnames collided on %s\n", fname);
            }
        }

        d += sizeOfEntry;
        s += sizeOfInField*2+1+strlen(s+sizeOfInField*2);
        foffset += fsize;
    }
    gputm(d+0, foffset, sizeOfOutField);            // sentinel offset

    gclose(gIn);

    if (optVerbose)
    {
        printf("Hashed %s: %u files, %u bytes (saved %u bytes), hashbits %u, ", inName, fileCount, newSize, hsize-newSize, hashBits);
        if (totalMSize)
            printf("Compressed %" FORMAT64 "u <- %" FORMAT64 "u\n",  totalFSize, totalMSize);
        else
            printf("Uncompressed %" FORMAT64 "u\n",   totalFSize);
    }

    HashCollisionRestore();

    hsize = newSize;

    return outBuf;
}

char * RemoveNames(int &hsize, const char *inName, const char *inBuf,bool optKillNames=false,bool optNoNames=false,bool optVerbose=false)
{
    int fileCount = BigCount(inBuf);
    int sizeOfField = BigEntrySize(inBuf);
    int sizeOfNull = optKillNames?1:0;
    int newSize = 16+fileCount*(sizeOfField*2+sizeOfNull);
    char *outBuf = (char *)galloc(newSize);

    // write new header
    memcpy(outBuf,inBuf,16);
    outBuf[3] = (char)(sizeOfField+'0');    // 3 or 4
    if (IsOld24(inBuf)) memcpy(outBuf,"BIG",3);    // update signature to new format
    if (optNoNames)     memcpy(outBuf,"BIN",3);    // change BIG4 to BIN4 to indicate no names

    gputi(outBuf+4,newSize,4);      // update file size
    gputm(outBuf+8,fileCount,4);
    gputm(outBuf+12,newSize,4); // update header size

    const char *s = inBuf+BigHeaderStart(inBuf);
    char *d = outBuf+16;
    for (int i=0; i<fileCount; ++i)
    {
        memcpy(d,s,(sizeOfField*2));
        if (optKillNames)
            d[sizeOfField*2] = '\0';
        d += sizeOfField*2+sizeOfNull;
        s += sizeOfField*2+1+strlen(s+sizeOfField*2);
    }

    if (optVerbose)
    {
        if (optNoNames)
            printf("Nonames %s: %u files, %u bytes (saved %u bytes)\n", inName, fileCount, newSize, hsize-newSize);
        else
            printf("Killnames %s: %u files, %u bytes (saved %u bytes)\n", inName, fileCount, newSize, hsize-newSize);
    }

    hsize = newSize;

    return outBuf;
}

void LowerNames(char *inBuf)
{
    int fileCount = ggetm(inBuf+8,4);
    int sizeOfField = BigEntrySize(inBuf);

    char *s = inBuf+16;
    for (int i=0; i<fileCount; ++i)
    {
        strlwr(s+sizeOfField*2);
        s += sizeOfField*2+1+strlen(s+sizeOfField*2);
    }
}

// read in the big file header.
// the header size is bigendian at bytes 12..15 and includes the 16 byte header
// size field filled in by reference

int GetBigHeadSize(const char *head)
{
    int hsize;

    if (IsOld24(head)) hsize = ggetm(head+2,2)+4;   // old 24
    else hsize = ggetm(head+12,4);
    return hsize;
}

static char *ReadBigHead(const char *inName,int &hsize)
{
    hsize = 0;
    char *headBuf = NULL;

    GSTREAM *gIn = gopen(inName);
    if (gIn)
    {
        char head[16];
        int read = gread(gIn,head,16);
        if (read==16)
        {
            hsize = GetBigHeadSize(head);
            headBuf = (char *) galloc(hsize);
            if (headBuf)
            {
                gseek(gIn,0);
                read = gread(gIn,headBuf,hsize);
                if (read!=hsize)
                {
                    gfree(headBuf);
                }
            }
        }
        gclose(gIn);
    }


    return headBuf;
}

// DeCapitate - copies header from bigfile (inNmae) to seperate BigHead file. (outName)
// remove names if optKillNames specified (makes them just a null)
// remove names if optNoNames specified
// hash names if optHashNames specified
// returns true if successful

static bool DeCapitate(const char *inName,const char *outName,bool optKillNames=false,bool optNoNames=false,int optHashNames=16,bool optWrite=true,bool optToLower=false,int optPackAlign=1, int optUnPackAlign=1, bool optVerbose=false)
{
    bool ok = false;

    int hsize=0;

    char *inBuf = ReadBigHead(inName, hsize);
    if (!inBuf)
    {
        printf("bighead - ERROR FILE NOT FOUND %s\n",inName);
    }
    else
    {
        if (optToLower)
        {
            LowerNames(inBuf);
        }

        if (optKillNames || optNoNames)
        {
            char *outBuf = RemoveNames(hsize, inName, inBuf, optKillNames, optNoNames, optVerbose);
            gfree(inBuf);
            inBuf = outBuf;
        }
        else if (optHashNames)
        {
            char *outBuf = HashNames(hsize, inName, inBuf, optHashNames, optPackAlign, optUnPackAlign, optVerbose);
            gfree(inBuf);
            inBuf = outBuf;
        }
        else if (optVerbose)
        {
            int fileCount = BigCount(inBuf);
            printf("BigHead %s: %u files, %u bytes\n", inName, fileCount, hsize);
        }

        if (optWrite)
        {
            GSTREAM *gOut = gwopen(outName);
            if (gOut)
            {
                gwrite(gOut,inBuf,hsize);
                gclose(gOut);
            }
            else
            {
                ++errorCount;
                printf("bighead - ERROR OPENING OUTPUT FILE '%s'.  READ ONLY?\n",outName);
            }
        }
        gfree(inBuf);

        ok = errorCount?false:true;
    }
    return ok;
}

int main(int argc, char **argv)
{
    bool optKillNames=false;
    bool optNoNames=false;
    int optHashNames=0;
    bool optToLower=false;
    bool optVerbose=false;
    bool optWrite=true;
	int optPackAlign=1;  // packed file align size fields
	int optUnPackAlign=1;  // packed file align size fields
    char *inName = NULL;
    char *outName = NULL;
    for (int argi=1; argi<argc; ++argi)
    {
        if (!strcmp(argv[argi],"-killnames") || !strcmp(argv[argi],"-k"))
            optKillNames=true;
        else if (!strcmp(argv[argi],"-nonames") || !strcmp(argv[argi],"-n"))
            optNoNames=true;
        else if (!strcmp(argv[argi],"-tolower"))
            optToLower=true;
        else if (!strcmp(argv[argi],"-verbose") || !strcmp(argv[argi],"-v"))
            optVerbose=true;
        else if (!strcmp(argv[argi],"-hash") || !strcmp(argv[argi],"-hash16") || !strcmp(argv[argi],"-h"))
            optHashNames=16;
        else if (!strcmp(argv[argi],"-hash12"))
            optHashNames=12;
        else if (!strcmp(argv[argi],"-hash20"))
            optHashNames=20;
        else if (!strcmp(argv[argi],"-hash64"))
            optHashNames=20;
        else if (!strcmp(argv[argi],"-hash8"))
            optHashNames=8;
        else if (!strncmp(argv[argi],"-palign",7))
            optPackAlign=atoi(argv[argi]+7);
        else if (!strncmp(argv[argi],"-ualign",7))
            optUnPackAlign=atoi(argv[argi]+7);
		else if (!strcmp(argv[argi],"-nowrite") || !strcmp(argv[argi],"-w0"))
            optWrite=false;
        else if (!inName)
            inName = argv[argi];
        else if (!outName)
            outName = argv[argi];
        else
        {
            printf("BigHead - ERROR Too Many arguments\n");
            exit(10);
        }
    }

    if (!inName)
    {
        printf(
            TOOL_NAME " v%s by Frank Barchard\n"
            "USAGE: " TOOL_NAME " -options <infile.big> <outfile.bh>\n"
            "  <infile.big> Name of BigFile to extract header from\n"
            "  <outfile.bh> Name of BigHead file to save to (optional)\n"
            "  -verbose     switch to enable more verbose output (aka -v)\n"
            "  -nowrite     disable write of BigHead file (aka -w0)\n"
            "  -killnames   switch to disable names in the output file (aka -k)\n"
            "  -nonames     switch to disable names entirely (aka -n)\n"
            "  -palign      packed file size alignment (default 1)\n"
//            "  -ualign      unpacked file size alignment (default 1)\n"  hidden because its dangerous and not that useful
            "  -hash        switch to hash format (options: 8,12,16,20). (-h is hash16)\n",TextVersion);
        exit(1);
    }

    // add default extensions
    char inNameBuf[512];
    {
        strcpy(inNameBuf,inName);
        inName = inNameBuf;
        char *ext = strchr(inName,'.');
        if (!ext)
            strcat(inName,".viv");
    }

    char outNameBuf[512];
    if (!outName)
    {
        outName = outNameBuf;
        strcpy(outName,inName);
        char *ext = strchr(outName,'.');
        if (ext)
            *ext = '\0';
        strcat(outName,".bh");
    }
    else if (outName[0]=='=')
        ++outName;

    bool ok = DeCapitate(inName,outName,optKillNames,optNoNames,optHashNames,optWrite,optToLower,optPackAlign,optUnPackAlign,optVerbose);
    if (!ok)
        return 100;
    return 0;
}
