// (c) Electronic Arts. All Rights Reserved.

/// \file
/// This file provides our wrappers for the most used stl containers.
/// Currently we use stlport implementation on top of which we derive
/// our own containers.

#ifndef PUBLIC_RW_CORE_STLTYPES_H
#define PUBLIC_RW_CORE_STLTYPES_H

#ifdef _MSC_VER
// Disable pointless warning about debugger truncation
#pragma warning(disable: 4786)
// Disable warning about assignment operator not being generated
// in MS headers
#pragma warning(push,3)
#pragma warning(disable: 4512)
#pragma warning(disable: 4511)
#pragma warning(disable: 4244)
// Disable warning about unreachable code in MS headers
#pragma warning(disable: 4702)
#pragma warning(disable: 4217) // Member template functions cannot be copy constructors
#pragma warning(disable: 4530) // Exception handler used (for ostream pulled in by string)
#pragma warning(disable: 4548) // xenon warning - expression before comma has no effect (in xlocal)
#endif

// ***********************************************************************************************************
// Includes

// If customers want to use the underlying default stl implementation, they have to add following
// defines: _STLP_WHOLE_NATIVE_STD and _STLP_WHOLE_VENDOR_STD or edit "stl_user_config.h" file, which
// they would have to do if they were using stlport anyway.

#include <string>
#include <vector>
#include <map>
#include <set>
#include <list>
#include <algorithm>
#include <queue>
#include <typeinfo>

#include <rw/core/base/ostypes.h>
#include <rw/core/memory/memoryfunctions.h>
#include <rw/core/string.h>
#include <rw/core/debug/debug.h>

#ifdef _MSC_VER
// Re-enable warning about assignment operator not being generated in MS headers
#pragma warning(pop)
#endif

#ifdef _STLPORT_VERSION

//Using STLPort so rwstd should map to _STL
//namespace rwstd = _STL;
/**
Depending on the whether STLport is used, the namespace rwstd is defined to be std or _STL.
\showinitializer
*/
#define rwstd _STL
#else //_STLPORT_VERSION
//Using vendor stl so rwstd should map to std
//namespace rwstd = std;
/**
Depending on the whether STLport is used, the namespace rwstd is defined to be std or _STL.
\showinitializer
*/
#define rwstd std
#endif //_STLPORT_VERSION

// ***********************************************************************************************************
// Defines + Enums + Consts


// ***********************************************************************************************************
// Forward Declarations


// ***********************************************************************************************************
// Typedefs


// ***********************************************************************************************************
// Global Variables


// ***********************************************************************************************************
// External Functions Prototypes


// ***********************************************************************************************************
// Structs + Unions + Classes

/**
Templated swap function based on std::swap
ValueType must be copyable and assignable.
\param lhs one ValueType to swap with the other
\param rhs one ValueType to swap with the other
*/
template <typename ValueType>
void RwSwap(ValueType& lhs, ValueType& rhs)
{
    rwFUNCTION();
    ValueType temp(lhs);
    lhs = rhs;
    rhs = temp;
    rwRETURNVOID();
}

#ifndef _STLPORT_VERSION

// Implement _IsPtr when not using STLPORT
namespace rwstd
{
    //We may need to simulate partial specialization for trait types from some compilers (like STLPort does)
    template <class _Tp>  struct _IsPtr { enum { _Ret = 0 }; };
    template <class _Tp>  struct _IsPtr<_Tp*> { enum { _Ret = 1 }; };

}

#endif //_STLPORT_VERSION


// ***********************************************************************************************************
//                                                RwAllocator CLASS
// ***********************************************************************************************************


//todo: clean up these allocators
#ifdef _STLPORT_VERSION

/**
An allocator for T's
Implements a custom STL memory allocator. It internally calls RwMalloc and RwFree functions to
do the actual memory allocations and deallocations.
*/
template<typename T>
class RwAllocator : public rwstd::allocator<T>
{
public:
    // These templated types are defined in the base class, but for some reason we
    // are getting warnings for them in the gcc3+, so we'll define them here as well.
    /**
    The size type of the allocator
    */
    typedef size_t  rw_size_type;
    /**
    The type of the pointer the allocator is returning
    */
    typedef T*      rw_pointer;

    /**
    Returns a pointer to memory that can host n T's
    \param n the number of T's to allocate memory for
    \param p an unused void pointer
    \return a pointer to the allocated memory
    */
    rw_pointer
    allocate(rw_size_type n, const void * RWUNUSED(p)= 0)
    {
        rwFUNCTION();

        rwASSERT(RwMemGetMalloc() != NULL);
        rwRETURN((rw_pointer)(RwMalloc(n * sizeof(T))));
    }

