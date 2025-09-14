// (c) Electronic Arts. All Rights Reserved.

/// \file
/// Dynamic memory allocation automatic initializer static variable definitions.

/*************************************************************************************************************
 Includes
 */
#include "rwsharedheaders.hpp"
#if defined(RWCROSS)
#include <cstdlib>
#include <windows.h>
#endif // defined(RWCROSS)

#include "rw/core/memory/memoryfunctions.h"
#include "rw/core/memory/memoryalignedfunctions.h"

#include "rw/core/memory/memoryprofilefunctions.h"

#include "rw/core/memory.h"
#include "rw/core/drvmemory.h"
#include "rw/core/memoryaligned.h"
#include "rw/core/drvmemoryaligned.h"
#include "rw/core/memoryprofile.h"
#include "rw/core/sharedglobal/detail/EAOSGlobal.h"

/*************************************************************************************************************
 Defines
 */

/*************************************************************************************************************
 Enums
 */

/*************************************************************************************************************
 Typedef Enums
 */

/*************************************************************************************************************
 Typedef Structs + Unions
 */

/*************************************************************************************************************
 Typedef Function Pointers
 */

/*************************************************************************************************************
 Structs + Unions
 */

/*************************************************************************************************************
 Static Variables
 */

#if defined(RWMEMORYPROFILE)
rw::internal::memory::Profile::PushStackFunc    rw::internal::memory::Profile::m_pushStackFunc = 0;
rw::internal::memory::Profile::PopStackFunc     rw::internal::memory::Profile::m_popStackFunc = 0;
rw::internal::memory::Profile::WriteFunc        rw::internal::memory::Profile::m_writeFunc = 0;
rw::internal::memory::Profile::DisableFunc      rw::internal::memory::Profile::m_disableFunc = 0;
#endif // defined(RWMEMORYPROFILE)

/*************************************************************************************************************
 Global Variables
 */

/*************************************************************************************************************
 Function Prototypes
 */

/*************************************************************************************************************
 Static Functions
 */

namespace
{

void MemoryFunctionsInitializeBase()
{
    rwFUNCTION();

#if defined(RWMEMORYPROFILE)
    RwMemSetMalloc(rw::internal::memory::Malloc);
    RwMemSetRealloc(rw::internal::memory::Realloc);
    RwMemSetFree(rw::internal::memory::Free);
#else // defined(RWMEMORYPROFILE)
#if defined(EA_DEBUG)
    RwMemSetMalloc(RwMallocMemDebug);
    RwMemSetRealloc(RwReallocMemDebug);
    RwMemSetFree(RwFreeMemDebug);
#else // defined(EA_DEBUG)
    RwMemSetMalloc(RwMallocGeneric);
    RwMemSetRealloc(RwReallocGeneric);
    RwMemSetFree(RwFreeGeneric);
#endif // defined(EA_DEBUG))
#endif // defined(RWMEMORYPROFILE)
    rwRETURNVOID();
}


void MemoryAlignedFunctionsInitializeBase()
{
    rwFUNCTION();

#if defined(RWMEMORYPROFILE)
    RwMemSetMallocAligned(rw::internal::memory::MallocAligned);
    RwMemSetReallocAligned(rw::internal::memory::ReallocAligned);
    RwMemSetFreeAligned(rw::internal::memory::FreeAligned);
#else // defined(RWMEMORYPROFILE)
#if defined(EA_DEBUG)
    RwMemSetMallocAligned(RwMallocAlignedMemDebug);
    RwMemSetReallocAligned(RwReallocAlignedMemDebug);
    RwMemSetFreeAligned(RwFreeAlignedMemDebug);
#else // defined(EA_DEBUG)
    RwMemSetMallocAligned(RwMallocAlignedGeneric);
    RwMemSetReallocAligned(RwReallocAlignedGeneric);
    RwMemSetFreeAligned(RwFreeAlignedGeneric);
#endif // defined(EA_DEBUG))
#endif // defined(RWMEMORYPROFILE)

    rwRETURNVOID();
}

#if defined(RWCROSS)

// A structure to hold function pointers to the various
// memory allocation routines.
struct MemoryFunctionsData : public rw::shared_globals::internal::OSGlobalNode
{
    MemoryFunctionsData()
    {
        rwFUNCTION();
        m_memoryFunctions.rwmalloc = 0;
        m_memoryFunctions.rwrealloc = 0;
        m_memoryFunctions.rwfree = 0;
        m_memoryAlignedFunctions.rwMallocAligned = 0;
        m_memoryAlignedFunctions.rwReallocAligned = 0;
        m_memoryAlignedFunctions.rwFreeAligned = 0;
        m_pushStackFunc = 0;
        m_popStackFunc = 0;
        m_writeFunc = 0;
        m_disableFunc = 0;
        rwRETURNVOID();
    }
    RwMemoryFunctions                               m_memoryFunctions;
    RwMemoryAlignedFunctions                        m_memoryAlignedFunctions;
    rw::internal::memory::Profile::PushStackFunc    m_pushStackFunc;
    rw::internal::memory::Profile::PopStackFunc     m_popStackFunc;
    rw::internal::memory::Profile::WriteFunc        m_writeFunc;
    rw::internal::memory::Profile::DisableFunc      m_disableFunc;

};

// Structure that holds a pointer to the
// global MemoryFunctionsData block.
struct MemoryFunctionsHolder
{
    MemoryFunctionsHolder()
        : m_memoryFunctionsData(static_cast<MemoryFunctionsData *>(
                                GetOSGlobal(0xFFFFFFFF, CreateMemoryFunctionsData)))
    {
    }

