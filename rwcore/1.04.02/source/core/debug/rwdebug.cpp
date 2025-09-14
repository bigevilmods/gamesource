// (c) Electronic Arts. All Rights Reserved.

/// \file
/// Generic debugging output functions.

#include "rwsharedheaders.hpp"
#ifndef RW_DEBUG_FULL_OVERLOAD
/*************************************************************************************************************
 Includes
 */

/*************************************************************************************************************
 Defines
 */

/*************************************************************************************************************
 Enums
 */

/*************************************************************************************************************
 Typedef Enums
 */

/*************************************************************************************************************
 Typedef Structs + Unions
 */

/*************************************************************************************************************
 Typedef Function Pointers
 */

/*************************************************************************************************************
 Structs + Unions
 */

/*************************************************************************************************************
 Static Variables
 */

/*************************************************************************************************************
 Global Variables
 */

/*************************************************************************************************************
 Function prototypes
 */

/*************************************************************************************************************
 Functions
 */

namespace rw
{

#ifdef EA_DEBUG
/**
Print out a message saying a function has been deprecated.

\param file the file containing the deprecated function.
\param line the line containing the deprecated function.
\param alternative a string describing an alternative to the deprecated function.
\param function the name of the deprecated function.
*/
void
Deprecated(RwChar *file, int32_t line, const RwChar *function, const RwChar *alternative)
{
    rwFUNCTION();

    rw::core::debug::Manager::GetInstance()->
        GetSystemChannel()->Print("%s(%d): Deprecated function %s. %s\n",
            file, line, function, alternative);

    rwRETURNVOID();
}
#endif // EA_DEBUG

/**
  sprintf style output to internal buffer
  \param format printf style format string
*/
RwChar *
rwDebugPrintHelper::WriteToBuffer(const RwChar* format, ...)
{
    rwFUNCTION();

    va_list ap;

    /* No debug macros here */

    va_start(ap, format);
    RwVsnprintf(m_buf, rwMAXDEBUGSTRINGLEN, format, ap);
    va_end(ap);
    rwRETURN(m_buf);
}

} // anonymous namespace


namespace rw
{
namespace core
{
namespace debug
{

// Global debug vectors, set to NULL by default
RwDebugPrintAssertCallback    gDebugPrintAssertCallback = 0;
RwDebugMessageCallback        gDebugMessage             = 0;
RwDebugMessageChannelCallback gDebugMessageChannel      = 0;

/**
  Set the debug message callback function

  This callback is called whenever RenderWare uses the
  rwMESSAGE or rwCMESSAGE macros. This callback only provides
  the channel name that should handle this message and not
  the channel itself.

  \see rw::core::debug::GetDebugPrintAssertFunction()
  \see rw::core::debug::GetDebugMessageFunction()
  \see rw::core::debug::GetDebugMessageChannelFunction()
  \see rw::core::debug::SetDebugMessageChannelFunction()
  \see rw::core::debug::SetDebugPrintAssertFunction()

  NOTE: If you initialize the debug system these callbacks
  will get set to use the debug manager object, only
  override these vectors if you do not plan to use
  the debug manager.
*/
void SetDebugMessageFunction(RwDebugMessageCallback        RWUNUSED_RELEASE(callback))
{
    rwFUNCTION();
#ifdef EA_DEBUG
    gDebugMessage = callback;
#endif
    rwRETURNVOID();
}


/**
  Set the debug message callback function

  This callback is called whenever RenderWare uses the
  rwCHMESSAGE macro. This callback is provided with a reference to
  a channel object directly.

  \see rw::core::debug::GetDebugPrintAssertFunction()
  \see rw::core::debug::GetDebugMessageFunction()
  \see rw::core::debug::GetDebugMessageChannelFunction()
  \see rw::core::debug::SetDebugMessageFunction()
  \see rw::core::debug::SetDebugPrintAssertFunction()

  NOTE: If you initialize the debug system these callbacks
  will get set to use the debug manager object, only
  override these vectors if you do not plan to use
  the debug manager.
*/
void SetDebugMessageChannelFunction(RwDebugMessageChannelCallback RWUNUSED_RELEASE(callback))
{
    rwFUNCTION();
#ifdef EA_DEBUG
    gDebugMessageChannel = callback;
#endif
    rwRETURNVOID();
}

/**
  Set the debug assert callback function

  This callback is called whenever RenderWare uses the
  rwASSERT* macros. This callback matches the prototype of
  the debug managers AssertFailed function.

  \see rw::core::debug::GetDebugPrintAssertFunction()
  \see rw::core::debug::GetDebugMessageFunction()
  \see rw::core::debug::GetDebugMessageChannelFunction()
  \see rw::core::debug::SetDebugMessageFunction()
  \see rw::core::debug::SetDebugMessageChannelFunction()

  NOTE: If you initialize the debug system these callbacks
  will get set to use the debug manager object, only
  override these vectors if you do not plan to use
  the debug manager.
*/
void SetDebugPrintAssertFunction(RwDebugPrintAssertCallback    RWUNUSED_RELEASE(callback))
{
    rwFUNCTION();
#ifdef EA_DEBUG
    gDebugPrintAssertCallback = callback;
#endif
    rwRETURNVOID();
}

// ------------------------------------------------------------
void  CallDebugMessageCallback(const char * RWUNUSED_RELEASE(channelname), const char * RWUNUSED_RELEASE(file),
                               uint32_t RWUNUSED_RELEASE(line), const char *RWUNUSED_RELEASE(format),
                               va_list RWUNUSED_RELEASE(args))
{
    rwFUNCTION();
#ifdef EA_DEBUG
    if(gDebugMessage != 0)
    {
        gDebugMessage(channelname,file,line,format,args);
    }
#endif
    rwRETURNVOID();
}

// ------------------------------------------------------------
void  CallDebugMessageChannelCallback(Channel * RWUNUSED_RELEASE(channel), const char * RWUNUSED_RELEASE(file),
                                      uint32_t RWUNUSED_RELEASE(line), const char *RWUNUSED_RELEASE(format),
                                      va_list RWUNUSED_RELEASE(args))
{
    rwFUNCTION();
#ifdef EA_DEBUG
    if(gDebugMessageChannel != 0)
    {
        gDebugMessageChannel(channel,file,line,format,args);
    }
#endif
    rwRETURNVOID();
}

// ------------------------------------------------------------
bool  CallDebugPrintAssertCallback(const char * RWUNUSED_RELEASE(condition), const char * RWUNUSED_RELEASE(file)   ,
                                   const char * RWUNUSED_RELEASE(function) , unsigned int RWUNUSED_RELEASE(line)   ,
                                   const char * RWUNUSED_RELEASE(format)   , va_list      RWUNUSED_RELEASE(argList),
                                   unsigned int RWUNUSED_RELEASE(flags))
{
    rwFUNCTION();
#ifdef EA_DEBUG
    if(gDebugPrintAssertCallback != 0)
    {
        rwRETURN(gDebugPrintAssertCallback(condition,file,function,line,format,argList,flags));
    }
#endif
    rwRETURN(true);
}

}
}
}

#endif /* RW_DEBUG_FULL_OVERLOAD */