    /**
    Frees the memory p is pointing to
    \param p A block of data, whose type is determined by the template instance.
    */
    void
    deallocate(rw_pointer p, rw_size_type RWUNUSED(s))
    {
        rwFUNCTION();

        rwASSERT(RwMemGetFree() != NULL);

        RwFree((void *)p);

        rwRETURNVOID();
    }

#ifdef _STLP_NO_MEMBER_TEMPLATE_CLASSES

// Redefine __stl_alloc_rebind and __stl_alloc_create.
// Need to do this for any compiler that doesn't support member templates

// Implements a specialization of the allocator rebind template to allow conversion of allocators
// in generaic allocator code.  This is deep and nasty internal magic that is simply required when
// specializing allocators for STLPort.
template <class _Tp1, class _Tp2> inline RwAllocator<_Tp2>&
__stl_alloc_rebind(RwAllocator<_Tp1>& __a, const _Tp2*)
{
    rwFUNCTION();
    rwRETURN((RwAllocator<_Tp2>&)(__a));
}

// Implements a specialization of the allocator creation template to allow creation of allocators
// in generaic allocator code.  This is deep and nasty internal magic that is simply required when
// specializing allocators for STLPort.
template <class _Tp1, class _Tp2> inline RwAllocator<_Tp2>
__stl_alloc_create(const RwAllocator<_Tp1>&, const _Tp2*)
{
    rwFUNCTION();
    RwAllocator<_Tp2> defAllocator;
    rwRETURN(defAllocator);
}

#else // _STLP_NO_MEMBER_TEMPLATE_CLASSES

    // A struct to rebind this allocator type to another type.
    // See STLport and C++ STL allocator library internal documentation
    // for details on the allocator rebinding mechanism.
    template <typename _Other>
    struct rebind
    {
        // The type of the Allocator for the _Other
        typedef RwAllocator<_Other> other;
    };
#endif // _STLP_NO_MEMBER_TEMPLATE_CLASSES
};

#else //_STLPORT_VERSION

/**
An STL allocator compatible memory allocator for objects of type \e  T.

\par Template_params
\li T Type of object to be allocated.

Implements an allocator compaitble with STL container alloactor requirements which forwards all
memory handling to RenderWare internal memory allocation routines.
*/
template<typename T>
class RwAllocator
{
public:
    /**
    The type of the size for this allocator.
    */
    typedef size_t     size_type;

    /**
    The type of the difference between pointers for this allocator.
    */
    typedef ptrdiff_t  difference_type;

    /**
    The type of the pointer this allocator returns.
    */
    typedef T*         pointer;

    /**
    The type of the pointer to const this allocator returns.
    */
    typedef const T*   const_pointer;

    /**
    The type of the reference this allocator uses.
    */
    typedef T&         reference;

    /**
    The type of the const reference this allocator uses.
    */
    typedef const T&   const_reference;

    /**
    The type of the things the allocators allocate.
    */
    typedef T          value_type;

    /**
    A struct to allow rebinding to allocators for other types.
    */
    template<typename T1>
    struct rebind
    {
        /**
        The type of the allocator for another type.
        */
        typedef RwAllocator<T1> other;
    };

    /**
    Default constructor.
    */
    RwAllocator() throw()
    { }

    RwAllocator(const RwAllocator& RWUNUSED(other)) throw()
    { }

    template<typename T1>
    RwAllocator(const RwAllocator<T1>& RWUNUSED(other)) throw()
    { }

    RwAllocator& operator=(const RwAllocator& RWUNUSED(other)) throw()
    {
        rwFUNCTION();
        rwRETURN(*this);
    }

    template<typename T1>
    RwAllocator& operator=(const RwAllocator<T1>& RWUNUSED(other)) throw()
    {
        rwFUNCTION();
        rwRETURN(*this);
    }

    // Destructor
    ~RwAllocator() throw()
    { }

    /**
    Returns a pointer to the passed reference.
    \param x An address to be return a pointer to.
    \return A pointer to the address.
    */
    pointer address(reference x) const
    {
        return &x;
    }

    /**
    Returns a const pointer to the passed reference.
    \param x An const address to be return a const pointer to.
    \return A const pointer to the address.
    */
    const_pointer address(const_reference x) const
    {
        return &x;
    }

