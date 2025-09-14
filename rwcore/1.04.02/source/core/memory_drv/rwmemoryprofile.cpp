// (c) Electronic Arts. All Rights Reserved.

/// \file
/// Memory profiling functions

/*************************************************************************************************************
 Includes
 */
#if defined(RWMEMORYPROFILE)

#include "rw/core/memory.h"
#include "rw/core/memoryaligned.h"
#include "rw/core/memoryprofile.h"
#include "eathread/eathread_spinlock.h"

#endif // defined(RWMEMORYPROFILE)

#include "rw/core/memoryprofile.h"


// ***********************************************************************************************************
// Defines
#define STACKDEPTH_MAX 32

#if defined(RWMEMORYPROFILE)

#define AUTO_CRITICAL_SECTION    EA::Thread::AutoSpinLock lock(g_criticalSection)

namespace rw
{
namespace internal
{
namespace memory
{

// ***********************************************************************************************************
// Typedefs


// ***********************************************************************************************************
// Enums + Consts


// ***********************************************************************************************************
// Structs + Unions + Classes
struct StackEntry
{
    StackEntry() :
        m_numMallocs(0),
        m_numReallocs(0),
        m_numFrees(0),
        m_num0Frees(0),
        m_smallestAllocated(0x7FFFFFFF),
        m_largestAllocated(0),
        m_currentAllocated(0),
        m_peakAllocated(0),
        m_numAlignedMallocs(0),
        m_numAlignedReallocs(0),
        m_numAlignedFrees(0),
        m_numAligned0Frees(0),
        m_smallestAlignedAllocated(0x7FFFFFFF),
        m_largestAlignedAllocated(0),
        m_currentAlignedAllocated(0),
        m_peakAlignedAllocated(0),
        m_current(0),
        m_peak(0)
    {
        rwFUNCTION();
        rwRETURNVOID();
    };

    int32_t m_numMallocs;
    int32_t m_numReallocs;
    int32_t m_numFrees;
    int32_t m_num0Frees;
    int32_t m_smallestAllocated;
    int32_t m_largestAllocated;
    int32_t m_currentAllocated;
    int32_t m_peakAllocated;

    int32_t m_numAlignedMallocs;
    int32_t m_numAlignedReallocs;
    int32_t m_numAlignedFrees;
    int32_t m_numAligned0Frees;
    int32_t m_smallestAlignedAllocated;
    int32_t m_largestAlignedAllocated;
    int32_t m_currentAlignedAllocated;
    int32_t m_peakAlignedAllocated;

