// (c) Electronic Arts. All Rights Reserved.

#include "spitimpl.h"

#include "../spit/spit.h"

#include <EABase/EABase.h>
#include <EAStdC/EAString.h>

SpitImpl *gpSpitInst = NULL;

SpitImpl::SpitImpl() :
    mMainCommand(CREATE),

    // compression is disabled by default.
    mCompressionFormat(CompressionTypeNone),

    // default value taken from glump.  The negative value means the file can
    // _grow_ by as much as 1024 bytes before compression will be disabled for
    // the file.
    mCompressionThreshold(-1024),

    // 16 is optimal for ps3, pc, xbox360
    mAlignBoundary(16),

    // 32-bit hash is fine in 99.99% of cases.  Can use a 64-bit hash though.
    mHashSize(32),

    // sort the index by hash value and store the files in the order they
    // were specified to the tool.
    mIndexSortOrder(IndexSortHash),

    // By default files are serialized into the big file archive based on
    // the order they are specified on the command line.
    mFilesSortedAlphabetically(false),

    // By default we use the destination path (ie, sourcefile=destinationfile)
    // when writing the bigfile, but if the user wishes to use the source file
    // name instead then we let them. 
    mUseSourcePaths(false),

    // By default don't write the hierarchical file system information to the
    // bigfile because it bloats the bigfile.
    mIncludeFAT(false),

    //default to off.
    mLowercaseNames(false),

    //default to off.
    mUppercaseNames(false)


{
}

SpitImpl::~SpitImpl()
{
}

const char HelpString[] = \
"For support, please email EATechCoreSupport@ea.com\n" \
"Usage: bigfile.exe outfile.big [command] [options] [files to add] [@responsefile.rsp]\n" \
"\n" \
"COMMANDS (specify 1 or none):\n" \
"    -l             List files in bigfile.\n" \
"    -x             Extract files from bigfile to the current directory.\n" \
"    -merge         Merges bigfiles into the first file name specified.\n"  \
" If no command is given, a new output bigfile will be created.\n" \
"\n" \
"OPTIONS affecting bigfile creation:\n" \
"    -a[16..1024]   Alignment boundary (must be a power of two)\n" \
"    -h[32|64]      Hash size in bits (default is 32)\n" \
"    -r             Recurse into subdirectories (paths will be preserved).\n" \
"    -compress[0-4] Compress files using the indexed algorithm.\n" \
"       0               No compression (default).\n" \
"       1               Compress with standard refpack.\n" \
"       2               Compress with chunked refpack.\n" \
"       3               Compress with chunked zlib.\n" \
"       4               Compress with LZX (can only be decompressed on Xbox 360).\n" \
"\n" \
"    -compressThreshold[n] If the file doesn't compress by at least n bytes,\n" \
"       disable compression completely for this file\n" \
"\n" \
"    -fat            Include the hierarchical filesystem information so that\n" \
"                        runtime searches of the bigfile may iterate through\n" \
"                        directories with similar behavior to most platform\n" \
"                        file system interfaces.\n" \
"\n" \
"    -sortIndex[0-3] Specifies the sort ordering of the index.\n" \
"       0               Index will be sorted by name hash value (default).\n" \
"                           Use this sort order if the most common runtime\n" \
"                           use-case will be to look up files by name or\n" \
"                           or Hash ID because this sort order allows binary\n" \
"                           searches. (O(lg n) time)\n" \
"       1               Index will be sorted alphabetically.  Useful if you\n" \
"                           plan on iterating over the index.  Searches will be\n"\
"                           O(n) time.\n" \
"       2               Index will be sorted by the file offset. Useful if you\n" \
"                           want to have the index correspond to files as they\n"\
"                           appear in the archive.\n"\
"       3               No sort applied, files will appear as specified on\n" \
"                           command line or in response file.\n" \
"\n" \
"    -sortFiles[0-1] Specifies whether or not to sort files before serialization.\n" \
"       0               Files are serialized in the order specified on the.\n" \
"                           command line (default).\n" \
"       1               Files are sorted alphabetically before serialization\n" \
"                           into the archive file.\n" \
"\n" \
"    -sourcefile    If input files are specified as [sourcefile]=[destinationfile]\n" \
"                       then this option forces the source file name to be used in\n" \
"                       the name index.\n" \
"\n" \
"    -nameLower     Convert names to lower case.\n" \
"    -nameUpper     Convert names to upper case.\n" \
;

