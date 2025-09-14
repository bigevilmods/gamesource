/******************************************************************************/
/*!
    Copyright 2010 Electronic Arts Inc.

    \file       AptDebugCallStack.cpp

    \brief      Apt call stack wrapper class
*/
/******************************************************************************/


/*** Include files ********************************************************************************/
#include "Apt.h"
#include "AptDebugger/AptDebugCallStack.h"
#include "AptObject/AptScriptFunction.h"
#ifdef APT_DEBUGGER_ENABLE


/*** Namespace ****************************************************************/

/*** Macros *******************************************************************/

/*** Constants ****************************************************************/

/*** Declarations *************************************************************/

/*** Variables ****************************************************************/
APT_DEFINE_OBJECT_METRICS_EXPLICIT(AptDebugCallStack, NULL);

/*** Implementation ***********************************************************/

/******************************************************************************/
/*!
    AptDebugCallStackItem::AptDebugCallStackItem

    \brief  AptDebugCallStackItem constructor
*/
/******************************************************************************/
AptDebugCallStackItem::AptDebugCallStackItem()
: mFileName(NULL)
, mFunctionName(NULL)
, mEntranceInstructionOffset(0)
, mCurrentContext(NULL)
, mFrameStack(NULL)
{
}

/******************************************************************************/
/*!
    AptDebugCallStackItem::AptDebugCallStackItem

    \brief  AptDebugCallStackItem destructor
*/
/******************************************************************************/
AptDebugCallStackItem::~AptDebugCallStackItem()
{
}

/******************************************************************************/
/*!
    AptDebugCallStackItem::GetFileName

    \brief  Return the file name where the function call is

    \return Return string of the file name.
*/
/******************************************************************************/
const char* AptDebugCallStackItem::GetFileName() const
{
    return mFileName;
}

/******************************************************************************/
/*!
    AptDebugCallStackItem::GetFunctionName

    \brief  Return the function name which has been called

    \return Return string of the function name.
*/
/******************************************************************************/
const char* AptDebugCallStackItem::GetFunctionName() const
{
    return mFunctionName;
}

/******************************************************************************/
/*!
    AptDebugCallStackItem::GetFunctionEntranceInstructionOffset

    \brief  Return the last executed instruction offset before enter function body.

    \return Return the last executed instruction offset before enter function body.
*/
/******************************************************************************/
unsigned int AptDebugCallStackItem::GetEntranceOffset() const
{
    return mEntranceInstructionOffset;
}

/******************************************************************************/
/*!
    AptDebugCallStackItem::GetCurrentContext

    \brief  Return runtime context which is mainly used to get local variables of each call stack item.

    \return Return runtime context.
*/
/******************************************************************************/
AptValue* AptDebugCallStackItem::GetCurrentContext() const
{
    return mCurrentContext;
}

/******************************************************************************/
/*!
    AptDebugCallStackItem::SetFileName

    \brief  Set file name where the function call is

    \param  fileName    String of the file name to be set.

    \note   File name must be always available when SWF file is in memory.
            So we don't allocate memory for the string of file name here.
*/
/******************************************************************************/
void AptDebugCallStackItem::SetFileName(const char* fileName)
{
    mFileName = fileName;
}

/******************************************************************************/
/*!
    AptDebugCallStackItem::SetTopThisObject

    \brief  Set "this" object

    \param  thisObject  "This" object
*/
/******************************************************************************/
void AptDebugCallStackItem::SetThisObject(AptValue* thisObject)
{
    mThisObject = thisObject;
}

/******************************************************************************/
/*!
    AptDebugCallStackItem::GetThisObject

    \brief  Get "this" object

    \return  Return "this" object
*/
/******************************************************************************/
AptValue* AptDebugCallStackItem::GetThisObject() const
{
    return mThisObject;
}

/******************************************************************************/
/*!
    AptDebugCallStackItem::SetFunctionName

    \brief  Set function name which has been called.

    \param  functionName    String of the function name to be set.
*/
/******************************************************************************/
void AptDebugCallStackItem::SetFunctionName(const char* functionName)
{
    mFunctionName = functionName;
}

/******************************************************************************/
/*!
    AptDebugCallStack::SetCurrentOffset

    \brief  Set current offset which is being hit.

    \param  Offset which is being hit.
*/
/******************************************************************************/
void AptDebugCallStack::SetCurrentOffset(unsigned int offset)
{
    mCurrentOffset = offset;
}

/******************************************************************************/
/*!
    AptDebugCallStackItem::SetCurrentInstructionOffset

    \brief  Set the last executed instruction offset in the function

    \param  offset      Offset of the last execute instruction to be set.
*/
/******************************************************************************/
void AptDebugCallStackItem::SetEntranceOffset(unsigned int offset)
{
    mEntranceInstructionOffset = offset;
}

