// (c) Electronic Arts. All Rights Reserved.

/// \file
/// Debugging functions

#ifndef RW_DEBUG_H
#define RW_DEBUG_H
#include <rw/core/base/ostypes.h>

#include "rw/core/debug/hostdbg.h"
#include "rw/core/debug/compiletimeassert.h"

#include <stdarg.h>

// FIFA_BEGIN | SENDBACK | bcauthers | 2015-04-07 | this define stomps the value of RWCORELIB_API and causes linker problems in bulk builds
//#if !defined(RWCORELIB_API)
// 2/2/2007 JDF - kept here for the benefit of rwarena, which still has published
// code with this macro in it.  They should patch their code to remove it, and then
// at some point far in the future, we can probably remove it from here too.
//#define RWCORELIB_API
//#endif
// FIFA_BEGIN

// The following compile time assertion macros need to be defined in both
// debug and release builds. They specify absolute compile time requirements
// rather than possible runtime failures which might be handled by the code
// itself.

/**
 Joins two preprocessor tokens with recursive token expansion

*/
#define rwJOIN_TOKEN(a,b)          rwJOIN_TOKEN2(a,b)
#define rwJOIN_TOKEN2(a,b)         rwJOIN_TOKEN3(a,b)
#define rwJOIN_TOKEN3(a,b)         a##b

// ------------------------------------------------------------
// SPU will probably have a different
// debug interface, for now provide
// null versions of the debugging
// macros
// ------------------------------------------------------------
#ifndef rwHOST_PS3_SPU

#ifndef RW_CURRENT_FUNCTION
#if defined __CWCC__ && (__CWCC__ < 0x4200)
// CodeWarrior v. < 4.2 has a bug with __FUNCTION__ in templated functions.
// Freescale won't fix it though, and the Radix 1.3 compiler is not affected.
// So until EA is able to drop support for that compiler, we can't use 
// __
#define RW_CURRENT_FUNCTION 0
#else
#define RW_CURRENT_FUNCTION __FUNCTION__
#endif  // CodeWarrior < 4.2
#endif  // RW_CURRENT_FUNCTION

namespace rw
{

// deprecated macro definitions
#if !defined(EA_DEBUG) || defined(rwHOST_PS2)

#ifndef rwDEPRECATED
#define rwDEPRECATED()
#endif // !rwDEPRECATED

#ifndef rwDEPRECATEDM
#define rwDEPRECATEDM(alternativeString)
#endif // !rwDEPRECATEDM

#else // defined(EA_DEBUG) && !defined(rwHOST_PS2)

/* Macro for functions which have been deprecated. */
#ifndef rwDEPRECATED
#define rwDEPRECATED() rw::Deprecated(__FILE__, __LINE__, RW_CURRENT_FUNCTION, "")
#endif // !rwDEPRECATED

#ifndef rwDEPRECATEDM
#define rwDEPRECATEDM(alternativeString) rw::Deprecated(__FILE__, __LINE__, RW_CURRENT_FUNCTION, alternativeString)
#endif // !rwDEPRECATEDM

#endif // defined(EA_DEBUG) && !defined(rwHOST_PS2)

#ifdef EA_DEBUG
extern void
Deprecated(RwChar *file, int32_t line, const RwChar *function, const RwChar *message);
#endif // EA_DEBUG

template<class T>
struct DeprecatedProxied
{
public:
    DeprecatedProxied(T &newMember)
        : m_forwardTo(newMember)
    {
    }
    operator T&()
    {
        rwFUNCTION();
        rwRETURN(m_forwardTo);
    }
    operator T&() const
    {
        rwFUNCTION();
        rwRETURN(m_forwardTo);
    }
    T& operator=(const T&rhs)
    {
        rwFUNCTION();
        rwRETURN(m_forwardTo=rhs);
    }
    DeprecatedProxied<T> & operator=(const DeprecatedProxied<T> &rhs)
    {
        rwFUNCTION();
        m_forwardTo=rhs.m_forwardTo;
        rwRETURN(*this);
    }
private:
    T &m_forwardTo;
};

template<class T>
class DeprecatedProxiedPtrType
{
public:
    DeprecatedProxiedPtrType(T *&newMember)
        : m_forwardTo(newMember)
    {
        rwFUNCTION();
        rwRETURNVOID();
    }

