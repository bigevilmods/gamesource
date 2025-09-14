// (c) Electronic Arts. All Rights Reserved.

/// \file
/// Kettle-specific Default memory allocation.

#ifndef PUBLIC_RW_DRVMEMORY_H
#define PUBLIC_RW_DRVMEMORY_H

#include <stdlib.h>

/*************************************************************************************************************
 Global Variables
 */

/*************************************************************************************************************
 Internal Functions Prototypes
 */

/*************************************************************************************************************
 External Functions Prototypes
 */
#define RwMallocMemDebug  RwMallocGeneric
#define RwReallocMemDebug RwReallocGeneric
#define RwFreeMemDebug    RwFreeGeneric

#endif // PUBLIC_RW_DRVMEMORY_H
