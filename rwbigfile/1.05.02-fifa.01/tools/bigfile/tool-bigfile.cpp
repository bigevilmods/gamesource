// (c) Electronic Arts. All Rights Reserved.

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <cassert>

#include "EABase/EABase.h"
#include "eastl/algorithm.h"
#include "eastl/list.h"
#include "eastl/map.h"
#include "eastl/sort.h"
#include "eastl/string.h"
#include "eastl/vector.h"
#include "EAStdC/EAAlignment.h"
#include "EAStdC/EAString.h"
#include "endian/endian.h"
#include "job_manager/job_scheduler.h"
#include "rw/core/codec/refpack.h"
#include "rw/core/codec/chunkcodec.h"
#include "rw/core/bigfile/fileformat.h"
#include "rw/core/bigfile/bigfile.h"
#include "rw/core/filesys/ifilestream.h"
#include "rw/core/filesys/path.h"

#include "spit/spit.h"
#include "fat/fat.h"
#include "bigfileindexentry.h"
#include "spitimpl.h"

using namespace EA::Endian;
using namespace eastl;
using namespace rw::core::filesys;

#if !defined (ERROR_SUCCESS)
#define ERROR_SUCCESS 0
#endif

// all buffers should be aligned to this address & size
const unsigned int BUFFER_ALIGNMENT = 16;

class MallocAllocator : public EA::Allocator::ICoreAllocator
{
public:
    virtual void *Alloc(size_t size, const char *, unsigned int, unsigned int alignment, unsigned int)
    {
        #if defined (EA_PLATFORM_WINDOWS)
            return _aligned_malloc(size, alignment);
        #else
            void *pointer;
            int success = posix_memalign(&pointer, alignment, size);
            (void)success;
            EA_ASSERT(success == 0);
            return pointer;
        #endif
    }

    virtual void *Alloc(size_t size, const char *, unsigned int)
    {
        return Alloc(size, NULL, 0, BUFFER_ALIGNMENT, 0);
    }

    virtual void Free(void *ptr, size_t)
    {
        #if defined (EA_PLATFORM_WINDOWS)
            return _aligned_free(ptr);
        #else
            return free(ptr);
        #endif
    }
};

typedef void (*EntryPrinter)(BigFileIndexEntry const &);

static MallocAllocator gMallocAllocator;
static EA::Jobs::JobScheduler *gJobScheduler;
rw::core::codec::BufferedChunkDecompressor *gChunkDecompressor;
static int gNumHashCollisions = 0;

static char** gMergeFileList = NULL;

// specialized global new operators required by EASTL
void *operator new[](size_t size, const char *, int, unsigned int, const char *, int)
{
    return malloc(size);
}

void *operator new[](size_t size, size_t, size_t, const char *, int, unsigned int, const char *, int)
{
    return malloc(size);
}

void operator delete[](void *ptr)
{
    free(ptr);
}

static int SortByHash(const BigFileIndexEntry &a, const BigFileIndexEntry &b)
{
    if (gpSpitInst->GetHashSize() == 32)
    {
        uint32_t ahash = static_cast<uint32_t>(a.hash);
        uint32_t bhash = static_cast<uint32_t>(b.hash);

        if (ahash > bhash)
        {
            // do not swap
            return 0;
        }
        else if (ahash < bhash)
        {
            // swap required
            return 1;
        }

        // This is bad.  Use 64-bit hashes to reduce this possiblity.
        ++gNumHashCollisions;

        return 0;
    }

    if (a.hash > b.hash)
    {
        // do not swap
        return 0;
    }
    else if (a.hash < b.hash)
    {
        // swap required
        return 1;
    }

    // This is really bad if we get a collision with a 64-bit hash
    ++gNumHashCollisions;

    return 0;
}

static int SortByPath(const BigFileIndexEntry &a, const BigFileIndexEntry &b)
{
    if (gpSpitInst->GetMainCommand() == SpitImpl::MERGE)
        return strcmp(a.srcpath, b.srcpath) < 0;
    else
        return strcmp(a.savepath, b.savepath) < 0;
}

int SortByOffset(const BigFileIndexEntry &a, const BigFileIndexEntry &b)
{
    if (a.offset < b.offset)
        return 1;

    // If the offsets are identical, which happens in the case of 0-byte files,
    // we don't care which one is listed first
    return 0;
}

struct PathData
{
    PathData() {}

    PathData(const char *path)
    {
        strcpy(this->path, path);
    }

    bool operator==(const PathData &other)
    {
        return strcmp(path, other.path) == 0;
    }

    char path[256];
};

static void PadFile(IFileStream &stream)
{
    // Pad sections of the file stream to the alignment specified.
    
    int padbytes = gpSpitInst->Alignment()-(int)(stream.Tell() % gpSpitInst->Alignment());

    if (padbytes == gpSpitInst->Alignment())
        padbytes = 0;

    while (padbytes != 0)
    {
        char data = 0;
        stream.Write(&data, sizeof(data));
        --padbytes;
    }
}

/// return the base-2 logarithm of n
static int log(int n)
{
    int i = -1;

    for ( ; n != 0; ++i)
        n >>= 1;

    return i;
}


