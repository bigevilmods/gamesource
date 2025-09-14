/*H************************************************************************************************/
/*!

    \File    Apt\_AptThread.h

    \Description
        Defines for namespaces that can be used inside Apt whe needing to do multithreading stuff

    \Notes
        None.

    \Copyright
        (c) 2012 Electronic Arts Inc.

*/
/************************************************************************************************H*/


#ifndef _Apt__AptThread_h
#define _Apt__AptThread_h

#if defined(_MSC_VER)
#pragma once
#endif

/*** Include files ********************************************************************************/

/*** Defines **************************************************************************************/

#if !defined(APT_SINGLE_THREADED) && (defined(EA_PLATFORM_XBOX) || defined(EA_PLATFORM_PLAYSTATION2) || defined(EA_PLATFORM_PSP) || defined(EA_PLATFORM_GAMECUBE))
#define APT_SINGLE_THREADED 1
#endif

// these #defines are for deciding which thread library to use.

#ifdef APT_SINGLE_THREADED

namespace AptSingleThreadNamespace
{
class MutexParameters
{
};
class Mutex
{
public:
    inline Mutex() {}
    inline Mutex(const MutexParameters *) {}
    inline void Lock()   {}
    inline void Unlock() {}
};

class Futex
{
public:
    inline Futex() {}
    inline Futex(const MutexParameters *) {}
    inline void Lock()   {}
    inline void Unlock() {}
};
}

#define APT_THREAD_NAMESPACE        AptSingleThreadNamespace
#define APT_INC_THREAD_H            "_Apt.h"
#define APT_INC_THREAD_MUTEX_H      "_Apt.h"
#define APT_INC_THREAD_FUTEX_H      "_Apt.h"
#define APT_INC_THREAD_ATOMIC_H     "_Apt.h"

#else

#define APT_THREAD_NAMESPACE EA::Thread
#define APT_INC_THREAD_H            "eathread\eathread.h"
#define APT_INC_THREAD_MUTEX_H      "eathread\eathread_mutex.h"
#define APT_INC_THREAD_FUTEX_H      "eathread\eathread_futex.h"
#define APT_INC_THREAD_ATOMIC_H     "eathread\eathread_atomic.h"

#endif

#endif // _Apt__AptThread_h
