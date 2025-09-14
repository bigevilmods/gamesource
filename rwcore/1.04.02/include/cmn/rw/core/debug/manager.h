// (c) Electronic Arts. All Rights Reserved.

/// \file
/// Manager class definition

#ifndef RW_CORE_DEBUG_MANAGER_H
#define RW_CORE_DEBUG_MANAGER_H
#include <rw/core/base/ostypes.h>

#if defined(_MSC_VER)
#pragma once
#endif

#include <stdarg.h>

#include "rw/core/debug/detail/debugcriticalsection.h"
#include "rw/core/debug/debug.h" // pull in ASSERT etc macros for backwards compatibility

namespace rw
{
    class IResourceAllocator;

    namespace core
    {
        /// Debug System classes.
        namespace debug
        {
            /**
              \enum MAXNAMELEN maximum length of names in debug system
            */
            enum
            {
                MAXNAMELEN = 32
            };

            extern const char *SYSTEMCHANNEL;
            extern const char *ASSERTFORMAT;

            class Channel;
            class Device;
            class IFormatter;
            class IAssertHandler;

            /**
              Print options for debug messages, used by the IFormatter interface
              to determine what kind of extra information to include in debug messages.

              \member mDate    append date to output
              \member mTime    append time to output
              \member mFile    append file name to output
              \member mLine    append line number to output
              \member mName    append channel name to output
              \member mNewLine append a newline character to the output
            */
            class PrintOpts
            {
            public:
                PrintOpts() :
                    mDate(false),
                    mTime(false),
                    mFile(false),
                    mLine(false),
                    mName(false),
                    mNewLine(false)
                {
                    rwFUNCTION();
                    rwRETURNVOID();
                }

                bool mDate;
                bool mTime;
                bool mFile;
                bool mLine;
                bool mName;
                bool mNewLine;
            };

#ifdef EA_DEBUG
            /**
              \brief Debug Manager.

              This is the main Debug Manager class. It is a singleton that should get
              created as early as possible during execution as soon as memory management
              is available. Once this singleton has been created, the use of debug channels
              and devices is possible.
              If the singleton has not been created yet, there are still a number of functions
              available for use. The static AssertFailed and Abort functions will provide low-level
              debug functionality that do not depend on any other systems (other than
              the very low level Hal library) and can be called from static constructors
              before main() gets executed. This means that assert macros can be used anywhere
              in the code.

              The Debug Manager is thread safe.
            */
            class Manager
            {
            public:

                /**
                  \enum MAXCHANNELS maximum nr. of channels supported
                */
                enum
                {
                    MAXCHANNELS = 64
                };

                static Manager *CreateInstance(rw::IResourceAllocator *);
                static Manager *GetInstance();
                static void DestroyInstance();
                static rw::IResourceAllocator *GetAllocator();
                static bool IsInitialized()
                {
                    rwFUNCTION();
                    rwRETURN(rw::core::debug::Manager::sInstance != NULL);
                }
                // AssertFailed and Abort are static so they
                // can always be called, even when the Manager singleton
                // hasn't been created yet.
                static bool AssertFailed(const char *condition, const char *file,
                                         const char *function, unsigned int line,
                                         const char *format, va_list args,
                                         unsigned int flags = ASSERTFLAG_FATAL);

                static void Abort(const char *message);

                void SetAssertHandler(IAssertHandler *assertHandler);
                static void SetDefaultPrintOpts(const PrintOpts &opts);

                static PrintOpts GetDefaultPrintOpts();

                /**
                  \brief Return the default debug channel.
                  \return The Channel used by the debug System for it's diagnostics, abort and assert output.
                */
                Channel *GetSystemChannel() const
                {
                    rwFUNCTION();
                    rwRETURN(mSystemChannel);
                }

                /**
                  \brief Return the default debug message formatter.
                  \return The Formatter used by the debug System.
                */
                IFormatter *GetSystemFormatter() const
                {
                    rwFUNCTION();
                    rwRETURN(mSystemFormatter);
                }

                void HaltSystem() const;