    // NB: n is permitted to be 0.  The C++ standard says nothing
    // about what the return value is when n == 0.
    /**
    Allocates memory for n T's
    \param n The number of T' to allocate memory for.
    \param p An unused pointer.
    \return A pointer to the allocated memory.
    */
    T* allocate(size_type n, const void* RWUNUSED(p) = 0)
    {
        rwFUNCTION();
        rwASSERT(RwMemGetMalloc() != NULL);
        rwRETURN((T*)(RwMalloc(n * sizeof(T))));
    }

    // NB: p is not permitted to be a null pointer.
    /**
    Frees the memory.
    \param p A pointer to the memory to be freed, should not be null.
    \param n An unused size.
    */
    void deallocate(pointer p, size_type RWUNUSED(n))
    {
        rwFUNCTION();
        rwASSERT(RwMemGetFree() != NULL);
        RwFree((void *)p);
    }

    /**
    Returns a count the maximal number of object of type T's that can be allocated.
    \return Integer specifyihng the largest number of T's which may be allocated.

    The returned number is limited only by the address space of the hosting OS.
    */
    size_type max_size() const throw()
    {
        rwFUNCTION();
        rwRETURN(size_t(-1)/sizeof(T));
    }

    /**
    Initializes the pointed to object of type \e  T with the given value.
    \param p A pointer to the memory to be initialized.
    \param val The value to use for initializing *p.

    A placement new call is made for the memory pointed to by \e  p, constructing an object
    of type \e  T at the location, with inital value \e  val.
    */
    void construct(pointer p, const T& val)
    {
        rwFUNCTION();
        new(p) T(val);
        rwRETURNVOID();
    }

    /**
    Destroys the pointed to object but does not free its allocated memory.
    \param p Pointer to object to be destroyed.

    The object's destructor is called. Its alloacted memory is not freed.
    */
    void destroy(pointer p)
    {
#ifdef _MSC_VER
        // MS compiler complains that p is unused
        (void)p;
#endif
        p->~T();
    }
};

/**
Compares two allocators for equality.

RwAllocators are always equal.
\return This is always true, because RwAllocators are all the same.
*/
template<class _Ty,
    class _Other> inline
    bool operator==(const RwAllocator<_Ty>&, const RwAllocator<_Other>&)
    {   // test for allocator equality (always true)
        return (true);
    }

/**
Compares two allocators for inequality.

RwAllocators are always equal.
\return This is always false, because RwAllocators are all the same.
*/
template<class _Ty,
    class _Other> inline
    bool operator!=(const RwAllocator<_Ty>&, const RwAllocator<_Other>&)
    {   // test for allocator inequality (always false)
        return (false);
    }
#endif //_STLPORT_VERSION


// ***********************************************************************************************************
//                                          RwString RwWString CLASSES
// ***********************************************************************************************************

// This is a workaround a parsing bug in the VC6. It has a problem with '_STL::char_traits<'. This only
// happens if we don't pull in the underlying stl and iostream support. I've tried not using typedefs and
// using a derived class from the basic_string like we do for the vectors and other containers, but it
// still didn't work. This is the only nice workaround I have found. If we don't have to support VC6 we
// can just have top two typedefs. Note that we have to use a different temporary name __RwString within
// _STL so that name doesn't clash with the final typedef at the end.
namespace rwstd
{
typedef basic_string<char, char_traits<char>, RwAllocator<char> > __RwString;
typedef basic_string<wchar_t, char_traits<wchar_t>, RwAllocator<wchar_t> > __RwWString;
}

// Required typedef for VC6 compiler bug workaround.
typedef rwstd::__RwString RwString;

// Required typedef for VC6 compiler bug workaround.
typedef rwstd::__RwWString RwWString;

// ***********************************************************************************************************
//                                                RwPair CLASS
// ***********************************************************************************************************

/**
A wrapper for the standard STL pair class.
\par Template_params
T1 the type of the first component in the pair.
T2 the type of the second component in the pair.

A pair contains two values which may be passed around as a group.
*/
template <class T1, class T2>
class RwPair : public rwstd::pair< T1, T2 >
{
public:

    /**
    Default constructor
    */
    RwPair() : rwstd::pair<T1, T2> () {}

    /**
    Constructor
    \param a The first component of the pair to construct.
    \param b The second component of the pair.
    */
    RwPair(const T1& a, const T2& b) : rwstd::pair<T1, T2> (a, b) {}

    // Converting copy constructor.
    /**
    Copy constructor to convert an (U1, U2) pair into a (T1, T2) pair.
    \param p A pair of different type to initialize this one with.
    */
    template <class U1, class U2>
    RwPair(const RwPair<U1, U2> &p) : rwstd::pair<T1, T2>(p) {}

