// (c) Electronic Arts. All Rights Reserved.

/// \file
/// incorporate hash container into core

// ***********************************************************************************************************
// Includes
#include "rwsharedheaders.hpp"
#include "rw/core/memory/memoryalignedfunctions.h"
#include "rw/core/hash/hashcontainer.h"

// ***********************************************************************************************************
// Typedefs


// ***********************************************************************************************************
// Defines + Enums + Consts

#ifdef EA_DEBUG
namespace rw
{
/**
A state variable used to assert on the first time the hash table reaches 50% capacity.
*/
RwBool rwHashContainerAssertOnHalfFull = TRUE;
}
#endif

// ***********************************************************************************************************
// Static Variables + Static Data Member Definitions


// ***********************************************************************************************************
// Structs + Unions + Classes


// ***********************************************************************************************************
// Static Functions


// ***********************************************************************************************************
// External Functions


// ***********************************************************************************************************
// Class Member Functions