static void BuildFAT(const list<BigFileIndexEntry> &fileList,
        vector<rw::core::BigFileDirectoryEntry> &fat,
        vector<unsigned> &constituents,
        vector<char> &stringTable)
{
    using namespace rw::core;
    using namespace eastl;
    using namespace EA::Endian;

    for (list<BigFileIndexEntry>::const_iterator I = fileList.begin(), E = fileList.end();
            I != E; ++I)
        FAT::Node::Create(&*I);

    stringTable.reserve(FAT::StringTableSize());

    for (vector<string>::const_iterator I = FAT::StringListBegin(), E = FAT::StringListEnd();
            I != E; ++I)
    {
        for (string::const_iterator II = I->begin(), EE = I->end(); II != EE; ++II)
            stringTable.push_back(*II);
        stringTable.push_back(0);
    }

    FAT::AddRoot();

    for (list<FAT::Node *>::const_iterator I = FAT::NodeListBegin(), E = FAT::NodeListEnd(); 
            I != E; ++I)
    {
        FAT::Node &FN = **I;
        rw::core::BigFileDirectoryEntry BFDE;
        BFDE.NameIndex = big_endian(FN.NameIndex);
        BFDE.EntryType = big_endian(FN.IsFile ? BIGFILE_FAT_FILE : BIGFILE_FAT_DIRECTORY);
        BFDE.ContainingNodeIndex = big_endian(FN.ContainingNodeIndex);

        if (FN.IsFile)
        {
            BFDE.FileIndex = big_endian(FN.FileIndex);
            BFDE.Unused = ~0U;
        }
        else
        {
            const unsigned CSI = constituents.size();
            BFDE.ConstituentStartIndex = big_endian(CSI);

            for (list<FAT::Node *>::const_iterator II = FN.Constituents.begin(), EE = FN.Constituents.end();
                    II != EE; ++II)
            {
                const unsigned index = (*II)->NodeIndex;
                constituents.push_back(big_endian(index));
            }

            const unsigned numConstituents = constituents.size() - CSI;
            BFDE.NumConstituents = big_endian(numConstituents);
        }

        fat.push_back(BFDE);
    }
}

static bool IsValidFile(bool &errorRaised, const BigFileIndexEntry &BFE)
{
    /// Validates that the file both exists and fits the name/path
    /// length constraints that we impose. 
    FILE *FH = fopen(BFE.srcpath, "rb");

    if (FH != NULL)
    {
        fclose(FH);

        if (BFE.nameLen + 3 > 255)
        {
            fprintf(stderr, "bigfile: file name '%s' is longer than the allowable 252 characters\n", BFE.name);
            errorRaised = true;
            return false;
        }

        if (BFE.pathLen + 1 > 255)
        {
            fprintf(stderr, "bigfile: path '%s' is longer than the allowable 254 characters\n", BFE.path);
            errorRaised = true;
            return false;
        }

        return true;
    }
    else
    {
        fprintf(stderr, "bigfile: failed to open %s for reading\n", BFE.srcpath);
        return false;
    }
}

static list<BigFileIndexEntry> VerifyInputs(list<BigFileIndexEntry> const &fileList)
{    
    list<BigFileIndexEntry> outputFileList;
    bool errorRaised = false;

    for (list<BigFileIndexEntry>::const_iterator I = fileList.begin(), E = fileList.end();
            I != E; ++I)
        // Do not verify inputs if we are merging; Inputs are not really files.
        if ((gpSpitInst->GetMainCommand() == SpitImpl::MERGE) || IsValidFile(errorRaised, *I))
            outputFileList.push_back(*I);

    // return an empty list if we have invalid data.
    return errorRaised ? list<BigFileIndexEntry>() : outputFileList;
}

static list<string> GeneratePathList(list<BigFileIndexEntry> const &fileList)
{
    list<string> pathList;
    for (list<BigFileIndexEntry>::const_iterator I = fileList.begin(), E = fileList.end();
            I != E; ++I)
    {
        string path(I->path, I->pathLen);
        if (find(pathList.begin(), pathList.end(), path) == pathList.end())
            pathList.push_back(path);
    }

    return pathList;
}

static size_t FindLongestPathLength(list<string> const &pathList)
{
    size_t maxLength = 0;

    for (list<string>::const_iterator I = pathList.begin(), E = pathList.end();
            I != E; ++I)
        maxLength = I->size() > maxLength ? I->size() : maxLength;

    return maxLength;
}

static size_t FindLongestNameLength(list<BigFileIndexEntry> const &fileList)
{
    int maxLength = 0;

    for (list<BigFileIndexEntry>::const_iterator I = fileList.begin(), E = fileList.end();
            I != E; ++I)
        maxLength = I->nameLen > maxLength ? I->nameLen : maxLength;

    return static_cast<size_t>(maxLength);
}

static void SortFileList(list<BigFileIndexEntry> &fileList)
{
    if (gpSpitInst->GetIndexSortOrder() == SpitImpl::IndexSortAlphabetical)
    {
        // if the user is sorting the index by name, assume they also want
        // the physical files stored in that order, for read optimization
        // purposes.
        fileList.sort(SortByPath);
    } else if (gpSpitInst->GetIndexSortOrder() == SpitImpl::IndexSortHash)
    {
        // sort the index by hash value        
        fileList.sort(SortByHash);
    }

    // If the user requests that the index be sorted by offset, the file list would have already been sorted by offset
}

static unsigned SerializeFAT(IFileStream &bigFile, list<BigFileIndexEntry> const &fileList)
{
    unsigned fatSize = 0;

    vector<rw::core::BigFileDirectoryEntry> fat;
    vector<unsigned> constituents;
    vector<char> stringTable;
    BuildFAT(fileList, fat, constituents, stringTable);

    uint32_t directoryEntrySize = big_endian(static_cast<uint32_t>(fat.size() * sizeof(rw::core::BigFileDirectoryEntry)));
    bigFile.Write(&directoryEntrySize, sizeof(uint32_t));
    bigFile.Write(fat.data(), fat.size() * sizeof(rw::core::BigFileDirectoryEntry));

    uint32_t constituentsSize = big_endian(static_cast<uint32_t>(constituents.size() * sizeof(unsigned)));
    bigFile.Write(&constituentsSize, sizeof(uint32_t));
    bigFile.Write(constituents.data(), constituents.size() * sizeof(unsigned));

    uint32_t stringTableSize = big_endian(static_cast<uint32_t>(stringTable.size()));
    bigFile.Write(&stringTableSize, sizeof(uint32_t));
    bigFile.Write(stringTable.data(), stringTable.size());

    FAT::Release();

    fatSize = EA::StdC::AlignUp(big_endian(directoryEntrySize) + 
        big_endian(constituentsSize) + 
        big_endian(stringTableSize) +
        3 * sizeof(uint32_t), 16);
    
    return fatSize;
}