    /**
    Copy constructor
    \param o A pair to initialize this one with.
    */
    RwPair(const RwPair<T1, T2> &o) : rwstd::pair<T1, T2>(o) {}

    RwPair& operator=(const RwPair& other)
    {
        rwstd::pair<T1, T2>::operator=(other);
        return (*this);
    }

    template <class U1, class U2>
    RwPair& operator=(const RwPair<U1, U2>& other)
    {
        rwstd::pair<T1, T2>::operator=(other);
        return (*this);
    }
};

/**
Helper function for creating RwPair objects.
\param x The first component of the pair.
\param y The second component of the pair.
\return The pair (x,y).
*/
template <class T1, class T2>
inline RwPair<T1, T2> RwMakePair(const T1 &x, const T2 &y)
{
  return RwPair<T1, T2>(x, y);
}

// ***********************************************************************************************************
//                                                RwVector CLASS
// ***********************************************************************************************************

/**
Implements a vector object based on STL vector.
*/
template <class T>
class RwVector : public rwstd::vector< T, RwAllocator<T> >
{
public:
    // These templated types are defined in the base class, but for some reason we
    // are getting warnings for them in the gcc3+, so we'll define them here as well.
    typedef size_t rw_size_type;

    /**
    Default constructor.
    */
    explicit RwVector() : rwstd::vector<T, RwAllocator<T> > () {}

    /**
    Copy constructor.
    \param x An other vector of the same type to initialize this one with.
    */
    RwVector(const RwVector<T> &_x) : rwstd::vector<T, RwAllocator<T> > (_x) {}

    /**
    Constructs a vector of a given size.
    \param _n The size of the vector.
    */
    explicit RwVector(rw_size_type _n) : rwstd::vector<T, RwAllocator<T> > (_n) {}

    /**
    Constructs a vector of a given size and sets the entries to a given default value.
    \param _n The size of the vector.
    \param _val The default value for the entries.
    */
    RwVector(rw_size_type _n, const T &_val) : rwstd::vector<T, RwAllocator<T> > (_n, _val) {}
#ifndef _STLP_NO_MEMBER_TEMPLATES
    /**
    Constructor that copies everythings between the given iterators into the vector.

    _first and _last should be iteration the same container. _last should be after _first.
    \param _first The iterator from where to start copying.
    \param _last The iterator where to stop copying.
    */
    template <class _InputIterator>
    RwVector(_InputIterator _first, _InputIterator _last)
        : rwstd::vector<T, RwAllocator<T> > (_first, _last) {}
#else // _STLP_NO_MEMBER_TEMPLATES
    /**
    Constructor that copies everythings between the given pointers into the vector.

    _first and _last should be pointing to entries in an array. _last should be after _first.
    \param _first The position from where to start copying.
    \param _last The position where to stop copying.
    */
    RwVector(const T *_first, const T *_last) : rwstd::vector<T, RwAllocator<T> > (_first, _last) {}
#endif // _STLP_NO_MEMBER_TEMPLATES

};

// ***********************************************************************************************************
//                                                RwList CLASS
// ***********************************************************************************************************

/**
Implements a list object based on STL list.
*/
template <class T>
class RwList : public rwstd::list< T, RwAllocator<T> >
{
public:
    // These templated types are defined in the base class, but for some reason we
    // are getting warnings for them in the gcc3+, so we'll define them here as well.
    typedef size_t                                              rw_size_type;
    typedef typename rwstd::list<T, RwAllocator<T> >::const_iterator      rw_const_iterator;
    /**
    Default constructor.
    */
    RwList() : rwstd::list<T, RwAllocator<T> > () {}

    /**
    Copy constructor.
    \param _x The list to initialize this one with.
    */
    RwList(const RwList<T> & _x) : rwstd::list<T, RwAllocator<T> > (_x) {}

    /**
    Constructs A list of a given size.
    \param _n The size of the list
    */
    explicit RwList(rw_size_type _n) : rwstd::list<T, RwAllocator<T> > (_n) {}