    static rw::shared_globals::internal::OSGlobalNode *
    CreateMemoryFunctionsData()
    {
        return new(&m_dataBuffer) MemoryFunctionsData;
    }

    static char m_dataBuffer[sizeof(MemoryFunctionsData)];
    MemoryFunctionsData * m_memoryFunctionsData;
};

char MemoryFunctionsHolder::m_dataBuffer[sizeof(MemoryFunctionsData)];

// Returns a reference to the shared global that holds
// function pointers to the various memory allocation routines.
MemoryFunctionsData& GetGlobalMemoryFunctionsData()
{
    rwFUNCTION();
    static MemoryFunctionsHolder memoryFunctionsHolder;

    rwRETURN(*memoryFunctionsHolder.m_memoryFunctionsData);
}


#if defined(RWMEMORYPROFILE)
void
MemoryProfileFunctionsInitialize()
{
    rwFUNCTION();

    MemoryFunctionsData& memoryFunctionsData = GetGlobalMemoryFunctionsData();

    rw::internal::memory::Profile::PushStackFunc& pushStackFunc = memoryFunctionsData.m_pushStackFunc;
    rw::internal::memory::Profile::PopStackFunc& popStackFunc = memoryFunctionsData.m_popStackFunc;
    rw::internal::memory::Profile::WriteFunc& writeFunc = memoryFunctionsData.m_writeFunc;
    rw::internal::memory::Profile::DisableFunc& disableFunc = memoryFunctionsData.m_disableFunc;

    if ((pushStackFunc == 0) || (popStackFunc == 0) || (writeFunc == 0) || (disableFunc == 0))
    {
        // We need to hook up the profiling functions
        pushStackFunc = rw::internal::memory::PushStack;
        popStackFunc = rw::internal::memory::PopStack;
        writeFunc = rw::internal::memory::Write;
        disableFunc = rw::internal::memory::Disable;
    }

    rw::internal::memory::Profile::m_pushStackFunc = pushStackFunc;
    rw::internal::memory::Profile::m_popStackFunc = popStackFunc;
    rw::internal::memory::Profile::m_writeFunc = writeFunc;
    rw::internal::memory::Profile::m_disableFunc = disableFunc;
    rwRETURNVOID();
}
#endif // defined(RWMEMORYPROFILE)

#endif //defined(RWCROSS)

} // namespace

/*************************************************************************************************************
 Internal Functions
 */