    T&
    operator*() const
    {
        rwFUNCTION();
        rwRETURN(*m_forwardTo);
    }
    T* &
    operator->() const
    {
        rwFUNCTION();
        rwRETURN(m_forwardTo);
    }
    T*& operator=(T *const&rhs)
    {
        rwFUNCTION();
        rwRETURN(m_forwardTo=rhs);
    }

    DeprecatedProxiedPtrType<T> & operator=(const DeprecatedProxiedPtrType<T> &rhs)
    {
        rwFUNCTION();
        m_forwardTo=rhs.m_forwardTo;
        rwRETURN(*this);
    }

    template<class U>
    operator U*()
    {
        rwFUNCTION();
        rwRETURN((U *)m_forwardTo);
    }

private:
    T* &m_forwardTo;
};

template<>
class DeprecatedProxiedPtrType<void>
{
public:
    DeprecatedProxiedPtrType(void *&newMember)
        : m_forwardTo(newMember)
    {
        rwFUNCTION();
        rwRETURNVOID();
    }

    DeprecatedProxiedPtrType& operator=(const DeprecatedProxiedPtrType& rhs)
    {
        rwFUNCTION();
        m_forwardTo = rhs.m_forwardTo;
        rwRETURN(*this);
    }

    void* &
    operator->() const
    {
        rwFUNCTION();
        rwRETURN(m_forwardTo);
    }
    void*& operator=(void* const &rhs)
    {
        rwFUNCTION();
        rwRETURN(m_forwardTo=rhs);
    }

    template<class U>
    operator U*()
    {
        rwFUNCTION();
        rwRETURN((U *)m_forwardTo);
    }
private:
    void* &m_forwardTo;
};

#if defined(__GNUC__)

#define rwDEPRECATEACCESS(T, oldMember, newMember)
#define rwDEPRECATEACCESSPTRTYPE(T, oldMember, newMember)

#else // !defined(__GNUC__)

#define rwDEPRECATEACCESS(T, oldMember, newMember) DeprecatedProxied<T> oldMember
#define rwDEPRECATEACCESSPTRTYPE(T, oldMember, newMember) DeprecatedProxiedPtrType<T> oldMember

#endif // defined(__GNUC__)

#define rwMAXDEBUGSTRINGLEN 1024

// Private helper function for the debug message system.
//
// This is used to take a variable length list of arguments passed into a macro and
// pass them to vsnsprintf to sprint them to a buffer contained within the object. The Char*
// returned by WriteToBuffer and GetBuffer should NOT be deleted as it will be destroyed
// with the object.
class rwDebugPrintHelper
{
public:
    rwDebugPrintHelper()
    {
        /* No debug macros here */
        rwFUNCTION();
        m_buf[0] = '\0';
        rwRETURNVOID();
    }

    /**
    Writes to buffer
    */
    RwChar* WriteToBuffer(const RwChar* format, ...);

    /**
    returns the buffer
    */
    RwChar* GetBuffer()
    {
        /* No debug macros here */
        rwFUNCTION();
        rwRETURN(m_buf);
    }
private:
    RwChar m_buf[rwMAXDEBUGSTRINGLEN];
};

} // namespace rw


