// (c) Electronic Arts. All Rights Reserved.

/// \file
/// Formatter class definition.

#ifndef RW_CORE_DEBUG_FORMATTER_H
#define RW_CORE_DEBUG_FORMATTER_H
#include <rw/core/base/ostypes.h>
#include <rw/core/debug/manager.h>

#if defined(_MSC_VER)
#pragma once
#endif

namespace rw
{
    namespace core
    {
        namespace debug
        {
            class Channel;


            /**
              \brief Default IFormatter implementation.

              This implementation of the IFormatter interface will use commas to separate
              the various elements of the message.
            */
            class Formatter : public IFormatter
            {
            public:

                // default constructor & destructor
                Formatter() {}
                virtual ~Formatter() {}

                virtual bool Format(char *buffer, size_t bufLen, Channel *channel, const char *format, va_list args);
            };
        }
    }
}

#endif
