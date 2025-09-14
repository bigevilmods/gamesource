/*H************************************************************************************************/
/*!

    \File    _AptBasePtrStack.h

    \Description
        Templated stack class

    \Notes
        Previously AptBaseStack is used which can take any object as
        parameter. We changed it to AptBasePtrStack as inside Apt there is
        not a single occasion where we create a stack of objects.
        We always need to push AptValue * on to stack and so there is no need to
        call constructor or destructor on those pointers.
        Also there is need to call pfnMemAlloc for allocation of all these stacks.
        Previously all stack allocations (new/delete) were going thru global new/delete
        which is not good as all allocations inside Apt should go thru pfnMemAlloc.
        So changed AptBaseStack to AptBasePtrStack and instead of defining the stack
        variable like this
        AptBaseStack<AptValue *> stack;
        user should define it like this
        AptBasePtrStack<AptValue> stack ;
        Event if T is AptValue here and not AptValue *, inside implementation of
        AptValuePtrStack it always uses T *.


    \Copyright
        (c) 2004 Electronic Arts Inc.

    \Version    1.0        04/21/04 (ASK) First Version
    \Version    1.1        11/01/04 (DC)  Changed to use allocation Macro to remove #if defines
                                          everywhere. There was not change to the logic.
    \Version    1.2        07/29/08 (RB)  Added ability to track AptHighwaterMemoryMetrics

*/
/************************************************************************************************H*/


#ifndef __AptBasePtrStack_h
#define __AptBasePtrStack_h

/*** Include files ********************************************************************************/
#include <EASTL/stack.h>
#include "AptAllocator.h"

/*** Defines **************************************************************************************/

/*** Macros ***************************************************************************************/

/*** Type Definitions *****************************************************************************/

template <class T>
class AptBasePtrStack
{
public:
    AptBasePtrStack()
    :
    m_pMetrics(NULL)
    {
        //  Do nothing...
    }

    ~AptBasePtrStack()
    {
        Shutdown();
    }

    void    Init(const int32_t nCapacity, AptHighwaterMemoryMetrics* pMetrics)
    {
        APT_ASSERT(m_aElements.size() == 0);
		m_aElements.get_container().set_capacity(nCapacity);
        m_pMetrics = pMetrics;
    }

    void    Shutdown()
    {
		APT_ASSERT(m_aElements.size() == 0);
		m_aElements.get_container().clear();
    }

    int32_t GetCapacity() const
    {
        return  (int32_t)m_aElements.get_container().capacity();
    }

    int32_t GetSize() const
    {
        return  (int32_t)m_aElements.size();
    }

    void    Push(T* element)
    {
		m_aElements.push(element);

		// 5/12/04.
        // this is added as if deferred vector is full then we need
        // to switch back to older way of APT_INC/APT_DEC for stack operations.
        // Actually using AptValue * is not correct as this is a templated class
        // but here we assume that T is always a T * as we are not using this class
        // for anythign else rightnow in Apt.
        AptValue * pValue = (AptValue *) element ;
        APT_INC(pValue) ;

#if defined(EA_DEBUG)
        if ( ((int32_t)m_aElements.size()) > m_pMetrics->mHighwaterMaxCount )
        {
            m_pMetrics->mHighwaterMaxCount = (int32_t)m_nElements.size();
        }
#endif // EA_DEBUG
    }

    void    PushNoInc(T* element)
    {
        m_aElements.push(element);
    }

    void    Pop()
    {
        if(m_aElements.empty())
        {
            APT_ASSERT(false && "[APT] Error, Popping from Stack with 0 elements. Please contact the Apt Team for Support.");
        }
        else
        {
            // 5/12/04.
            // this is added as if deferred vector is full then we need
            // to switch back to older way of APT_INC/APT_DEC for stack operations.
            AptValue * pValue = (AptValue *) m_aElements.top();
            APT_DEC(pValue) ;
            m_aElements.pop();
        }
    }

    void    Pop(const int32_t nItems)
    {
        APT_ASSERT(nItems >= 0);
        if((int32_t)m_aElements.size() < nItems)
        {
            APT_ASSERT(false && "[APT] Error, Popping more elements than the stack contains. Please contact the Apt Team for Support.");
        }
        else
        {
            // 5/12/04.
            // this is added as if deferred vector is full then we need
			// to switch back to older way of APT_INC/APT_DEC for stack operations.
            for(int iLoop = 1; iLoop <= nItems; iLoop++)
            {
                AptValue * pValue = (AptValue *) m_aElements.top();
				APT_DEC(pValue);
				m_aElements.pop();
            }
        }
    }

    void    PopNoDec()
    {
        if(m_aElements.empty())
        {
            APT_ASSERT(false && "[APT] Error, Popping from Stack with 0 elements. Please contact the Apt Team for Support.");
        }
        else
        {
            m_aElements.pop();
        }
    }

    void    PopAndPush(const int32_t nItems, T* element )
    {
        APT_ASSERT(nItems >= 0);

        if((int32_t)m_aElements.size() < nItems)
        {
            APT_ASSERT(false && "[APT] Error, Popping more elements than the stack contains. Please contact the Apt Team for Support.");
        }
        else
        {
            {
                AptValue * pValue = (AptValue *) element;
                APT_INC(pValue);
            }

            // 5/12/04.
            // this is added as if deferred vector is full then we need
			// to switch back to older way of APT_INC/APT_DEC for stack operations.
            for(int iLoop = 1; iLoop <= nItems; iLoop++)
            {
                AptValue * pValue = (AptValue *) m_aElements.top();
				APT_DEC(pValue);
				m_aElements.pop();
            }

            m_aElements.push(element);
        }
    }

    void    SafePop(const int32_t nItems)
    {

        if (nItems <= 0)
        {
            return;
        }

        if((int32_t)m_aElements.size() < nItems)
        {
            APT_ASSERT(false && "[APT] Error, Popping more elements than the stack contains. Please contact the Apt Team for Support.");
        }
        else
        {

            // 5/12/04.
            // this is added as if deferred vector is full then we need
			// to switch back to older way of APT_INC/APT_DEC for stack operations.
            for(int iLoop = 1; iLoop <= nItems; iLoop++)
            {
                AptValue * pValue = (AptValue *) m_aElements.top();
				APT_DEC(pValue);
				m_aElements.pop();
            }
        }
    }

    T*       Top()
    {
        return  m_aElements.top();
    }

    T*      At(const int32_t nPos) const
    {
        APT_ASSERT(m_aElements.size() - nPos > 0);
        return  (m_aElements.get_container()[m_aElements.size() - nPos - 1]);
    }

    T*       GetPop()
    {
        APT_ASSERT(GetSize() >= 1);
		T* pValue = m_aElements.top();
		m_aElements.pop();
        return  pValue;
    }

private:
    eastl::stack<T*, eastl::vector<T*, AptStdAllocator>> m_aElements;
    AptHighwaterMemoryMetrics* m_pMetrics;
};

/*** Variables ************************************************************************************/

/*** Functions ************************************************************************************/

#endif // __AptBasePtrStack_h
