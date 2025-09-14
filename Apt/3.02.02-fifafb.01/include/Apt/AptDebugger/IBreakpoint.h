/******************************************************************************/
/*!
    Copyright 2010 Electronic Arts Inc.

    \file       IBreakpoint.h

    \brief      Interface for breakpoint information
*/
/******************************************************************************/



#ifndef _AptDebugger_IBreakpoint_h
#define _AptDebugger_IBreakpoint_h

/*** Include files ********************************************************************************/

/*** Defines **************************************************************************************/

/*** Macros ***************************************************************************************/

/*** Type Definitions *****************************************************************************/

/******************************************************************************/
/*!
    \brief  Saved breakpoint information
*/
/******************************************************************************/
class IBreakpoint
{
public:
    // Constructor/destructor
                                IBreakpoint() {}
    virtual                     ~IBreakpoint() {}

    /******************************************************************************/
    /*!
        IBreakpoint::GetFileName
        \brief  Returns file name where the breakpoint is placed

        \return File name for this breakpoint if there is no error. Otherwise
                return a empty string.
    */
    /******************************************************************************/
    virtual const char*         GetFileName() const = 0;

    /******************************************************************************/
    /*!
        IBreakpoint::GetOffset
        \brief  Returns the offset in bytecode stream where is breakpoint is placed.

        \return Offset of the breakpoint in bytecode stream.
    */
    /******************************************************************************/
    virtual unsigned int        GetOffset() const = 0;

    /******************************************************************************/
    /*!
        IBreakpoint::IsActivated()
        \brief  Returns activated/deactivated state of the breakpoint.
                Activated means the breakpoint is working now.
                Deactivated means it's not working until the script code is loaded.

        \return Returns true if the breakpoint is activated.
    */
    /******************************************************************************/
    virtual bool                IsActivated() const = 0;

    /******************************************************************************/
    /*!
        IBreakpoint::GetHitCount()
        \brief  Returns hit count of the breakpoint

        \return Returns hit count of the breakpoint
    */
    /******************************************************************************/
    virtual unsigned int        GetHitCount() = 0;

    /******************************************************************************/
    /*!
        IBreakpoint::SetHitCount()
        \brief  Set hit count of the breakpoint

        \param  count       New value for the hit count.

        \return Returns true if success. Otherwise return false.
    */
    /******************************************************************************/
    virtual bool                SetHitCount(unsigned int count) = 0;
private:
                                IBreakpoint(const IBreakpoint& instance);
    IBreakpoint&                operator=(const IBreakpoint& instance);
};
/*** Variables ************************************************************************************/

/*** Functions ************************************************************************************/

#endif // _AptDebugger_IBreakpoint_h
