// (c) Electronic Arts. All Rights Reserved.

/// \file
/// Used for profiling memory.  To enable, define the macro RWMEMORYPROFILE

#ifndef PUBLIC_RW_CORE_MEMORYPROFILE_H
#define PUBLIC_RW_CORE_MEMORYPROFILE_H

// ***********************************************************************************************************
// Defines
#if defined(RWMEMORYPROFILE)

namespace rw
{
namespace internal
{
namespace memory
{

// ***********************************************************************************************************
// Forward Declarations


// ***********************************************************************************************************
// Typedefs


// ***********************************************************************************************************
// Enums + Consts


// ***********************************************************************************************************
// Global Variables


// ***********************************************************************************************************
// External Functions Prototypes


// ***********************************************************************************************************
// Structs + Unions + Classes


// ***********************************************************************************************************
//                                                Class Definition
// ***********************************************************************************************************

RWAPI void
PushStack();

RWAPI void
PopStack();

RWAPI void
Write(const RwChar *string);

RWAPI void
Disable();

RWAPI void *
#if (defined(EA_DEBUG))
Malloc(size_t size, const RwChar *filename, int32_t line);
#else /* (defined(EA_DEBUG)) */
Malloc(size_t size);
#endif /* (defined(EA_DEBUG)) */

RWAPI void *
#if (defined(EA_DEBUG))
Realloc(void *mem, size_t newSize, const RwChar *filename, int32_t line);
#else /* (defined(EA_DEBUG)) */
Realloc(void *mem, size_t newSize);
#endif /* (defined(EA_DEBUG)) */

RWAPI void
Free(void *mem);

RWAPI void *
#if (defined(EA_DEBUG))
MallocAligned(size_t size, size_t alignment, const RwChar *filename, int32_t line);
#else /* (defined(EA_DEBUG)) */
MallocAligned(size_t size, size_t alignment);
#endif /* (defined(EA_DEBUG)) */

RWAPI void *
#if (defined(EA_DEBUG))
ReallocAligned(void *mem, size_t newSize, size_t alignment, const RwChar *filename, int32_t line);
#else /* (defined(EA_DEBUG)) */
ReallocAligned(void *mem, size_t alignment, size_t newSize);
#endif /* (defined(EA_DEBUG)) */

RWAPI void
FreeAligned(void *mem);

} // namespace memory

} // namespace internal

} // namespace rw

#endif // defined(RWMEMORYPROFILE)

#endif // PUBLIC_RW_CORE_MEMORYPROFILE_H
