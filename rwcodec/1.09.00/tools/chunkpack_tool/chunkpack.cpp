// Copyright (c) Electronic Arts Inc.

#include "EABase/eabase.h"

EA_DISABLE_VC_WARNING(4574 4548 4350)
#include <stdio.h>
#include <stdlib.h>

#if !defined(EA_PLATFORM_APPLE) && !defined(EA_PLATFORM_BSD)
    #include <malloc.h>
#endif

#include <vector>
#include <string>
EA_RESTORE_VC_WARNING()

#include "EAAssert/eaassert.h"
#include "coreallocator/icoreallocator_interface.h"
#include "job_manager/job_scheduler.h"
#include "rw/core/codec/chunkcodec.h"
#include "detail/decompressors.h"
#include "detail/packinfo.h"


#ifdef EA_PLATFORM_WINDOWS
#   ifndef WIN32_LEAN_AND_MEAN
#   define WIN32_LEAN_AND_MEAN
#   endif
#   include <Windows.h>

    inline int vsnprintf(char *str, size_t size, const char *format, va_list v)
    {
        return _vsnprintf(str, size, format, v);
    }

#else

    static inline void *_aligned_malloc(size_t size, size_t align)
    {
        void *mem;
        int rv = posix_memalign(&mem, static_cast<size_t>(align), size);
        EA_ASSERT(rv == 0);
        (void)rv;
        return mem;
    }

    static inline void _aligned_free(void *ptr)
    {
        free(ptr);
    }

#endif

using namespace rw::core::codec;
using namespace EA::Jobs;


class MallocICore : public EA::Allocator::ICoreAllocator
{
public:
    void *Alloc(size_t size, const char *name, unsigned int flags)
    {
        return Alloc(size, name, flags, sizeof(void *));
    }

    void Free(void *ptr, size_t)
    {
        _aligned_free(ptr);
    }

    virtual void *Alloc(size_t size, const char *, unsigned int, unsigned int align, unsigned int alignOffset = 0)
    {
        return _aligned_malloc(size, align);
    }
};


struct Codec
{
    const char *CodecName;
    uint32_t ChunkSize;
    bool Enabled;
    CHUNK_COMPRESSION_TYPE Type;
};


static Codec gCodecList[] = {
    { "refpack", 128 * 1024, false, CHUNK_COMPRESSION_TYPE_REFPACK },
    { "zlib", 128 * 1024, false, CHUNK_COMPRESSION_TYPE_ZLIB },
    { "lzx", 256 * 1024, false, CHUNK_COMPRESSION_TYPE_LZX },
    { "lzma", 64 * 1024, false, CHUNK_COMPRESSION_TYPE_LZMA },
    { "lz4", 128 * 1024, false, CHUNK_COMPRESSION_TYPE_LZ4 }
};
static const size_t gNumCodecs = EAArrayCount(gCodecList);


static bool gUncompress = false;
static bool gInPlace = false;
static std::vector<std::string> gInputFiles;
static std::vector<std::string> gOutputFiles;
static MallocICore gAllocatorInstance;


static std::pair<void *, size_t> ReadContentsOfFile(const std::string &inputFilename)
{
    FILE *fp = fopen(inputFilename.c_str(), "rb");
    if (!fp)
        return std::pair<void *, size_t>((void *)NULL, 0);

    fseek(fp, 0, SEEK_END);
    const size_t fileSize = static_cast<size_t>(ftell(fp));
    fseek(fp, 0, SEEK_SET);
    void * const mem = _aligned_malloc(fileSize, MINIMUM_CHUNK_ALIGNMENT);

    const bool readSucceeded = fread(mem, 1, fileSize, fp) == fileSize;
    fclose(fp);
    return readSucceeded ? std::make_pair(mem, fileSize) : std::pair<void *, size_t>((void *)NULL, 0); 
}


static bool WriteDataToFile(const std::string &outputFilename, std::pair<void *, size_t> dataSizePair)
{
    void *mem = dataSizePair.first;
    size_t size = dataSizePair.second;

    if (!mem || !size)
        return false;

    FILE *fp = fopen(outputFilename.c_str(), "wb");
    if (!fp)
        return false;

    const bool writeSucceeded = fwrite(mem, 1, size, fp) == size;
    fclose(fp);
    return writeSucceeded;
}


static std::pair<void *, size_t> Compress(std::pair<void *, size_t> inputData)
{
    std::pair<void *, size_t> data((void *)NULL, ~static_cast<size_t>(0));

    for (size_t i = 0; i < gNumCodecs; ++i)
    {
        if (!gCodecList[i].Enabled)
            continue;

        const void * const inputBuffer = inputData.first;
        const size_t inputSize = inputData.second;
        const uint32_t chunkSize = gCodecList[i].ChunkSize;
        BufferedChunkCompressor BCC(&gAllocatorInstance, gCodecList[i].Type);
        BCC.SetBlockSize(chunkSize);

        // numChunks counts the initial file header as a chunk, plus the overflow if
        // the input size is not a multiple of chunk size.
        const size_t numChunks = inputSize / chunkSize + 2;
        const size_t outputBufferSize = ChunkReader::GetFileHeaderSize() 
            + numChunks * (ChunkReader::GetChunkInfoSize() + MINIMUM_CHUNK_ALIGNMENT)
            + inputSize;
        void * const outputBuffer = _aligned_malloc(outputBufferSize, MINIMUM_CHUNK_ALIGNMENT);

        size_t compressedSize = BCC.Compress(outputBuffer, outputBufferSize, inputBuffer, inputSize);

        if (compressedSize < data.second)
        {
            _aligned_free(data.first);
            data = std::make_pair(outputBuffer, compressedSize);
        }
    }

    return data;
}


