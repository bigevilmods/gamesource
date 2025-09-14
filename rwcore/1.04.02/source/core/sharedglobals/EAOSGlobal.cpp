///////////////////////////////////////////////////////////////////////////////
// EAOSGlobal.cpp
//
// (c) Electronic Arts. All Rights Reserved.
// Maintained by Avery Lee and Paul Pedriana, Maxis
//
// This is a copy of the EAOSGlobal mechanism found in the UTF Foundation package
//
// OS globals are process-wide globals and are shared between an EXE and
// DLLs. The OS global system works at the operating system level and has
// auto-discovery logic so that no pointers or init calls need to be made
// between modules for them to link their OS global systems together.
//
// Note that the interface to OS globals is a bit convoluted because the
// core system needs to be thread-safe, cross-module, and independent of
// app-level allocators. For objects for which order of initialization is
// clearer, EASingleton is probably a better choice.
//
///////////////////////////////////////////////////////////////////////////////

/// \file
/// EA System-wide globals.  Ripped from UTF.

#include "rw/core/sharedglobal/detail/EAOSGlobal.h"

#include <new>
#include <stdlib.h>
#include <stdio.h>

#include "EAThread/eathread_atomic.h"

#if defined(rwHOST_WIN) || defined(rwHOST_XBOX) || defined(rwHOST_XBOX2)
    #pragma warning(push, 1)

    #include <tchar.h>

    #if defined(rwHOST_XBOX) || defined(rwHOST_XBOX2)
        #include <xtl.h>
    #else
        #include <windows.h>
    #endif

    #pragma warning(pop)
#else // defined(rwHOST_WIN) || defined(rwHOST_XBOX) || defined(rwHOST_XBOX2)
#include "EAThread/eathread_mutex.h"
#endif // defined(rwHOST_WIN) || defined(rwHOST_XBOX) || defined(rwHOST_XBOX2)


#if defined(rwHOST_WIN) || defined(rwHOST_XBOX) || defined(rwHOST_XBOX2)
    #pragma warning(push)
    #pragma warning(disable: 4075)          // warning C4075: initializers put in unrecognized initialization area
    #pragma warning(disable: 4355)          // warning C4355: 'this' : used in base member initializer list

    // Set initialization order between init_seg(compiler) (.CRT$XCC) and
    // init_seg(lib) (.CRT$XCL). The MSVC linker sorts the .CRT sections
    // alphabetically so we simply need to pick a name that is between
    // XCC and XCL. This works on both Windows and XBox.
    //#pragma init_seg(".CRT$XCF")

    namespace {
        // For Win32, we need a custom mutex because the OSGlobal system is used
        // by the shared allocator system before static ctors run, and that is too
        // early to use EAThread's mutex (it uses some static structures in debug).
        class Win32Mutex {
        public:
            Win32Mutex() {
                InitializeCriticalSection(&mLock);
            }

            ~Win32Mutex() {
                DeleteCriticalSection(&mLock);
            }

            void Lock() {
                EnterCriticalSection(&mLock);
            }

            void Unlock() {
                LeaveCriticalSection(&mLock);
            }

        private:
            CRITICAL_SECTION mLock;
        };

        typedef Win32Mutex OSGlobalListMutex;
    }
#else
    namespace {
        typedef EA::Thread::Mutex OSGlobalListMutex;
    }
#endif

namespace rw {
namespace shared_globals {
namespace internal {

EA::Thread::AtomicUint32& OSGlobalNode::OSGlobalRefCount()
{
    return *reinterpret_cast<EA::Thread::AtomicUint32*>(&mOSGlobalRefCount);
    
}

namespace {
    struct OSGlobalManager {

        // A mini implimentation of intrusive_list
        class OSGlobalList
        {
        public:
            class iterator
            {
            public:
                typedef OSGlobalNode* pointer;
                typedef OSGlobalNode& reference;

                iterator()
                {
                    // Empty
                }

                explicit iterator(pointer pNode)
                    : mpNode(pNode)
                {
                    // Empty
                }

                iterator(const iterator& x)
                    : mpNode(x.mpNode)
                {
                    // Empty
                }

                reference operator*() const
                {
                    return *mpNode;
                }

                pointer   operator->() const
                {
                    return mpNode;
                }

                iterator& operator++()
                {
                    mpNode = mpNode->mpNext;
                    return *this;
                }

