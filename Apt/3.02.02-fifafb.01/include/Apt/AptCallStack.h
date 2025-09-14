#ifndef _APTCALLSTACK_H
#define _APTCALLSTACK_H

/*H************************************************************************************************/
/*!

    \File
        AptCallStack.h

    \Description
        Extremely basic call stack for ActionScript use in opt builds of Apt.

    \Notes
        AptCallStack is designed to be safe with respect to overflow and underflow.  Because this
        is essentially a debugging aid and not a critical system, it is meant to be resilient and
        gracefully handle failure (ie, no asserts on overflow/underflow).

    \Copyright
        (c) 2008 Electronic Arts Inc.

    \Version    1.0        03/19/08 ($senzee) First version
    \Version    1.1        08/14/08 ($senzee) Adding further scope and file information to callstack
    \Version    1.2        06/22/09 (RB)      Moved APT_NEW_DELETE_OPERATORS from AptCallStack to AptStackItem;
                                              nobody should instantiate AptCallStack outside of Apt.

*/
/************************************************************************************************H*/

#include <EASTL/stack.h>
#include "AptValue/AptValue.h"
#include "AptAllocator.h"
#include "AptDefine.h"

class AptStackItem
{
public:

    APT_NEW_DELETE_OPERATORS
    APT_DECLARE_OBJECT_METRICS

    const char8_t *file;
    const char8_t *scope;
    const char8_t *function;
    AptValue      *context;

    inline AptStackItem(const char8_t *function = 0, const char8_t *file = "<native>", AptValue *context = 0, const char8_t *scope = 0)
        : file(file), scope(scope), function(function), context(context) {}
    inline AptStackItem(const AptStackItem &item)
        : file(item.file), scope(item.scope), function(item.function), context(item.context) {}
};


/// AptCallStack is not intended to be instantiated by user code.
/// An instance named gAptOptCallStack is provided for your debugging convenience:
/// in any build (even opt) set a watchpoint for gAptOptCallStack->mData,10
/// where 10 is the depth of the callstack. Then you can view the ActionScript
/// callstack in your C++ debugger.
class AptCallStack
{
public:

    AptCallStack()
    {
    }

    void Resize(int size)
    {
		mData.get_container().clear();
        mData.get_container().set_capacity(size);
    }

    ~AptCallStack()
    {
        Resize(0);
    }

    // Push() pushes a stack item (including the name of the function) onto the stack
    inline void Push(const AptStackItem &item)
    {
		mData.push(item);
    }

    // Push() pushes a stack item (including the name of the function) onto the stack
    inline void Push(const char8_t *function, const char8_t *file = "<native>", AptValue *context = 0, const char8_t *scope = 0)
    {
		Push(AptStackItem(function, file, context, scope));
    }

    // Top() returns the top item on the stack
    inline AptStackItem &Top()
    {
		if (mData.empty())
			return mNull;
        return mData.top();
    }

    // Pop() pops a string pointer (the name of the function) from the stack.
    inline void Pop()
    {
		if (!mData.empty())
		{
			mData.pop();
		}
    }

    // GetItem() returns the item corresponding to a given index.  If the index is out of range [0, MAX_ITEMS), it returns null.
    inline const AptStackItem &GetItem(int index) const
    {
        if (index >= 0 && index < (int32_t)mData.size())
        {
            // Only return an item at the index if we are in bounds.
            return mData.get_container()[index];
        }

        // Default case just returns a null pointer.
        return mNull;
    }

    // GetItemCount() returns the number of items in the stack.
    inline int GetItemCount() const
    {
        // clamp it at zero
        return mData.size();
    }

private:

	eastl::stack<AptStackItem, eastl::vector<AptStackItem, AptStdAllocator>> mData;
    AptStackItem    mNull;
};

#endif // _APTCALLSTACK_H
