// (c) Electronic Arts. All Rights Reserved.

/// \file
/// AssertHandler class implementation.

#ifdef EA_DEBUG

#include "rw/core/debug/asserthandler.h"
#include "rw/core/debug/channel.h"

namespace rw {
namespace core {
namespace debug {

// lint warns about this header not being used in this file
//lint -efile(964, coreallocator\IAllocator.h)

/**
  \brief Default assert handler, prints to the default debug channel
  \param condition string version of failed condition
  \param file source file where assert failed
  \param function string holding function name where assert happened
  \param line line number in source file where assert happened
  \param format printf style format string with assert message
  \param args variable length argument list to use in format string
  \param flags flags affecting message output
  \return true: system must abort, false: assert was ignored.
*/
bool AssertHandler::AssertFailed(const char *condition, const char *file,
                                 const char *function, unsigned int line,
                                 const char *format, va_list args,
                                 unsigned int flags)
{
    rwFUNCTION();

    Channel *channel = Manager::GetInstance()->GetSystemChannel();

    channel->Print(ASSERTFORMAT, file, line, condition, function);
    if(format != NULL)
    {
        channel->Print("\t");
        channel->Vprint(format, args);
    }
    channel->Print("\n");

    rwRETURN((flags & ASSERTFLAG_FATAL) != 0);
}

}
}
}

#endif // EA_DEBUG
