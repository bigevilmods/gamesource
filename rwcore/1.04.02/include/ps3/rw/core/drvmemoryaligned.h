// (c) Electronic Arts. All Rights Reserved.

/// \file
/// PS3-specific aligned memory allocation.

#ifndef PUBLIC_RW_DRVMEMORYALIGNED_H

#if defined(RWCROSS)

#include "../../../win/rw/core/drvmemoryaligned.h"

#else

#define PUBLIC_RW_DRVMEMORYALIGNED_H

/*************************************************************************************************************
 Includes
 */

#include <stdlib.h>

/*************************************************************************************************************
 Defines
 */
#define RwMallocAlignedMemDebug  RwMallocAlignedGeneric
#define RwReallocAlignedMemDebug RwReallocAlignedGeneric
#define RwFreeAlignedMemDebug    RwFreeAlignedGeneric

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
 Global Variables
 */

/*************************************************************************************************************
 Internal Functions Prototypes
 */

/*************************************************************************************************************
 External Functions Prototypes
 */

#endif // defined(RWCROSS)
#endif // PUBLIC_RW_DRVMEMORYALIGNED_H