                bool RegisterChannel(Channel *channel);
                bool DeleteChannel(const Channel *channel);
                void DisableChannel(const char *name);
                Channel *GetChannel(const char *name) const;

            private:
                static void *operator new(size_t);
                static void operator delete(void *);

                Manager();
                ~Manager();

                void Init();

                // Prevent default generation of these functions by not defining them
                Manager(const Manager& rhs);            // copy constructor
                Manager& operator=(const Manager& rhs); // assignment operator

                static Manager *sInstance;
                static rw::IResourceAllocator *sAllocator;
                static rw::core::debug::detail::DebugCriticalSection sMutex;

                Channel *mChannels[MAXCHANNELS];
                char mDisabledChannelList[MAXCHANNELS][MAXNAMELEN];
                unsigned int mDisabledChannelCount;

                Device *mSystemDevice;
                Channel *mSystemChannel;
                IAssertHandler *mAssertHandler;
                bool mAssertHandlerOwner;
                IFormatter *mSystemFormatter;
                static PrintOpts sPrintOpts;
            };


            /**
              \brief Debug Assert Handler interface.

              This interface is used to handle asserts after they have failed. This allows
              the client to implement their own assert handlers if they wish. The RenderWare Core provides
              an implementation that outputs the assert message to the Debug system channel
              and aborts.
            */
            class IAssertHandler
            {
            public:
                static void *operator new(size_t);
                static void operator delete(void *);


                /**
                  \brief Assert Handler function with va_list passing.
                  \param condition the condition that failed, causing the assert.
                  \param file the source file name the assert originated from.
                  \param function the name of the function the assert originated from.
                  \param line the line number the assert originated from.
                  \param format a message that can provide more information about why the assert failed, this can contain \c printf()-style formatting.
                  \param argList a variable argument list containing the parameters refered to in the \c format parameter.
                  \param flags 0 or one of the AssertFlag bits.

                  \return \c true: the application should abort, \c false: the assert was ignored.
                  \see #RWDEBUG_ASSERT
                */
                virtual bool
                    AssertFailed(const char *condition, const char *file,
                                 const char *function, unsigned int line,
                                 const char *message, va_list argList,
                                 unsigned int flags) = 0;

                // default constructor & destructor
                IAssertHandler() {}
                virtual ~IAssertHandler() {}
            };
#else
            // release version of manager is much, much smaller. It only maintains
            // an allocator and a formatter
            class Manager
            {
            public:
                static Manager *CreateInstance(rw::IResourceAllocator *allocator);
                static void DestroyInstance();
                static rw::IResourceAllocator *GetAllocator();
                static bool IsInitialized()
                {
                    rwFUNCTION();
                    rwRETURN(rw::core::debug::Manager::sAllocator != NULL &&
                            rw::core::debug::Manager::sFormatter != NULL );
                }

                static IFormatter *GetFormatter()
                {
                    rwFUNCTION();
                    rwRETURN(sFormatter);
                }
            private:
                static rw::IResourceAllocator *sAllocator;
                static IFormatter *sFormatter;
            };
#endif // EA_DEBUG


            /**
              \brief Debug Formatter interface.

              This interface is used to format debug messages before they are output
              by the Debug Device. This allows each Device to implement custom formatting
              where needed. RenderWare Core provides an implementation of IFormatter that does simple
              comma-separated formatting.
            */
            class IFormatter
            {
            public:
                static void *operator new(size_t);
                static void operator delete(void *);


                /**
                  \brief Format debug output.
                  \param buffer buffer to use for formatted message.
                  \param bufLen length of buffer
                  \param channel Debug Channel where this formatting request originated.
                  \param format \c printf style format for the message.
                  \param args arguments for the specified format.
                */
                virtual bool
                    Format(char *buffer, size_t bufLen, Channel *channel,
                           const char *format, va_list args) = 0;

                // default constructor & destructor
                IFormatter() {}
                virtual ~IFormatter() {}
            };
        } // namespace debug
    } // namespace core
} // namespace rw

#endif // RW_CORE_MANAGER_H
