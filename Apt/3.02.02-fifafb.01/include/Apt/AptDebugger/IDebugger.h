/******************************************************************************/
/*!
    Copyright 2010 Electronic Arts Inc.

    \file       IDebugger.h

    \brief      Debugger interface declaration
*/
/******************************************************************************/



#ifndef _AptDebugger_IDebugger_h
#define _AptDebugger_IDebugger_h

/*** Include files ********************************************************************************/

/*** Defines **************************************************************************************/

/*** Macros ***************************************************************************************/

/*** Type Definitions *****************************************************************************/
class IDebuggerListener;
class IBreakpoint;
class ICallStack;

/******************************************************************************/
/*!
    \brief  Interface of debugger

    Provide breakpoint, execution control interfaces to allow user debug action
    script at runtime.
*/
/******************************************************************************/

class IDebugger
{
public:
                                IDebugger() {}
    virtual                     ~IDebugger() {}

    /******************************************************************************/
    /*!
        IDebugger::RegisterListener
        \brief  Register a debug event listener.

        \param  listener        Pointer to a Callback function which will handle
                                the debug events.

        \note   Currently, only one listener is supported.
    */
    /******************************************************************************/
    virtual void                RegisterListener(IDebuggerListener* listener) = 0;

    /******************************************************************************/
    /*!
        IDebugger::UnregisterListener
        \brief  Unregister debug event listener.
    */
    /******************************************************************************/
    virtual void                UnregisterListener() = 0;

    /******************************************************************************/
    /*!
        IDebugger::AddBreakpoint
        \brief  Add a new breakpoint at the place which is specified by fileName
                and offset.

        \param  fileName        Indicate the file name which contains the breakpoint.
        \param  offset          Offset in byte-code stream where the breakpoint is.
        \param  timeStamp       Time stamp of host APT file.
    */
    /******************************************************************************/
    virtual void                AddBreakpoint(const char* fileName, unsigned int offset, char* timeStamp) = 0;

    /******************************************************************************/
    /*!
        IDebugger::RemoveBreakpoint
        \brief  Remove a breakpoint at the place which is specified by fileName
                and offset.

        \param  fileName        Indicate the file name which contains the breakpoint.
        \param  offset          Offset in byte-code stream where the breakpoint is.
    */
    /******************************************************************************/
    virtual void                RemoveBreakpoint(const char* fileName, unsigned int offset) = 0;

    /******************************************************************************/
    /*!
        IDebugger::RemoveAllBreakpoints
        \brief  Remove all breakpoints.
    */
    /******************************************************************************/
    virtual void                RemoveAllBreakpoints() = 0;

    /******************************************************************************/
    /*!
        IDebugger::GetBreakpoint
        \brief  Return a breakpoint's information which is specified by fileName
                and offset.

        \param  fileName        Indicate the file name which contains the breakpoint.
        \param  offset          Offset in byte-code stream where the breakpoint is.

        \return Return non-zero if breakpoint is found. Otherwise return NULL.
    */
    /******************************************************************************/
    virtual IBreakpoint*        GetBreakpoint(const char* fileName, unsigned int offset) = 0;

    /******************************************************************************/
    /*!
        IDebugger::GetBreakpointsNumber
        \brief  Return all instantiated breakpoints number

        \return Number of instantiated breakpoints.
    */
    /******************************************************************************/
    virtual int                 GetBreakpointsNumber() const = 0;

    /******************************************************************************/
    /*!
        IDebugger::GetBreakpoint
        \brief  Return a breakpoint's information by index.

        \param  index       Index of the breakpoint information which will be return.

        \return Return non-zero if breakpoint is found. Otherwise return NULL.

        \note   index must be less than number of instantiated breakpoints number.
    */
    /******************************************************************************/
    virtual IBreakpoint*        GetBreakpoint(int32_t index) const = 0;

    /******************************************************************************/
    /*!
        IDebugger::StepInto
        \brief  Execute instructions until research the end offset, if there is a
                function, step into it and stop execution.

        \param  endOffset   Array of offsets which are possible destination for the step into operation.
        \param  offsetCount Offsets counts in the offset array.
    */
    /******************************************************************************/
    virtual void                StepInto(unsigned int endOffset[], int offsetCount) = 0;

    /******************************************************************************/
    /*!
        IDebugger::StepOver
        \brief  Execute instructions until research the end offset, doesn't step
                into function.

        \param  endOffset   Array of offsets which are possible destination for the step over operation.
        \param  offsetCount Offsets counts in the offset array.
    */
    /******************************************************************************/
    virtual void                StepOver(unsigned int endOffset[], int offsetCount) = 0;

    /******************************************************************************/
    /*!
        IDebugger::StepOut

        \brief  Execute instructions until current function finished.
    */
    /******************************************************************************/
    virtual void                StepOut() = 0;

    /******************************************************************************/
    /*!
        IDebugger::Continue

        \brief  Continue execution
    */
    /******************************************************************************/
    virtual void                Continue() = 0;

    /******************************************************************************/
    /*!
        IDebugger::IsStopped

        \brief  To know if AS is under stop state.
    */
    /******************************************************************************/
    virtual bool                IsStopped() = 0;

    /******************************************************************************/
    /*!
        AptDebugger::GetTimeStamp
        \brief  Get time stamp of specific file name.

        \param  fileName        File name.
        \param  buf             Result bytes buffer

        \return  If the file exist return true, else return false;

        \note    Time stamp is 16 Hex bytes.
    */
    /******************************************************************************/
    virtual bool                GetTimeStamp(const char* fileName, char* buf) = 0;

    /******************************************************************************/
    /*!
        IDebugger::GetCallStack

        \brief   Get apt call stack

        \return  Get the pointer of call stack
    */
    /******************************************************************************/
    virtual ICallStack*          GetCallStack() = 0;

private:
                                IDebugger(const IDebugger& instance);
    IDebugger&                  operator=(const IDebugger& instance);
};

/*** Variables ************************************************************************************/

/*** Functions ************************************************************************************/

/******************************************************************************/
/*!
    AptGetDebugger
    \brief  Return pointer to the instance of IDebugger

    \return Return pointer to the instance of IDebugger
*/
/******************************************************************************/
IDebugger*                      AptGetDebugger();

#endif // _AptDebugger_IDebugger_h
