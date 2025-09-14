/*H************************************************************************************************/
/*!

    \File
        AptSharedPtr.h

    \Description
        A simple shared-pointer implementation. Although the class is generic
        it is only intended to be used with AptFile and AptLinkerThingy data types.

    \Notes
        None.

    \Copyright
        (c) 2004 Electronic Arts Inc.

    \Version
        1.0        03/07/04 (ONA) First Version
        1.1        04/16/08 (RB)  Replaced AptSharedPtr<T>::Dispose() with AptSharedPtrDispose(AptFile *p)
                                  and AptSharedPtrDispose(AptLinkerThingy *p). This removes the
                                  need to lock when p==NULL.
*/
/************************************************************************************************H*/

#ifndef __APTSHAREDPTR_H__
#define __APTSHAREDPTR_H__

#include <stddef.h>
#include "AptDefine.h"

template <typename T> int AptSharedPtrIncRef(T *p);
template <typename T> int AptSharedPtrDecRef(T *p);
template <typename T> void AptSharedPtrDelete(T *p);

// Ugliness for stupid compilers (this ought to be covered by the above templates)
#if defined(EA_DEBUG)          // $BUG253 LEO    23/47
void AptSharedPtrSetFileSize(AptFile *p, int nSize);
int AptSharedPtrGetFileSize(AptFile *p);
#endif

struct AptFile;
int AptSharedPtrIncRef(AptFile *p);
void AptSharedPtrDispose(AptFile *p);

struct AptLinkerThingy;
int AptSharedPtrIncRef(AptLinkerThingy *p);
void AptSharedPtrDispose(AptLinkerThingy *p);

template<class T>
class AptSharedPtr
{
    public:

        explicit AptSharedPtr(T* p = 0) : pData(p)
        {
#if defined(REFCOUNT_DEBUGGING)
            if (pData)
                AptSharedPtrIncRef(pData, this);
#else
            if (pData)
                AptSharedPtrIncRef(pData);
#endif
        }

        AptSharedPtr(const AptSharedPtr &copy) : pData(copy.pData)
        {
#if defined(REFCOUNT_DEBUGGING)
            if (pData)
                AptSharedPtrIncRef(pData, this);
#else
            if (pData)
                AptSharedPtrIncRef(pData);
#endif
        }

        ~AptSharedPtr()
        {
            T * tempData = pData;
            pData = NULL;
            AptSharedPtrDispose(tempData);
        }
        AptSharedPtr& operator=(const AptSharedPtr& r)
        {
            if (&r == this)
                return *this;

            T * tempData = pData;

            pData = r.pData;
#if defined(REFCOUNT_DEBUGGING)
            if (pData) AptSharedPtrIncRef(pData, this);
#else
            if (pData) AptSharedPtrIncRef(pData);
#endif

            AptSharedPtrDispose(tempData);
            return *this;
        }

        void Reset()
        {
            T * tempData = pData;
            pData = 0;
            AptSharedPtrDispose(tempData);
        }

        T& operator*() const { return *pData; }
        T* operator->() const { return pData; }
        T* Get() const { return pData; }

        operator bool() const { return pData != 0; }
        bool operator! () const { return pData == 0; }

        T *pData;

#if defined(EA_DEBUG)          // $BUG253 LEO    24/47
        void SetFileSize(int nSize)
        {
            AptSharedPtrSetFileSize(pData, nSize);
        }
        int GetFileSize()
        {
            return AptSharedPtrGetFileSize(pData);
        }
#endif
    APT_NEW_DELETE_OPERATORS
    // DC - Added Metric defines
    APT_DECLARE_OBJECT_METRICS

};

template<typename T, typename U>
inline bool operator ==(const AptSharedPtr<T>& a, const AptSharedPtr<U>& b) { return a.Get() == b.Get(); }

template<typename T>
inline bool operator ==(const AptSharedPtr<T>& a, const intptr_t b) { return a.Get() == (T *)b; }

template<typename T, typename U>
inline bool operator !=(const AptSharedPtr<T>& a, const AptSharedPtr<U>& b) { return a.Get() != b.Get(); }

template<typename T>
inline bool operator !=(const AptSharedPtr<T>& a, const intptr_t b) { return a.Get() != (T *)b; }

#endif  //  __APTSHAREDPTR_H__

/*H*************************************************************************************************
    EOF
*************************************************************************************************H*/