                iterator& operator--()
                {
                    mpNode = mpNode->mpPrev;
                    return *this;
                }

                bool operator==(const iterator& other) const
                {
                    return mpNode == other.mpNode;
                }

                bool operator!=(const iterator& other) const
                {
                    return mpNode != other.mpNode;
                }

                pointer mpNode;
            };

            OSGlobalList()
            {
                mAnchor.mpNext = &mAnchor;
                mAnchor.mpPrev = &mAnchor;
            }

            void push_front(OSGlobalNode& x)
            {
                x.mpNext = mAnchor.mpNext;
                x.mpPrev = &mAnchor;
                mAnchor.mpNext = &x;
                x.mpNext->mpPrev = &x;
            }

            iterator begin()
            {
                return iterator(mAnchor.mpNext);
            }

            iterator end()
            {
                return iterator(&mAnchor);
            }

            iterator locate(OSGlobalNode& x)
            {
                for(OSGlobalNode* p = mAnchor.mpNext; p != &mAnchor; p = p->mpNext)
                {
                    if(p == &x)
                        return iterator(p);
                }

                return iterator(&mAnchor);
            }

            iterator erase(iterator pos)
            {
                OSGlobalNode& prev = *pos.mpNode->mpPrev;
                OSGlobalNode& next = *pos.mpNode->mpNext;
                prev.mpNext = &next;
                next.mpPrev = &prev;
                return iterator(&next);
            }

        private:
                OSGlobalNode mAnchor;
        };

        OSGlobalList            mOSGlobalList;
        EA::Thread::AtomicUint32 mRefCount;     //< Reference count so that the allocator persists as long as the last module that needs it.
        OSGlobalListMutex       mcsLock;

        OSGlobalManager();
        OSGlobalManager(const OSGlobalManager&);
       ~OSGlobalManager();

        OSGlobalManager& operator=(const OSGlobalManager&);

        void Lock() {
            mcsLock.Lock();
        }

        void Unlock() {
            mcsLock.Unlock();
        }

        OSGlobalNode *Find(uint32_t id);
        void Add(OSGlobalNode *p);
        void Remove(OSGlobalNode *p);
    };

    OSGlobalManager::OSGlobalManager() 
        : mRefCount(0)
    {
    }

    OSGlobalManager::~OSGlobalManager() {
    }

    OSGlobalNode *OSGlobalManager::Find(uint32_t id) {
        OSGlobalList::iterator it(mOSGlobalList.begin()), itEnd(mOSGlobalList.end());

        for(; it!=itEnd; ++it) {
            OSGlobalNode& node = *it;

            if (node.mOSGlobalID == id)
                return &node;
        }

        return NULL;
    }

    void OSGlobalManager::Add(OSGlobalNode *p) {
        mOSGlobalList.push_front(*p);
    }

    void OSGlobalManager::Remove(OSGlobalNode *p) {
        mOSGlobalList.erase(mOSGlobalList.locate(*p));
    }

    OSGlobalManager        *gpOSGlobalManager;
    EA::Thread::AtomicUint32 gOSGlobalRefs;
}



#if defined(rwHOST_WIN) || defined(rwHOST_XBOX) || defined(rwHOST_XBOX2)

    namespace {

        #define RW_GLOBAL_UNIQUE_NAME_FORMAT "RwSingleMgrMutex%08x"

        #if defined(rwHOST_XBOX) || defined(rwHOST_XBOX2)
            HANDLE ghOSGlobalManagerPtrSemaphore;
        #endif

        bool InitOSGlobalSystem();
        void ShutdownOSGlobalSystem();
        

        OSGlobalManager * CreateOSGlobalManager() {
            // Allocate the OSGlobal manager in the heap. We use the heap so that it can
            // hop between DLLs if the EXE itself doesn't use the manager. Note that this
            // must be the operating system heap and not an app-level heap (i.e. PPMalloc).
            const size_t kAlignment = 16;

            void *p = HeapAlloc(GetProcessHeap(), 0, sizeof(OSGlobalManager) + kAlignment - 1 + sizeof(void *));
            void *pAligned = (void *)(((uintptr_t)p + sizeof(void *) + kAlignment - 1) & ~(kAlignment-1));
            ((void **)pAligned)[-1] = p;

            // Placement-new the global manager into the new memory.
            return new(pAligned) OSGlobalManager;
        }


