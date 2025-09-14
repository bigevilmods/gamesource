// (c) Electronic Arts. All Rights Reserved.

/// \file
/// Templates for a hash table container class

#ifndef RW_HASHCONTAINER_HPP
#define RW_HASHCONTAINER_HPP
#include <rw/core/base/ostypes.h>
#include <rw/core/debug/debug.h>
#include <rw/core/debug/exception.h>
#include <rw/core/resource/baseresource.h>
#include <rw/core/resource/resource.h>
#include <rw/core/resource/resourcetarget.h>

// ***********************************************************************************************************
// Includes


// ***********************************************************************************************************
// Defines


namespace rw
{

// ***********************************************************************************************************
// Forward Declarations


// ***********************************************************************************************************
// Typedefs


// ***********************************************************************************************************
// Enums + Consts


// ***********************************************************************************************************
// Global Variables

#ifdef EA_DEBUG
extern RwBool rwHashContainerAssertOnHalfFull;
#endif

// ***********************************************************************************************************
// External Functions Prototypes


// ***********************************************************************************************************
// Structs + Unions + Classes


// ***********************************************************************************************************
//                                                SeparateKeyAndDatumContained Declaration
// ***********************************************************************************************************

/**
A class for holding key and data parameters for use in a  HashContainer.
KeyType and DatumType must be copyable and default constructable.
*/
template<typename KeyType, typename DatumType>
struct SeparateKeyAndDatumContained
{
public:
    /**
    Default Constructor
     */
    SeparateKeyAndDatumContained()
        : key(), data()
    {
        rwFUNCTION();
        rwRETURNVOID();
    }

    /**
    Construct with a key and data value.
    \param key the key value
    \param data the data value
    */
    SeparateKeyAndDatumContained(const KeyType& key_, const DatumType& data_)
        : key(key_), data(data_)
    {
        rwFUNCTION();
        rwRETURNVOID();
    }

    // defaults are acceptable
    // ~SeparateKeyAndDatumContained();
    // SeparateKeyAndDatumContained(const SeparateKeyAndDatumContained& other);
    // SeparateKeyAndDatumContained& operator=(const SeparateKeyAndDatumContained& rhs);


    // Public Enums, Consts, Typedefs and Static variables

    // Interface Implementations
    /**
    Get the key element.
    \returns a const reference to the key element
    */
    const KeyType& GetKey() const
    {
        rwFUNCTION();
        rwRETURN(key);
    }

    /**
    Get the key element.
    \returns a reference to the key element
    */
    KeyType& GetKey()
    {
        rwFUNCTION();
        rwRETURN(key);
    }

    /**
    Get the data element.
    \returns a const reference to the data element
    */
    const DatumType& GetData() const
    {
        rwFUNCTION();
        rwRETURN(data);
    }

    /**
    Get the data element.
    \returns a reference to the data element
    */
    DatumType& GetData()
    {
        rwFUNCTION();
        rwRETURN(data);
    }

    /**
    Equality operator
    \returns true if both the keys and data are equal
    */
    RwBool operator==(const SeparateKeyAndDatumContained& rhs) const
    {
        rwFUNCTION();
        rwRETURN(static_cast<RwBool>((key == rhs.key) && (data == rhs.data)));
    }

    /**
    Inequality operator
    \returns true if either the keys or data are equal
    */
    RwBool operator!=(const SeparateKeyAndDatumContained& rhs) const
    {
        rwFUNCTION();
        rwRETURN(static_cast<RwBool>(!(*this == rhs)));
    }

    // Methods

    // Properties


protected:
    // Virtual Functions


private:
    // Private Enums, Consts, Typedefs and Static variables

    // Helper Member Functions

    // Member Variables
    KeyType   key;
    DatumType data;
};


// ***********************************************************************************************************
//                                                FixedSizeBufferPolicy Declaration
// ***********************************************************************************************************
/**
A policy used by  HashContainer to specify the memory where the table will be stored. This policy uses a
fixed size buffer which is instantiated statically on construction.

CAPACITY is the desired capacity of the hash table.
HashTraitsT is a traits class used to obtain the type of objects stored in the HashContainer including the
DefaultContained value which will be used to instantiate the table. Requirements for HashTraitsT are
documented in the class documentation for HashContainer.
*/
template<size_t CAPACITY, typename HashTraitsT>
struct FixedSizeBufferPolicy
{
    /**
    A class to provide extra details to the  FixedSizeBufferPolicy during construction.
    */
    struct BufferPolicyDetails
    {
    };