static void SetFileAndPathIndices(list<BigFileIndexEntry> &fileList, list<string> const &pathList)
{
    unsigned fileIndex = 0;
    for (list<BigFileIndexEntry>::iterator I = fileList.begin(), E = fileList.end();
            I != E; ++I, ++fileIndex)
    {
        I->index = fileIndex;

        int i = 0;
        for (list<string>::const_iterator II = pathList.begin(), EE = pathList.end(); 
                II != EE; ++II, ++i)
            if (strcmp(II->c_str(), I->path) == 0)
                break;
        I->pathIndex = static_cast<uint16_t>(i);
    }
}

static bool VerifyNoHashCollisions()
{
    if (gNumHashCollisions != 0)
    {
        fprintf(stderr, "bigfile: Hash collisions have been found. "
                "Rename one of them, or use a 64-bit hash instead of a 32-bit hash\n");
        return false;
    }

    return true;
}

static void *SlurpFile(BigFileIndexEntry &BFE)
{
    /// Slurp file reads the entire file described in the 
    /// BigFileIndexEntry into memory.

    FILE *subfile;
    uint32_t uncompressedFileSize = 0; // Save the original filesize here.

    // Check if the current operation is a merge or not.
    // If it is a merge, open the original bigfile it belonged to
    // and seek to the correct position
    if (gpSpitInst->GetMainCommand() == SpitImpl::MERGE)
    {        
        subfile = fopen(gMergeFileList[BFE.mergeBigfileIndex], "rb");

        if (subfile == NULL)
        {
            return 0;
        }

        if (BFE.zSize > 0)
        {
            uncompressedFileSize = BFE.fileSize;
            BFE.fileSize = BFE.zSize; // Since we will not uncompress, the file size remains the same.
        }

        // Seek to the correct position in the bigfile.
        fseek(subfile, static_cast<long int>(BFE.offset), SEEK_SET);
    }
    else
    {
        subfile = fopen(BFE.srcpath, "rb");

        if (subfile == NULL)
        {
            // This shouldn't happen because we already tried to open all the
            // files in this function and any that failed were removed.
            return 0;
        }

        // Find the file size.
        fseek(subfile, 0, SEEK_END);
        long int fileSize = ftell(subfile);
        fseek(subfile, 0, SEEK_SET);
        
        BFE.fileSize = static_cast<uint32_t>(fileSize);
    }

    void *buffer = NULL;
    buffer = gMallocAllocator.Alloc(static_cast<size_t>(BFE.fileSize), NULL, 0);
    
    size_t dwNumberOfBytesRead = fread(buffer, 1, static_cast<size_t>(BFE.fileSize), subfile);

    if (dwNumberOfBytesRead != BFE.fileSize)
    {
        fprintf(stderr, "bigfile warning: failed to completely read %s\n", BFE.srcpath);
        fclose(subfile);
        return NULL;
    }

    fclose(subfile);

    if (gpSpitInst->GetMainCommand() == SpitImpl::MERGE && uncompressedFileSize > 0)
    {
        // Restore the uncompressed filesize.
        BFE.fileSize = uncompressedFileSize;
    }

    return buffer;
}

static void *TryCompress(void *buffer, BigFileIndexEntry &BFE)
{
    // Try to compress the buffer provided. If the compressed size is smaller
    // or meets certain threshold values, return the newly compressed data. If
    // not, return the uncompressed buffer.

    // Do not compress when merging. Keep the original format of the file.
    if (gpSpitInst->GetMainCommand() == SpitImpl::MERGE)
        return buffer;

    BFE.zSize = 0;

    if (BFE.fileSize == 0)
        return buffer;

    if (gpSpitInst->GetCompressionType() == SpitImpl::CompressionTypeNone)
        return buffer;

    size_t packBufSize = 0;
    void *packbuf = NULL;
    size_t compressedSize = 0;

    rw::core::codec::RefpackDeflate refpackDeflator(&gMallocAllocator, 
        rw::core::codec::ICodec::DEFLATE_BEST_SIZE);

    // possible TODO: allow the tool to pass user-settable options
    // for chunk size & alignments.  Or allow them to pass a profile
    // of the system on which they'll be decompressed (for example,
    // you might use 224K on the SPU with refpack).
    rw::core::codec::BufferedChunkCompressor c(&gMallocAllocator,
        rw::core::codec::CHUNK_COMPRESSION_TYPE_UNCOMPRESSED);

    packBufSize = (size_t)(((double)BFE.fileSize)*1.2+1024.0);
    packbuf = gMallocAllocator.Alloc(packBufSize, NULL, 0);

    switch (gpSpitInst->GetCompressionType())
    {
    case SpitImpl::CompressionTypeRefpack:

        refpackDeflator.SetInputBuffer(buffer, static_cast<size_t>(BFE.fileSize));
        refpackDeflator.SetOutputBuffer(packbuf, packBufSize);
        refpackDeflator.Exec(rw::core::codec::ICodec::FINISH);

        compressedSize = refpackDeflator.GetOutputBytesAvailable();

        BFE.flags = rw::core::BIGFILE_FORMAT_FLAG_COMPRESSION_TYPE_REFPACK;
        break;

    case SpitImpl::CompressionTypeChunkedRefpack:

        c.SetCompressionType(rw::core::codec::CHUNK_COMPRESSION_TYPE_REFPACK);

        compressedSize = c.Compress(packbuf, packBufSize, buffer,
            static_cast<size_t>(BFE.fileSize));

        BFE.flags = rw::core::BIGFILE_FORMAT_FLAG_COMPRESSION_TYPE_REFPACK_CHUNKED;
    break;

    case SpitImpl::CompressionTypeChunkedZlib:
        c.SetCompressionType(rw::core::codec::CHUNK_COMPRESSION_TYPE_ZLIB);

        compressedSize = c.Compress(packbuf, packBufSize, buffer,
            static_cast<size_t>(BFE.fileSize));

        BFE.flags = rw::core::BIGFILE_FORMAT_FLAG_COMPRESSION_TYPE_ZLIB_CHUNKED;
        break;

    case SpitImpl::CompressionTypeChunkedLzx:
        c.SetCompressionType(rw::core::codec::CHUNK_COMPRESSION_TYPE_LZX);

        compressedSize = c.Compress(packbuf, packBufSize, buffer,
            static_cast<size_t>(BFE.fileSize));

        BFE.flags = rw::core::BIGFILE_FORMAT_FLAG_COMPRESSION_TYPE_LZX;
        break;
    default:
        fprintf(stderr, "Warning: compression type not supported. Compression ignored.\n");
        gMallocAllocator.Free(packbuf, 0);
        return buffer;
    }

    if (compressedSize < static_cast<int64_t>(BFE.fileSize) - gpSpitInst->GetCompressionThreshold())
    {
        // the file decreased in size by at least as much as the
        // compression threshold (note that compression threshold
        // can be negative)
        BFE.zSize = compressedSize;

        // Use the packed buffer as the input buffer
        void *tmp = buffer;
        buffer = packbuf;
        gMallocAllocator.Free(tmp, 0);
    }
    else
    {
        // compression wasn't worth it, keep the uncompressed data
        gMallocAllocator.Free(packbuf, 0);
        BFE.flags = rw::core::BIGFILE_FORMAT_FLAG_COMPRESSION_TYPE_NONE;
    }

    return buffer;
}

