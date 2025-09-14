// (c) Electronic Arts. All Rights Reserved.

/// \file
/// Console Device class implementation.

#include "rw/core/debug/consoledevice.h"

#include "rw/core/debug/host.h"

#include <string.h>

namespace rw {
namespace core {
namespace debug {

/**
  We more or less want to treat the "buffer" as a string, not
  an arbitrary stream of data.
  \param buf data source to write to device
  \param bufSize number of bytes to copy to destination device
*/
void ConsoleDevice::Output(const void *buf, size_t bufSize)
{
    rwFUNCTION();
    char printBuf[4096];

    this->EnterCritical();

    if ((bufSize < (sizeof(printBuf)-1)) && (static_cast<const char *>(buf) + bufSize != NULL))
    {
        // If there isn't too much data to be output,
        // and there is no NULL-terminator in the output
        // buffer, put one in here.
        memcpy(printBuf, buf, bufSize);
        printBuf[bufSize] = '\0';
        host::PrintDebugString(printBuf);
    }
    else
    {
        // Otherwise just print the data verbatim.
        host::PrintDebugString(static_cast<const char *>(buf));
    }

    this->LeaveCritical();
    rwRETURNVOID();
}

}
}
}