    /**
    Constructor accepting a BufferPolicyDetails object.
    This constructor will clear the memory to the DefaultContained value specified by the HashTraits.
    \param details the policy details
    */
    FixedSizeBufferPolicy(BufferPolicyDetails &RWUNUSED(details))
    {
        rwFUNCTION();
        typename HashTraitsT::Contained defaultValue = HashTraitsT::DefaultContained();
        for (typename HashTraitsT::Contained *ptr = contents; ptr < contents + CAPACITY; ++ptr)
        {
            *ptr = defaultValue;
        }
        rwRETURNVOID();
    }

    /**
    \return the maximum number of elements this buffer may hold
    */
    size_t Capacity() const
    {
        rwFUNCTION();
        rwRETURN(CAPACITY);
    }

    /**
    \return the initial number of elements in the buffer
    */
    size_t GetInitialSize() const
    {
        rwFUNCTION();
        rwRETURN(0);
    }

    typename HashTraitsT::Contained contents[CAPACITY];
};


// ***********************************************************************************************************
//                                                PreallocatedBufferPolicy Declaration
// ***********************************************************************************************************
/**
A policy used by  HashContainer to specify the memory where the table will be stored. This policy uses
memory which has been preallocated by the caller and passed into the class on construction. This memory will
be initialized to the DefaultContained value specified by the HashTraits type.

HashTraitsT is a traits class used to obtain the type of objects stored in the HashContainer including the
default value. Requirements for HashTraitsT are documented in the class documentation for HashContainer.
*/
template<typename HashTraitsT>
struct PreallocatedBufferPolicy
{
    /**
    A class to provide extra details to the  PreallocatedBufferPolicy during construction.
    */
    struct BufferPolicyDetails
    {
        /**
        Default Constructor
        */
        BufferPolicyDetails()
            : contents(0), capacity(0)
        {
            rwFUNCTION();
            rwRETURNVOID();
        }

        /**
        Constructor accepting a pointer to the memory to be used and its capacity.
        This constructor will clear the memory to the DefaultContained value specified by the HashTraits.
        \param contents_ a pointer to the memory for the hash table to use
        \param capacity the maximum capacity of the hash table
        */
        BufferPolicyDetails(typename HashTraitsT::Contained *contents_, size_t capacity_)
            : contents(contents_), capacity(capacity_)
        {
            rwFUNCTION();
            typename HashTraitsT::Contained defaultValue = HashTraitsT::DefaultContained();
            for (typename HashTraitsT::Contained *ptr = contents; ptr < contents + capacity; ++ptr)
            {
                *ptr = defaultValue;
            }
            rwRETURNVOID();
        }

        typename HashTraitsT::Contained *contents;
        size_t capacity;
    };

    /**
    Constructor accepting a BufferPolicyDetails object.
    */
    PreallocatedBufferPolicy(BufferPolicyDetails &details)
        : contents(details.contents),
          capacity(details.capacity)
    {
        rwFUNCTION();
        rwRETURNVOID();
    }

    /**
    \return the maximum number of elements this buffer may hold
    */
    size_t Capacity() const
    {
        rwFUNCTION();
        rwRETURN(capacity);
    }

    /**
    \return the initial number of elements in the buffer
    */
    size_t GetInitialSize() const
    {
        rwFUNCTION();
        rwRETURN(0);
    }

    typename HashTraitsT::Contained *contents;
    size_t capacity;
};


// ***********************************************************************************************************
//                                                PreinitializedBufferPolicy Declaration
// ***********************************************************************************************************
/**
A policy used by  HashContainer to specify the memory where the table will be stored. This policy uses
memory which has been preallocated and preinitialized and passed into the class on construction. This policy
class may be used to instantiate a table from a pre-existing table in raw memory.

HashTraitsT is a traits class used to obtain the type of objects stored in the HashContainer including the
default value. Requirements for HashTraitsT are documented in the class documentation for HashContainer.
*/
template<typename HashTraitsT>
struct PreinitializedBufferPolicy
{
    /**
    A class to provide extra details to the  PreinitializedBufferPolicy during construction.
     */
    struct BufferPolicyDetails
    {
        /**
        Default Constructor
        */
        BufferPolicyDetails()
            : contents(0), capacity(0)
        {
            rwFUNCTION();
            rwRETURNVOID();
        }

