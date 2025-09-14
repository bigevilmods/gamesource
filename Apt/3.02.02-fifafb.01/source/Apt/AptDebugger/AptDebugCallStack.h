/******************************************************************************/
/*!
    Copyright 2010 Electronic Arts Inc.

    \file       AptDebugCallStack.h

    \brief      Apt call stack wrapper class
*/
/******************************************************************************/



#ifndef _AptDebug_Callstack_h
#define _AptDebug_Callstack_h

/*** Include files ********************************************************************************/
#include "AptDebugger/ICallStack.h"

/*** Defines **************************************************************************************/
class AptFrameStack;
/*** Macros ***************************************************************************************/

/*** Type Definitions *****************************************************************************/

/******************************************************************************/
/*!
    \brief  Apt call stack information wrapper class
*/
/******************************************************************************/
class AptDebugCallStackItem : public ICallStackItem
{
public:
                                AptDebugCallStackItem();
    virtual                     ~AptDebugCallStackItem();

    // ICallStackItem interface implementation.

    // Return the file name where the function call is.
    virtual const char*         GetFileName() const;

    // Return the function name which has been called
    // (By 2011-1-17 the usage of function name is just used to debug C++ side codes easily)
    // (At the end of the project if function name is still useless, we can delete this field)
    virtual const char*         GetFunctionName() const;

    // Return the last executed instruction offset before enter the function body.
    virtual unsigned int        GetEntranceOffset() const;

    // Return current runtime context.
    virtual AptValue*           GetCurrentContext() const;

    // Set file name where the function call is.
    void                        SetFileName(const char* fileName);

    // Set function name which has been called.
    void                        SetFunctionName(const char* functionName);

    // Set first statement offset of the target function.
    void                        SetEntranceOffset(unsigned int offset);

    //Set current context every time enter a new function.
    void                        SetCurrentContext(AptValue* currentContext);

    // Set FrameStack
    void                        SetFrameStack(AptFrameStack* );

    // Get FrameStack
    AptFrameStack*              GetFrameStack() const;

    // Get this object in the current context.
    AptValue*                   GetThisObject() const;

    // Set this object in the current context.
    void                        SetThisObject(AptValue* thisObject);

    // Reset all field of this object.
    void                        Clear();
private:
    const char*                 mFileName;                            //!< File name where the function call is.
    const char*                 mFunctionName;                        //!< Function name which has been called.
    unsigned int                mEntranceInstructionOffset;           //!< The last executed instruction offset before run into the function body.
    AptValue*                   mCurrentContext;                      //!< The current context of each call stack item.
    AptValue*                   mThisObject;                          //!< This object in the current context,
    AptFrameStack*              mFrameStack;                          //!< The current function's FrameStack which includes all locals.
};

/******************************************************************************/
/*!
    \brief  Apt call stack wrapper class

    Provides functions to record current function call stack information.
*/
/******************************************************************************/
class AptDebugCallStack : public ICallStack
{
public:
    APT_NEW_DELETE_OPERATORS
	APT_DECLARE_OBJECT_METRICS
                                AptDebugCallStack();
    virtual                     ~AptDebugCallStack();

    // Push a new item into call stack (Any time enter a function)
    void                        Push(const char* fileName, const char* functionName, const unsigned int offset, AptValue* currentContext, AptValue* thisObject);

    // Set the latest FrameStack into call stack
    void                        SetTopFrameStack(AptFrameStack* frameStack);

    // Set "this" object at the top item of call stack
    void                        SetTopThisObject(AptValue* thisObject);

    // Pop the last item from call stack (Any time out of a function)
    void                        Pop();

    // Set the current position when codes is being broken
    void                        SetCurrentOffset(unsigned int offset);

    // Set the current file name when codes is being broken.
    void                        SetCurrentFileName(const char* fileName);

    // Return the top item of the call stack
    const ICallStackItem*       GetTop() const;

    // ICallStack interface implementation

    // Get "this" object
    virtual AptValue*          GetThisObject(int level);

    // Return depth of the call stack
    virtual int                 GetDepth() const;

    // Return pointer to a specified call stack item.
    virtual const ICallStackItem* GetItem(int index) const;

    // Get the hit breakpoint break point offset.
    virtual unsigned int        GetCurrentOffset() const;

    // Get the hit breakpoint file name.
    virtual const char*         GetCurrentFileName() const;

    // Get function runtime context of specific called function included in the call stack items.
    virtual void                GetContext(int level, AptValue** owner) const;

    // Take the correct FrameStack got from call stack take instead of spFrameStack.
    virtual bool                ActivateOrderedFrameStack(int level);

    // Restore the current running function's FrameStack.
    virtual void                RecoverFrameStack(int level);

    // Get call stack information as string format.
    virtual int                GetCallStackAsString(char* buf, int contentBufferSize) const;

    enum
    {
        MAX_CALL_STACK_DEPTH       = 65,    // Max number of call stack depth
        MAX_CALL_STACK_ITEM_LENGTH = 128,   // Max string length of each call stack item
        MAX_CALL_STACK_CONTENT_LENGTH = MAX_CALL_STACK_DEPTH * MAX_CALL_STACK_ITEM_LENGTH, // Max string length of call stack content
        CALL_STACK_TOP_LEVEL       = -1    // If the parameter level of GetContext is -1 which means get top call stack item

    };

    // Get top item non-const version just used internally.
    AptDebugCallStackItem*      GetTop();

    // Get item non-const version just used internally.
    AptDebugCallStackItem*      GetItem(int index);

private:

    AptDebugCallStackItem       mItemList[MAX_CALL_STACK_DEPTH];    //!< Call stack item array
    int                         mCount;                             //!< Current used items' count
    int                         mHighCount;                         //!< High water mark for mCount
    unsigned int                mCurrentOffset;                     //!< The offset of broken position in source codes
    char*                       mCurrentFileName;                   //!< File name of broken position in source codes
};

/*** Variables ************************************************************************************/

/*** Functions ************************************************************************************/

#endif // _AptDebug_Callstack_h