static bool SerializeFileData(IFileStream &bigfile, list<BigFileIndexEntry> &fileList)
{
    PadFile(bigfile);

    for (list<BigFileIndexEntry>::iterator file = fileList.begin(); 
        file != fileList.end();
        ++file)
    {
        void *srcbuf = SlurpFile(*file);
        if (!srcbuf)
            return false;

        srcbuf = TryCompress(srcbuf, *file);

        PadFile(bigfile);

        file->offset = bigfile.Tell() / gpSpitInst->Alignment();

        // now copy the subfile into the bigfile
        bigfile.Write(srcbuf,
            file->zSize != 0 ? static_cast<size_t>(file->zSize) : static_cast<size_t>(file->fileSize));

        if (OPTverbose >= 2)
        {
            printf("Added file %s at offset %lld , ", file->srcpath,
                file->offset * gpSpitInst->Alignment());

            switch (file->flags)
            {
            case rw::core::BIGFILE_FORMAT_FLAG_COMPRESSION_TYPE_NONE:
                printf("not compressed (%d bytes)\n", file->fileSize);
                break;

            case rw::core::BIGFILE_FORMAT_FLAG_COMPRESSION_TYPE_REFPACK:
                printf("compressed with refpack (%d bytes -> %d bytes)\n",
                    file->fileSize, file->zSize);
                break;

            case rw::core::BIGFILE_FORMAT_FLAG_COMPRESSION_TYPE_REFPACK_CHUNKED:
                printf("compressed with refpack chunked (%d bytes -> %d bytes)\n",
                    file->fileSize, file->zSize);
                break;

            case rw::core::BIGFILE_FORMAT_FLAG_COMPRESSION_TYPE_ZLIB_CHUNKED:
                printf("compressed with zlib chunked (%d bytes -> %d bytes)\n",
                    file->fileSize, file->zSize);
                break;
            }
        }

        gMallocAllocator.Free(srcbuf, 0);
    }

    PadFile(bigfile);

    return true;
}

static void SerializeIndex(char *indexBuffer, list<BigFileIndexEntry> const &fileList)
{
    unsigned const hashSize = gpSpitInst->GetHashSize() / 8;

    for (list<BigFileIndexEntry>::const_iterator I = fileList.begin(), E = fileList.end();
            I != E; ++I)
    {
        big_put(indexBuffer, static_cast<uint32_t>(I->offset), 4);
        indexBuffer += 4;
        big_put(indexBuffer, I->zSize, 4);
        indexBuffer += 4;
        big_put(indexBuffer, I->fileSize, 4);
        indexBuffer += 4;
        big_put64(indexBuffer, I->hash, hashSize);
        indexBuffer += hashSize;
    }
}

static void SerializeNames(char *nameBuffer, size_t const nameMaxLength, list<BigFileIndexEntry> const &fileList)
{
    for (list<BigFileIndexEntry>::const_iterator I = fileList.begin(), E = fileList.end();
            I != E; ++I)
    {
        big_put(nameBuffer, I->pathIndex, sizeof I->pathIndex);
        strcpy(nameBuffer + 2, I->name);
        nameBuffer += nameMaxLength;
    }
}

static void SerializeFlags(char *flagsBuffer, list<BigFileIndexEntry> const &fileList)
{
    for (list<BigFileIndexEntry>::const_iterator I = fileList.begin(), E = fileList.end();
            I != E; ++I)
    {
        big_put(flagsBuffer, I->flags, sizeof I->flags);
        flagsBuffer += sizeof I->flags;
    }
}

static void SerializePaths(char *pathBuffer, size_t const pathMaxLength, list<string> const &pathList)
{
    for (list<string>::const_iterator I = pathList.begin(), E = pathList.end();
            I != E; ++I)
    {
        strcpy(pathBuffer, I->c_str());
        pathBuffer += pathMaxLength;
    }
}

