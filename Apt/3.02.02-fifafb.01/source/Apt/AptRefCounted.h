/*H************************************************************************************************/
/*!

    \file    Apt/AptRefCounted.h

    \brief         Simple description of main purpose of file.


        (c) 2005 Electronic Arts Inc.

    \version    1.0        06/17/07 ($senzee-lockless) First Version

*/
/************************************************************************************************H*/

#ifndef _Apt_AptRefCounted_h
#define _Apt_AptRefCounted_h

#include "AptDefine.h"

template <typename T>
class AptRefCounted : public T
{
    mutable int mnRefCount;

public:

    inline AptRefCounted() : mnRefCount(1) {}

    void AddRef() const
    {
        mnRefCount++;
    }
    void Release() const
    {
        if (!--mnRefCount)
        {
            AptRefCounted<T> *pTemp = const_cast<AptRefCounted<T> *>(this);
            delete pTemp;
        }
    }
};

template <typename T> inline T *SafeAddRef (T *p) { if (p) p->AddRef();  return p; }
template <typename T> inline T *SafeRelease(T *p) { if (p) p->Release(); return 0; }

#endif // _Apt_AptRefCounted_h