namespace
{
#if defined(RWCROSS)

void
MemoryFunctionsInitializeCross()
{
    rwFUNCTION();
    
    MemoryFunctionsData& memoryFunctionsData = GetGlobalMemoryFunctionsData();
    if (memoryFunctionsData.m_memoryFunctions.rwmalloc == 0)
    {
        // We have not set up any memory functions yet/

        // Set up the memory functions as required
        MemoryFunctionsInitializeBase();

        // Copy functions to global
        memoryFunctionsData.m_memoryFunctions.rwmalloc = rwMemoryFunctions.rwmalloc;
        memoryFunctionsData.m_memoryFunctions.rwrealloc = rwMemoryFunctions.rwrealloc;
        memoryFunctionsData.m_memoryFunctions.rwfree = rwMemoryFunctions.rwfree;
    }
    else
    {
        // Copy functions from global
        rwMemoryFunctions.rwmalloc = memoryFunctionsData.m_memoryFunctions.rwmalloc;
        rwMemoryFunctions.rwrealloc = memoryFunctionsData.m_memoryFunctions.rwrealloc;
        rwMemoryFunctions.rwfree = memoryFunctionsData.m_memoryFunctions.rwfree;
    }

#if defined(RWMEMORYPROFILE)
    MemoryProfileFunctionsInitialize();
#endif // defined(RWMEMORYPROFILE)

    rwRETURNVOID();
}

#else // defined(RWCROSS)

// Initializes the memory allocation function table with pointers to internal allocation routines.
void
MemoryFunctionsInitializeNative()
{
    rwFUNCTION();

    MemoryFunctionsInitializeBase();

    rwRETURNVOID();
}

#endif // defined(RWCROSS)

#if defined(RWCROSS)

void
MemoryAlignedFunctionsInitializeCross()
{
    rwFUNCTION();
    
    MemoryFunctionsData& memoryFunctionsData = GetGlobalMemoryFunctionsData();
    if (memoryFunctionsData.m_memoryAlignedFunctions.rwMallocAligned == 0)
    {
        // We have not set up any memory functions yet/

        // Set up the memory functions as required
        MemoryAlignedFunctionsInitializeBase();

        // Copy functions to global
        memoryFunctionsData.m_memoryAlignedFunctions.rwMallocAligned = rwMemoryAlignedFunctions.rwMallocAligned;
        memoryFunctionsData.m_memoryAlignedFunctions.rwReallocAligned = rwMemoryAlignedFunctions.rwReallocAligned;
        memoryFunctionsData.m_memoryAlignedFunctions.rwFreeAligned = rwMemoryAlignedFunctions.rwFreeAligned;
    }
    else
    {
        // Copy functions from global
        rwMemoryAlignedFunctions.rwMallocAligned = memoryFunctionsData.m_memoryAlignedFunctions.rwMallocAligned;
        rwMemoryAlignedFunctions.rwReallocAligned = memoryFunctionsData.m_memoryAlignedFunctions.rwReallocAligned;
        rwMemoryAlignedFunctions.rwFreeAligned = memoryFunctionsData.m_memoryAlignedFunctions.rwFreeAligned;
    }

#if defined(RWMEMORYPROFILE)
    MemoryProfileFunctionsInitialize();
#endif // defined(RWMEMORYPROFILE)

    rwRETURNVOID();
}

#else // defined(RWCROSS)

// Initializes the memory allocation function table with pointers to internal allocation routines.
void
MemoryAlignedFunctionsInitializeNative()
{
    rwFUNCTION();

    MemoryAlignedFunctionsInitializeBase();

    rwRETURNVOID();
}

#endif // defined(RWCROSS)

} // anonymous namespace

/*************************************************************************************************************
 External Functions
 */
namespace rw
{
namespace host
{

void
MemoryFunctionsInitialize()
{
    rwFUNCTION();

#if defined(RWCROSS)
    MemoryFunctionsInitializeCross();
#else // defined(RWCROSS)
    MemoryFunctionsInitializeNative();
#endif // defined(RWCROSS)

    rwRETURNVOID();
}

void
MemoryFunctionsTerminate()
{
    rwFUNCTION();

    RwMemSetMalloc(0);
    RwMemSetRealloc(0);
    RwMemSetFree(0);

    rwRETURNVOID();
}

void
MemoryAlignedFunctionsInitialize()
{
    rwFUNCTION();

#if defined(RWCROSS)
    MemoryAlignedFunctionsInitializeCross();
#else // defined(RWCROSS)
    MemoryAlignedFunctionsInitializeNative();
#endif // defined(RWCROSS)

    rwRETURNVOID();
}

void
MemoryAlignedFunctionsTerminate()
{
    rwFUNCTION();

    RwMemSetMallocAligned(0);
    RwMemSetReallocAligned(0);
    RwMemSetFreeAligned(0);

    rwRETURNVOID();
}

/**
Returns the initialization status of the memory allocation system
\return true if memory system initialized successfully
*/
bool
MemorySystemInitialized()
{
    rwFUNCTION();
#if defined(RWCROSS)
    // Ensure global is set up.
    GetGlobalMemoryFunctionsData();
#endif // defined(RWCROSS)
    rwRETURN(true);
}

} // namespace host

} // namespace rw