    /**
    Constructs a list of a given size with a given default value for the entries.
    \param _n The size of the list.
    \param _val The default value for the entries.
    */
    RwList(rw_size_type _n, const T &_val) : rwstd::list<T, RwAllocator<T> > (_n, _val) {}
#ifndef _STLP_NO_MEMBER_TEMPLATES
    /**
    Constructs a list and copies everything between the given iterators.

    The iterators should be iterating the same container and _last should be after _first.
    \param _first The iterator where to start copying.
    \param _last The iterator where to stop copying.
    */
    template <class _InputIterator>
    RwList(_InputIterator _first, _InputIterator _last) : rwstd::list<T, RwAllocator<T> > (_first, _last) {}
#else // _STLP_NO_MEMBER_TEMPLATES
    /**
    Constructor that copies everythings between the given pointer into the list.

    _first and _last should be pointing to entries in an array. _last should be after _first.
    \param _first The position from where to start copying.
    \param _last The position where to stop copying.
    */
    RwList(const T *_first, const T *_last) : rwstd::list<T, RwAllocator<T> > (_first, _last) {}
    /**
    Constructs a list and copies everything between the given iterators.

    The iterators should be iterating the same container and _last should be after _first.
    \param _first The iterator where to start copying.
    \param _last The iterator where to stop copying.
    */
    RwList(rw_const_iterator _first, rw_const_iterator _last)
        : rwstd::list<T, RwAllocator<T> > (_first, _last) {}
#endif // _STLP_NO_MEMBER_TEMPLATES
};

// ***********************************************************************************************************
//                                                RwSet CLASS
// ***********************************************************************************************************

/**
Implements a set object based on STL set.

A set that stores items of type T sorted by the comparator comp1.
*/
template <class T, class comp1 = rwstd::less<T> >
class RwSet : public rwstd::set< T, comp1, RwAllocator<T> >
{
public:
    // These templated types are defined in the base class, but for some reason we
    // are getting warnings for them in the gcc3+, so we'll define them here as well.
    typedef T                                                             rw_value_type;
    typedef typename rwstd::set<T, comp1, RwAllocator<T> >::const_iterator  rw_const_iterator;

    /**
    Default constructor.
    */
    RwSet() : rwstd::set<T, comp1, RwAllocator<T> > () {}

    /**
    Constructs an empty set specifying a comparator.
    \param _comp An instance of the comparator for use within the instance.
    */
    explicit RwSet(const comp1 &_comp) : rwstd::set<T, comp1, RwAllocator<T> >(_comp) {}

    /**
    Copy constructor.
    \param _x The set to copy.
    */
    RwSet(const RwSet<T> &_x) : rwstd::set<T, comp1, RwAllocator<T> > (_x) {}
#ifndef _STLP_NO_MEMBER_TEMPLATES
    /**
    Constructs a set by copying everything between to given iterators

    The iterators should be iterating the same container and _last should be after _first.

    \param _first The iterator where to start copying.
    \param _last The iterator where to stop copying.
    */
    template <class _InputIterator>
    RwSet(_InputIterator _first, _InputIterator _last)
        : rwstd::set<T, comp1, RwAllocator<T> > (_first, _last) {}

    /**
    Constructs a set by copying everything between to given iterators and specifying a comparator

    The iterators should be iterating the same container and _last should be after _first.

    \param _first The iterator where to start copying.
    \param _last The iterator where to stop copying.
    \param _comp An instance of the comparator to use.
    */
    template <class _InputIterator>
    RwSet(_InputIterator _first, _InputIterator _last, const comp1 &_comp)
        : rwstd::set<T, comp1, RwAllocator<T> > (_first, _last, _comp) {}
#else // _STLP_NO_MEMBER_TEMPLATES

    /**
    Constructs a set copying everything between to given pointers.

    _first and _last should be pointing into an array and _last should be after _first.
    \param _first The position where to start copying.
    \param _last The position where to stop copying.
    */
    RwSet(const rw_value_type *_first, const rw_value_type *_last)
        : rwstd::set<T, comp1, RwAllocator<T> > (_first, _last) {}

    /**
    Constructs a set copying everything between to given pointers with a specified comparator.

    _first and _last should be pointing into an array and _last should be after _first.
    \param _first The position where to start copying.
    \param _last The position where to stop copying.
    \param _comp The comparator to use.
    */
    RwSet(const rw_value_type *_first, const rw_value_type *_last, const comp1 &_comp)
        : rwstd::set<T, comp1, RwAllocator<T> > (_first, _last, _comp) {}

    /**
    Constructs a set copying everything between to given iterators.

    _first and _last should be iterating the same container and _last should be after _first.
    \param _first The position where to start copying.
    \param _last The position where to stop copying.
    */
    RwSet(rw_const_iterator _first, rw_const_iterator _last)
        : rwstd::set<T, comp1, RwAllocator<T> > (_first, _last) {}

