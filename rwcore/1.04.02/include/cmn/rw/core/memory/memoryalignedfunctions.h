// (c) Electronic Arts. All Rights Reserved.

/// \file
/// Dynamic aligned memory allocation functions.

#ifndef RW_MEMORYALIGNEDFUNCTIONS_H
#define RW_MEMORYALIGNEDFUNCTIONS_H
#include <rw/core/base/ostypes.h>

/*************************************************************************************************************
 Includes
 */

/*************************************************************************************************************
 Defines
 */

#if (defined(EA_DEBUG))
#define RwMallocAligned(S, A)       rwMemoryAlignedFunctions.rwMallocAligned(S, A, __FILE__,  __LINE__)
#define RwReallocAligned(P, S, A)   rwMemoryAlignedFunctions.rwReallocAligned(P, S, A, __FILE__, __LINE__)
#define RwFreeAligned(P)            rwMemoryAlignedFunctions.rwFreeAligned(P)
#else /* (defined(EA_DEBUG)) */
#define RwMallocAligned(S, A)       rwMemoryAlignedFunctions.rwMallocAligned(S, A)
#define RwReallocAligned(P, S, A)   rwMemoryAlignedFunctions.rwReallocAligned(P, S, A)
#define RwFreeAligned(P)            rwMemoryAlignedFunctions.rwFreeAligned(P)
#endif /* (defined(EA_DEBUG)) */

#define RwMemSetMallocAlignedMacro(F)   rwMemoryAlignedFunctions.rwMallocAligned = (F)
#define RwMemSetReallocAlignedMacro(F)  rwMemoryAlignedFunctions.rwReallocAligned = (F)
#define RwMemSetFreeAlignedMacro(F)     rwMemoryAlignedFunctions.rwFreeAligned = (F)

#define RwMemGetMallocAlignedMacro()    (rwMemoryAlignedFunctions.rwMallocAligned)
#define RwMemGetReallocAlignedMacro()   (rwMemoryAlignedFunctions.rwReallocAligned)
#define RwMemGetFreeAlignedMacro()      (rwMemoryAlignedFunctions.rwFreeAligned)

/*************************************************************************************************************
 Enums
 */

/*************************************************************************************************************
 Typedef Enums
 */

/*************************************************************************************************************
 Typedef Structs + Unions
 */
typedef struct RwMemoryAlignedFunctions RwMemoryAlignedFunctions;

/*************************************************************************************************************
 Typedef Function Pointers
 */
/* Typedef as complex return types don't seem to work properly */
#if (defined(EA_DEBUG))
typedef void *(*RwMallocAlignedFunc)(size_t size, size_t alignment,
                                     const RwChar *filename, int32_t line);
typedef void *(*RwReallocAlignedFunc)(void *mem, size_t newSize, size_t alignment,
                                      const RwChar *filename, int32_t line);
#else /* (defined(EA_DEBUG)) */
typedef void *(*RwMallocAlignedFunc)(size_t size, size_t alignment);
typedef void *(*RwReallocAlignedFunc)(void *mem, size_t newSize, size_t alignment);
#endif /* (defined(EA_DEBUG)) */
typedef void  (*RwFreeAlignedFunc)(void *mem);

/*************************************************************************************************************
 Structs + Unions
 */

struct RwMemoryAlignedFunctions
{
    RwMallocAlignedFunc  rwMallocAligned;
    RwReallocAlignedFunc rwReallocAligned;
    RwFreeAlignedFunc    rwFreeAligned;
};

/*************************************************************************************************************
 Global Variables
 */

/* In bss so defaults to all NULLs at startup */
extern RwMemoryAlignedFunctions rwMemoryAlignedFunctions;

/*************************************************************************************************************
 Internal Functions Prototypes
 */

/*************************************************************************************************************
 External Functions Prototypes
 */
extern void RwMemSetMallocAligned(RwMallocAlignedFunc mallocAlignedFunc);
extern void RwMemSetReallocAligned(RwReallocAlignedFunc reallocAlignedFunc);
extern void RwMemSetFreeAligned(RwFreeAlignedFunc freeAlignedFunc);

extern RwMallocAlignedFunc  RwMemGetMallocAligned(void);
extern RwReallocAlignedFunc RwMemGetReallocAligned(void);
extern RwFreeAlignedFunc    RwMemGetFreeAligned(void);

// Stub for backwards compatibility.
class RwMemoryAlignedFunctionsAutoInitializer
{
public:

    RwMemoryAlignedFunctionsAutoInitializer()
    {
    }
    ~RwMemoryAlignedFunctionsAutoInitializer()
    {
    }
};

#endif /* RW_MEMORYALIGNEDFUNCTIONS_H */