static int CreateBigFile(IFileStream &bigfile, list<BigFileIndexEntry> const &inputFileList)
{
    rw::core::BigFileFormatHeader hdr;
    memset(&hdr, 0, sizeof(hdr));

    list<BigFileIndexEntry> fileList = VerifyInputs(inputFileList);
    if (fileList.size() == 0)
        return -1;

    list<string> pathList = GeneratePathList(fileList);

    // Each name is prefixed with a two-byte path index and both names and paths 
    // have a trailing null character which we add to their actual lengths.
    size_t const nameMaxLength = FindLongestNameLength(fileList) + 3;
    size_t const pathMaxLength = FindLongestPathLength(pathList) + 1;

    // may be 16 or 20 bytes, depending on hash width.  Default to 16.
    int const indexEntrySize = gpSpitInst->GetHashSize() == 64 ? 20 : 16;    

    hdr.nameLength = static_cast<uint8_t>(nameMaxLength);
    hdr.pathLength = static_cast<uint8_t>(pathMaxLength);

    size_t const headerBufferSize = EA::StdC::AlignUp(sizeof(rw::core::BigFileFormatHeader), 16);
    size_t const indexBufferSize = EA::StdC::AlignUp(fileList.size() * indexEntrySize, 16);
    size_t const flagsBufferSize = EA::StdC::AlignUp(fileList.size(), 16);
    size_t const nameBufferSize = EA::StdC::AlignUp(hdr.nameLength * fileList.size(), 16);
    size_t const pathBufferSize = EA::StdC::AlignUp(hdr.pathLength * pathList.size(), 16);
    size_t const totalHeaderSize = headerBufferSize + indexBufferSize + flagsBufferSize + nameBufferSize + pathBufferSize;

    char *headerBuffer = static_cast<char *>(calloc(headerBufferSize, 1));
    char *indexBuffer = static_cast<char *>(calloc(indexBufferSize, 1));
    char *flagsBuffer = static_cast<char *>(calloc(flagsBufferSize, 1));
    char *nameBuffer = static_cast<char *>(calloc(nameBufferSize, 1));
    char *pathBuffer = static_cast<char *>(calloc(pathBufferSize, 1));

    // skip over the header
    bigfile.Seek(totalHeaderSize, IFileStream::SEEKORIGIN_BEGINNING);

    if (gpSpitInst->FilesSortedAlphabetically())
        fileList.sort(SortByPath);

    if (!VerifyNoHashCollisions())
        return -1;

    SetFileAndPathIndices(fileList, pathList);
    
    uint32_t fatSize = 0;
    // If the user specifies that they want to build the directory structure then
    // create it and write it to the bigfile header.
    if (gpSpitInst->GetFATInclusion())
        fatSize = SerializeFAT(bigfile, fileList);

    if (!SerializeFileData(bigfile, fileList))
        return -1;

    SortFileList(fileList);

    hdr.numPaths = static_cast<uint16_t>(pathList.size());
    hdr.signature = BIGFILE_HEADER_SIGNATURE;
    hdr.headerVersion = BIGFILE_HEADER_VERSION;
    hdr.fileCount = fileList.size();
    hdr.flags = 0;

    if (gpSpitInst->GetHashSize() == 64)
        hdr.flags |= rw::core::BIGFILE_FORMAT_FLAG_64BITHASH;

    if (gpSpitInst->GetIndexSortOrder() == SpitImpl::IndexSortHash)
        hdr.flags |= rw::core::BIGFILE_FORMAT_FLAG_SORTORDER_HASH;
    else if (gpSpitInst->GetIndexSortOrder() == SpitImpl::IndexSortAlphabetical)
        hdr.flags |= rw::core::BIGFILE_FORMAT_FLAG_SORTORDER_ALPHABETICAL;
    else if (gpSpitInst->GetIndexSortOrder() == SpitImpl::IndexSortOffset)
        hdr.flags |= rw::core::BIGFILE_FORMAT_FLAG_SORTORDER_OFFSET;

    if (gpSpitInst->GetFATInclusion())
        hdr.flags |= rw::core::BIGFILE_FORMAT_FLAG_INCLUDES_FAT;

    hdr.alignment = static_cast<uint8_t>(log(gpSpitInst->Alignment()));
    hdr._reserved0 = 0;

    // main header consists of the bigfile header, the main index, and
    // the flags buffer
    hdr.baseHeaderSize = headerBufferSize + indexBufferSize + flagsBufferSize;

    // The names are kept in a separate index, which may or may not be loaded
    hdr.nameHeaderSize = nameBufferSize + pathBufferSize;
    hdr.fileSize = bigfile.Tell();
    hdr.fatSize = fatSize;

    // write out the header to the stream in big-endian format.
    big_put(headerBuffer+ 0, hdr.signature,     sizeof(hdr.signature));
    big_put(headerBuffer+ 2, hdr.headerVersion, sizeof(hdr.headerVersion));
    big_put(headerBuffer+ 4, hdr.fileCount,     sizeof(hdr.fileCount));
    big_put(headerBuffer+ 8, hdr.flags,         sizeof(hdr.flags));
    big_put(headerBuffer+10, hdr.alignment,     sizeof(hdr.alignment));
    big_put(headerBuffer+11, hdr._reserved0,    sizeof(hdr._reserved0));
    big_put(headerBuffer+12, hdr.baseHeaderSize,sizeof(hdr.baseHeaderSize));
    big_put(headerBuffer+16, hdr.nameHeaderSize,sizeof(hdr.nameHeaderSize));
    big_put(headerBuffer+20, hdr.nameLength,    sizeof(hdr.nameLength));
    big_put(headerBuffer+21, hdr.pathLength,    sizeof(hdr.pathLength));
    big_put(headerBuffer+22, hdr.numPaths,      sizeof(hdr.numPaths));
    big_put64(headerBuffer+24, hdr.fileSize,    sizeof(hdr.fileSize));
    big_put(headerBuffer+32, hdr.fatSize,       sizeof(hdr.fatSize));

    SerializeIndex(indexBuffer, fileList);
    SerializeNames(nameBuffer, nameMaxLength, fileList);
    SerializeFlags(flagsBuffer, fileList);
    SerializePaths(pathBuffer, pathMaxLength, pathList);

    // write out the index information to the start of the file
    bigfile.Seek(0, IFileStream::SEEKORIGIN_BEGINNING);
    bigfile.Write(headerBuffer, headerBufferSize);
    bigfile.Write(indexBuffer, indexBufferSize);
    bigfile.Write(flagsBuffer, flagsBufferSize);
    bigfile.Write(nameBuffer, nameBufferSize);
    bigfile.Write(pathBuffer, pathBufferSize);

    // release allocated resources
    free(headerBuffer);
    free(indexBuffer);
    free(flagsBuffer);
    free(nameBuffer);
    free(pathBuffer);

    fileList.clear();
    pathList.clear();

    return ERROR_SUCCESS;
}

