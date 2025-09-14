// (c) Electronic Arts. All Rights Reserved.

#ifndef RWSHAREDHEADERS_HPP
#define RWSHAREDHEADERS_HPP

#include "rw/core/base/ostypes.h"
#include "rw/core/debug/debug.h"

#if !defined(rwHOST_PS3_SPU)

#include "rw/core/sharedglobal/sharedglobal.h"

#include "rw/core/debug/exception.h"
#include "rw/core/debug/manager.h"
#include "rw/core/debug/channel.h"
#include "rw/core/debug/device.h"
#include "rw/core/debug/consoledevice.h"
#include "rw/core/debug/formatter.h"
#include "rw/core/debug/asserthandler.h"

#include "rw/core/resource/baseresource.h"
#include "rw/core/resource/resource.h"
#include "rw/core/resource/targetresource.h"
#include "rw/core/resource/resourceallocator.h"
#include "rw/core/resource/resourceallocatorregistry.h"
#include "rw/core/resource/resourcetarget.h"

#include <EAStdC/EAMemory.h>
#include <EAStdC/EAString.h>


#endif // !defined(rwHOST_PS3_SPU)

#include "rw/core/string.h"

#endif