// ------------------------------------------------------------
// Debug output system.
//
// The RenderWare debug system is 3 tiered starting with
// a debug hooks and ending with devices and assert handlers:
//
// DEBUG MACRO (in rw or user code)
//   |
//   RWCALLBACK  -> REGISTERED DEBUG HOOK        (1)
//                          |
//                     DEBUG MANAGER             (2)
//                          |
//                       + / \ +
//          OUTPUT DEVICES     ASSERT HANDLER    (3)
//
//
// The RenderWare debug macros currently include:
//      o  rwABORT(message)
//      o  rwASSERT(condition)
//      o  rwASSERTM(condition, message)
//      o  rwASSERTMFL(condition, message, file, line)
//      o  rwASSERTALIGN(ADDR, ALIGN)
//      o  rwMESSAGE(message)
//      o  rwCMESSAGE(channel, message)
//      o  rwCHMESSAGE(channel, message)
//      o  rwWARNING(condition, message)
//      o  rwVERBOSE(I)
//
// (1) Each of these macros in turn call one of the following
// callback hooks:
//      o CallDebugMessageCallback        - message being output to a named channel
//      o CallDebugMessageChannelCallback - message being output to a channel object directly
//      o CallDebugPrintAssertCallback    - assert message, either fatal or non fatal
//
// Users have several choices:
//    o (1) Register their own debugging hooks and NOT use the RenderWare debug manager
//    o (2) Initialize the debug manager and use the default devices and assert handler
//    o (3) Initialize the debug manager and customize it by writing their own
//             * Output device (registered per channel)
//             * Assert handler (registered globally on the manager)
//
namespace rw
{
    namespace core
    {
        namespace debug
        {
            // Channel class is forward declared here
            // users that do not want to use the
            // RenderWare debug manager could potentially
            // define their own channel class as long as
            // RW takes care to only operate on a channel
            // through the debug callback.
            class Channel;

            /**
              Assert handler flags

              The assert handler called through the registered RwDebugPrintAssertCallback
              function can use these flags to determine if it should continue execution or
              attempt to terminate execution.

              \enum ASSERTFLAG_FATAL this failed assertion is fatal.  Further processsing
                    will result in undefined behaviour, such as a bus
                    error, illegal address error, etc.
              \enum ASSERTFLAG_NONFATAL This failed assertion is not fatal.  The application
                    may continue processing normally.
            */
            enum AssertFlags
            {
                ASSERTFLAG_FATAL    = 0x1,
                ASSERTFLAG_NONFATAL = 0x2
            };

            // ---------------------------------------------
            // DEBUG HOOK TYPES
            // ---------------------------------------------

            /**
              Callback type for named channel output
            */
            typedef void (* RwDebugMessageCallback)(const char * channelname,const char * file , uint32_t line, const char *format, va_list args);
            /**
              Callback type for a request to output to a channel object
            */
            typedef void (* RwDebugMessageChannelCallback)(Channel * channel,const char * file , uint32_t line, const char *format, va_list args);
            /**
              Callback type for assert messages: type of assert is controlled by the rw::core::debug::AssertFlags.
              \see rw::core::debug::Manager::AssertFailed, rwASSERT
            */
            typedef bool (* RwDebugPrintAssertCallback)(const char * condition, const char * file   ,
                                                        const char * function , unsigned int line   ,
                                                        const char * format   , va_list      argList,
                                                        unsigned int flags);

            // ---------------------------------------------
            // DEBUG HOOK PUBLIC API
            // ---------------------------------------------

            /**
              Register the debug callback to be called for named channel output
              \param callback Callback to be used in debug builds to output to a named channel
            */
            void SetDebugMessageFunction       (RwDebugMessageCallback        RWUNUSED_RELEASE(callback));
            /**
              Register the debug callback to be called for output directed at a particular channel object
              \param callback Callback to be used in debug builds to output to the provided channel object
            */
            void SetDebugMessageChannelFunction(RwDebugMessageChannelCallback RWUNUSED_RELEASE(callback));
            /**
              Register the debug callback to be called for assert and warning output
              \param callback Callback to be used in debug builds for assert messages
            */
            void SetDebugPrintAssertFunction   (RwDebugPrintAssertCallback    RWUNUSED_RELEASE(callback));

            /**
               Semi private method for calling the registered RwDebugMessageCallback, used by the debug macros
               to access the global debug hooks. Users wanting to write their own macros can use this interface
               to call the global debug hooks.
               \param channelname unique string name identifying the channel on which output should be output
               \param file filename in which this function was called
               \param line line number from which this function was called
               \param format printf style format string
               \param args variable length argument list corresponding to the format string
            */
            void CallDebugMessageCallback(const char * channelname,const char * file , uint32_t line, const char *format, va_list args);

            /**
               Semi private method for calling the registered RwDebugMessageChannelCallback, used by the debug macros
               to access the global debug hooks. Users wanting to write their own macros can use this interface
               to call the global debug hooks.
               \param channel channel object to be used for output
               \param file filename in which this function was called
               \param line line number from which this function was called
               \param format printf style format string
               \param args variable length argument list corresponding to the format string
            */
            void CallDebugMessageChannelCallback(Channel * channel,const char * file , uint32_t line, const char *format, va_list args);