void GetList(IFileStream &strm,
             vector<BigFileIndexEntry> &index,
             uint16_t &headerVersion, int &hashFieldSize)
{
    char tmp[sizeof(rw::core::BigFileFormatHeader)];

    strm.Read(tmp, sizeof(tmp));

    size_t headerSize = rw::core::Big::GetHeaderSize(tmp) + 
        rw::core::Big::GetNameIndexSize(tmp);
    
    if (rw::core::Big::HasFAT(tmp))
    {
        headerSize = EA::StdC::AlignUp(headerSize, 16);
        headerSize += rw::core::Big::GetFATSize(tmp);
    }

    char *headerbuf = static_cast<char *>(malloc(headerSize));

    strm.Seek(0, IFileStream::SEEKORIGIN_BEGINNING);
    strm.Read(headerbuf, headerSize);

    rw::core::BigFileFormatHeader hdr;

    // write out the header to the stream in big-endian format.
    hdr.signature = static_cast<uint16_t>(big_get(headerbuf+0, sizeof(hdr.signature)));
    hdr.headerVersion = static_cast<uint16_t>(big_get(headerbuf+2, sizeof(hdr.headerVersion)));
    hdr.fileCount = big_get(headerbuf+4, sizeof(hdr.fileCount));
    hdr.flags = static_cast<uint16_t>(big_get(headerbuf+8, sizeof(hdr.flags)));

    if (hdr.signature != BIGFILE_HEADER_SIGNATURE)
    {
        fprintf(stderr, "bigfile: this is not a recognized bigfile");
        return;
    }

    if (hdr.headerVersion > BIGFILE_HEADER_VERSION)
    {
        fprintf(stderr, "bigfile: this bigfile has a newer format than this tool can handle. "
                "Please get the latest version of the tool.\n"
                "This file has version %d, and the tool handles up to version %d\n",
            hdr.headerVersion, BIGFILE_HEADER_VERSION);
        return;
    }

    headerVersion = hdr.headerVersion;

    hashFieldSize =
        (hdr.flags & rw::core::BIGFILE_FORMAT_FLAG_64BITHASH) == 0 ? 4 : 8;

    rw::core::Big myBig(headerbuf);
    myBig.SetNameIndex(headerbuf + myBig.GetHeaderSize(headerbuf));

    index.reserve(myBig.GetCount());
    for (int i = 0; i < myBig.GetCount(); ++i)
    {
        uint64_t size, zSize;
        unsigned int flags;

        BigFileIndexEntry BFE;
        memset(&BFE, 0, sizeof BFE);

        myBig.FindNameByIndex(i, BFE.path, sizeof(BFE.path));
        BFE.hash = BFE.CalculateHash(BFE.path);

        myBig.FindFileByIndex(i, &BFE.offset, &size, &zSize, &flags);
        BFE.fileSize = static_cast<uint32_t>(size);
        BFE.zSize = static_cast<uint32_t>(zSize);
        BFE.flags = static_cast<unsigned char>(flags);
        BFE.index = i;

        strncpy(BFE.srcpath, BFE.path, 255);
        BFE.srcpath[255] = 0;

        index.push_back(BFE);
    }

    free(headerbuf);
}

static int Create(char const *bigFileName, list<BigFileIndexEntry> const &fileList)
{
    DefaultFileStream strm;
    // create new bigfile or add and/or update files to existing bigfile
    int ret = strm.Open(bigFileName,
        AsyncOp::FOPEN_READWRITE |
        AsyncOp::FOPEN_CREATE |
        AsyncOp::FOPEN_OVERWRITE);

    if (ret != 0)
    {
        fprintf(stderr, "bigfile: unable to open bigfile %s for writing.\n",
            bigFileName);
        return -1;
    }

    int rv = CreateBigFile(strm, fileList);
    strm.Close();

    return rv;
}

static int Create(char const *bigFileName, vector<BigFileIndexEntry> &fileList)
{
    // Convert the vector to a list.
    list<BigFileIndexEntry> newList;
    newList.insert(newList.end(), fileList.begin(), fileList.end());

    return Create(bigFileName, newList);
}

bool OpenReadOnly(DefaultFileStream &strm, char const *fileName)
{
    if (strm.Open(fileName, AsyncOp::FOPEN_READONLY) != 0)
    {
        fprintf(stderr, "bigfile: unable to open file %s for reading.\n", fileName);
        return false;
    }
    return true;
}

static void EntryPrinter4(BigFileIndexEntry const &entry)
{
    printf("%08X %12llu %10u %10u %s\n", 
            static_cast<uint32_t>(entry.hash), 
            entry.offset,
            static_cast<uint32_t>(entry.fileSize), 
            static_cast<uint32_t>(entry.zSize), 
            static_cast<const char*>(entry.path));
}

static void EntryPrinter8(BigFileIndexEntry const &entry)
{
    printf("%16llx %12llu %10u %10u %s\n", 
            entry.hash, 
            entry.offset, 
            static_cast<uint32_t>(entry.fileSize), 
            static_cast<uint32_t>(entry.zSize), 
            static_cast<const char*>(entry.path));
}

static bool List(DefaultFileStream &, BigFileIndexEntry const &entry, EntryPrinter EP)
{
    EP(entry);
    return true;
}