void SpitImpl::sUsage(const char * /*prgname*/, const char * /*envname*/, const char * /*urlname*/, int /*verboseLevel*/)
{
    fprintf(stderr, "%s v%2d.%02d.%02d, by EATech.\n", ToolName,
        (ToolVersion/10000)%100,
        (ToolVersion/100)%100,
        (ToolVersion/1)%100);
    fprintf(stderr, "%s", HelpString);
}

char *SpitImpl::sOpt(const char *optptr)
{
    return gpSpitInst->Opt(optptr);
}

const char *SpitImpl::sHistory[] = \
{
" Version Date       Changes:",
" ------- ---------- --------",
" 1.01.00 2009-10-20 compressThreshold[n] option added, source=dest renaming added",
" 1.00.00 2009-02-10 Initial release (code mainly taken from glump tool)"
};

const char *SpitImpl::ToolName = "bigfile";

char *SpitImpl::Opt(const char *optptr)
{
    char *d=NULL;
    int OptValue;

    switch (tolower(*optptr))
    {
    case 'a':
        if (!EA::StdC::Strnicmp(optptr, "a", 1))
        {
            d = SPIT_getopt( &OptValue,optptr+1, "alignment", 16, 1024);
            if (!OptValue)
            {
                fprintf(stderr, "alignment value is out of range.  Ignored\n");
            }
            else
            {
                // Not zero, check for power of two.
                if (OptValue & (OptValue - 1))
                {
                    fprintf(stderr, "Alignment value must be a power of 2\n");
                    fprintf(stderr, "The alignment will be not be changed\n");
                }
                else
                {
                    mAlignBoundary = OptValue;
                }
            }
        }
        break;

    case 'c':
        if (!EA::StdC::Strnicmp(optptr, "compressThreshold", 17))
        {
            d = SPIT_getopt( &OptValue, optptr+17, "compression threshold", INT32_MIN, INT32_MAX);
            mCompressionThreshold = OptValue;
        }
        else if (!EA::StdC::Strnicmp(optptr, "compress", 8))
        {
            d = SPIT_getopt( &OptValue, optptr+8, "compression format",
                CompressionTypeNone, CompressionTypeTotalOptions-1);
            mCompressionFormat = static_cast<CompressionType>(OptValue);
        }
        break;

    case 'f':
        if (!EA::StdC::Strnicmp(optptr, "fat", 3))
        {
            d = SPIT_getopt(&OptValue, optptr+3, "include the hierarchical filesystem table", 0, 1);
            mIncludeFAT = true;
        }
        break;

    case 'h':
        d = SPIT_getopt( &OptValue,optptr+1, "hash size", 0, INT32_MAX);
        if (OptValue != 32 && OptValue != 64)
        {
            fprintf(stderr, "Hash size must be 32 or 64 bits\n");
        }
        mHashSize = OptValue;
        break;

    case 'l':
        d = SPIT_getopt( &OptValue,optptr+1, "list", 0, 1);
        mMainCommand = LIST;
        break;

    case 'n':
        if (!EA::StdC::Strnicmp(optptr, "nameLower", 9))
        {
            d = SPIT_getopt(&OptValue, optptr+9, "Convert names to lower case", 0, 1);
            mLowercaseNames = true;
        }
        else if (!EA::StdC::Strnicmp(optptr, "nameUpper", 9))
        {
            d = SPIT_getopt(&OptValue, optptr+9, "Convert names to upper case", 0, 1);
            mUppercaseNames = true;
        }
        break;

    case 's':
        if (!EA::StdC::Strnicmp(optptr, "sortIndex", 9))
        {
            d = SPIT_getopt( &OptValue, optptr+9, "Index sort order", 0, 3);
            mIndexSortOrder = static_cast<IndexSortOrder>(OptValue);
        } 
        else if (!EA::StdC::Strnicmp(optptr, "sortFiles", 9))
        {
            d = SPIT_getopt( &OptValue, optptr + 9, "File sort order", 0, 1);
            mFilesSortedAlphabetically = OptValue == 1;
        }
        else if (!EA::StdC::Strnicmp(optptr, "sourcefile", 10))
        {
            mUseSourcePaths = true;
            d = SPIT_getopt( &OptValue, optptr+10, "Use source file names instead of destination", 0, 1);
        }

        break;

    case 'x':
        d = SPIT_getopt( &OptValue,optptr+1, "extract", 0, 1);
        mMainCommand = EXTRACT;
        break;
    

    case 'm':
        if (!EA::StdC::Strnicmp(optptr, "merge", 5))
        {
            d = SPIT_getopt( &OptValue, optptr+5, "merge", 0, 1 );
            mMainCommand = MERGE; 
        }
        break;
    }

    return d;
}