/******************************************************************************/
/*!
    AptDebugCallStackItem::SetCurrentContext

    \brief  Set the current runtime context which used to be push into call stack

    \param  currentContext  Function runtime context
*/
/******************************************************************************/
void AptDebugCallStackItem::SetCurrentContext(AptValue* currentContext)
{
    mCurrentContext = currentContext;
}

/******************************************************************************/
/*!
    AptDebugCallStackItem::SetTopFrameStack

    \brief  Set the target function runtime frameStack

    \param  frameStack  Target function runtime frameStack
*/
/******************************************************************************/
void AptDebugCallStackItem::SetFrameStack(AptFrameStack* frameStack)
{
    mFrameStack = frameStack;
}

/******************************************************************************/
/*!
    AptDebugCallStackItem::GetFrameStack

    \brief  Get the target function runtime frameStack

    \return  Function runtime frameStack
*/
/******************************************************************************/
AptFrameStack* AptDebugCallStackItem::GetFrameStack() const
{
    return mFrameStack;
}

/******************************************************************************/
/*!
    AptDebugCallStackItem::Clear

    \brief  Clear call stack item information
*/
/******************************************************************************/
void AptDebugCallStackItem::Clear()
{
    mFileName                   = NULL;
    mFunctionName               = NULL;
    mCurrentContext             = NULL;
    mFrameStack                 = NULL;
    mEntranceInstructionOffset   = 0;
}

/******************************************************************************/
/*!
    AptDebugCallStack::AptDebugCallStack

    \brief  Constructor of AptDebugCallStack
*/
/******************************************************************************/
AptDebugCallStack::AptDebugCallStack()
: mCount(0)
, mHighCount(0)
{
}

/******************************************************************************/
/*!
    AptDebugCallStack::AptDebugCallStack

    \brief  Destructor of AptDebugCallStack
*/
/******************************************************************************/
AptDebugCallStack::~AptDebugCallStack()
{
    mCount  = 0;
}

/******************************************************************************/
/*!
    AptDebugCallStack::Push

    \brief  Push a new item into call stack

    \param  fileName              Name of the file where the function is
    \param  functionName          Name of the function
    \param  entranceOffset        Offset where call the function
    \param  currentContext        Function runtime context
    \param  thisObject            "this" object
*/
/******************************************************************************/
void AptDebugCallStack::Push(const char* fileName, const char* functionName, const unsigned int entranceOffset, AptValue* currentContext, AptValue* thisObject)
{
    APT_ASSERT(mCount >= 0 && mCount < MAX_CALL_STACK_DEPTH);

    if (mCount < MAX_CALL_STACK_DEPTH)
    {
    mItemList[mCount].SetFileName(fileName);
    mItemList[mCount].SetFunctionName(functionName);
    mItemList[mCount].SetEntranceOffset(entranceOffset);
    mItemList[mCount].SetCurrentContext(currentContext);
    mItemList[mCount].SetFrameStack(NULL);
    mItemList[mCount].SetThisObject(thisObject);
    }
    mCount++;

    if (mCount > mHighCount)
    {
        mHighCount = mCount;
}
}

/******************************************************************************/
/*!
    AptDebugCallStack::SetTopFrameStack

    \brief  Set FrameStack at the top item in the call stack

    \param  frameStack  frameStack need to be set

    \note   Since frameStack doesn't exist in every function for there is possible for
            a function which hasn't any local variable. Further more, when we call function
            AptDebugCallStack::Push the FrameStack is unavailable at that time, so these are
            why we have to add this function instead of put these codes into function AptDebugCallStack::Push.
*/
/******************************************************************************/
void AptDebugCallStack::SetTopFrameStack(AptFrameStack* frameStack)
{
    APT_ASSERT(mCount > 0);
    if (mCount <= MAX_CALL_STACK_DEPTH)
    {
    mItemList[mCount - 1].SetFrameStack(frameStack);
}
}

/******************************************************************************/
/*!
    AptDebugCallStack::SetTopThisObject

    \brief  Set "this" object.

    \param  thisObject  "this" object
*/
/******************************************************************************/
void AptDebugCallStack::SetTopThisObject(AptValue* thisObject)
{
    APT_ASSERT(mCount > 0);
    if (mCount <= MAX_CALL_STACK_DEPTH)
    {
    mItemList[mCount - 1].SetThisObject(thisObject);
}
}