static void InitializeDecompressor()
{
    if (gJobScheduler != NULL )
        return;
    rw::core::filesys::Options opts(&gMallocAllocator);
    rw::core::filesys::Manager::CreateInstance(opts);
    EA::Jobs::SetAllocator(gMallocAllocator);
    gJobScheduler = new EA::Jobs::JobScheduler;
    gJobScheduler->Initialize();
    EA::Jobs::StartWithDefaultThreads(*gJobScheduler);

    gChunkDecompressor = new rw::core::codec::BufferedChunkDecompressor(&gMallocAllocator, gJobScheduler);
}

static bool Extract(DefaultFileStream &inputStream, BigFileIndexEntry const &entry, EntryPrinter EP)
{
    List(inputStream, entry, EP);

    // Read the correct number of bytes depending on whether the file is compressed or not.
    uint32_t bytesToRead = entry.zSize > 0? entry.zSize : entry.fileSize;

    void *compressedReadBuffer = gMallocAllocator.Alloc(static_cast<size_t>(bytesToRead), NULL, 0);
    void *decompressedBuffer = gMallocAllocator.Alloc(static_cast<size_t>(entry.fileSize), NULL, 0);

    inputStream.Seek(entry.offset, IFileStream::SEEKORIGIN_BEGINNING);

    uint64_t bytesRead = inputStream.Read(compressedReadBuffer, static_cast<size_t>(bytesToRead));
    if (bytesRead != static_cast<uint64_t>(bytesToRead))
        return false;

    switch (entry.flags)
    {
        case rw::core:: BIGFILE_FORMAT_FLAG_COMPRESSION_TYPE_NONE:
            assert(entry.zSize == 0);
            memcpy(decompressedBuffer, compressedReadBuffer, entry.fileSize);
            break;
        case rw::core::BIGFILE_FORMAT_FLAG_COMPRESSION_TYPE_REFPACK:
            {
                rw::core::codec::RefpackBuf refpackInflater;
                refpackInflater.Decompress(decompressedBuffer, compressedReadBuffer);
            }
            break;
        case rw::core::BIGFILE_FORMAT_FLAG_COMPRESSION_TYPE_REFPACK_CHUNKED:
        case rw::core::BIGFILE_FORMAT_FLAG_COMPRESSION_TYPE_ZLIB_CHUNKED:
        case rw::core::BIGFILE_FORMAT_FLAG_COMPRESSION_TYPE_LZX:
            gChunkDecompressor->Decompress(decompressedBuffer, entry.fileSize, compressedReadBuffer, entry.zSize);
            break;
        default:
            fprintf(stderr, "bigfile: unrecognized flags in file %s", entry.path);
            return false;
    }

    DefaultFileStream outputFile;
    int ret = outputFile.Open(entry.path, AsyncOp::FOPEN_READWRITE | AsyncOp::FOPEN_CREATE | AsyncOp::FOPEN_OVERWRITE);
    if (ret != 0)
    {
        fprintf(stderr, "Failed to open file %s for writing.\n", entry.path);
        return false;
    }

    outputFile.Write(decompressedBuffer, static_cast<size_t>(entry.fileSize));
    outputFile.Close();

    gMallocAllocator.Free(compressedReadBuffer, 0);
    gMallocAllocator.Free(decompressedBuffer, 0);

    return true;
}

void GenerateCorrectPath(BigFileIndexEntry& entry)
{
    // We want to recreate the path/name format. Split the "path" into a real path and a real name.
    // This is because GetList puts the path/name into the path field.
    // Create needs more information than ListOrExtract, therefore this manipulation must be done.
            
    string path;

    memset(entry.name, 0, 256);

    char *name = strrchr(entry.path, '/');
    bool havePath = false;

    if (name != NULL)
    {
        ++name;
        havePath = true;
    }
    else
    {
        name = entry.path;
    }

    entry.nameLen = strlen(name);
    memcpy(entry.name, name, entry.nameLen + 1);

    if (havePath)
    {
        char *path = entry.path;
        entry.pathLen = name - path;
        entry.path[entry.pathLen - 1] = '\0';
    }
    else
    {
        strcpy(entry.path, ".");
        entry.pathLen = 1;
    }
}

// This method checks for duplicates during merge.
// Returns true if duplicates exists, false otherwise
bool NameDuplicatesExist(vector<BigFileIndexEntry> fileList)
{
    // Convert the list to a sorted file list. This will allow us to find 
    // duplicates faster in practice. However in theory this is actually 
    // slower than brute force. (Quicksort(average case) + Check = O(n*n*log n), Brute Force = O(n*n)
    // This will not matter if the fileList is smaller than 1000

    quick_sort(fileList.begin(), fileList.end(), SortByPath);

    for (vector<BigFileIndexEntry>::iterator I = fileList.begin(); I != fileList.end(); I++)
    {
        vector<BigFileIndexEntry>::iterator X = I;
        X++;

        if (X == fileList.end())
            break;

        // Duplicates are not case-sensitive.

        if (EA::StdC::Stricmp((*I).path, (*X).path) == 0)
        {
            fprintf(stderr, "Two files were found with the same path: %s and %s\n", (*I).path, (*X).path);
            return true;
        }
    }

    return false;
}