        /**
        Constructor accepting a pointer to the memory to be used and its capacity.
        \param contents_ a pointer to the memory for the hash table to use
        \param capacity_ the maximum capacity of the hash table
        */
        BufferPolicyDetails(typename HashTraitsT::Contained *contents_, size_t capacity_)
            : contents(contents_), capacity(capacity_)
        {
            rwFUNCTION();
            rwRETURNVOID();
        }

        typename HashTraitsT::Contained *contents;
        size_t capacity;
    };

    /**
    Constructor accepting a BufferPolicyDetails object.
    */
    PreinitializedBufferPolicy(BufferPolicyDetails &details)
        : contents(details.contents),
          capacity(details.capacity)
    {
        rwFUNCTION();
        rwRETURNVOID();
    }

    /**
    \return the maximum number of elements this buffer may hold
    */
    size_t Capacity() const
    {
        rwFUNCTION();
        rwRETURN(capacity);
    }

    /**
    \return the initial number of elements in the buffer
    */
    size_t GetInitialSize() const
    {
        rwFUNCTION();
        // we don't know how many elements there initially are in the table so count them
        size_t size = 0;
        for (typename HashTraitsT::Contained *ptr = contents; ptr < contents + capacity; ++ptr)
        {
            if (!HashTraitsT::EqualTo(HashTraitsT::BadKey(), HashTraitsT::GetKey(*ptr)))
            {
                ++size;
            }
        }
        rwRETURN(size);
    }

    typename HashTraitsT::Contained *contents;
    size_t capacity;
};


// ***********************************************************************************************************
//                                                HashContainer Declaration
// ***********************************************************************************************************
/**
A hash table class. This class maps a key against a data element. Elements are stored in a table with the
index equal

The hashing function and the type of the elements is provided by the HashTraitsT template parameter.
The type of buffer used to store the hash table is provided by the BufferPolicy template parameter.

BufferPolicy is used to control where the memory for the HashContainer is allocated.
Buffer Policy must define:
    RwSizeT Capacity();           // get the maximum number of elements the container may old
    RwSizeT GetInitialSize();     // get the initial number of elements in the container


HashTraitsT must define the following:
   typedef KeyType;               // must be copyable
   typedef DatumType;             // must be copyable
   typedef Contained;             // the type of element stored in the hash table, must be copyable
   static const KeyType BadKey();
                                  // returns an invalid keytype to mark nonexistant hashes
   static const Contained DefaultContained();
                                  // returns a default contained object with a key equal to the bad key
                                  // require EqualTo(GetKey(DefaultContained()), BadKey()) == true
   static KeyType& GetKey(Contained& obj)
   static const KeyType& GetKey(const Contained& obj)
                                  // returns the key element of an object
   static DatumType& GetData(Contained& obj)
   static const DatumType& GetData(const Contained& obj)
                                  // returns the data element of an object
   static size_t Hash(size_t size, const KeyType &key)
                                  // Hashing function to use
   static bool EqualTo(const KeyType& lhs, const KeyType& rhs)
                                  // Equality test
   static size_t GetAlign()      // Return max alignment of KeyType
*/
template <typename BufferPolicy, typename HashTraitsT>
class HashContainer : private BufferPolicy
{
public:
    typedef HashTraitsT                     HashTraits;
    typedef typename HashTraitsT::KeyType   KeyType;
    typedef typename HashTraitsT::DatumType DatumType;
    typedef typename HashTraitsT::Contained Contained;
    typedef typename BufferPolicy::BufferPolicyDetails SPDetails;

    /**
    Simple forward readonly iterator for iterating through the HashContainer. Iterators can not be used
    to modify objects in the container

    T is the type of object stored in the Container.

    \note Iterators may be invalidated by Erase and Insert.
    */
    template<class T>
    class IteratorT
    {
    public:
        friend class HashContainer<BufferPolicy, HashTraitsT>;

