// (c) Electronic Arts. All Rights Reserved.

/// \file
/// Host specifics for messages that are internationalized and managed by the docs team.

#include "rwsharedheaders.hpp"
#include "rw/core/docmessage/docmessage.h"
#include "rw/core/docmessage/hostdocmsg.h"

#pragma warning(push)
#pragma warning(disable: 4365) // conversion from 'LONG64' to 'unsigned __int64', signed/unsigned mismatch
#include <windows.h>
#pragma warning(pop)

#if defined(RWENABLEDOCMESSAGE)

namespace rw
{

/*************************************************************************************************************
 External Functions
 */

void
rwHOSTSPECIFICDOCMESSAGEHANDLERFN(const RwChar* RWUNUSED(file),
                                  uint32_t RWUNUSED(line),
                                  uint32_t RWUNUSED(messageId),
                                  const RwChar *format,
                                  va_list args)
{
    #define HOSTDOCMSG_BUFFERSIZE 1024
    RwChar buffer[HOSTDOCMSG_BUFFERSIZE];

    RwVsnprintf(buffer, HOSTDOCMSG_BUFFERSIZE, format, args);
    // avoid printing blank lines
    if (buffer[0])
    {
#if defined(EA_PLATFORM_XBOXONE)
		OutputDebugStringA(buffer);
        OutputDebugStringA("\n");
#else
		OutputDebugString(buffer);
		OutputDebugString("\n");
#endif
    }
}

} // namespace rw

#endif /* defined(RWENABLEDOCMESSAGE) */
