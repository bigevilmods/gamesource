// (c) Electronic Arts. All Rights Reserved.

/// \file
/// Memory functions interface.

#ifndef RW_MEMORYFUNCTIONS_H
#define RW_MEMORYFUNCTIONS_H
#include <rw/core/base/ostypes.h>

/*************************************************************************************************************
 Includes
 */

/*************************************************************************************************************
 Defines
 */

#if (defined(EA_DEBUG))
#define RwMalloc(S)         rwMemoryFunctions.rwmalloc(S, __FILE__,  __LINE__)
#define RwRealloc(P, S)     rwMemoryFunctions.rwrealloc(P, S, __FILE__, __LINE__)
#define RwFree(P)           rwMemoryFunctions.rwfree(P)
#else /* (defined(EA_DEBUG)) */
#define RwMalloc(S)         rwMemoryFunctions.rwmalloc(S)
#define RwRealloc(P, S)     rwMemoryFunctions.rwrealloc(P, S)
#define RwFree(P)           rwMemoryFunctions.rwfree(P)
#endif /* (defined(EA_DEBUG)) */

#define RwMemSetMallocMacro(F)  rwMemoryFunctions.rwmalloc = (F)
#define RwMemSetReallocMacro(F) rwMemoryFunctions.rwrealloc = (F)
#define RwMemSetFreeMacro(F)    rwMemoryFunctions.rwfree = (F)

#define RwMemGetMallocMacro()   (rwMemoryFunctions.rwmalloc)
#define RwMemGetReallocMacro()  (rwMemoryFunctions.rwrealloc)
#define RwMemGetFreeMacro()     (rwMemoryFunctions.rwfree)

/**
Per-class overrides of new and delete, including standard and placement versions of each. These are simple
wrappers for RwMalloc and RwFree. The placement delete is required only to satisfy constructors which may
throw exceptions.
*/
#define RWMEMORYFUNCTIONS_NEWANDDELETE()                                \
void *operator new(size_t size)                                         \
{                                                                       \
    rwFUNCTION();                                                       \
    rwRETURN(RwMalloc(size));                                           \
}                                                                       \
void operator delete(void *memory)                                      \
{                                                                       \
    rwFUNCTION();                                                       \
    RwFree(memory);                                                     \
    rwRETURNVOID();                                                     \
}                                                                       \
void *operator new(size_t RWUNUSED(size), void *memory)                 \
{                                                                       \
    rwFUNCTION();                                                       \
    rwRETURN(memory);                                                   \
}                                                                       \
void operator delete(void *, void *)                                    \
{                                                                       \
    rwFUNCTION();                                                       \
    rwRETURNVOID();                                                     \
}

/*************************************************************************************************************
 Enums
 */

/*************************************************************************************************************
 Typedef Enums
 */

/*************************************************************************************************************
 Typedef Structs + Unions
 */

typedef struct RwMemoryFunctions RwMemoryFunctions;

/*************************************************************************************************************
 Typedef Function Pointers
 */

/* Typedef as complex return types don't seem to work properly */
#if (defined(EA_DEBUG))
typedef void *(*RwMallocFunc)(size_t size,
                              const RwChar *filename, int32_t line);
typedef void *(*RwReallocFunc)(void *mem, size_t newSize,
                               const RwChar *filename, int32_t line);
#else /* (defined(EA_DEBUG)) */
typedef void *(*RwMallocFunc)(size_t size);
typedef void *(*RwReallocFunc)(void *mem, size_t newSize);
#endif /* (defined(EA_DEBUG)) */
typedef void  (*RwFreeFunc)(void *mem);

/*************************************************************************************************************
 Structs + Unions
 */

struct RwMemoryFunctions
{
    RwMallocFunc  rwmalloc;
    RwReallocFunc rwrealloc;
    RwFreeFunc    rwfree;
};


/*************************************************************************************************************
 Global Variables
 */

/* In bss so defaults to all NULLs at startup */
extern RwMemoryFunctions rwMemoryFunctions;

/*************************************************************************************************************
 Internal Functions Prototypes
 */

/*************************************************************************************************************
 External Functions Prototypes
 */
extern void RwMemSetMalloc(RwMallocFunc mallocFunc);
extern void RwMemSetRealloc(RwReallocFunc reallocFunc);
extern void RwMemSetFree(RwFreeFunc freeFunc);

extern RwMallocFunc  RwMemGetMalloc(void);
extern RwReallocFunc RwMemGetRealloc(void);
extern RwFreeFunc    RwMemGetFree(void);

// Stub for backwards compatibility.
class RwMemoryFunctionsAutoInitializer
{
public:

    RwMemoryFunctionsAutoInitializer()
    {
    }
    ~RwMemoryFunctionsAutoInitializer()
    {
    }
};

#endif /* RW_MEMORYFUNCTIONS_H */