        /**
        Default Constructor
         */
        IteratorT()
            : ptr(0), end(0)
        {
            rwFUNCTION();
            rwRETURNVOID();
        }

        /**
        Constructor taking a pointer to an object in the container.
        \param ptr_ a pointer to an object in the container
        \param end_ the end of the container
        */
        IteratorT(T *ptr_, T* end_)
            : ptr(ptr_), end(end_)
        {
            rwFUNCTION();
            rwRETURNVOID();
        }

        /**
        Move the pointer to the next object in the container.
        \return an iterator to the next object or end if there are no more objects in the container
        */
        IteratorT operator++()
        {
            rwFUNCTION();

            rwASSERTM(ptr <= end, ("Invalid iterator: ptr is greater than end"));

            // check that we aren't already at the end of the container
            if (ptr == end)
            {
                rwRETURN(*this);
            }

            do
            {
              ++ptr;
            }
            while ((ptr < end) && HashTraitsT::EqualTo(HashTraitsT::BadKey(), HashTraitsT::GetKey(*ptr)));

            rwRETURN(*this);
        }

        /**
        Dereferencing Operator
        \return the object the iterator is currently pointing to
        */
        const T& operator*() const
        {
            rwFUNCTION();
            rwASSERTM(ptr < end, ("Invalid iterator: ptr is greater than end"));
            rwRETURN(*ptr);
        }

        /**
        Dereferencing Operator
        \return a pointer to the object the iterator is currently pointing to
        */
        const T* operator->() const
        {
            rwFUNCTION();
            rwASSERTM(ptr < end, ("Invalid iterator: ptr is greater than end"));
            rwRETURN(ptr);
        }

        /**
        Equality Operator
        \return true if the iterators point to the same object
        */
        bool operator==(const IteratorT& rhs) const
        {
            rwFUNCTION();
            rwASSERTM(end == rhs.end, ("Invalid iterator comparison: end ptrs differ."));
            rwRETURN((ptr == rhs.ptr));
        }

        /**
        Inequality Operator
        \return true if the iterators do not point to the same object
        */
        bool operator!=(const IteratorT& rhs) const
        {
            rwFUNCTION();
            rwRETURN(!(ptr == rhs.ptr));
        }

    private:
        T *ptr;
        T *end;
    };

    typedef IteratorT<Contained>       iterator;
    typedef IteratorT<const Contained> const_iterator;


    /**
    Construct with the buffer policy details.
    \param details the details for the buffer policy used for this HashContainer
    */
    HashContainer(SPDetails details = SPDetails())
        : BufferPolicy(details), size(BufferPolicy::GetInitialSize())
    {
        rwFUNCTION();
        rwRETURNVOID();
    }

    // defaults are acceptable
    // HashContainer(const HashContainer& other);
    // HashContainer& operator=(const HashContainer& rhs);


    // RW4 Object interface
    /**
    \return the resource requirements of the hash container
    */
    static ResourceDescriptor
    GetResourceDescriptor()
    {
        rwFUNCTION();
        rwRETURN(ResourceDescriptor(sizeof(HashContainer), HashTraitsT::GetAlign()));
    }

    /**
    \deprecated This is deprecated functionality

    \return the size of the hash container
    */
    static size_t
    GetSize()
    {
        rwFUNCTION();
        rwDEPRECATED();
        rwRETURN(GetResourceDescriptor().GetSize());
    }

    /**
    \deprecated This is deprecated functionality

    \return the alignment requirement of the hash container.
    */
    static size_t
    GetAlign()
    {
        rwFUNCTION();
        rwDEPRECATED();
        rwRETURN(GetResourceDescriptor().GetAlignment());
    }

    /**
    Create a hash table from a resource.
    \param resource the Resource the HashContainer is initialized into.
    */
    static HashContainer *
    Initialize(const Resource& resource, SPDetails details = SPDetails())
    {
        rwFUNCTION();
        rwRETURN(new (resource.GetMemoryResource()) HashContainer(details));
    }

    /**
    Create a hash table from raw memory.
    \deprecated This is deprecated functionality

    \param ptr a pointer to the raw memory
    */
    static HashContainer *
    Initialize(void *ptr, SPDetails details = SPDetails())
    {
        rwFUNCTION();
        rwDEPRECATED();
        rwRETURN(Initialize(Resource(ptr), details));
    }