    /**
    Constructs a set copying everything between to given iterators with a specified comparator.

    _first and _last should be iterating the same container and _last should be after _first.
    \param _first The position where to start copying.
    \param _last The position where to stop copying.
    \param _comp The comparator to use.
    */
    RwSet(rw_const_iterator _first, rw_const_iterator _last, const comp1 &_comp)
        : rwstd::set<T, comp1, RwAllocator<T> > (_first, _last, _comp) {}
#endif // _STLP_NO_MEMBER_TEMPLATES
};

// ***********************************************************************************************************
//                                                RwMultiSet CLASS
// ***********************************************************************************************************

/**
Implements a multi-set object based on STL multiset.

A set that stores items of type T sorted by the comparator comp1. Multiple entries with the same key
are supported.
*/
template <class T, class comp1 = rwstd::less<T> >
class RwMultiSet : public rwstd::multiset< T, comp1, RwAllocator<T> >
{
public:
    // These templated types are defined in the base class, but for some reason we
    // are getting warnings for them in the gcc3+, so we'll define them here as well.
    typedef T                                                             rw_value_type;
    typedef typename rwstd::multiset<T, comp1, RwAllocator<T> >::const_iterator  rw_const_iterator;

    /**
    Default constructor.
    */
    RwMultiSet() : rwstd::multiset<T, comp1, RwAllocator<T> > () {}

    /**
    Constructs an empty set specifying a comparator.
    \param _comp An instance of the comparator for use within the instance.
    */
    explicit RwMultiSet(const comp1 &_comp) : rwstd::multiset<T, comp1, RwAllocator<T> >(_comp) {}

    /**
    Copy constructor
    \param _x the set to copy
    */
    RwMultiSet(const RwSet<T> &_x) : rwstd::multiset<T, comp1, RwAllocator<T> > (_x) {}
#ifndef _STLP_NO_MEMBER_TEMPLATES

    /**
    Constructs a set by copying everything between to given iterators

    The iterators should be iterating the same container and _last should be after _first.

    \param _first The iterator where to start copying.
    \param _last The iterator where to stop copying.
    */
    template <class _InputIterator>
    RwMultiSet(_InputIterator _first, _InputIterator _last)
        : rwstd::multiset<T, comp1, RwAllocator<T> > (_first, _last) {}

    /**
    Constructs a set by copying everything between to given iterators and specifying a comparator

    The iterators should be iterating the same container and _last should be after _first.

    \param _first The iterator where to start copying.
    \param _last The iterator where to stop copying.
    \param _comp An instance of the comparator to use.
    */
    template <class _InputIterator>
    RwMultiSet(_InputIterator _first, _InputIterator _last, const comp1 &_comp)
        : rwstd::multiset<T, comp1, RwAllocator<T> > (_first, _last, _comp) {}

#else // _STLP_NO_MEMBER_TEMPLATES

    /**
    Constructs a set copying everything between to given pointers.

    _first and _last should be pointing into an array and _last should be after _first.
    \param _first The position where to start copying.
    \param _last The position where to stop copying.
    */
    RwMultiSet(const rw_value_type *_first, const rw_value_type *_last)
        : rwstd::multiset<T, comp1, RwAllocator<T> > (_first, _last) {}

    /**
    Constructs a set copying everything between to given pointers with a specified comparator.

    _first and _last should be pointing into an array and _last should be after _first.
    \param _first The position where to start copying.
    \param _last The position where to stop copying.
    \param _comp The comparator to use.
    */
    RwMultiSet(const rw_value_type *_first, const rw_value_type *_last, const comp1 &_comp)
        : rwstd::multiset<T, comp1, RwAllocator<T> > (_first, _last, _comp) {}

    /**
    Constructs a set copying everything between to given iterators.

    _first and _last should be iterating the same container and _last should be after _first.
    \param _first The position where to start copying.
    \param _last The position where to stop copying.
    */
    RwMultiSet(rw_const_iterator _first, rw_const_iterator _last)
        : rwstd::multiset<T, comp1, RwAllocator<T> > (_first, _last) {}

    /**
    Constructs a set copying everything between to given iterators with a specified comparator.

    _first and _last should be iterating the same container and _last should be after _first.
    \param _first The position where to start copying.
    \param _last The position where to stop copying.
    \param _comp The comparator to use.
    */
    RwMultiSet(rw_const_iterator _first, rw_const_iterator _last, const comp1 &_comp)
        : rwstd::multiset<T, comp1, RwAllocator<T> > (_first, _last, _comp) {}
#endif // _STLP_NO_MEMBER_TEMPLATES
};

