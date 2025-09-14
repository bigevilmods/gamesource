// (c) Electronic Arts. All Rights Reserved.

/// \file
/// Dynamic memory allocation functions.

/*************************************************************************************************************
 Includes
 */

//#define _CRTDBG_MAP_ALLOC
#pragma warning(disable: 4548)  // disable warning coming from SDK headers

#include <stdlib.h>
#include <crtdbg.h>

#include "rw/core/base/ostypes.h"
#include "rw/core/debug/debug.h"
#include "rw/core/memory.h"

#include "rw/core/string.h"

/*************************************************************************************************************
 Defines
 */

#define rw_CLIENT_TAG (0x0100)
#define rw_BLOCK_TYPE (_CLIENT_BLOCK | ((rw_CLIENT_TAG) << 16))

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

/* Functions only used in debug builds. */
#ifdef EA_DEBUG

void *
RwMallocMemDebug(size_t size, const RwChar *filename, int32_t line)
{
    rwFUNCTION();

    rwRETURN(_malloc_dbg(size, rw_BLOCK_TYPE, filename, line));
}

void *
RwReallocMemDebug(void *mem, size_t newSize, const RwChar *filename, int32_t line)
{
    rwFUNCTION();

    rwRETURN(_realloc_dbg(mem, newSize, rw_BLOCK_TYPE, filename, line));
}

void
RwFreeMemDebug(void *mem)
{
    rwFUNCTION();

    _free_dbg(mem, rw_BLOCK_TYPE);

    rwRETURNVOID();
}

#endif /* EA_DEBUG */
