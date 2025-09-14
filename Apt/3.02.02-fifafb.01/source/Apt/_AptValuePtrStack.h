/*H************************************************************************************************/
/*!

    \File    _AptValuePtrStack.h

    \Description
        Templated stack class

    \Notes
        Previously AptValueStack is used which can take any object as
        parameter. We changed it to AptValuePtrStack as inside Apt there is
        not a single occasion where we create a stack of objects.
        We always need to push AptValue * on to stack and so there is no need to
        call constructor or destructor on those pointers.
        Also there is need to call pfnMemAlloc for allocation of all these stacks.
        Previously all stack allocations new/delete were going thru global new/delete
        which is not good as all allocations inside Apt should go thru pfnMemAlloc.
        So changed AptValueStack to AptValuePtrStack and instead of defining the stack
        variable like this
        AptValueStack<AptValue *> stack;
        user should define it like this
        AptValuePtrStack<AptValue> stack ;
        Event if T is AptValue here and not AptValue *, inside implementation of
        AptValuePtrStack it always uses T *.

    \Copyright
        (c) 2004 Electronic Arts Inc.

    \Version    1.0        04/20/04 (ASK) First Version
    \Version    1.1        11/01/04 (DC)  Changed to use allocation Macro to remove #if defines
                                          everywhere. There was no change to the logic.
    \Version    1.2        07/29/08 (RB)  Added ability to track AptHighwaterMemoryMetrics (debug only)

*/
/************************************************************************************************H*/


#ifndef __AptValuePtrStack_h
#define __AptValuePtrStack_h

/*** Include files ********************************************************************************/

#include <EABase/eabase.h>
#include <EASTL/stack.h>
#include "AptAllocator.h"
#include "AptMemoryMetrics.h"

/*** Defines **************************************************************************************/

/*** Macros ***************************************************************************************/

/*** Type Definitions *****************************************************************************/



template <class T>
class AptValuePtrStack
{
public:
    AptValuePtrStack()
#if defined(EA_DEBUG)
          : m_pMetrics(NULL)
#endif //EA_DEBUG
    {
    }

    AptValuePtrStack(int _nSize, AptHighwaterMemoryMetrics*
#if defined(EA_DEBUG)
        pMetrics
#endif //EA_DEBUG
        )
#if defined(EA_DEBUG)
          : m_pMetrics(pMetrics)
#endif //EA_DEBUG
    {
        APT_ASSERT(_nSize > 0);

        m_aElements.get_container().set_capacity(_nSize);
    }

    ~AptValuePtrStack()
    {
		shutdown();
    }

    void init(int _nSize, AptHighwaterMemoryMetrics*
#if defined(EA_DEBUG)
        pMetrics
#endif //EA_DEBUG
    )
    {
		APT_ASSERT(m_aElements.size() == 0);
        m_aElements.get_container().set_capacity(_nSize);

#if defined(EA_DEBUG)
        m_pMetrics = pMetrics;
#endif //EA_DEBUG
    }

    void shutdown()
    {
		APT_ASSERT(m_aElements.size() == 0);
		m_aElements.get_container().clear();
    }

    int capacity() { return (int32_t)m_aElements.get_container().capacity(); }

    int size()
    {
        return (int32_t)m_aElements.size();
    }

    void push(T * element)
    {
        m_aElements.push(element);
        APT_INC(element);

#if defined(EA_DEBUG)
        if ( ((int32_t)m_aElements.size()) > m_pMetrics->mHighwaterMaxCount )
        {
            m_pMetrics->mHighwaterMaxCount = (int32_t)m_aElements.size();
        }
#endif // EA_DEBUG
    }

    void pop()
    {
        APT_ASSERT(m_aElements.size() > 0);
		T* element = m_aElements.top();
        APT_DEC(element);
		m_aElements.pop();
    }

    T * top()
    {
        return m_aElements.top();
    }

    T * at(int nPos)
    {
        APT_ASSERT(m_aElements.size() - nPos > 0);
        return m_aElements.get_container()[m_aElements.size() - nPos - 1];
    }

    void SetAt(int nPos, T * element)
    {
        APT_ASSERT(m_aElements.size() - nPos > 0);
        if(element != m_aElements.get_container()[m_aElements.size() - nPos - 1])
        {
            //APT_DECSAFE(m_aElements[m_nElements - nPos - 1]);
            m_aElements.get_container()[m_aElements.size() - nPos - 1] = element;
            //APT_INC(element);
        }
    }
private:
    eastl::stack<T*, eastl::vector<T*, AptStdAllocator>> m_aElements;
#if defined(EA_DEBUG)
    AptHighwaterMemoryMetrics* m_pMetrics;
#endif //EA_DEBUG
};

/*** Variables ************************************************************************************/

/*** Functions ************************************************************************************/

#endif // __AptValuePtrStack_h