// ***********************************************************************************************************
//                                                RwMap CLASS
// ***********************************************************************************************************

/**
Implements a map object based on STL map.

A map that stores pairs of (K, T) sorted in K by the comparator comp1.
*/
template <class K, class T, class comp1 = rwstd::less<K> >
class RwMap : public rwstd::map< K, T, comp1, RwAllocator<rwstd::pair<const K, T> > >
{
public:
    // These templated types are defined in the base class, but for some reason we
    // are getting warnings for them in the gcc3+, so we'll define them here as well.
    typedef rwstd::pair<const K, T>                                              rw_value_type;
    typedef RwAllocator<rw_value_type>                                           rw_allocator;

    //The MS STL has the value type as pair<const K, T> which cause problems when trying
    //list<map<K,T>::value_type>
    typedef rwstd::pair<K, T>                                                    rw_value_type_non_const;
    typedef typename rwstd::map< K, T, comp1, rw_allocator>::const_iterator  rw_const_iterator;

    /**
    Default constructor.
    */
    RwMap() : rwstd::map<K, T, comp1, rw_allocator> () {}

    /**
    Constructor.
    \param _comp A comparator for use within the instance.
    */
    explicit RwMap(const comp1 &_comp) : rwstd::map<K, T, comp1, rw_allocator>(_comp) {}

    /**
    Copy constructor.
    \param _x The map to copy.
    */
    RwMap(const RwMap<K, T, comp1> &_x) : rwstd::map<K, T, comp1, rw_allocator> (_x) {}

#ifndef _STLP_NO_MEMBER_TEMPLATES
    /**
    Constructs a map by copying everything between two given iterators.

    _first and _last should be iterating the same container, _last should be after _first.
    \param _first The iterator where to start copying.
    \param _last The iterator where to stop copying.
    */
    template <class _InputIterator>
    RwMap(_InputIterator _first, _InputIterator _last)
        : rwstd::map<K, T, comp1, rw_allocator> (_first, _last) {}

    /**
    Constructs a map by copying everything between two given iterators specifying a comparator.

    _first and _last should be iterating the same container, _last should be after _first.
    \param _first The iterator where to start copying.
    \param _last The iterator where to stop copying.
    \param _comp The comparator to use.
    */
    template <class _InputIterator>
    RwMap(_InputIterator _first, _InputIterator _last, const comp1 &_comp)
        : rwstd::map<K, T, comp1, rw_allocator> (_first, _last,  _comp) {}

#else // _STLP_NO_MEMBER_TEMPLATES

    /**
    Constructs a map by copying everything between two given pointers.

    _first and _last should be pointing into an array, _last should be after _first.
    \param _first The position where to start copying.
    \param _last The position where to stop copying.
    */
    RwMap(const rw_value_type* _first, const rw_value_type* _last)
        : rwstd::map<K, T, comp1, rw_allocator> (_first, _last) {}

    /**
    Constructs a map by copying everything between two given pointers specifying a comparator.

    _first and _last should be pointing into an array, _last should be after _first.
    \param _first The position where to start copying.
    \param _last The position where to stop copying.
    \param _comp The comparator to use.
    */
    RwMap(const rw_value_type* _first, const rw_value_type* _last, const comp1 &_comp)
        : rwstd::map<K, T, comp1, rw_allocator> (_first, _last, _comp) {}

    /**
    Constructs a map by copying everything between two given iterators.

    _first and _last should be iterating the same container, _last should be after _first.
    \param _first The iterator where to start copying.
    \param _last The iterator where to stop copying.
    */
    RwMap(rw_const_iterator _first, rw_const_iterator _last)
        : rwstd::map<K, T, comp1, rw_allocator> (_first, _last) {}

    /**
    Constructs a map by copying everything between two given iterators specifying a comparator.

    _first and _last should be iterating the same container, _last should be after _first.
    \param _first The iterator where to start copying.
    \param _last The iterator where to stop copying.
    \param _comp The comparator to use.
    */
    RwMap(rw_const_iterator _first, rw_const_iterator _last, const comp1 &_comp)
        : rwstd::map<K, T, comp1, rw_allocator> (_first, _last, _comp) {}
#endif // _STLP_NO_MEMBER_TEMPLATES
};

// ***********************************************************************************************************
//                                                RwMultiMap CLASS
// ***********************************************************************************************************