int Merge(char const *bigFileName, list<BigFileIndexEntry> const &fileList)
{
    // Create an array that holds the file list.
    gMergeFileList = new char*[fileList.size()];

    int indexIntoArray = 0;
    for (list<BigFileIndexEntry>::const_iterator X = fileList.begin(); X != fileList.end(); X++ )
    {
        gMergeFileList[indexIntoArray] = const_cast<char*>((*X).srcpath);
        indexIntoArray++;
    }

    vector<BigFileIndexEntry> globalIndex;
    
    int i = 0; // The bigfile index in fileList
    // Create a new file list that contain all the files in all the bigfiles.
    for (list<BigFileIndexEntry>::const_iterator I = fileList.begin(); I != fileList.end(); I++)
    {
        vector<BigFileIndexEntry> localIndex;
        // Get the file list of each bigfile.
        const char* bigFileLocation = static_cast<const char*>((*I).srcpath);

        uint16_t headerVersion = 0;
        int hashFieldSize = 0;

        // Read the bigfile into memory
        DefaultFileStream stream;
        if (!OpenReadOnly(stream, bigFileLocation))
            return -1;

        GetList(stream, localIndex, headerVersion, hashFieldSize);

        // Make sure all entries point to this file.
        for (vector<BigFileIndexEntry>::iterator J = localIndex.begin(); J != localIndex.end(); J++)
        {
            BigFileIndexEntry& entry = *J;
            entry.mergeBigfileIndex = i;
        }

        // Add the local index to the global index.
        globalIndex.insert(globalIndex.end(), localIndex.begin(), localIndex.end());
        
        stream.Close();

        i++;
    }

    // Check for duplicates.
    if (NameDuplicatesExist(globalIndex))
        return -1;
    
    // Set the correct path.
    for (vector<BigFileIndexEntry>::iterator it = globalIndex.begin(); it != globalIndex.end(); it++)
    {
        GenerateCorrectPath(*it);
    }

    // Create the bigfile.
    if (Create(bigFileName, globalIndex) == -1)
        return -1;

    delete [] gMergeFileList;

    return ERROR_SUCCESS;
}


template<typename T>
static int ListOrExtract(char const *bigFileName, T fileOperation)
{
    DefaultFileStream strm;
    if (!OpenReadOnly(strm, bigFileName))
        return -1;

    vector<BigFileIndexEntry> index;
    uint16_t headerVersion = 0;
    int hashFieldSize = 0;
    int rv = ERROR_SUCCESS;

    GetList(strm, index, headerVersion, hashFieldSize);
    quick_sort(index.begin(), index.end(), SortByOffset);

    if (hashFieldSize == 4)
        printf("Hash-32        Offset       Size     Packed File name\n");
    else
        printf("Hash-64                Offset       Size     Packed File name\n");

    EntryPrinter EP = hashFieldSize == 4 ? EntryPrinter4 : EntryPrinter8;

    for (vector<BigFileIndexEntry>::const_iterator I = index.begin(), E = index.end();
            I != E; ++I)
        if (!fileOperation(strm, *I, EP))
        {
            rv = -1;
            break;
        }

    strm.Close();
    return rv;
}

int main(int argc, char *argv[])
{
    int retVal = ERROR_SUCCESS;
    list<BigFileIndexEntry> fileList;
    int bigFileArgumentIndex = -1;
    char bigFileName[256];

    gpSpitInst = new SpitImpl;
    InitializeDecompressor();

    SPIT_init(NULL, SpitImpl::sUsage, SpitImpl::sOpt, SpitImpl::sHistory);

    // We do not call the spit parser in a loop, which means that you can
    // only operate on a single bigfile with each invocation of the tool
    // program.  If you want to operate on another bigfile, call the eabig
    // program again.
    SPIT_parse(argc, argv, SpitImpl::ToolName, SpitImpl::ToolVersion);

    if (SPIThelpgiven || SPITargnum == 0)
        return -1;

    // Put all the file names on a list
    // (excluding the first one, which is the bigfile)
    for (int i=0; i<SPITargnum; ++i)
    {
        // if this argument is an option, just move onto the next one
        if (SPITsargv[i][0]=='-' || SPITsargv[i][0]=='+' ||
            (SPITsargv[i][0]==OPTdot && SPITsargv[i][1]!='.' && SPITsargv[i][1]!='/' && SPITsargv[i][1]!='\\'))
        {
            if (Zop(SPITsargv[i]+1) != NULL)
                continue;
        }
        else if (SPITsargv[i][0]=='=')
        {
            if (Zop(SPITsargv[i]) != NULL)
                continue;
        }

        // the first non-option argument is taken as the name of the
        // bigfile.  All other non-option arguments are member files.
        if (bigFileArgumentIndex<0)
        {
            bigFileArgumentIndex = i;
            strcpy(bigFileName, SPITsargv[i]);
            continue;
        }

        if (!strcmp(bigFileName, SPITsargv[i]))
        {
            // if this SPIT argument is the same as the bigfile
            // name, skip it.  This can happen when the output bigfile
            // exists and you pass an wildcarded argument that includes the
            // bigfile (e.g. like "bigfile.exe out.big *.*"  SPIT will
            // make one argument called "out.big" from the command-line,
            // and a second when the wildcard is expanded
            continue;
        }

        // all remaining non-option arguments are files to be added
        const char *sourceFilename = SPITsargv[i];
        const char *destinationFilename = gpSpitInst->UseSourcePaths() ? SPITsargv[i] : SPITdargv[i];
        
        if (gpSpitInst->GetLowercaseNames())
        {
            strlwr((char*)destinationFilename);
        }
        else if (gpSpitInst->GetUppercaseNames())
        {
            strupr((char*)destinationFilename);
        }

        // Add roots to all paths

        fileList.push_back(BigFileIndexEntry(sourceFilename, destinationFilename));
    }

    if (bigFileArgumentIndex == -1)
    {
        fprintf(stderr, "bigfile: a bigfile was not specified");
    }

    if (gpSpitInst->GetMainCommand() == SpitImpl::CREATE && fileList.size() == 0)
    {
        // No files were specified to be added, so we'll change the
        // main command to list the current set of files.
        gpSpitInst->SetMainCommand(SpitImpl::LIST);
    }

    switch (gpSpitInst->GetMainCommand())
    {
        case SpitImpl::CREATE:
            retVal = Create(bigFileName, fileList);
            break;
        case SpitImpl::MERGE:
            retVal = Merge(bigFileName, fileList);
            break;
        case SpitImpl::LIST:
            retVal = ListOrExtract(bigFileName, List);
            break;
        case SpitImpl::EXTRACT:
            retVal = ListOrExtract(bigFileName, Extract);
            break;
        default:
            assert(0 && "Unhandled case");
            break;
    }

    fileList.clear();
    delete gpSpitInst;
    gpSpitInst = NULL;

    return retVal;
}