    int32_t m_current;
    int32_t m_peak;
};


// ***********************************************************************************************************
// Static Variables
namespace
{

EA::Thread::SpinLock g_criticalSection;

StackEntry g_dataStack[STACKDEPTH_MAX];
int32_t g_dataStackDepth = int32_t(0);
bool g_disable(false);

}

// ***********************************************************************************************************
// Static Functions


// ***********************************************************************************************************
// External Functions

void
PushStack()
{
    rwFUNCTION();

    if (!g_disable)
    {
        AUTO_CRITICAL_SECTION;

        ++g_dataStackDepth;
    }

    rwRETURNVOID();
}

void
PopStack()
{
    rwFUNCTION();

    if (!g_disable)
    {
        AUTO_CRITICAL_SECTION;

        if (g_dataStackDepth > 0)
        {
            StackEntry &parentStackEntry = g_dataStack[g_dataStackDepth - 1];
            StackEntry &currentEntry = g_dataStack[g_dataStackDepth];

            parentStackEntry.m_numMallocs  += currentEntry.m_numMallocs;
            parentStackEntry.m_numReallocs += currentEntry.m_numReallocs;
            parentStackEntry.m_numFrees    += currentEntry.m_numFrees;
            parentStackEntry.m_num0Frees   += currentEntry.m_num0Frees;

            if (currentEntry.m_smallestAllocated < parentStackEntry.m_smallestAllocated)
            {
                parentStackEntry.m_smallestAllocated = currentEntry.m_smallestAllocated;
            }

            if (currentEntry.m_largestAllocated > parentStackEntry.m_largestAllocated)
            {
                parentStackEntry.m_largestAllocated = currentEntry.m_largestAllocated;
            }

            parentStackEntry.m_currentAllocated += currentEntry.m_currentAllocated;

            if (currentEntry.m_peakAllocated > parentStackEntry.m_peakAllocated)
            {
                parentStackEntry.m_peakAllocated = currentEntry.m_peakAllocated;
            }

            if (parentStackEntry.m_currentAllocated > parentStackEntry.m_peakAllocated)
            {
                parentStackEntry.m_peakAllocated = parentStackEntry.m_currentAllocated;
            }

            parentStackEntry.m_numAlignedMallocs  += currentEntry.m_numAlignedMallocs;
            parentStackEntry.m_numAlignedReallocs += currentEntry.m_numAlignedReallocs;
            parentStackEntry.m_numAlignedFrees    += currentEntry.m_numAlignedFrees;
            parentStackEntry.m_numAligned0Frees   += currentEntry.m_numAligned0Frees;

            if (currentEntry.m_smallestAlignedAllocated < parentStackEntry.m_smallestAlignedAllocated)
            {
                parentStackEntry.m_smallestAlignedAllocated = currentEntry.m_smallestAlignedAllocated;
            }

            if (currentEntry.m_largestAlignedAllocated > parentStackEntry.m_largestAlignedAllocated)
            {
                parentStackEntry.m_largestAlignedAllocated = currentEntry.m_largestAlignedAllocated;
            }

            parentStackEntry.m_currentAlignedAllocated += currentEntry.m_currentAlignedAllocated;

            if (currentEntry.m_peakAlignedAllocated > parentStackEntry.m_peakAlignedAllocated)
            {
                parentStackEntry.m_peakAlignedAllocated = currentEntry.m_peakAlignedAllocated;
            }

            if (parentStackEntry.m_currentAlignedAllocated > parentStackEntry.m_peakAlignedAllocated)
            {
                parentStackEntry.m_peakAlignedAllocated = parentStackEntry.m_currentAlignedAllocated;
            }

            parentStackEntry.m_current += currentEntry.m_current;

            if (currentEntry.m_peak > parentStackEntry.m_peak)
            {
                parentStackEntry.m_peak = currentEntry.m_peak;
            }

            if (parentStackEntry.m_current > parentStackEntry.m_peak)
            {
                parentStackEntry.m_peak = parentStackEntry.m_current;
            }

            currentEntry.m_numMallocs = 0;
            currentEntry.m_numReallocs = 0;
            currentEntry.m_numFrees = 0;
            currentEntry.m_num0Frees = 0;
            currentEntry.m_smallestAllocated = 0x7FFFFFFF;
            currentEntry.m_largestAllocated = 0;
            currentEntry.m_currentAllocated = 0;
            currentEntry.m_peakAllocated = 0;
            currentEntry.m_numAlignedMallocs = 0;
            currentEntry.m_numAlignedReallocs = 0;
            currentEntry.m_numAlignedFrees = 0;
            currentEntry.m_numAligned0Frees = 0;
            currentEntry.m_smallestAlignedAllocated = 0x7FFFFFFF;
            currentEntry.m_largestAlignedAllocated = 0;
            currentEntry.m_currentAlignedAllocated = 0;
            currentEntry.m_peakAlignedAllocated = 0;

            currentEntry.m_current = 0;
            currentEntry.m_peak = 0;

            --g_dataStackDepth;
        }
    }

    rwRETURNVOID();
}

void
Write(const RwChar *string)
{
    rwFUNCTION();

    if (!g_disable)
    {
        AUTO_CRITICAL_SECTION;

        const StackEntry &stackEntry = g_dataStack[g_dataStackDepth];

        if (string)
        {
            fprintf(stdout, "%s\n", string);
        }

        fprintf(stdout, "UNALIGNED\n");
        fprintf(stdout, "mallocs:  %10d\n", stackEntry.m_numMallocs);
        fprintf(stdout, "reallocs: %10d\n", stackEntry.m_numReallocs);
        fprintf(stdout, "frees:    %10d\n", stackEntry.m_numFrees);
        fprintf(stdout, "0 frees:  %10d\n", stackEntry.m_num0Frees);

        if (0x7FFFFFFF == stackEntry.m_smallestAllocated)
        {
            fprintf(stdout, "smallest: %10d\n", 0);
        }
        else
        {
            fprintf(stdout, "smallest: %10d\n", stackEntry.m_smallestAllocated);
        }

        fprintf(stdout, "largest:  %10d\n", stackEntry.m_largestAllocated);
        fprintf(stdout, "current:  %10d\n", stackEntry.m_currentAllocated);
        fprintf(stdout, "peak:     %10d\n", stackEntry.m_peakAllocated);
        fprintf(stdout, "\n");

        fprintf(stdout, "ALIGNED\n");
        fprintf(stdout, "mallocs:  %10d\n", stackEntry.m_numAlignedMallocs);
        fprintf(stdout, "reallocs: %10d\n", stackEntry.m_numAlignedReallocs);
        fprintf(stdout, "frees:    %10d\n", stackEntry.m_numAlignedFrees);
        fprintf(stdout, "0 frees:  %10d\n", stackEntry.m_numAligned0Frees);

        if (0x7FFFFFFF == stackEntry.m_smallestAlignedAllocated)
        {
            fprintf(stdout, "smallest: %10d\n", 0);
        }
        else
        {
            fprintf(stdout, "smallest: %10d\n", stackEntry.m_smallestAlignedAllocated);
        }

        fprintf(stdout, "largest:  %10d\n", stackEntry.m_largestAlignedAllocated);
        fprintf(stdout, "current:  %10d\n", stackEntry.m_currentAlignedAllocated);
        fprintf(stdout, "peak:     %10d\n", stackEntry.m_peakAlignedAllocated);
        fprintf(stdout, "\n");

        fprintf(stdout, "OTHER\n");
        fprintf(stdout, "current:  %10d\n", stackEntry.m_current);
        fprintf(stdout, "peak:     %10d\n", stackEntry.m_peak);
        fprintf(stdout, "\n");

        fflush(stdout);
    }

    rwRETURNVOID();
}

RWAPI void
Disable()
{
    rwFUNCTION();

    AUTO_CRITICAL_SECTION;

    g_disable = true;

    rwRETURNVOID();
}

void *
#if (defined(EA_DEBUG))
Malloc(size_t size, const RwChar *filename, int32_t line)
#else /* (defined(EA_DEBUG)) */
Malloc(size_t size)
#endif /* (defined(EA_DEBUG)) */
{
    rwFUNCTION();

    if (!g_disable)
    {
        AUTO_CRITICAL_SECTION;

        g_dataStack[g_dataStackDepth].m_numMallocs++;

        if (size < (size_t)g_dataStack[g_dataStackDepth].m_smallestAllocated)
        {
            g_dataStack[g_dataStackDepth].m_smallestAllocated = size;
        }

        if (size > (size_t)g_dataStack[g_dataStackDepth].m_largestAllocated)
        {
            g_dataStack[g_dataStackDepth].m_largestAllocated = size;
        }

        g_dataStack[g_dataStackDepth].m_current += size;

        if (g_dataStack[g_dataStackDepth].m_current > g_dataStack[g_dataStackDepth].m_peak)
        {
            g_dataStack[g_dataStackDepth].m_peak = g_dataStack[g_dataStackDepth].m_current;
        }

        g_dataStack[g_dataStackDepth].m_currentAllocated += size;

        if (g_dataStack[g_dataStackDepth].m_currentAllocated > g_dataStack[g_dataStackDepth].m_peakAllocated)
        {
            g_dataStack[g_dataStackDepth].m_peakAllocated = g_dataStack[g_dataStackDepth].m_currentAllocated;
        }
    }

    size_t tmpSize = size + sizeof(void*) + sizeof(size_t);

#if (defined(EA_DEBUG))
    void *newMem = RwMallocGeneric(tmpSize, filename, line);
#else /* (defined(EA_DEBUG)) */
    void *newMem = RwMallocGeneric(tmpSize);
#endif /* (defined(EA_DEBUG)) */

    void *retMem = (void*)((uint8_t*)newMem + sizeof(void*) + sizeof(size_t));

    /* Store base address */
    *((void **)retMem - 1) = newMem;

    /* Store size */
    *((size_t *)((void **)retMem - 1) - 1) = size;

    rwRETURN(retMem);
}

void *
#if (defined(EA_DEBUG))
Realloc(void *mem, size_t newSize, const RwChar *filename, int32_t line)
#else /* (defined(EA_DEBUG)) */
Realloc(void *mem, size_t newSize)
#endif /* (defined(EA_DEBUG)) */
{
    rwFUNCTION();

    if (!g_disable)
    {
        AUTO_CRITICAL_SECTION;

        g_dataStack[g_dataStackDepth].m_numReallocs++;

        size_t oldSize = 0;

        if (mem)
        {
            oldSize = *((size_t *)((void **)mem - 1) - 1);
        }

        if (newSize > oldSize)
        {
            g_dataStack[g_dataStackDepth].m_current += (newSize - oldSize);
            g_dataStack[g_dataStackDepth].m_currentAllocated += (newSize - oldSize);
        }
        else
        {
            g_dataStack[g_dataStackDepth].m_current -= (oldSize - newSize);
            g_dataStack[g_dataStackDepth].m_currentAllocated -= (oldSize - newSize);
        }

        if (g_dataStack[g_dataStackDepth].m_current > g_dataStack[g_dataStackDepth].m_peak)
        {
            g_dataStack[g_dataStackDepth].m_peak = g_dataStack[g_dataStackDepth].m_current;
        }

        if (g_dataStack[g_dataStackDepth].m_currentAllocated > g_dataStack[g_dataStackDepth].m_peakAllocated)
        {
            g_dataStack[g_dataStackDepth].m_peakAllocated = g_dataStack[g_dataStackDepth].m_currentAllocated;
        }
    }

    void *base = 0;
    size_t padSize = 0;

    if (mem)
    {
        base = *((void **)mem - 1);

        if (newSize != 0)
        {
            // realloc is being used to resize memory
            padSize = sizeof(void*) + sizeof(size_t);
        }
    }
    else
    {
        // realloc is being used to allocate memory
        padSize = sizeof(void*) + sizeof(size_t);
    }

#if (defined(EA_DEBUG))
    void *newMem = RwReallocGeneric(base, newSize + padSize, filename, line);
#else /* (defined(EA_DEBUG)) */
    void *newMem = RwReallocGeneric(base, newSize + padSize);
#endif /* (defined(EA_DEBUG)) */

    void *retMem = 0;

    if (newMem)
    {
        retMem = (void*)((uint8_t*)newMem + padSize);

        /* Store base address */
        *((void **)retMem - 1) = newMem;

        /* Store size */
        *((size_t *)((void **)retMem - 1) - 1) = newSize;
    }

    rwRETURN(retMem);
}

void
Free(void *mem)
{
    rwFUNCTION();

    if (mem)
    {
        if (!g_disable)
        {
            size_t size = *((size_t *)((void **)mem - 1) - 1);

            AUTO_CRITICAL_SECTION;

            g_dataStack[g_dataStackDepth].m_numFrees++;
            g_dataStack[g_dataStackDepth].m_current -= size;
            g_dataStack[g_dataStackDepth].m_currentAllocated -= size;
        }

        void *base = *((void **)mem - 1);

        RwFreeGeneric(base);
    }
    else
    {
        if (!g_disable)
        {
            AUTO_CRITICAL_SECTION;

            g_dataStack[g_dataStackDepth].m_num0Frees++;
        }

        RwFreeGeneric(mem);
    }

    rwRETURNVOID();
}

void *
#if (defined(EA_DEBUG))
MallocAligned(size_t size, size_t alignment, const RwChar *filename, int32_t line)
#else /* (defined(EA_DEBUG)) */
MallocAligned(size_t size, size_t alignment)
#endif /* (defined(EA_DEBUG)) */
{
    rwFUNCTION();

    if (!g_disable)
    {
        AUTO_CRITICAL_SECTION;

        g_dataStack[g_dataStackDepth].m_numAlignedMallocs++;

        if (size < (size_t)g_dataStack[g_dataStackDepth].m_smallestAlignedAllocated)
        {
            g_dataStack[g_dataStackDepth].m_smallestAlignedAllocated = size;
        }

        if (size > (size_t)g_dataStack[g_dataStackDepth].m_largestAlignedAllocated)
        {
            g_dataStack[g_dataStackDepth].m_largestAlignedAllocated = size;
        }

        g_dataStack[g_dataStackDepth].m_current += size;

        if (g_dataStack[g_dataStackDepth].m_current > g_dataStack[g_dataStackDepth].m_peak)
        {
            g_dataStack[g_dataStackDepth].m_peak = g_dataStack[g_dataStackDepth].m_current;
        }

        g_dataStack[g_dataStackDepth].m_currentAlignedAllocated += size;

        if (g_dataStack[g_dataStackDepth].m_currentAlignedAllocated > g_dataStack[g_dataStackDepth].m_peakAlignedAllocated)
        {
            g_dataStack[g_dataStackDepth].m_peakAlignedAllocated = g_dataStack[g_dataStackDepth].m_currentAlignedAllocated;
        }
    }

    size_t padSize = RwRoundUp(sizeof(void*) + sizeof(size_t), alignment);

#if (defined(EA_DEBUG))
    void *newMem = RwMallocAlignedGeneric(size + padSize, alignment, filename, line);
#else /* (defined(EA_DEBUG)) */
    void *newMem = RwMallocAlignedGeneric(size + padSize, alignment);
#endif /* (defined(EA_DEBUG)) */

    void *retMem = (void*)((uint8_t*)newMem + padSize);

    /* Store base address */
    *((void **)retMem - 1) = newMem;

    /* Store size */
    *((size_t *)((void **)retMem - 1) - 1) = size;

    rwRETURN(retMem);
}

void *
#if (defined(EA_DEBUG))
ReallocAligned(void *mem, size_t newSize, size_t alignment, const RwChar *filename, int32_t line)
#else /* (defined(EA_DEBUG)) */
ReallocAligned(void *mem, size_t alignment, size_t newSize)
#endif /* (defined(EA_DEBUG)) */
{
    rwFUNCTION();

    if (!g_disable)
    {
        AUTO_CRITICAL_SECTION;

        g_dataStack[g_dataStackDepth].m_numAlignedReallocs++;

        size_t oldSize = 0;

        if (mem)
        {
            oldSize = *((size_t *)((void **)mem - 1) - 1);
        }

        if (newSize > oldSize)
        {
            g_dataStack[g_dataStackDepth].m_current += (newSize - oldSize);
            g_dataStack[g_dataStackDepth].m_currentAlignedAllocated += (newSize - oldSize);
        }
        else
        {
            g_dataStack[g_dataStackDepth].m_current -= (oldSize - newSize);
            g_dataStack[g_dataStackDepth].m_currentAlignedAllocated -= (oldSize - newSize);
        }

        if (g_dataStack[g_dataStackDepth].m_current > g_dataStack[g_dataStackDepth].m_peak)
        {
            g_dataStack[g_dataStackDepth].m_peak = g_dataStack[g_dataStackDepth].m_current;
        }

        if (g_dataStack[g_dataStackDepth].m_currentAlignedAllocated > g_dataStack[g_dataStackDepth].m_peakAlignedAllocated)
        {
            g_dataStack[g_dataStackDepth].m_peakAlignedAllocated = g_dataStack[g_dataStackDepth].m_currentAlignedAllocated;
        }
    }

    void *base = 0;
    size_t padSize = 0;

    if (mem)
    {
        base = *((void **)mem - 1);

        if (newSize != 0)
        {
            // realloc is being used to resize memory
            padSize = RwRoundUp(sizeof(void*) + sizeof(size_t), alignment);
        }
    }
    else
    {
        // realloc is being used to allocate memory
        padSize = RwRoundUp(sizeof(void*) + sizeof(size_t), alignment);
    }

#if (defined(EA_DEBUG))
    void *newMem = RwReallocAlignedGeneric(base, newSize + padSize, alignment, filename, line);
#else /* (defined(EA_DEBUG)) */
    void *newMem = RwReallocAlignedGeneric(base, newSize + padSize, alignment);
#endif /* (defined(EA_DEBUG)) */

    void *retMem = 0;

    if (newMem)
    {
        retMem = (void*)((uint8_t*)newMem + padSize);

        /* Store base address */
        *((void **)retMem - 1) = newMem;

        /* Store size */
        *((size_t *)((void **)retMem - 1) - 1) = newSize;
    }

    rwRETURN(retMem);
}

void
FreeAligned(void *mem)
{
    rwFUNCTION();

    if (mem)
    {
        if (!g_disable)
        {
            size_t size = *((size_t *)((void **)mem - 1) - 1);

            AUTO_CRITICAL_SECTION;

            g_dataStack[g_dataStackDepth].m_numAlignedFrees++;
            g_dataStack[g_dataStackDepth].m_current -= size;
            g_dataStack[g_dataStackDepth].m_currentAlignedAllocated -= size;
        }

        void *base = *((void **)mem - 1);

        RwFreeAlignedGeneric(base);
    }
    else
    {
        if (!g_disable)
        {
            AUTO_CRITICAL_SECTION;

            g_dataStack[g_dataStackDepth].m_numAligned0Frees++;
        }

        RwFreeAlignedGeneric(mem);
    }

    rwRETURNVOID();
}

} // namespace memory

} // namespace internal

} // namespace rw

#endif // defined(RWMEMORYPROFILE)
