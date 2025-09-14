// (c) Electronic Arts. All Rights Reserved.

/// \file
/// AssertHandler class definition.

#ifndef RW_CORE_DEBUG_ASSERTHANDLER_H
#define RW_CORE_DEBUG_ASSERTHANDLER_H
#include <rw/core/base/ostypes.h>
#include <rw/core/debug/manager.h>

#if defined(_MSC_VER)
#pragma once
#endif

#ifdef EA_DEBUG

namespace rw
{
namespace core
{
namespace debug
{

/**
  \brief default implementation of the IAssertHandler interface.

  Send the assert output to the system channel and break to the
  debugger if the assert is fatal.  If the assert is non-fatal,
  this function returns false.

  This version takes a printf() format and a va_list.
*/
class AssertHandler : public IAssertHandler
{
public:
    AssertHandler() {}
    virtual ~AssertHandler() {}

    virtual bool
        AssertFailed(const char *condition, const char *file,
        const char *function, unsigned int line,
        const char *format, va_list argList,
        unsigned int flags);

private:
    // Prevent default generation of these functions by not defining them
    AssertHandler(const AssertHandler& rhs);            // copy constructor
    AssertHandler& operator=(const AssertHandler& rhs); // assignment operator
};

} // namespace debug
} // namespace core
} // namespace rw

#endif // EA_DEBUG
#endif