    /**
    \return true if the hash container is empty.
    */
    bool Empty() const
    {
        rwFUNCTION();
        rwRETURN(0 == size);
    }

    /**
    \return the number of elements in the hash container.
    */
    size_t Size() const
    {
        rwFUNCTION();
        rwRETURN(size);
    }

    /**
    \return the maximum number of elements the hash container can hold.
    */
    size_t Capacity() const
    {
        rwFUNCTION();
        rwRETURN(BufferPolicy::Capacity());
    }


    /**
    \return an iterator to the first object in the hash container
    */
    iterator Begin();

    /**
    \return a const iterator to the first object in the hash container
    */
    const_iterator Begin() const;

    /**
    \return an iterator to the end of the hash container
    */
    iterator End();

    /**
    \return a const iterator to the end of the hash container
    */
    const_iterator End() const;


    /**
    Find the object with this key value in the hash container.
    \return an iterator to the object, or end if there is no object with this key in the container.
    */
    iterator
    Find(const KeyType& key);
    /**
    Find the object with this key value in the hash container.
    \return a const iterator to the object, or end if there is no object with this key in the container.
    */
    const_iterator
    Find(const KeyType& key) const;


    /**
    Insert an object into the hash container
    An object with this key value must not exist in the table.
    \param obj the object to be inserted
    */
    void
    Insert(const Contained& obj);

    /**
    Get the data component of the object with this key value.
    If an object with this key doesn't already exist in the table one is automatically added.

    \note behavior is undefined if the value of the key component is altered through this reference.
    \param key the key value
    \return a reference to the data component of the object with this key value
    */
    DatumType&
    operator[](const KeyType &key);

    /**
    Erase the object with this key value from the hash container.
    \param key the key value
    */
    void
    Erase(const KeyType& key);

    /**
    Erase the object at this position within the hash container.
    \param position an iterator pointing to the object to be erased
    */
    void
    Erase(const iterator& position);


    /**
    Clear the hash container.
    */
    void Clear();

    /**
    Swap the internals of one hash container for another.
    */
    void
    Swap(HashContainer &rhs);



private:
    class Index
    {
    public:
        Index(size_t pos_, size_t size_)
            : pos(pos_), size(size_)
        {
            rwFUNCTION();
            rwASSERTM(pos < size, ("Invalid index. pos must be less than size"));
            rwRETURNVOID();
        }

        operator size_t() const
        {
            rwFUNCTION();
            rwRETURN(pos);
        }

        Index operator++()
        {
            rwFUNCTION();
            ++pos;
            if (pos == size)
            {
                pos = 0;
            }
            rwRETURN(*this);
        }

        Index operator--()
        {
            rwFUNCTION();
            if (pos == 0)
            {
                pos = size;
            }
            --pos;
            rwRETURN(*this);
        }

        // is pos in range between lower inclusive and upper inclusive
        RwBool IsInRange(Index lower, Index upper)
        {
            rwFUNCTION();
            if (upper >= lower)
            {
                rwRETURN((RwBool)((pos >= lower) && (pos <= upper)));
            }
            else
            {
                rwRETURN((RwBool)((pos >= lower) || (pos <= upper)));
            }
        }


    private:
        size_t pos;
        size_t size;
    };

    /**
    Find an index for a key.
    If the key exists in the table the correct index is returned.
    If the key doesn't exist the empty spot where it should go is returned.
    */
    Index
    FindIndexForKey(const KeyType &key) const;

    /**
    Remove an index from the table.
    Shuffle successive items into this hole if required.
    */
    void
    Remove(Index i);

