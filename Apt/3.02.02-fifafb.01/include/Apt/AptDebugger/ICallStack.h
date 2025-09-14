/******************************************************************************/
/*!
    Copyright 2010 Electronic Arts Inc.

    \file       ICallStack.h

    \brief      Interface for call stack
*/
/******************************************************************************/



#ifndef _AptDebugger_ICallStack_h
#define _AptDebugger_ICallStack_h

/*** Include files ********************************************************************************/

/*** Defines **************************************************************************************/

/*** Macros ***************************************************************************************/

/*** Type Definitions *****************************************************************************/

/********************************************************************************/
/*!
    \brief  Record a function call information.
*/
/********************************************************************************/
class ICallStackItem
{
public:
    // Constructor/destructor
                                ICallStackItem() {}
    virtual                     ~ICallStackItem() {}

    /****************************************************************************/
    /*!
        ICallStackItem::GetFileName
        \brief  Returns the file name where the call stack item is.

        \return Returns the file name where the call stack item is.
    */
    /****************************************************************************/
    virtual const char*         GetFileName() const = 0;

    /****************************************************************************/
    /*!
        ICallStackStackItem::GetEntranceOffset
        \brief  Return the offset which is the entrance of calling the function in this callstack item.
        return  Return the offset which is the entrance of calling the function in this callstack item.
    */
    /****************************************************************************/
    virtual unsigned int        GetEntranceOffset() const = 0;

    /****************************************************************************/
    /*!
        ICallStackStackItem::GetFunctionName
        \brief  Return the function name which has been called.
        return  Return the function name which has been called.
    */
    /****************************************************************************/
    virtual const char*         GetFunctionName() const = 0;

    /****************************************************************************/
    /*!
        ICallStackItem::GetCurrentContext
        \brief  Return the current function runtime context
        return  Return the current function runtime context
    */
    /****************************************************************************/
    virtual AptValue*           GetCurrentContext() const = 0;

private:
    // Copy constructor/Assign operation
                                ICallStackItem(const ICallStackItem& instance);
    ICallStackItem&             operator=(const ICallStackItem& instance);
};

/*********************************************************************************/
/*!
    \brief  Record function call stack information
*/
/*********************************************************************************/
class ICallStack
{
public:
    // Constructor/destructor
                                ICallStack() {}
    virtual                     ~ICallStack() {}

    /*****************************************************************************/
    /*!
        ICallStack::GetDepth
        \brief  Returns depth of the call stack.

        \return Returns depth of the call stack.
    */
    /*****************************************************************************/
    virtual int                 GetDepth() const = 0;

    /*****************************************************************************/
    /*!
        ICallStack::GetItem
        \brief  Returns a ICallStackItem which is specified by index.

        \param  index       Specifies the index of item to be returned.

        \return Returns a pointer to ICallStackItem if there is no error. Otherwise
                returns NULL.
        \note   Index for root of the call stack is 0.
    */
    /******************************************************************************/
    virtual const ICallStackItem* GetItem(int index) const = 0;

    /*****************************************************************************/
    /*!
        ICallStack::GetContext
        \brief  Get function runtime context of specific called function included in the call stack items

        \param  level  level of call stack (specify which function in the call stack do you want to inquire.)
        \param  owner  Used to keep the inquire result.
    */
    /******************************************************************************/
    virtual void                GetContext(int level, AptValue** owner) const = 0;

    /*****************************************************************************/
    /*!
        ICallStack::ActivateOrderedFrameStack
        \brief  When user selected any level on call stack view, the frame stack should be
                set to corresponding one.

        \param  level  The selected level in the call stack view.

        \return  If spFrameStack has been changed.
    */
    /******************************************************************************/
    virtual bool                ActivateOrderedFrameStack(int level) = 0;

    /*****************************************************************************/
    /*!
        ICallStack::GetThisObject
        \brief  Get "this" object from call stack according to the call stack level.

        \param  level  Call stack level.

        \return  Corresponding "this" object
    */
    /******************************************************************************/
    virtual AptValue*           GetThisObject(int level) = 0;

    /*****************************************************************************/
    /*!
        ICallStack::RecoveryFrameStack
        \brief  After function ActivateOrdererFrameStack is called, Use RecoveryFrameStack to restore
                the right frameStack which is used to continue running action script.

        \param  level  The selected level in the call stack view.
    */
    /******************************************************************************/
    virtual void                RecoverFrameStack(int level) = 0;

    /*****************************************************************************/
    /*!
        ICallStack::GetCallStackAsString
        \brief  Get call stack information as string format

        \param  buf              Call stack buffer.
        \param  callStackLength  Call stack content buffer size.

        \return  Content length of call stack, the format is something like "xxx:offset;xx:offset"
    */
    /******************************************************************************/
    virtual int                GetCallStackAsString(char* buf, int callStackBufferSize) const = 0;
private:
    // Copy constructor/Assign operation
                                ICallStack(const ICallStack& instance);
    ICallStack&                 operator=(const ICallStack& instance);
};
/*** Variables ************************************************************************************/

/*** Functions ************************************************************************************/

#endif // _AptDebugger_ICallStack_h
