// (c) Electronic Arts. All Rights Reserved.

/// \file
/// Global memory initialization.

#ifndef RW_MEMORYINITIALIZERS_HPP
#define RW_MEMORYINITIALIZERS_HPP

// ***********************************************************************************************************
// Includes

// ***********************************************************************************************************
// Defines

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
namespace rw
{
namespace host
{

/**
Setup for the core memory functions.
\warning
If you are using RenderWare from managed code, this function must be
explicitly called to set up internal dynamic memory allocation function
vectors.  Call MemoryFunctionsTerminate to cleanly shut down these
function vectors when you are closing your application down.
*/
void
MemoryFunctionsInitialize();

/**
Cleanup for the core memory functions.
\warning
If you called MemoryFunctionsInitialize, use MemoryFunctionsTerminate to
restore the system to its state before MemoryFunctionsInitialize was
called.
*/
void
MemoryFunctionsTerminate();

void
MemoryAlignedFunctionsInitialize();

void
MemoryAlignedFunctionsTerminate();

bool
MemorySystemInitialized();

} // namespace host
} // namespace rw

// ***********************************************************************************************************
// Structs + Unions + Classes

// ***********************************************************************************************************
//                                                Class Definition
// ***********************************************************************************************************

#endif // RW_MEMORYINITIALIZERS_HPP
