///////////////////////////////////////////////////////////////////////////////
// EAOSGlobal.h
//
// (c) Electronic Arts. All Rights Reserved.
// Maintained by Avery Lee and Paul Pedriana, Maxis
///////////////////////////////////////////////////////////////////////////////

// This is a copy of the EAOSGlobal mechanism found in the UTF Foundation package

///////////////////////////////////////////////////////////////////////////////
// OS globals are process-wide globals (singletons, actually) and are shared 
// between an EXE and DLLs, though you can use them on platforms that don't  
// support DLLs. The Windows OS does not directly support this concept of  
// global object sharing, as the only way to accomplish this under Windows  
// is to sequester the global object in its own DLL of which the EXE and all  
// DLLs explicitly reference. OS Globals allow you to have the global object  
// anywhere (including the EXE) and does so without requiring explicit linking.  
// The OS global system works at the operating system level and has  
// auto-discovery logic so that no pointers or init calls need to be made  
// between modules for them to link their OS global systems together.  
// The OS global system consists of EAOSGlobal and EAGlobal. The latter  
// provides simpler functionality but has less overhead. 
// 
// A primary use of OS globals is in the creation of application singletons  
// such as the main heap, messaging servers, asset managers, etc. 
//  
// Note that the implementation behind OS globals (EAOSGlobal.cpp) may seem  
// a bit convoluted; this is because it needs to be thread-safe, cross-module,  
// and independent of application-level memory allocators. For objects for  
// which order of initialization is clearer and you don't need to reference  
// singletons across DLLs, EASingleton is probably a better choice, as it is  
// simpler and has lower overhead. Indeed, another way of looking at OS globals  
// is to view them as process-wide singletons. 
///////////////////////////////////////////////////////////////////////////////


#ifndef DETAIL_RW_SHAREDGLOBAL_EAOSGLOBAL_H
#define DETAIL_RW_SHAREDGLOBAL_EAOSGLOBAL_H

#include <new>
#include <rw/core/base/ostypes.h>

// Forward Declare atomic integer
namespace EA {
namespace Thread {
    template <class T>
    class AtomicInt;
    typedef AtomicInt<uint32_t> AtomicUint32;
} // namespace Thread
} // namespace EA

namespace rw {
namespace shared_globals {
namespace internal {
    //////////////////////////////////////////////////////////////////////////
    /// OSGlobalNode
    ///
    /// All OS globals must derive from this object or act as if they do.
    /// If you are using AutoOSGlobalPtr or AutoStaticOSGlobalPtr this 
    /// derivation is handled for you.
    ///
    /// Note that above we say 'or act as if they do'. You can make a smart
    /// pointer container that overrides operator ->, operator *, etc. while
    /// referring to an actual object that is somewhere else. In fact, the 
    /// AutoOSGlobalPtr and AutoStaticOSGlobalPtr templates we provide do this.
    ///
    class OSGlobalNode 
    {
    public:
        OSGlobalNode()
            : mpNext(0),
              mpPrev(0)
        {
        }

        OSGlobalNode* mpNext;
        OSGlobalNode* mpPrev;
        uint32_t      mOSGlobalID;

        // We use the following function to remove the dependance of this header
        // on EAThread
        EA::Thread::AtomicUint32& OSGlobalRefCount();

    private:
        // The following makes the assumption that an atomic int can be stored 
        // in the same space as a uint32 and has no special alignment needs!
        uint32_t mOSGlobalRefCount;
    };


    //////////////////////////////////////////////////////////////////////////
    /// OSGlobalFactoryPtr
    ///
    /// Defines a factory function for a given OSGlobalNode.
    /// 
    typedef OSGlobalNode *(*OSGlobalFactoryPtr)();


    //////////////////////////////////////////////////////////////////////////
    /// GetOSGlobal
    ///
    /// Browses for a OS global with the given ID and either returns the
    /// existing object or attempts to create and register one. Returns NULL
    /// if the OS global could not be created. This won't happen in practice
    /// because the memory allocator will have already successfully 
    /// initialized the OSGlobal system.
    ///
    /// Each successful call to GetOSGlobal must be matched with a call to
    /// ReleaseOSGlobal.
    ///
    /// This function doesn't directly allocate any memory. However, the 
    /// user-supplied factory function may allocate memory, depending on 
    /// the implementation of the function.
    ///
    /// This function can safely be called from multiple threads. 
    ///
    OSGlobalNode *GetOSGlobal(uint32_t id, OSGlobalFactoryPtr pFactory);


    //////////////////////////////////////////////////////////////////////////
    /// ReleaseOSGlobal
    ///
    /// Releases a reference to a OS global obtained from GetOSGlobal.
    ///
    /// Returns false if the OS global is still in use, and true if the last
    /// reference was just released. The caller is responsible for destroying
    /// the OS global in the latter case.
    ///
    /// This function can safely be called from multiple threads. 
    ///
    bool ReleaseOSGlobal(OSGlobalNode *p);