static std::pair<void *, size_t> Uncompress(std::pair<void *, size_t> inputData, JobScheduler& jobScheduler)
{
    std::pair<void *, size_t> data((void *)NULL, 0);

    BufferedChunkDecompressor BCD(&gAllocatorInstance, &jobScheduler);
    size_t size = BCD.GetDecompressedSize(inputData.first, inputData.second);
    void *mem = _aligned_malloc(size, MINIMUM_CHUNK_ALIGNMENT);
    BCD.DecompressAsync(mem, size, inputData.first, inputData.second);
    BCD.SleepOn();

    return std::make_pair(mem, size);
}


static bool PerformChunkpackOperation(const std::string &inputFilename, const std::string &outputFilename, JobScheduler& jobScheduler)
{
    std::pair<void *, size_t> inputData = ReadContentsOfFile(inputFilename);
    if (!inputData.first)
        return false;

    std::pair<void *, size_t> data = gUncompress ? Uncompress(inputData, jobScheduler) : Compress(inputData);

    if (!WriteDataToFile(outputFilename, data))
        return false;

    printf("[chunkpack] Input file '%s' (%lu bytes) %s to output file '%s' (%lu bytes)\n",
            inputFilename.c_str(), 
            inputData.second,
            gUncompress ? "uncompressed" : "compressed",
            outputFilename.c_str(), 
            data.second);

    _aligned_free(inputData.first);
    _aligned_free(data.first);

    return true;
}


static void PrintUsage()
{
    fprintf(stderr, 
"Usage: chunkpack [OPTION]... [inputfile1] [outputfile1]...\n"
"\n"
"Compression options:\n");

    for (size_t i = 0; i < gNumCodecs; ++i)
        fprintf(stderr, "  -%-16s compress using %s\n", gCodecList[i].CodecName, gCodecList[i].CodecName);

    fprintf(stderr, "\n");

    for (size_t i = 0; i < gNumCodecs; ++i) 
    {
        char buf[64];
        sprintf(buf, "%schunksize X", gCodecList[i].CodecName);
        fprintf(stderr, "  -%-16s set the %s chunk size to X kb\n", buf, gCodecList[i].CodecName);
    }

    fprintf(stderr, 
"\n"
"  -inplace          ignore output files and compress the file in-place\n"
"  -u                uncompress the input file into either the output file\n"
"                    specified, or in-place.\n"
"\n"
"Multiple compression options can be specified and chunkpack will\n"
"select the option that produces the best compression results.\n"
"\n"
"NOTE: LZX compressed data can only be decompressed on the Xbox360.\n"
"NOTE: LZMA compressed data cannot use a chunk size larger than\n"
"      64kb if decompression will be performed on the PS3 SPU.\n");
}


static Codec *FindCodec(const char *name)
{
    for (size_t i = 0; i < gNumCodecs; ++i)
    {
        size_t codecNameLength = strlen(gCodecList[i].CodecName);
        if (!memcmp(name, gCodecList[i].CodecName, codecNameLength))
            return gCodecList + i;
    }

    return NULL;
}


static bool ParseArgs(int argc, char **argv)
{
    bool codecSelected = false;

    for (int i = 1; i < argc; ++i)
    {
        char *arg = argv[i];
        if (arg[0] == '-') 
        {
            if (!strcmp(arg, "-u"))
            {
                gUncompress = true;
                continue;
            }

            if (!strcmp(arg, "-inplace"))
            {
                gInPlace = true;
                continue;
            }

            Codec *codec = FindCodec(arg + 1);
            if (!codec)
            {
                fprintf(stderr, "ERROR: Unable to determine chunk type!\n\n");
                return false;
            }

            if (strstr(arg, "chunksize"))
            {
                const uint32_t chunkSize = static_cast<uint32_t>(strtol(argv[++i], NULL, 0));
                if (!chunkSize)
                {
                    fprintf(stderr, "ERROR: Unable to determine chunk size!\n\n");
                    return false;
                }
                codec->ChunkSize = chunkSize * 1024;
            }
            else
            {
                codec->Enabled = true;
                codecSelected = true;
            }
            continue;
        }

        if (!gInPlace && i + 1 >= argc)
            return false;

        std::string inputFile(arg);
        std::string outputFile(gInPlace ? arg : argv[++i]);

        gInputFiles.push_back(inputFile);
        gOutputFiles.push_back(outputFile);
    }

    return codecSelected || gUncompress;
}


int main(int argc, char **argv)
{
    if (argc < 2 || !ParseArgs(argc, argv))
    {
        PrintUsage();
        return -1;
    }

    SetAllocator(gAllocatorInstance);
    JobScheduler::Parameters P;
    P.SetPriorityJobQueueSizeAll(128);
    JobScheduler jobScheduler;
    jobScheduler.Initialize(P);
    StartWithDefaultThreads(jobScheduler);

    EA_ASSERT(gInputFiles.size() == gOutputFiles.size());

    for (std::vector<std::string>::iterator i1 = gInputFiles.begin(),
            i2 = gOutputFiles.begin(),
            e1 = gInputFiles.end(),
            e2 = gOutputFiles.end(); 
            i1 != e1 && i2 != e2; 
            ++i1, ++i2)
    {
        if (!PerformChunkpackOperation(*i1, *i2, jobScheduler))
        {
            fprintf(stderr, "ERROR: Unable to perform %s operation on '%s'!", 
                    gUncompress ? "uncompress" : "compress",
                    i1->c_str());
            return -1;
        }
    }

    return 0;
}