        bool InitOSGlobalSystem() {
            // The following check is not thread-safe. On most platforms this isn't an 
            // issue in practice because this function is called on application startup
            // and other threads won't be active. The primary concern is if the 
            // memory changes that result below are visible to other processors later.
            if (!gpOSGlobalManager) {
                #if defined(rwHOST_XBOX) || defined(rwHOST_XBOX2)
                    HANDLE hMutex = CreateMutex(NULL, FALSE, _T("RwGlobalMgrMutex"));
                #else
                    // The kernel object namespace is global on Win32 so we have to choose
                    // a unique name.
                    TCHAR uniqueName[64];
                    wsprintf(uniqueName, _T(RW_GLOBAL_UNIQUE_NAME_FORMAT), (unsigned)GetCurrentProcessId());
                    HANDLE hMutex = CreateMutex(NULL, FALSE, uniqueName);
                #endif

                if (!hMutex)
                    return false;

                if (WAIT_FAILED != WaitForSingleObject(hMutex, INFINITE)) {
                    #if defined(rwHOST_XBOX) || defined(rwHOST_XBOX2)
                        // Xenon does not have memory mapping so we can't use the same technique
                        // as for Win32, and lacks GetModuleHandle() so the old "get global proc
                        // from main executable" trick won't work. What we do here is create
                        // a semaphore whose value is the pointer to the allocator. Semaphores
                        // can't hold negative values so we shift the pointer down by 4 bits
                        // before storing it (it has 16 byte alignment so this is OK). Also,
                        // there is no way to read a semaphore without modifying it so a mutex
                        // is needed around the operation.

                        // Bomb if pointers are not 32 bits, as semphore counts are only 32 bits.
                        (void)sizeof(char[sizeof(void *) == 4 ? 1 : -1]);

                        ghOSGlobalManagerPtrSemaphore = CreateSemaphore(NULL, 0, 0x7FFFFFFF, _T("RwGlobalMgr"));

                        if (ghOSGlobalManagerPtrSemaphore) {
                            const bool bSemaphoreExists = (GetLastError() == ERROR_ALREADY_EXISTS);

                            if (bSemaphoreExists) {
                                LONG ptrValue;

                                // Read the semaphore value.
                                if (ReleaseSemaphore(ghOSGlobalManagerPtrSemaphore, 1, &ptrValue)) {
                                    // Undo the +1 we added to the semaphore.
                                    WaitForSingleObject(ghOSGlobalManagerPtrSemaphore, INFINITE);

                                    // Recover the allocator pointer from the semaphore's original value.
                                    gpOSGlobalManager = (OSGlobalManager *)((uintptr_t)ptrValue << 4);
                                }
                            } else {
                                gpOSGlobalManager = CreateOSGlobalManager();

                                // Set the semaphore to the pointer value. It was created with
                                // zero as the initial value so we add the desired value here.
                                ReleaseSemaphore(ghOSGlobalManagerPtrSemaphore, (LONG)((uintptr_t)gpOSGlobalManager >> 4), NULL);
                            }
                        }

                    #else

                        // Under Win32 and Win64, we use system environment variables to store the gpOSGlobalManager value.
                        TCHAR stringPtr[32];
                        const DWORD dwResult = GetEnvironmentVariable(uniqueName, stringPtr, 32);

                        if((dwResult > 0) && stringPtr[0]) { // If the variable was found...
                            gpOSGlobalManager = (OSGlobalManager *)(uintptr_t)_strtoui64(stringPtr, NULL, 16); // _strtoui64 is a VC++ extension function.
                        } else {
                            // GetLastError() should be ERROR_ENVVAR_NOT_FOUND. But what do we do if it isn't?
                            gpOSGlobalManager = CreateOSGlobalManager();

                            wsprintf(stringPtr, _T("%I64x"), (uint64_t)(uintptr_t)gpOSGlobalManager);
                            SetEnvironmentVariable(uniqueName, stringPtr); // There's not much we can do if this call fails.
                        }

                    #endif

                    ++gpOSGlobalManager->mRefCount;

                    ReleaseMutex(hMutex);
                    CloseHandle(hMutex);
                }

                if (!gpOSGlobalManager) {
                    ShutdownOSGlobalSystem();
                    return false;
                }

                ++gOSGlobalRefs;
            }

            return true;
        }