    //////////////////////////////////////////////////////////////////////////
    /// AutoOSGlobalPtr
    ///
    /// Holds a reference to an OS global of the specified type and Id. 
    /// If the OS global does not exist, a new one is created in the 
    /// shared heap. The Id parameter is an arbitrary guid and allows the
    /// user to have multiple OSGlobalPtrs of the same stored type T.
    ///
    /// AutoStaticOSGlobalPtrs and AutoOSGlobalPtrs should not be mixed when
    /// referring to a OS global. You should reference an OSGlobal via either
    /// one or more AutoOSGlobalPtrs, one or more AutoStaticOSGlobalPtrs, but
    /// not both at the same time.
    ///
    /// OS global lookup is not very fast so the preferred usage of this
    /// class is to wrap it in an accessor. This also ensures that the
    /// OS global stays around while created.
    ///
    /// This class can safely be used from multiple threads. 
    ///
    /// Example usage:
    ///     AutoOSGlobalPtr<Widget, 0x11111111> gpWidget1A;
    ///     AutoOSGlobalPtr<Widget, 0x11111111> gpWidget1B;
    ///     AutoOSGlobalPtr<Widget, 0x22222222> gpWidget2;
    ///     
    ///     void Foo() {
    ///         assert(gpWidget1A == gpWidget1B);
    ///         assert(gpWidget1A != gpWidget2);
    ///
    ///         gpWidget1A->DoSomething();
    ///         gpWidget2->DoSomething();
    ///     }
    ///
    /// Example of how to write an accessor wrapper:
    ///     Foo *GetFoo() {
    ///         static AutoOSGlobalPtr<Foo, kFooId> sPtr;
    ///         return sPtr;
    ///     }
    ///
    ///     Better yet, double-wrap it so clients don't have 
    ///     to #include this header:
    ///
    ///     Foo *InlineGetFoo() {
    ///        extern void GetFoo();
    ///        static Foo * const sPtr = GetFoo();
    ///        return sPtr;
    ///     }
    ///
    template<class T, uint32_t id>
    class AutoOSGlobalPtr {
    public:
        /// value_type
        /// This is an alias for the contained class T.
        typedef T value_type;

        /// this_type
        /// This is an alias for this class.
        typedef AutoOSGlobalPtr<T, id> this_type;

    public:
        /// AutoOSGlobalPtr
        ///
        /// Default constructor. Creates a new version of the OSGlobalPtr
        /// object if it hasn't been created yet and sets its reference
        /// count to one.
        ///
        AutoOSGlobalPtr();

        /// AutoOSGlobalPtr (copy constructor)
        ///
        /// Copy constructor. Copies the OSGlobalPtr from the source.
        /// Increases the reference count on the copied OSGlobalPtr.
        ///
        AutoOSGlobalPtr(const this_type& x);

        /// AutoOSGlobalPtr
        ///
        /// Decrements the reference count on the held OSGlobalPtr if
        /// it has been set. Destroys the OSGlobalPtr node if the reference
        /// count goes to zero.
        ///
       ~AutoOSGlobalPtr();

        /// operator =
        ///
        /// Assignment operator. Copies the OSGlobalPtr from the source.
        /// Increases the reference count on the copied OSGlobalPtr.
        /// Decreases the reference count on the previous OSGlobalPtr.
        ///
        this_type& operator=(const this_type& x);

        /// operator T*
        ///
        /// Allows this class to act like a pointer.
        ///
        operator T*() const;

        /// operator *
        ///
        /// Allows this class to act like a pointer.
        ///
        T& operator*() const;

        /// operator ->
        ///
        /// Allows this class to act like a pointer.
        ///
        T *operator->() const;

        /// get
        ///
        /// Returns the owned pointer.
        ///
        T *get() const;

    protected:
        struct Node : public OSGlobalNode {
            T mObject;
            Node() : mObject() {}
        };

        static OSGlobalNode *Create();

        Node * const mpOSGlobal;
    };




///////////////////////////////////////////////////////////////////////////////
// Implementation
///////////////////////////////////////////////////////////////////////////////


    ///////////////////////////////////////////////////////////////////////////
    // AutoOSGlobalPtr
    ///////////////////////////////////////////////////////////////////////////

    template <class T, uint32_t id>
    inline AutoOSGlobalPtr<T, id>::AutoOSGlobalPtr()
        : mpOSGlobal(static_cast<Node *>(GetOSGlobal(id, Create)))
    {
    }

    template <class T, uint32_t id>
    inline AutoOSGlobalPtr<T, id>::AutoOSGlobalPtr(const this_type&)
        : mpOSGlobal(static_cast<Node *>(GetOSGlobal(id, Create)))
    {
    }

    template <class T, uint32_t id>
    inline AutoOSGlobalPtr<T, id>::~AutoOSGlobalPtr() {
        if (ReleaseOSGlobal(mpOSGlobal))
            delete mpOSGlobal;
    }

    template <class T, uint32_t id>
    inline typename AutoOSGlobalPtr<T, id>::this_type& 
    AutoOSGlobalPtr<T, id>::operator=(const this_type&) { 
        // Intentionally do nothing. We should already point to the correct object.
        return *this;
    }

    template <class T, uint32_t id>
    inline AutoOSGlobalPtr<T, id>::operator T*() const {
        return &mpOSGlobal->mObject;
    }

    template <class T, uint32_t id>
    inline T * AutoOSGlobalPtr<T, id>::operator->() const {
        return &mpOSGlobal->mObject;
    }

    template <class T, uint32_t id>
    inline T& AutoOSGlobalPtr<T, id>::operator*()  const {
        return mpOSGlobal->mObject;
    }

    template <class T, uint32_t id>
    inline T * AutoOSGlobalPtr<T, id>::get() const {
        return &mpOSGlobal->mObject;
    }

    template <class T, uint32_t id>
    inline OSGlobalNode * AutoOSGlobalPtr<T, id>::Create() {
        return new Node;
    }


} // namespace internal
} // namespace shared_globals
} // namespace rw

#endif // DETAIL_RW_SHAREDGLOBAL_EAOSGLOBAL_H











