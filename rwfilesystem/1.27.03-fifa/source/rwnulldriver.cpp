// (c) Electronic Arts. All Rights Reserved.
/**
\file
Implements the NullDriver that is needed for NullFileDevice.
*/

#include "private_rwnulldriver.h"


// This module implements a device driver which does nothing.  It is
// needed so the NullFileDevice has a driver which has the GetName
// method.
namespace rw {
namespace core {
namespace filesys {

NullFileDriver NullFileDriverInst;

}}}