            /**
               Semi private method for calling the registered RwDebugPrintAssertCallback, used by the debug macros
               to access the global debug hooks. Users wanting to write their own macros can use this interface
               to call the global debug hooks.
               \param condition string describing the assertion that failed
               \param file filename where this function was called
               \param line line number where this function was called
               \param format printf style format string
               \param argList variable length argument list corresponding to the format string
               \param flags one of the rw::core::debug::AssertFlags identifying if the assertion is a warning or a fatal error
               \return true if program execution should stop
            */
            bool CallDebugPrintAssertCallback(const char * condition, const char * file   ,
                                               const char * function, unsigned int line,
                                               const char * format, va_list argList,
                                               unsigned int flags);

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4793)  // native code generated for 'vararg' function
#endif

#ifdef EA_DEBUG

            // ------------------------------------------------------------
            class PrintMacroHelper
            {
            public:
                PrintMacroHelper(const char*file, unsigned int line,const char * channelName)
                    : mChannelName(channelName)
                    , mFile(file)
                    , mChannel(NULL)
                    , mLine(line)
                {
                    rwFUNCTION();
                    rwRETURNVOID();
                }

                PrintMacroHelper(const char*file, unsigned int line,Channel * channel)
                    : mChannelName(NULL)
                    , mFile(file)
                    , mChannel(channel)
                    , mLine(line)
                {
                    rwFUNCTION();
                    rwRETURNVOID();
                }

                //lint -e{1916}  ellipsis OK here
                void PrintFormatted(const char* pFormat, ...)
                {
                    rwFUNCTION();
                    va_list argList;
                    va_start(argList, pFormat);
                    CallDebugMessageCallback(mChannelName,mFile,mLine,pFormat,argList);
                    va_end(argList);
                    rwRETURNVOID();
                }

                //lint -e{1916}  ellipsis OK here
                void PrintFormattedChannel(const char* pFormat, ...)
                {
                    rwFUNCTION();
                    va_list argList;
                    va_start(argList, pFormat);
                    CallDebugMessageChannelCallback(mChannel,mFile,mLine,pFormat,argList);
                    va_end(argList);
                    rwRETURNVOID();
                }

            private:
                PrintMacroHelper& operator=(const PrintMacroHelper& rhs); // assignment operator

                const char        * mChannelName;
                const char        * mFile;
                Channel           * mChannel;
                const unsigned int  mLine;
            };

            // ------------------------------------------------------------
            class AssertMacroHelper
            {
            public:
                AssertMacroHelper(const char *condition, const char*file,
                                  const char * function, unsigned int line,
                                  unsigned int flags)
                    : mCondition(condition)
                    , mFile(file)
                    , mFunction(function)
                    , mLine(line)
                    , mFlags(flags)
                {
                    rwFUNCTION();
                    rwRETURNVOID();
                }

                //lint -e{1916}  ellipsis OK here
                bool AssertFormatted(const char* pFormat, ...)
                {
                    rwFUNCTION();
                    va_list argList;
                    va_start(argList, pFormat);
                    const bool ret = CallDebugPrintAssertCallback(mCondition, mFile, mFunction, mLine, pFormat, argList, mFlags);
                    va_end(argList);
                    rwRETURN(ret);
                }

            private:
                AssertMacroHelper& operator=(const AssertMacroHelper& rhs); // assignment operator

                const char        * mCondition;
                const char        * mFile;
                const char        * mFunction;
                const unsigned int  mLine;
                const unsigned int  mFlags;
            };

            /**
              \brief System abort macro.

              This macro will halt the system after printing the message string.  The message
              string cannot contain additional parameters or format specifiers.  If you want
              to print a message which has format specifiers, use rwASSERTM.
              \see rw::core::debug::Manager::Abort, rwASSERTM
            */
#define rwABORT(message)                                                \
                if(CallDebugPrintAssertCallback("User Abort", __FILE__, RW_CURRENT_FUNCTION, __LINE__, message, NULL, rw::core::debug::ASSERTFLAG_FATAL)) {rwBREAK();}