/**
Implements a multi-map object based on STL multimap.

A map that stores pairs of (K, T) sorted in K by the comparator comp1. Multiple entries with the same key
are supported.
*/
template <class K, class T, class comp1 = rwstd::less<K> >
class RwMultiMap : public rwstd::multimap< K, T, comp1, RwAllocator<rwstd::pair<const K, T> > >
{
public:
    // These templated types are defined in the base class, but for some reason we
    // are getting warnings for them in the gcc3+, so we'll define them here as well.
    typedef rwstd::pair<const K, T>                                              rw_value_type;
    typedef RwAllocator<rw_value_type>                                           rw_allocator;

    //The MS STL has the value type as pair<const K, T> which cause problems when trying
    //list<map<K,T>::value_type>
    typedef rwstd::pair<K, T>                                                    rw_value_type_non_const;
    typedef typename rwstd::multimap< K, T, comp1, rw_allocator>::const_iterator  rw_const_iterator;

    /**
    Default constructor.
    */
    RwMultiMap() : rwstd::multimap<K, T, comp1, rw_allocator> () {}

    /**
    Constructor.
    \param _comp A comparator for use within the instance.
    */
    explicit RwMultiMap(const comp1 &_comp) : rwstd::multimap<K, T, comp1, rw_allocator>(_comp) {}

    /**
    Copy constructor.
    \param _x The map to copy.
    */
    RwMultiMap(const RwMap<K, T, comp1> &_x) : rwstd::multimap<K, T, comp1, rw_allocator> (_x) {}

#ifndef _STLP_NO_MEMBER_TEMPLATES
    /**
    Constructs a map by copying everything between two given iterators.

    _first and _last should be iterating the same container, _last should be after _first.
    \param _first The iterator where to start copying.
    \param _last The iterator where to stop copying.
    */
    template <class _InputIterator>
    RwMultiMap(_InputIterator _first, _InputIterator _last)
        : rwstd::multimap<K, T, comp1, rw_allocator> (_first, _last) {}

    /**
    Constructs a map by copying everything between two given iterators specifying a comparator.

    _first and _last should be iterating the same container, _last should be after _first.
    \param _first The iterator where to start copying.
    \param _last The iterator where to stop copying.
    \param _comp The comparator to use.
    */
    template <class _InputIterator>
    RwMultiMap(_InputIterator _first, _InputIterator _last, const comp1 &_comp)
        : rwstd::multimap<K, T, comp1, rw_allocator> (_first, _last,  _comp) {}

#else // _STLP_NO_MEMBER_TEMPLATES

    /**
    Constructs a map by copying everything between two given pointers.

    _first and _last should be pointing into an array, _last should be after _first.
    \param _first The position where to start copying.
    \param _last The position where to stop copying.
    */
    RwMultiMap(const rw_value_type* _first, const rw_value_type* _last)
        : rwstd::multimap<K, T, comp1, rw_allocator> (_first, _last) {}

    /**
    Constructs a map by copying everything between two given pointers specifying a comparator.

    _first and _last should be pointing into an array, _last should be after _first.
    \param _first The position where to start copying.
    \param _last The position where to stop copying.
    \param _comp The comparator to use.
    */
    RwMultiMap(const rw_value_type* _first, const rw_value_type* _last, const comp1 &_comp)
        : rwstd::multimap<K, T, comp1, rw_allocator> (_first, _last, _comp) {}

    /**
    Constructs a map by copying everything between two given iterators.

    _first and _last should be iterating the same container, _last should be after _first.
    \param _first The iterator where to start copying.
    \param _last The iterator where to stop copying.
    */
    RwMap(rw_const_iterator _first, rw_const_iterator _last)
        : rwstd::multimap<K, T, comp1, rw_allocator> (_first, _last) {}

    /**
    Constructs a map by copying everything between two given iterators specifying a comparator.

    _first and _last should be iterating the same container, _last should be after _first.
    \param _first The iterator where to start copying.
    \param _last The iterator where to stop copying.
    \param _comp The comparator to use.
    */
    RwMap(rw_const_iterator _first, rw_const_iterator _last, const comp1 &_comp)
        : rwstd::multimap<K, T, comp1, rw_allocator> (_first, _last, _comp) {}
#endif // _STLP_NO_MEMBER_TEMPLATES
};

struct rw_StrLess
{
    bool operator()(const RwChar *s1, const RwChar *s2) const
    {
        rwFUNCTION();
        rwRETURN(RwStrcmp(s1, s2) < 0);
    }
};

typedef RwSet<const RwChar *, rw_StrLess> CStringSet;

#endif // PUBLIC_RW_CORE_STLTYPES_H
