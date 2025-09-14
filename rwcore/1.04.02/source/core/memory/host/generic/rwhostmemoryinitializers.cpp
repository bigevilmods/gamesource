// (c) Electronic Arts. All Rights Reserved.

/// \file
/// Dynamic memory allocation automatic initializer static variable definitions.

/*************************************************************************************************************
 Includes
 */
#include "rwsharedheaders.hpp"

/* If you need to include a header for malloc & free then add it into rwdrvmemory.h */

#include "rw/core/memory/memoryfunctions.h"
#include "rw/core/memory/memoryalignedfunctions.h"

#if defined(RWMEMORYPROFILE)
#include "rwmemoryprofilefunctions.h"
#endif // defined(RWMEMORYPROFILE)

#include "rw/core/libmemory.h"
#include "rw/core/memory/memoryinitializers.h"

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
rw::internal::memory::Profile::PushStackFunc rw::internal::memory::Profile::m_pushStackFunc =
    rw::internal::memory::PushStack;

rw::internal::memory::Profile::PopStackFunc  rw::internal::memory::Profile::m_popStackFunc =
    rw::internal::memory::PopStack;

rw::internal::memory::Profile::WriteFunc     rw::internal::memory::Profile::m_writeFunc =
    rw::internal::memory::Write;

rw::internal::memory::Profile::DisableFunc   rw::internal::memory::Profile::m_disableFunc =
    rw::internal::memory::Disable;

namespace
{

bool WriteMemoryProfile = true;

}
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

/*************************************************************************************************************
 Internal Functions
 */

/*************************************************************************************************************
 External Functions
 */
#if defined(RWMEMORYPROFILE)
namespace
{

void
WriteGlobalMemoryProfile()
{
    rwFUNCTION();
    if (true == WriteMemoryProfile)
    {
        rw::internal::memory::Profile::m_writeFunc("ACP Memory - Global");
        WriteMemoryProfile = false;
    }
    rwRETURNVOID();
}

} // namespace
#endif // defined(RWMEMORYPROFILE)


namespace rw
{
namespace host
{

void
MemoryFunctionsInitialize()
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

void
MemoryFunctionsTerminate()
{
    rwFUNCTION();

#if defined(RWMEMORYPROFILE)
    WriteGlobalMemoryProfile();
#endif // defined(RWMEMORYPROFILE)

    RwMemSetMalloc(0);
    RwMemSetRealloc(0);
    RwMemSetFree(0);

    rwRETURNVOID();
}

void
MemoryAlignedFunctionsInitialize()
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

void
MemoryAlignedFunctionsTerminate()
{
    rwFUNCTION();

#if defined(RWMEMORYPROFILE)
    WriteGlobalMemoryProfile();
#endif // defined(RWMEMORYPROFILE)

    RwMemSetMallocAligned(0);
    RwMemSetReallocAligned(0);
    RwMemSetFreeAligned(0);

    rwRETURNVOID();
}

/**
Returns the initialization status of the memory allocation system
\return true if memory system initialized successfully, false if fallback internal allocators are being used.
*/
bool
MemorySystemInitialized()
{
    rwFUNCTION();
    rwRETURN(true);
}

} // namespace host

} // namespace rw
