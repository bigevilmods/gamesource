/*H************************************************************************************************/
/*!

    \File    _AptBaseStack.h

    \Description
        Templated stack class

    \Notes
        None.

    \Copyright
        (c) 2004 Electronic Arts Inc.

    \Version    1.0        03/10/04 (JSH) First Version

*/
/************************************************************************************************H*/


#ifndef __AptBaseStack_h
#define __AptBaseStack_h

/*** Include files ********************************************************************************/

/*** Defines **************************************************************************************/

/*** Macros ***************************************************************************************/

/*** Type Definitions *****************************************************************************/

template <class T>
class AptBaseStack
{
public:
    AptBaseStack()
    :
    m_nElements(0),
    m_nCapacity(0),
    m_aElements(NULL)
    {
        //  Do nothing...
    }

    AptBaseStack(const int32_t nCapacity)
    :
    m_nElements(0),
    m_nCapacity(nCapacity)
    {
        APT_ASSERT(_nSize > 0);
        m_aElements = new T[nCapacity];
    }

    ~AptBaseStack()
    {
        APT_ASSERT(m_nElements == 0);
        delete [] m_aElements;
    }

    void    Init(const int32_t nCapacity)
    {
        APT_ASSERT(m_nCapacity == 0);
        m_nCapacity = nCapacity;
        m_aElements = new T[nCapacity];
    }

    void    Shutdown()
    {
        APT_ASSERT(m_nElements == 0);
        m_nCapacity = 0;
        m_nElements = 0;
        delete [] m_aElements;
        m_aElements = NULL;
    }

    int32_t GetCapacity() const
    {
        return  (m_nCapacity);
    }

    int32_t GetSize() const
    {
        return  (m_nElements);
    }

    void    Push(T element)
    {
        APT_ASSERT(m_nElements < m_nCapacity);
        m_aElements[m_nElements++] = element;
    }

    void    Pop()
    {
        APT_ASSERT(GetSize() >= 1);
        --m_nElements;
    }

    void    Pop(const int32_t nItems)
    {
        APT_ASSERT(GetSize() >= nItems);
        APT_ASSERT(nItems >= 0);
        m_nElements -= nItems;
    }

    void    SafePop(const int32_t nItems)
    {
        APT_ASSERT(GetSize() >= nItems);
        if (nItems <= 0)
        {
            return;
        }
        m_nElements -= nItems;
    }

    T       Top()
    {
        return  (At(0));
    }

    T       At(const int32_t nPos) const
    {
        APT_ASSERT(m_nElements - nPos > 0);
        return  (m_aElements[m_nElements - nPos - 1]);
    }

    T       GetPop()
    {
        APT_ASSERT(GetSize() >= 1);
        return  (m_aElements[--m_nElements]);
    }

private:
    int32_t m_nElements;
    int32_t m_nCapacity;
    T *     m_aElements;
};

/*** Variables ************************************************************************************/

/*** Functions ************************************************************************************/

#endif // __AptBaseStack_h