/******************************************************************************/
/*!
    AptDebugCallStack::Pop

    \brief  Pop the last item from call stack
*/
/******************************************************************************/
void AptDebugCallStack::Pop()
{
    APT_ASSERT(mCount > 0 && mCount <= MAX_CALL_STACK_DEPTH);

    if (mCount <= MAX_CALL_STACK_DEPTH)
    {
    mItemList[mCount - 1].Clear();
    }

    mCount--;
}

/******************************************************************************/
/*!
    AptDebugCallStack::GetTop

    \brief  Return the top item of the call stack

    \return Pointer to a AptDebugCallStackItem at top of the call stack

    \note   If current call stack depth is greater than MAX_CALL_STACK_DEPTH,
            NULL is returned.
*/
/******************************************************************************/
const ICallStackItem* AptDebugCallStack::GetTop() const
{
    APT_ASSERT(mCount >= 0 && mCount <= MAX_CALL_STACK_DEPTH);

    if (mCount >= MAX_CALL_STACK_DEPTH)
    {
        return &mItemList[MAX_CALL_STACK_DEPTH - 1];
    }
    if (mCount > 0)
    {
        return &mItemList[mCount - 1];
    }
    return NULL;
}

/******************************************************************************/
/*!
    AptDebugCallStack::GetTop

    \brief  Return the top item of the call stack, non-const version just used internally.

    \return Pointer to a AptDebugCallStackItem at top of the call stack

    \note   If current call stack depth is greater than MAX_CALL_STACK_DEPTH,
            NULL is returned.
*/
/******************************************************************************/
AptDebugCallStackItem* AptDebugCallStack::GetTop()
{
    APT_ASSERT(mCount >= 0 && mCount <= MAX_CALL_STACK_DEPTH);

    if (mCount >= MAX_CALL_STACK_DEPTH)
    {
        return &mItemList[MAX_CALL_STACK_DEPTH - 1];
    }
    if (mCount > 0)
    {
        return &mItemList[mCount - 1];
    }
    return NULL;
}

/******************************************************************************/
/*!
    AptDebugCallStack::GetDepth

    \brief  Return depth of the call stack

    \return Depth of the call stack
*/
/******************************************************************************/
int AptDebugCallStack::GetDepth() const
{
    return mCount;
}

/******************************************************************************/
/*!
    AptDebugCallStack::GetThisObject

    \brief  Return "this" object

    \param  Ordered call stack level

    \return "this" object
*/
/******************************************************************************/
AptValue* AptDebugCallStack::GetThisObject(int level)
{
    int itemIndex = GetDepth() - level - 1;
    return GetItem(itemIndex)->GetThisObject();
}

/******************************************************************************/
/*!
    AptDebugCallStack::GetItem

    \brief  Return pointer to a specified call stack item by index.

    \param  index       Index of the item to be returned.

    \return Pointer to a call stack item instance if index is in range of
            [0, MAX_CALL_STACK_DEPTH), otherwise return NULL.
*/
/******************************************************************************/
const ICallStackItem* AptDebugCallStack::GetItem(int index) const
{
    APT_ASSERT(index >= 0 && index < mCount);

    if ( index >= 0 && index < MAX_CALL_STACK_DEPTH )
    {
        return &mItemList[index];
    }
    return NULL;
}

/******************************************************************************/
/*!
    AptDebugCallStack::GetItem

    \brief  Return pointer to a specified call stack item by index non-const version just used internally.

    \param  index       Index of the item to be returned.

    \return Pointer to a call stack item instance if index is in range of
            [0, MAX_CALL_STACK_DEPTH), otherwise return NULL.
*/
/******************************************************************************/
AptDebugCallStackItem* AptDebugCallStack::GetItem(int index)
{
    APT_ASSERT(index >= 0 && index < mCount);

    if ( index >= 0 && index < MAX_CALL_STACK_DEPTH )
    {
        return &mItemList[index];
    }
    return NULL;
}

/******************************************************************************/
/*!
    AptDebugCallStack::GetCurrentOffset

    \brief  return current offset which is being hit.

    \return current offset which is being hit
*/
/******************************************************************************/
unsigned int AptDebugCallStack::GetCurrentOffset() const
{
    return mCurrentOffset;
}

/******************************************************************************/
/*!
    AptDebugCallStack::SetCurrentFileName

    \brief  Set current running file name (Only happened when hit a breakpoint).

    \param  fileName Current running file name

    \note   File name must be always available when SWF file is in memory.
            So we don't allocate memory for the string of file name here.
*/
/******************************************************************************/
void AptDebugCallStack::SetCurrentFileName(const char* fileName)
{
    mCurrentFileName = const_cast<char*>(fileName);
}

