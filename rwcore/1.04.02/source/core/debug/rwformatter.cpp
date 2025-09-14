// (c) Electronic Arts. All Rights Reserved.

/// \file
/// Formatter class implementation.

#include "rw/core/debug/formatter.h"
#include "rw/core/base/ostypes.h"
#include "rw/core/debug/debug.h"
#include "rw/core/debug/channel.h"

#include <string.h>
#include <stdio.h>
#include <time.h>

namespace rw {
namespace core {
namespace debug {

// internal helper function
static void AddStringToBuffer(char *buffer, const char *s)
{
    rwFUNCTION();
    strcat(buffer, s);
    strcat(buffer, ",");
    rwRETURNVOID();
}

#if !defined(_MSC_VER) && !defined(_snprintf) && !defined(_vsnprintf)
#define _snprintf snprintf
#define _vsnprintf vsnprintf
#endif

/**
  \brief printf style formatting
  \param buffer destination of formatted string
  \param bufLen max size of formatted string
  \param channel printopts to use in formatting
  \param format printf style format string
  \param args variable length argument list
*/
bool Formatter::Format(char *buffer, size_t bufLen, Channel *channel, const char *format, va_list args)
{
    rwFUNCTION();
    const unsigned int TEMPBUFSIZE = 32;
    PrintOpts opts = channel->GetPrintOpts();
    bool haveInfo = channel->HaveInfo();

    *buffer = '\0';

    // check for formatting options
    if(opts.mName)
        AddStringToBuffer(buffer, channel->GetName());

    if(haveInfo && opts.mFile)
        AddStringToBuffer(buffer, channel->GetFileName());

    if(haveInfo && opts.mLine)
    {
        char lineNo[TEMPBUFSIZE];
        _snprintf(lineNo, TEMPBUFSIZE, "%d", channel->GetLine());
        AddStringToBuffer(buffer, lineNo);
    }

    if(opts.mDate)
    {
#if defined(_MSC_VER)
        char tmpBuf[TEMPBUFSIZE];
        _strdate(tmpBuf);
        AddStringToBuffer(buffer, tmpBuf);
#else
        // There is no _strdate function on this platform
        AddStringToBuffer(buffer, "Date not available on this platform");
#endif
    }

    if(opts.mTime)
    {
#if defined(_MSC_VER)
        char tmpBuf[TEMPBUFSIZE];
        _strtime(tmpBuf);
        AddStringToBuffer(buffer, tmpBuf);
#else
        // There is no _strtime function on this platform
        AddStringToBuffer(buffer, "Time not available on this platform");
#endif
    }

    size_t len = strlen(buffer);
    _vsnprintf(buffer+len, bufLen-len, format, args);

    if (opts.mNewLine)
        strcat(buffer, "\n");

    rwRETURN(true);
}

}
}
}
