// (c) 1998-2010 Electronic Arts Inc.
#include <eabase/eabase.h>
#include "ea3mem.h"
#include "PPMalloc/EAGeneralAllocatorDebug.h"
//#include "StompAllocator.h"

#define REPORT_MEMORY_LEAKS 0

#if REPORT_MEMORY_LEAKS
    #include "EACallstack/EAAddressRep.h"
#endif

typedef EA::Allocator::GeneralAllocatorDebug MemoryAllocator;
//typedef EA::Ant::Tool::Memory::StompAllocator MemoryAllocator;

static MemoryAllocator& Allocator();

extern "C" 
{
    EA3_EXPORT void EA3Free (void *ptr) 
    {
        if (ptr != NULL)
        {
            Allocator().Free(ptr);
        }
    }

    EA3_EXPORT void* EA3Malloc (size_t size) 
    {
        return Allocator().Malloc(size);
    }

    EA3_EXPORT void* EA3Realloc (void *ptr, size_t size) 
    {
        return Allocator().Realloc(ptr, size);
    }
}

#if 1 //def EA_DLL

static MemoryAllocator& Allocator()
{
    using namespace EA::Allocator;

#ifdef EA_DEBUG
    static MemoryAllocator allocator;
    static bool initialized = false;
    if (!initialized)
    {
		allocator.SetOption(GeneralAllocatorDebug::kOptionEnablePtrValidation, 1);
		allocator.SetDelayedFreePolicy(GeneralAllocatorDebug::kDelayedFreePolicyVolume, 896 * 1024);
		allocator.SetDefaultDebugDataFlag(GeneralAllocatorDebug::kDebugDataIdGuard);
		allocator.SetGuardSize(0.1f, 4, 64);

    #if REPORT_MEMORY_LEAKS
        allocator.SetDefaultDebugDataFlag(GeneralAllocatorDebug::kDebugDataIdCallStack);
        allocator.SetOption(GeneralAllocatorDebug::kOptionEnableMetrics, 1);
    #endif

        initialized = true;
    }
#else
    static MemoryAllocator allocator;
#endif

    return allocator;
};

