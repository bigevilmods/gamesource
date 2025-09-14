// (c) Electronic Arts. All Rights Reserved.
/*************************************************************************************************************

 File: rwspr.cpp

 Purpose: A short description of the file.

 */

// ***********************************************************************************************************
// Includes
#include "rw/core/base/ostypes.h"
#include "rw/core/debug/debug.h"
#include "rw/core/debug/exception.h"
#include "rw/core/dmac.h"

namespace rw
{

// ***********************************************************************************************************
//                                                Dmac Definition
// ***********************************************************************************************************

#ifdef EA_DEBUG
uint32_t Dmac::mLocked = 0;
#endif

} // namespace rw