            /**
              \brief Fatal assert macro without message

              Use this macro for asserts without a message
              \see rw::core::debug::Manager::AssertFailed, rwASSERTM, rwWARNING

              Note that the message format string plus any
              args must be enclosed in parenthesis print on
              a given channel, channel is specified by name
            */
#define rwASSERT(condition)                                             \
                if((condition) ? false : CallDebugPrintAssertCallback(#condition, __FILE__, RW_CURRENT_FUNCTION, __LINE__, NULL, NULL, rw::core::debug::ASSERTFLAG_FATAL)) {rwBREAK();}

            /**
              Note that the message format string plus any
              args must be enclosed in parenthesis print on
              a given channel, channel is specified by name
            */
// use the conditional operator to get around constant condition warnings when a formatted rwABORT is desired
#define rwASSERTMFL(condition, message, file, line)                     \
                if((condition) ? false : rw::core::debug::AssertMacroHelper(#condition, file, RW_CURRENT_FUNCTION, line, rw::core::debug::ASSERTFLAG_FATAL).AssertFormatted message) {rwBREAK();}

            /**
              \brief Fatal assert macro with formatted message

              Use this macro when there are additional arguments after the message argument.
              \see rw::core::debug::Manager::AssertFailed, rwASSERT
            */
#define rwASSERTM(condition, message)                                   \
                rwASSERTMFL(condition, message, __FILE__, __LINE__)

#define rwASSERTALIGN(ADDR, ALIGN)                                      \
                if(!rw::IsMemAligned(ADDR, ALIGN) && CallDebugPrintAssertCallback("Misaligned address", __FILE__, RW_CURRENT_FUNCTION, __LINE__, #ADDR" must have alignment of at least "#ALIGN, NULL, rw::core::debug::ASSERTFLAG_FATAL)) {rwBREAK();}

            /**
              \brief debug print to system channel.

              this macro will print the given printf() formatted
              string to the system channel. The nature of this macro requires that the entire
              printf() formatted string be enclosed in parenthesis.
              \par Example:
              <tt>rwMESSAGE(("testing \%d \%d \%d", 1, 2, 3));</tt>

              Note that the message format string plus any
              args must be enclosed in parenthesis print on
              a given channel, channel is specified by name
            */
#define rwMESSAGE(message)                                              \
                rw::core::debug::PrintMacroHelper(__FILE__,__LINE__,(rw::core::debug::Channel *)NULL).PrintFormattedChannel message

            /**
              \brief debug print to named channel.

              this macro will print the given printf() formatted
              string to the named channel. The nature of this macro requires that the entire
              printf() formatted string be enclosed in parenthesis.
              \par Example:
              <tt>rwCMESSAGE("my channel",("testing \%d \%d \%d", 1, 2, 3));</tt>

              Note that the message format string plus any
              args must be enclosed in parenthesis print on
              a given channel, channel is specified by name
            */
#define rwCMESSAGE(channelname, message)                                \
                rw::core::debug::PrintMacroHelper(__FILE__,__LINE__,(const char *)channelname).PrintFormatted message

            /**
              \brief debug print to channel pointer.

              Given a channel pointer, this macro will print the given printf() formatted
              string to the channel. The nature of this macro requires that the entire
              printf() formatted string be enclosed in parenthesis.
              \par Example:
              <tt>rwCHMESSAGE(&myChannel, ("testing \%d \%d \%d", 1, 2, 3));</tt>

              print on a given channel object (channel
              object specified, not the name of a channel)
            */
#define rwCHMESSAGE(pChannel, message)                                  \
                rw::core::debug::PrintMacroHelper(__FILE__,__LINE__,(rw::core::debug::Channel *)pChannel).PrintFormattedChannel message

            /**
              \brief Non-fatal assert macro with formatted message

              Warnings are considered non-fatal, so processing can continue after the
              assert is handled.  Use this macro when there are additional arguments after
              the message argument.
              \see rw::core::debug::Manager::AssertFailed, rwASSERT, rwASSERTM

              Note that the message format string plus
              any args must be enclosed in parenthesis
            */
#define rwWARNING(condition, message)                                   \
                if (!(condition)                                        \
                    && rw::core::debug::AssertMacroHelper(#condition, __FILE__, RW_CURRENT_FUNCTION, __LINE__, rw::core::debug::ASSERTFLAG_NONFATAL).AssertFormatted message) \
                {rwBREAK();}


// rwVERBOSE can be defined per host, or defaults to rwMESSAGE if RWVERBOSE is defined, or dissolves otherwise
#ifndef rwVERBOSE
# ifdef RWVERBOSE
#  define rwVERBOSE(I) rwMESSAGE(I)
# else
#  define rwVERBOSE(I)
# endif
#endif

#else

#define rwABORT(message)
#define rwASSERT(condition)
#define rwASSERTM(condition, message)
#define rwASSERTMFL(condition, message, file, line)
#define rwASSERTALIGN(ADDR, ALIGN)
#define rwMESSAGE(message)
#define rwCMESSAGE(channel, message)
#define rwCHMESSAGE(channel, message)
#define rwWARNING(condition, message)
#define rwVERBOSE(I)

#endif

#ifdef _MSC_VER
#pragma warning(pop)
#endif

        } // end namespace debug
    } // end namespace core
}// end namespace rw


// ------------------------------------------------------------
// SPU VERSION RIGHT NOW THESE ARE ALL STUBS
// WE WILL PROBABLY CHANGE THE PROTOTYPES OF THESE
// MACROS FOR THE SPU TO HELP KEEP CODE SIZE SMALL
// ON THE SPU.
// ------------------------------------------------------------
#else // defined(rwHOST_PS3_SPU)

#include <spu_printf.h>
#include <sys/spu_stop_and_signal.h>

#ifndef rwDEPRECATED
    #define rwDEPRECATED()
#endif

#ifndef rwDEPRECATEDM
    #define rwDEPRECATEDM(alternativeString)
#endif

    #define rwDEPRECATEACCESS(T, oldMember, newMember)
    #define rwDEPRECATEACCESSPTRTYPE(T, oldMember, newMember)
    #define rwMAXDEBUGSTRINGLEN 1024

#ifndef rwVERBOSE
    #define rwVERBOSE(I)
#endif

#if defined(EA_DEBUG)

    #define rwABORT(message) \
        spu_printf("Abort at file=%s, line=%d\n", __FILE__, __LINE__); \
        spu_printf message; \
        spu_stop(STOP_BREAK);

    #define rwASSERT(condition) \
        if(!(condition)) \
        { \
            spu_printf("Assert at %s, file=%s, line=%d\n", #condition, __FILE__, __LINE__); \
            spu_stop(STOP_BREAK); \
        }

    #define rwASSERTM(condition, message) \
        if(!(condition)) \
        { \
            spu_printf("Assert at %s, file=%s, line=%d\n", #condition, __FILE__, __LINE__); \
            spu_printf message; \
            spu_printf("\n"); \
            spu_stop(STOP_BREAK); \
        }

    #define rwASSERTMFL(condition, message, file, line) \
        if(!(condition)) \
        { \
            spu_printf("Assert at %s, file=%s, line=%d\n", #condition, file, line); \
            spu_printf message; \
            spu_printf("\n"); \
            spu_stop(STOP_BREAK); \
        }

    #define rwASSERTALIGN(ADDR, ALIGN)
    #define rwMESSAGE(message)
    #define rwCMESSAGE(channel, message)
    #define rwCHMESSAGE(channel, message)

    #define rwWARNING(condition, message) \
        spu_printf("Warning file=%s, line=%d\n", __FILE__, __LINE__); \
        spu_printf message; \
        spu_printf("\n");

#else // defined(EA_DEBUG)

    #define rwABORT(message)
    #define rwASSERT(condition)
    #define rwASSERTM(condition, message)
    #define rwASSERTMFL(condition, message, file, line)
    #define rwASSERTALIGN(ADDR, ALIGN)
    #define rwMESSAGE(message)
    #define rwCMESSAGE(channel, message)
    #define rwCHMESSAGE(channel, message)
    #define rwWARNING(condition, message)
    #define rwVERBOSE(I)

#endif // defined(EA_DEBUG)

#endif // !defined(rwHOST_PS3_SPU)

#endif /* RW_DEBUG_H */
