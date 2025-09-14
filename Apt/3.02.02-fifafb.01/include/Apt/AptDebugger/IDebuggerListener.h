/******************************************************************************/
/*!
    Copyright 2010 Electronic Arts Inc.

    \file       IDebuggerListener.h

    \brief      Interface for debugger event receiver.
*/
/******************************************************************************/



#ifndef _AptDebugger_IDebuggerListener_h
#define _AptDebugger_IDebuggerListener_h

/*** Include files ********************************************************************************/

/*** Defines **************************************************************************************/

/*** Macros ***************************************************************************************/

/*** Type Definitions *****************************************************************************/


/******************************************************************************/
/*!
    \brief  IDebuggerListener provides interfaces for debugger event handler.

    External module need to implement this class and pass it to AptDebugger.
*/
/******************************************************************************/
class IDebuggerListener
{
public:
    enum DeactivateReason
    {
        NORMAL_DEACTIVATE = 0,
        FILE_NOT_LOADED   = 1,
        FILE_OVERDUE      = 2
    };
    /******************************************************************************/
    /*! IDebuggerListener::~IDebuggerListener

        \brief  Virtusl destructor

    */
    /******************************************************************************/
    virtual                     ~IDebuggerListener() {};

    /******************************************************************************/
    /*! IDebuggerListener::BreakpointHit

        \brief  Event BreakpointHit receiver function. It's called by AptDebugger
                when a breakpoint is hit.

        \param  fileName    .swf file name which contains the breakpoint.
        \param  offset      Offset of the breakpoint in byte-code stream.
        \param  callStack   Current function call path in string format.

        \note   Make sure that this function implementation is blocked inside until
                one of these functions: AptDebugger::StepInto, AptDebugger::StepOver,
                AptDebugger::StepOut, AptDebugger::Continue is called.
    */
    /******************************************************************************/
    virtual void                BreakpointHit(const char* fileName, unsigned int offset, const char* callStack) = 0;

    /******************************************************************************/
    /*! IDebuggerListener::BreakpointAdd

        \brief  Event BreakpointAdd receiver function. It's called by AptDebugger
                when a breakpoint is add.

        \param  fileName    .swf file name which contains the breakpoint.
        \param  offset      Offset of the breakpoint in byte-code stream.
    */
    /******************************************************************************/
    virtual void                BreakpointAdd(const char* fileName, unsigned int offset) = 0;

    /******************************************************************************/
    /*! IDebuggerListener::BreakpointRemove

        \brief  Event BreakpointRemove receiver function. It's called by AptDebugger
                when a breakpoint is remove.

        \param  fileName    .swf file name which contains the breakpoint.
        \param  offset      Offset of the breakpoint in byte-code stream.
    */
    /******************************************************************************/
    virtual void                BreakpointRemove(const char* fileName, unsigned int offset) = 0;

    /******************************************************************************/
    /*! IDebuggerListener::BreakpointActivate

        \brief  Event BreakpointActivate receiver function. It's called by AptDebugger
                when a breakpoint is activate.

        \param  fileName    .swf file name which contains the breakpoint.
        \param  offset      Offset of the breakpoint in byte-code stream.
    */
    /******************************************************************************/
    virtual void                BreakpointActivate(const char* fileName, unsigned int offset) = 0;

    /******************************************************************************/
    /*! IDebuggerListener::BreakpointDeactivate

        \brief  Event BreakpointDeactivate receiver function. It's called by AptDebugger
                when a breakpoint is deactivate.

        \param  fileName    .swf file name which contains the breakpoint.
        \param  offset      Offset of the breakpoint in byte-code stream.
        \param  reason      The reason of breakpoint deactivate
    */
    /******************************************************************************/
    virtual void                BreakpointDeactivate(const char* fileName, unsigned int offset, DeactivateReason reason) = 0;

    /******************************************************************************/
    /*! IDebuggerListener::Trace

        \brief  Event BreakpointDeactivate receiver function. It's called by AptDebugger
                when a trace is executed.

        \param  message     The message to trace
    */
    /******************************************************************************/
    virtual void Trace(const char* message) = 0;
};

/*** Variables ************************************************************************************/

/*** Functions ************************************************************************************/

#endif // _AptDebugger_IDebuggerListener_h