        void ShutdownOSGlobalSystem() {
            if (gOSGlobalRefs.Decrement() == 0) { // If the (atomic) integer decrement results in a refcount of zero...
                if (gpOSGlobalManager) {
                    if (gpOSGlobalManager->mRefCount.Decrement() == 0) {
                        gpOSGlobalManager->~OSGlobalManager();
                        HeapFree(GetProcessHeap(), 0, ((void **)gpOSGlobalManager)[-1]);
                    }

                    gpOSGlobalManager = NULL;
                }

                #if defined(EA_PLATFORM_XENON)
                    if (ghOSGlobalManagerPtrSemaphore) {
                        CloseHandle(ghOSGlobalManagerPtrSemaphore);
                        ghOSGlobalManagerPtrSemaphore = NULL;
                    }
                #else
                    // Clear the gpOSGlobalManager environment variable.
                    // This code needs to be called in a thread-safe way by the user, usually by calling it once on shutdown. 
                    // We have a problem if this function is executing at the same time some other entity in this process 
                    // is currently doing some new use of the OS global system, as that can cause an instance of gpOSGlobalManager
                    // to be created while we are in the process of destroying it.
                    TCHAR uniqueName[64];

                    wsprintf(uniqueName, _T(RW_GLOBAL_UNIQUE_NAME_FORMAT), (unsigned)GetCurrentProcessId());
                    SetEnvironmentVariable(uniqueName, NULL);
                #endif
            }
        }
    }

#else // #if defined(EA_PLATFORM_WINDOWS) || defined(EA_PLATFORM_XENON)

    namespace {

        #if defined(__GNUC__)
            // By adding the 'constructor' attribute to this function, we tell GCC to
            // have this function be called on startup before main(). We have the 
            // AutoinitOSGlobalManager mechanism below, but GCC ignores it because the 
            // object isn't externally reference and GCC thinks it can thus be eliminated.
            //
            // Disabled until we have a GCC compiler at hand to test this. I am writing 
            // this code from home. Note also that we may want to disabled the 
            // AutoinitOSGlobalManager under GCC.
            // bool InitOSGlobalSystem()     __attribute__ ((constructor));
            // bool ShutdownOSGlobalSystem() __attribute__ ((destructor));
        #endif

        bool InitOSGlobalSystem() {
            static OSGlobalManager OSGlobalMgr;

            gpOSGlobalManager = &OSGlobalMgr;
            ++gOSGlobalRefs;
            return true;
        }

        void ShutdownOSGlobalSystem() {
            if (!gOSGlobalRefs.Decrement()) // If the (atomic) integer decrement results in a refcount of zero...
                gpOSGlobalManager = NULL;
        }
    }

#endif // #if defined(EA_PLATFORM_WINDOWS) || defined(EA_PLATFORM_XENON)




OSGlobalNode *GetOSGlobal(uint32_t id, OSGlobalFactoryPtr pFactory) {
    // bring up the OSGlobal system if we are getting called before
    // static init, i.e. allocator
    if (!InitOSGlobalSystem())
        return NULL;

    gpOSGlobalManager->Lock();

    OSGlobalNode *p = gpOSGlobalManager->Find(id);

    if (!p && pFactory) {
        p = pFactory();
        p->mOSGlobalID = id;
        p->OSGlobalRefCount() = 0;
        gpOSGlobalManager->Add(p);
    }

    if (p) {
        ++p->OSGlobalRefCount();
        ++gOSGlobalRefs;
    }

    gpOSGlobalManager->Unlock();

    return p;
}

bool ReleaseOSGlobal(OSGlobalNode *p) {
    gpOSGlobalManager->Lock();

    const bool shouldDestroyManager  = !gOSGlobalRefs.Decrement();
    const bool shouldDestroyOSGlobal = !--p->OSGlobalRefCount();

    if (shouldDestroyOSGlobal)
        gpOSGlobalManager->Remove(p);

    gpOSGlobalManager->Unlock();

    if (shouldDestroyManager)
        ShutdownOSGlobalSystem();

    return shouldDestroyOSGlobal;
}


// Force the OSGlobal manager to be available for the life of the app.
// It's OK if this comes up too late for some uses because GetOSGlobal()
// will bring it online earlier in that case.
namespace {
    struct AutoinitOSGlobalManager {
        AutoinitOSGlobalManager() {
            InitOSGlobalSystem();
        }

        ~AutoinitOSGlobalManager() {
            ShutdownOSGlobalSystem();
        }
    } gAutoinitOSGlobalManager;
}

} // namespace internal
} // namespace shared_globals
} // namespace rw