    // the number of elements currently in the hash table
    size_t size;
};


// ***********************************************************************************************************
//                                                HashContainer Member Definitions
// ***********************************************************************************************************
// Note: These are currently documented in the declarations.
template <typename BufferPolicy, typename HashTraitsT>
typename HashContainer<BufferPolicy, HashTraitsT>::iterator
HashContainer<BufferPolicy, HashTraitsT>::Begin()
{
    rwFUNCTION();
    iterator iter(&this->contents[0], &this->contents[0] + Capacity());
    if (HashTraitsT::EqualTo(HashTraitsT::BadKey(), HashTraitsT::GetKey(this->contents[0])))
    {
        ++iter;
    }
    rwRETURN(iter);
}


template <typename BufferPolicy, typename HashTraitsT>
typename HashContainer<BufferPolicy, HashTraitsT>::const_iterator
HashContainer<BufferPolicy, HashTraitsT>::Begin() const
{
    rwFUNCTION();
    const_iterator iter(&this->contents[0], &this->contents[0] + Capacity());
    if (HashTraitsT::EqualTo(HashTraitsT::BadKey(), HashTraitsT::GetKey(this->contents[0])))
    {
        ++iter;
    }
    rwRETURN(iter);
}


template <typename BufferPolicy, typename HashTraitsT>
typename HashContainer<BufferPolicy, HashTraitsT>::iterator
HashContainer<BufferPolicy, HashTraitsT>::End()
{
    rwFUNCTION();
    rwRETURN(iterator(&this->contents[0] + Capacity(), &this->contents[0] + Capacity()));
}


template <typename BufferPolicy, typename HashTraitsT>
typename HashContainer<BufferPolicy, HashTraitsT>::const_iterator
HashContainer<BufferPolicy, HashTraitsT>::End() const
{
    rwFUNCTION();
    rwRETURN(const_iterator(&this->contents[0] + Capacity(), &this->contents[0] + Capacity()));
}


template<typename BufferPolicy, typename HashTraitsT>
void
HashContainer<BufferPolicy, HashTraitsT>::Insert(const Contained& obj)
{
    rwFUNCTION();
    Index i = FindIndexForKey(HashTraitsT::GetKey(obj));
    rwASSERTM(HashTraitsT::EqualTo(HashTraitsT::BadKey(), HashTraitsT::GetKey(this->contents[uint32_t(i)])), \
              ("HashContainer::Insert: Key is already in the table."));
    this->contents[uint32_t(i)] = obj;
    ++size;
#ifdef EA_DEBUG
    if ((size > Capacity()/2) && rwHashContainerAssertOnHalfFull)
    {
        // warn once that the hashed container is half full
        rwASSERTM((size < Capacity()/2) && rwHashContainerAssertOnHalfFull, \
                  ("HashTable is 50%% full. rwHashContainerAssertOnHalfFull now disabled."));
        rwHashContainerAssertOnHalfFull = FALSE;
    }
#endif
    rwRETURNVOID();
}


template<typename BufferPolicy, typename HashTraitsT>
typename HashContainer<BufferPolicy, HashTraitsT>::DatumType&
HashContainer<BufferPolicy, HashTraitsT>::operator[]
    (const typename HashContainer<BufferPolicy, HashTraitsT>::KeyType &key)
{
    rwFUNCTION();
    Index i = FindIndexForKey(key);
    if (!HashTraitsT::EqualTo(key, HashTraitsT::GetKey(this->contents[uint32_t(i)])))
    {
        // if the object wasn't already in the table create one
        HashTraitsT::GetKey(this->contents[uint32_t(i)]) = key;
        ++size;
#ifdef EA_DEBUG
        if ((size > Capacity()/2) && rwHashContainerAssertOnHalfFull)
        {
            // warn once that the hashed container is half full
            rwASSERTM((size < Capacity()/2) && rwHashContainerAssertOnHalfFull, \
                      ("HashTable is 50%% full. rwHashContainerAssertOnHalfFull now disabled."));
            rwHashContainerAssertOnHalfFull = FALSE;
        }
#endif
    }
    rwRETURN(HashTraitsT::GetData(this->contents[uint32_t(i)]));
}


template <typename BufferPolicy, typename HashTraitsT>
void
HashContainer<BufferPolicy, HashTraitsT>::Erase(const KeyType& key)
{
    rwFUNCTION();
    Index i = FindIndexForKey(key);
    Remove(i);
    rwRETURNVOID();
}


template <typename BufferPolicy, typename HashTraitsT>
void
HashContainer<BufferPolicy, HashTraitsT>::Erase(const iterator& position)
{
    rwFUNCTION();
    Index i((size_t)(position.ptr - &this->contents[0]), Capacity());
    Remove(i);
    rwRETURNVOID();
}


template<typename BufferPolicy, typename HashTraitsT>
void
HashContainer<BufferPolicy, HashTraitsT>::Clear()
{
    rwFUNCTION();
    Contained* start  = &this->contents[0];
    Contained* finish = &this->contents[0] + Capacity();
    const typename HashTraitsT::Contained defaultValue = HashTraitsT::DefaultContained();
    for (Contained* ptr = start; ptr < finish; ++ptr)
    {
        *ptr = defaultValue;
    }

    size = 0;
    rwRETURNVOID();
}



template<typename BufferPolicy, typename HashTraitsT>
void
HashContainer<BufferPolicy, HashTraitsT>::Swap(HashContainer &rhs)
{
    rwFUNCTION();
    HashContainer temp(rhs);
    rhs = *this;
    *this = temp;
    rwRETURNVOID();
}


template <typename BufferPolicy, typename HashTraitsT>
typename HashContainer<BufferPolicy, HashTraitsT>::iterator
HashContainer<BufferPolicy, HashTraitsT>::Find(const KeyType& key)
{
    rwFUNCTION();
    Index i = FindIndexForKey(key);
    if (HashTraitsT::EqualTo(key, HashTraitsT::GetKey(this->contents[uint32_t(i)])))
    {
        rwRETURN(iterator(&this->contents[uint32_t(i)], &this->contents[0] + Capacity()));
    }
    else
    {
        rwRETURN(End());
    }
}


template <typename BufferPolicy, typename HashTraitsT>
typename HashContainer<BufferPolicy, HashTraitsT>::const_iterator
HashContainer<BufferPolicy, HashTraitsT>::Find(const KeyType& key) const
{
    rwFUNCTION();
    Index i = FindIndexForKey(key);
    if (HashTraitsT::EqualTo(key, HashTraitsT::GetKey(this->contents[uint32_t(i)])))
    {
        rwRETURN(const_iterator(&this->contents[uint32_t(i)], &this->contents[0] + Capacity()));
    }
    else
    {
        rwRETURN(End());
    }
}



template<typename BufferPolicy, typename HashTraitsT>
typename HashContainer<BufferPolicy, HashTraitsT>::Index
HashContainer<BufferPolicy, HashTraitsT>::FindIndexForKey(const KeyType &key) const
{
    rwFUNCTION();

    // start at the index given by the hash value
    Index start(HashTraitsT::Hash(Capacity(), key), Capacity());
    Index i = start;

    // keep looking until we find our key, or a blank spot
    while (!HashTraitsT::EqualTo(key, HashTraitsT::GetKey(this->contents[uint32_t(i)])) &&
           !HashTraitsT::EqualTo(HashTraitsT::BadKey(), HashTraitsT::GetKey(this->contents[uint32_t(i)])))
    {
        ++i;
        rwASSERTM((i != start), ("HashTable is full."));
    };

    rwRETURN(i);
}


template<typename BufferPolicy, typename HashTraitsT>
void
HashContainer<BufferPolicy, HashTraitsT>::Remove
    (typename HashContainer<BufferPolicy, HashTraitsT>::Index i)
{
    rwFUNCTION();

    rwASSERTM(!HashTraitsT::EqualTo(HashTraitsT::BadKey(), HashTraitsT::GetKey(this->contents[uint32_t(i)])), \
              ("Removed object wasn't in hash container."));

    Index start = i;
    Index j = i;
    for(;;)
    {
        ++j;

        if (j == start)
        {
            // we have looked around the whole table, there is nothing to move to i
            break;
        }

        const KeyType& j_key = HashTraitsT::GetKey(this->contents[uint32_t(j)]);

        if (HashTraitsT::EqualTo(HashTraitsT::BadKey(), j_key))
        {
            // if we have found an empty spot, there is nothing to move to i
            break;
        }

        Index j_hash(HashTraitsT::Hash(Capacity(), j_key), Capacity());
        if (!j_hash.IsInRange(++Index(i), j))
        {
            // this item needs shuffling down;
            this->contents[uint32_t(i)] = this->contents[uint32_t(j)];
            i = j;
        }
    }

    // mark i as being removed
    this->contents[uint32_t(i)] = HashTraitsT::DefaultContained();

    --size;

    rwRETURNVOID();
}

} // namespace rw

#endif // RW_HASHCONTAINER_HPP