// --> Memory snapshot output <------------------------------------------------

    EA3_EXPORT void ReportMemoryLeaks(const char* aFilename)
    {
        using namespace EA::Allocator;

    #if REPORT_MEMORY_LEAKS && defined(EA_DEBUG)
        GeneralAllocatorDebug& allocator = static_cast<GeneralAllocatorDebug&>(Allocator());
        
        const void* context = allocator.ReportBegin();
        if (context != NULL)
        {
            FILE* file = fopen(aFilename, "w");
            if (file != NULL)
            {
                EA::Callstack::AddressRepCache cache;
                cache.AddDatabaseFile(L"F:/dev/pc-vc-dll-dev-debug/miscellaneous/EA3_SDK_pc/3.29.08/pc-vc-dll-dev-debug/bin/EA3_LIB.pdb");
                cache.AddDatabaseFile(L"F:/dev/pc-vc-dll-dev-debug/miscellaneous/EA3_SDK_pc/3.29.08/pc-vc-dll-dev-debug/bin/EA3Util.pdb");
                cache.AddDatabaseFile(L"F:/dev/pc-vc-dll-dev-debug/miscellaneous/EA3_SDK_pc/3.29.08/pc-vc-dll-dev-debug/bin/ManagedEA3.pdb");
                cache.EnableAutoDatabaseFind(true);
                EA::Callstack::SetAddressRepCache(&cache);

                fprintf(file, "<MemorySnapshot>\n");
                const GeneralAllocator::BlockInfo* info;
                while ((info = allocator.ReportNext(context, GeneralAllocator::kBlockTypeAllocated)) != NULL)
                {
                    fprintf(file, "<Block bytes=\"%d\" usable=\"%d\" address=\"0x%08x\">\n", info->mnBlockSize, info->mnDataSize, info->mpBlock);
                    
                    void* pReturnAddressArray[24];
                    memset(pReturnAddressArray, 0, sizeof(pReturnAddressArray));
                    if (allocator.GetDebugData(info->mpData, GeneralAllocatorDebug::kDebugDataIdCallStack, &pReturnAddressArray, sizeof(pReturnAddressArray)))
                    {
                        for (int32_t i = 3; i < 24; ++i)
                        {
                            if (pReturnAddressArray[i] != 0)
                            {
                                const char*		pStrResults[EA::Callstack::kARTCount];
                                int				pIntResults[EA::Callstack::kARTCount];
                                int				inputFlags = (EA::Callstack::kARTFlagFunctionOffset | EA::Callstack::kARTFlagFileLine);
							    int				outputFlags = EA::Callstack::kARTFlagNone;

                                fprintf(file, "    <callstack%d address=\"%d\" ", i, pReturnAddressArray[i]);

                                outputFlags = cache.GetAddressRep(inputFlags, pReturnAddressArray[i], pStrResults, pIntResults);
							    if (outputFlags & EA::Callstack::kARTFlagFunctionOffset)
							    {
								    if (outputFlags & EA::Callstack::kARTFlagFileLine)
								    {
                                        fprintf(file, "file=\"%s\" line=\"%d\" ", 
											    pStrResults[EA::Callstack::kARTFileLine], 
											    pIntResults[EA::Callstack::kARTFileLine]);
								    }

                                    // make sure we dont start xml tags.
                                    char method[512];
                                    strncpy(method, pStrResults[EA::Callstack::kARTFunctionOffset], 511);
                                    int len = strlen(method);
                                    for (int i=0; i<len; i++)
                                    {
                                        if (method[i] == '<') method[i] = '[';
                                        if (method[i] == '>') method[i] = ']';
                                    }

								    fprintf(file, "method=\"%s\" offset=\"%d\" ", 
									    method, pIntResults[EA::Callstack::kARTFunctionOffset]);
							    }

                                fprintf(file, "/>\n");
                            }
                        }
                    }
                    fprintf(file, "</Block>\n");
                }
                allocator.ReportEnd(context);
                EA::Callstack::SetAddressRepCache(NULL);
                fprintf(file, "</MemorySnapshot>\n");
                fclose(file);
            }
        }
    #endif
    } 

#if 0
void* operator new(size_t size) 
{
	return Allocator().Malloc(size);
}

void* operator new[](size_t size) 
{
    return Allocator().Malloc(size);
}

void operator delete(void* block)
{
    return Allocator().Free(block);
}

void operator delete[](void* block)
{
    return Allocator().Free(block);
}
#endif

#else

static MemoryAllocator& Allocator()
{
    static MemoryAllocator allocator;
    return allocator;
};

#endif

#if 0
// these are needed for EASTL.
void* operator new(size_t size, char const*, int, unsigned int, char const *, int) 
{
    return Allocator().Malloc(size);
}

void*  operator new[](size_t size, char const *, int, unsigned int, char const *, int) 
{
    return Allocator().Malloc(size);
}

void* operator new(size_t size, size_t, size_t, char const *, int, unsigned int, char const *, int) 
{
    return Allocator().Malloc(size);
}

void* operator new[](size_t size, size_t, size_t, char const *, int, unsigned int, char const *, int) 
{
    return Allocator().Malloc(size);
}

void operator delete(void* block, char const *, int, unsigned int, char const *, int) 
{
    return Allocator().Free(block);
}

void operator delete[](void* block, char const *, int, unsigned int, char const *, int) 
{
    return Allocator().Free(block);
}

void operator delete(void* block, size_t, size_t, char const *, int, unsigned int, char const *, int) 
{
    return Allocator().Free(block);
}

void operator delete[](void* block, size_t, size_t, char const *, int, unsigned int, char const *, int) 
{
    return Allocator().Free(block);
}
#endif