/******************************************************************************/
/*!
    AptDebugCallStack::GetCurrentFileName

    \brief  Get current running file name (Only available when hit a breakpoint)

    \return Return current running file name.
*/
/******************************************************************************/
const char* AptDebugCallStack::GetCurrentFileName() const
{
    return mCurrentFileName;
}

/******************************************************************************/
/*!
    AptDebugCallStack::GetContext

    \brief  Return current runtime context which keeps in call stack

    \param  level  level of call stack (specify which function in the call stack do you want to inquire.)
    \param  owner  Used to keep the inquire result.

    \note   Level starts from 0. 0 means get top level and 1 means hypo-top level.
*/
/******************************************************************************/
void AptDebugCallStack::GetContext(int level, AptValue** owner) const
{
    APT_ASSERT(0 <= level);
    int itemIndex = GetDepth() - level - 1;
    *owner = GetItem(itemIndex)->GetCurrentContext();
}

/******************************************************************************/
/*!
    AptDebugCallStack::ActivateOrderedFrameStack

    \brief  Corresponding frame stack of the specified call stack level will be activated
            in order to get its local variables.

    \param  level  User ordered level in call stack
    \param  Whether frameStack has been changed.

    \note   This function must be called before get local variable name list or get local variable value.
*/
/******************************************************************************/
bool AptDebugCallStack::ActivateOrderedFrameStack(int level)
{
    bool isFrameStackChanged = false;
    AptValue * owner = NULL;
    GetContext(level, &owner);
    int itemIndex = GetDepth() - level - 1;
    if(owner->isScriptFunction())
    {
        AptScriptFunctionBase* functionContext = reinterpret_cast<AptScriptFunctionBase*>(owner);
        AptDebugCallStackItem* callStackItem = GetItem(itemIndex);
        functionContext->SetTopFrameStack(callStackItem->GetFrameStack());
        isFrameStackChanged = true;
    }
    return isFrameStackChanged;
};

/******************************************************************************/
/*!
    AptDebugCallStack::RecoverFrameStack

    \brief  This function only works when user click any item in the CallStack view.
            And in this case, just after user gets the correct locals, we should use
            this function to restore the current running function's FrameStack.

    \param  level  User ordered level in call stack
*/
/******************************************************************************/
void AptDebugCallStack::RecoverFrameStack(int level)
{
    AptValue* owner = NULL;
    // Level starts from 0. 0 means get top level and 1 means hypo-top level.
    GetContext(level, &owner);
    if (owner->isScriptFunction())
    {
        AptScriptFunctionBase* functionContext = reinterpret_cast<AptScriptFunctionBase*>(owner);
        AptDebugCallStackItem* topItem = GetTop();
        // The current running function must be the top function in the call stack.
        functionContext->SetTopFrameStack(topItem->GetFrameStack());
    }
}

/******************************************************************************/
/*!
    AptDebugCallStack::GetCallStackAsString

    \brief  Organize all call stack information as string format.

    \param  buf  The result string buffer.
    \param  contentLength  Content buffer size of call stack.

    \return  The content length of call stack.

    \note  If buf equal NULL which means Just get content length of call stack.
           And contentLength is not used.
*/
/******************************************************************************/
int AptDebugCallStack::GetCallStackAsString(char* buf, int contentBufferSize) const
{
    char oneItemString[MAX_CALL_STACK_ITEM_LENGTH];
    sprintf(oneItemString, "%s:%x", GetCurrentFileName(), GetCurrentOffset());

    if (NULL != buf)
    {
        strcat(buf, oneItemString);

    }
    else
    {
        contentBufferSize = MAX_CALL_STACK_CONTENT_LENGTH;
    }

    size_t stringLength = strlen(oneItemString);
    for (int i = GetDepth() - 1; i >= 0; --i)
    {
        const ICallStackItem* callStackItem = GetItem(i);
        char oneItemString[MAX_CALL_STACK_ITEM_LENGTH]= {0};
		// FIFA_BEGIN | DIVERGENCE | nmeyer | 2020-02-03 | Add function name to call stack record.
        sprintf(oneItemString, ";%s::%s:%x", callStackItem->GetFileName(),callStackItem->GetFunctionName(),callStackItem->GetEntranceOffset());
		// FIFA_END
        size_t itemLength = strlen(oneItemString);

        if (stringLength + itemLength + 1 <= contentBufferSize) // "+1" means '\0' at the end of string
        {
            if (NULL != buf)
            {
                strcat(buf, oneItemString);
            }
            stringLength += itemLength;
        }
    }
    return (int)stringLength + 1; // Include "\0" at the end of string
}

#endif
