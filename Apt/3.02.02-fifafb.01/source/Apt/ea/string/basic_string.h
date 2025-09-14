// -----------------------------------------------------------------------
// Copyright: (c) 2002 by Electronic Arts Canada. All ri
// -----------------------------------------------------------------------

/*! \file */

#ifndef EA_BASIC_STRING_H
#define EA_BASIC_STRING_H

#include "ea/string/default_policy.h"
#include "ea/string/meta_if.h"

#include <stddef.h>

#ifdef EA_BASIC_STRING_STD_STRING_COMPATIBLE
    #include <string>
#endif

#if defined _MSC_VER && _MSC_VER <= 1400
    #define EA_STRING_DEDUCED_TYPENAME
    #pragma warning( push )
    #pragma warning( disable : 4127 ) // conditional expression is constant
    #pragma warning( disable : 4291 ) // no matching operator delete found; memory will not be freed if initialization throws an exception
    #pragma warning( disable : 4284 ) // complaints about operator -> for built-in types;
    #pragma warning( disable : 4292 ) // terminating debug information;
#else
    #define EA_STRING_DEDUCED_TYPENAME typename
#endif

#if defined _MSC_VER && _MSC_VER <= 1200
    #define DEPENDENT_TEMPLATE
#else
    #define DEPENDENT_TEMPLATE template
#endif

// ---------------------------------------------------------------
namespace EA {
    namespace String {
        namespace Impl {

            //! \todo;
            template < bool >
            class UseCapacity
            {
            protected:
                UseCapacity( int size ) : mSize( size ), mCapacity( size ) {}

                int mSize;
                void Capacity( int c ) { mCapacity = c; }

            public:
                int  Capacity() const { return mCapacity; }

            private:
                int mCapacity;
            };

            // Specialization for string types that don't use capacity;
            template <>
            class UseCapacity< false >
            {
            protected:
                UseCapacity( int size ) : mSize( size ) {}

                int mSize;
                void Capacity( int ) {}

            public:
                int  Capacity() const { return mSize; }
            };
        }

// ---------------------------------------------------------------
/*!
    \brief Basic policy-based string class

    This highly customizable string class can be used as a factory
    for your own string types. Just write the policies (or use the
    defaults) and do a typedef to a shorter name. For example:

    \begincode
        typedef EA::String::BasicString< MyEncoding, MyPolicy > MyLocalString;
    \endcode

    \param Encoding Todo;
                    Concept requirements:

    \param Policy   This is the core string policy providing several constants and static member functions that control string behavior.
                    Concept requirements:
                                constant Policy::SMALL_STRING_SIZE                Determines the size of the small-string optimization;
                                constant Policy::ALLOW_LARGE_STRING               Determines whether or not large-strings are allowed.
                                constant Policy::USE_CAPACITY                     Determines whether or not the string type has capacity features (increases the size of the class);
                                constant Policy::ALLOW_NULL_STRING                True if you want to allow strings constructed from null-pointers.

                                constant Policy::BOUNDS_CHECKING                  True if you want to do bounds checking on all indexing operator.
                                constant Policy::USE_DEBUG_ITERATORS              If true, the string type will use debug iterators;

                                static member function: static void Policy::Copy( const C* srcBegin, const C* srcEnd, C* targetBegin );
                                                        This function should copy the elements in the range [\a srcBegin, \a srcEnd> to \a targetBegin.
                                                        Note that \a srcEnd is a past-the-end-iterator.
                                static member function: template < class T > static T* Policy::ReAlloc( T* ptr, int oldNum, int newNum );
                                                        This function should behave like the standard C realloc function, except that it allocates objects.
                                                        If \a ptr is 0, it should allocate \a newNum default instances of T and return those. If \a ptr is non-null and \a newNum
                                                        is 0, it should free the ptr. If \a ptr is non-null and \a newNum is not 0, it should make sure there is room for \a newNum
                                                        instances somewhere (with correct alignment for T) (potentially in the same place as \a ptr if the memory manager can do that)
                                                        and then copy min( \a oldSize, \a newSize ) elements to this new location. Notice that \a oldSize is not necessarily the size
                                                        of the memory pointed to by \a ptr. It may be smaller.
                                static member function: static void Policy::Assert( bool expr, const char* message )
                                                        This should handle the assert if the \a expr value is false.
                                static member function: static float Policy::GrowthFactor()
                                                        This should return the growth factor with which a capacity should grow;
*/
// ---------------------------------------------------------------
template < class Encoding_, class Policy_ = DefaultPolicy >
class BasicString : public Impl::UseCapacity< Policy_::USE_CAPACITY >
{
public:

    //! \todo
    typedef Encoding_   Encoding;

    //! \todo
    typedef Policy_     Policy;

private:

    //! \todo
    typedef typename Encoding::CharT C;

    /*!
        \brief Checks the member-function concepts the template parameters have to satisfy;

        This function is actually never called. However, it provides an early (first in the class) check for
        the member-functions the given template parameters need to provide. This helps error messages;

        \note This only applies to compiler that do aggresive template parsing so forget about MSVC anyway;
    */
    void FunctionConceptChecks()
    {
        Policy::Copy( static_cast< C* >( 0 ), static_cast< C* >( 0 ), static_cast< C* >( 0 ) );
        C* ptr = ReAlloc( static_cast< const C* >( 0 ), 0, 0 );
        Policy::Assert( true, "message" );
        float growthFactor = Policy::GrowthFactor();
    };

    /*!
        \brief Checks the constant concepts the template parameters have to satisfy;

        This enumeration defines several constants that are never actually used. However, they depend on
        constants defined within the template parameters, and hence force their existence. Because this
        is the first thing that happens in this class, it will be the first thing a compiler might fail on
        if one of the template parameters does not satisfy the concepts. Thus, providing a more understandable
        error message to the user.
    */
    enum ConstantConceptChecks
    {
        MustHave_SMALL_STRING_SIZE                = Policy::SMALL_STRING_SIZE,           //!< Make sure your Policy type defines a constant named SMALL_STRING_SIZE;
        MustHave_ALLOW_LARGE_STRING               = Policy::ALLOW_LARGE_STRING,          //!< Make sure your Policy type defines a constant named ALLOW_LARGE_SRING;
        MustHave_USE_CAPACITY                     = Policy::USE_CAPACITY,                //!< Make sure your Policy type defines a constant named USE_CAPACITY;
        MustHave_ALLOW_NULL_STRING                = Policy::ALLOW_NULL_STRING,           //!< Make sure your Policy type defines a constant named ALLOW_NULL_STRING;
        MustHave_BOUNDS_CHECKING                  = Policy::BOUNDS_CHECKING,             //!< Make sure your Policy type defines a constant named BOUNDS_CHECKING;
        MustHave_USE_DEBUG_ITERATORS              = Policy::USE_DEBUG_ITERATORS          //!< Make sure your Policy type defines a constant named USE_DEBUG_ITERATORS;
    };

    //! Simple typedef for the base-class so we don't have to type as much every time;
    typedef Impl::UseCapacity< Policy_::USE_CAPACITY > Base;

    //! \todo
    static C* ReAlloc( const C* ptr, int oldNum, int newNum )
    {
        #if defined _MSC_VER && _MSC_VER <= 1200
            return Policy::ReAlloc( ptr, oldNum, newNum );
        #else
            return Policy::DEPENDENT_TEMPLATE ReAlloc< C >( ptr, oldNum, newNum );
        #endif
    }

    static char* ReAllocRaw( const char* ptr, int oldNum, int newNum )
    {
        #if defined _MSC_VER && _MSC_VER <= 1200
            return Policy::ReAlloc( ptr, oldNum, newNum );
        #else
            return Policy::DEPENDENT_TEMPLATE ReAlloc< char >( ptr, oldNum, newNum );
        #endif
    }

public:

    // New and delete operators;
    void* operator new( size_t size )       { return ReAllocRaw( 0, 0, static_cast< int >( size ) ); }
    void* operator new []( size_t size )    { return ReAllocRaw( 0, 0, static_cast< int >( size ) ); }
    void  operator delete( void* ptr )      { ReAllocRaw( static_cast< char* >( ptr ), 0, 0 ); }
    void  operator delete [] ( void* ptr )  { ReAllocRaw( static_cast< char* >( ptr ), 0, 0 ); }

    /*!
        \brief Default constructor, makes an empty string of zero size;
    */
    BasicString() : Base( 0 ), mString( mSmallString ), mSmallString() {}

    /*!
        \brief Constructor that takes a C-style string (pointer to character), and does not allow implicit conversions;
    */
    explicit BasicString( const C* string ) : Base( 0 ), mString( mSmallString ), mSmallString()
    {
        if ( string == 0 )
        {
            if ( Policy::ALLOW_NULL_STRING )
            {
                return;
            }
            else
            {
                Policy::Assert( string != 0, "Null pointer exception" );
            }
        }
        const C* s = string;
        while ( *s++ ) ++this->mSize;

        if ( this->mSize > SMALL_STRING_SIZE )
        {
            Policy::Assert( Policy::ALLOW_LARGE_STRING, "No large string violation" );
            this->mString = ReAlloc( static_cast< const C* >( 0 ), 0, this->mSize+1 );
        }

        Policy::Copy( string, string + this->mSize, this->mString );
        mString[ this->mSize ] = C();
        this->Capacity( this->mSize );
    }

    /*!
        \brief Copy Constructor
    */
    BasicString( const BasicString& other )
        : Base( other.Size() ), mString( mSmallString ), mSmallString()
    {
        if ( this->mSize > SMALL_STRING_SIZE )
        {
            this->Capacity( other.Size() );
            Policy::Assert( Policy::ALLOW_LARGE_STRING, "No large string violation" );
            mString = ReAlloc( static_cast< const C* >( 0 ), 0, Size()+1 );
        }
        else
        {
            this->Capacity( this->mSize );
        }

        Policy::Copy( other.mString, other.mString + this->mSize, mString );
        mString[ this->mSize ] = C();
    }

    //! Constructor that takes a range and makes stuff out of it;
    template < class ForwardIterator >
    BasicString( ForwardIterator begin, const ForwardIterator& end )
        : Base( static_cast< int >( end - begin ) ), mString( mSmallString ), mSmallString()
    {
        int size = end - begin;
        if ( size > SMALL_STRING_SIZE )
        {
            this->Capacity( size );
            Policy::Assert( Policy::ALLOW_LARGE_STRING, "No large string violation" );
            mString = ReAlloc( static_cast< const C* >( 0 ), 0, size+1 );
        }
        else
        {
            this->Capacity( size );
        }

        Policy::Copy( begin, end, mString );
        mString[ this->mSize ] = C();
    }

    /*!
        \brief Reserving constructor;

        This constructor takes a size for the string, and then initializes the string to the given character.

        \param size         Length of the string (without the ending null-terminator);
        \param initializer  Default value: the null-character. This is used to initialize the string.

        Note that this constructor will actually set the 'size' to be the given value, not the capacity.
        Thus, String::End() - String::Begin() == size after this constructor;
    */
    explicit BasicString( int size, C initializer = C() ) : Base( size ), mString( mSmallString ), mSmallString()
    {
        Policy::Assert( size >= 0, "Invalid size to reserve a string with" );

        if ( this->mSize > SMALL_STRING_SIZE )
        {
            Policy::Assert( Policy::ALLOW_LARGE_STRING, "No large string violation" );
            mString = ReAlloc( static_cast< const C* >( 0 ), 0, this->mSize+1 );
        }

        if ( this->mSize > 0 )
            for ( int i = 0; i < this->mSize; ++i )
                mString[ i ] = initializer;

        mString[ this->mSize ] = C();

        this->Capacity( this->mSize );
    }

    /*!
        \brief Templatized 'copy constructor' that allows us to copy construct from strings that have a different policy.

        Unfortunately, this is not supported on MSVC6;
    */
    #if !defined _MSC_VER || _MSC_VER > 1200
        template < class PolicyRhs >
        BasicString( const BasicString< Encoding, PolicyRhs > & other )
            : Base( other.Size() ), mString( mSmallString ), mSmallString()
        {
            if ( this->mSize <= SMALL_STRING_SIZE )
            {
                mString = mSmallString;
                this->Capacity( this->mSize );
            }
            else
            {
                this->Capacity( other.Capacity() );
                Policy::Assert( Policy::ALLOW_LARGE_STRING, "No large string violation" );
                mString = ReAlloc( static_cast< const C* >( 0 ), 0, this->Capacity()+1 );
            }

            Policy::Copy( other.ConstRawPtr(), other.ConstRawPtr() + this->mSize, mString );
            mString[ this->mSize ] = C();
        }
    #endif

    // This part is only included if we want to be std::string compatible;
    #ifdef EA_BASIC_STRING_STD_STRING_COMPATIBLE

        /*!
            \brief Constructs a BasicString out of an std::basic_string
        */
        template < class CharTraits, class Allocator >
        BasicString( const std::basic_string< C, CharTraits, Allocator >& str ) : Base( static_cast< int >( str.size() ) ), mSmallString, mSmallString()
        {
            if ( this->mSize <= SMALL_STRING_SIZE )
                mString = mSmallString;
            else
            {
                Policy::Assert( Policy::ALLOW_LARGE_STRING, "No large string violation" );
                mString = ReAlloc( static_cast< const C* >( 0 ), 0, this->mSize+1 );
            }

            Policy::Copy( str.begin(), str.end(), mString );
            mString[ this->mSize ] = C();
            this->Capacity( this->mSize );
        }

        /*!
            \brief Constructs an std::basic_string from this;
        */
        template < class CharTraits, class Allocator >
        std::basic_string< C, CharTraits, Allocator > ToStdString() const
        {
            return std::basic_string< C, CharTraits, Allocator >( mString );
        }
    #endif

    /*!
        \brief Assignment operator;

        We don't define the assignment op for MSVC 6.0 because the templatized copy ctor takes care of it (crappy compiler);
    */
    BasicString& operator = ( const BasicString& rhs )
    {
        BasicString tmp( rhs );
        Swap( tmp );
        return *this;
    }

    //! Assigment operator that takes a raw-character pointer;
    BasicString& operator = ( const C* rhs )
    {
        BasicString tmp( rhs );
        Swap( tmp );
        return *this;
    }

    /*
        \brief Templatized assignment operator that allows us to assign from strings that have
                different policies.

        Unfortunately, this is not supported on MSVC 6.0
    */
    #if !defined _MSC_VER || _MSC_VER > 1200
        template < class PolicyRhs >
        BasicString& operator = ( const BasicString< Encoding, PolicyRhs >& rhs )
        {
            BasicString tmp( rhs );
            Swap( tmp );
            return *this;
        }
    #endif

    /*!
        \brief Destructor;
    */
    ~BasicString()
    {
        if ( !DoingSmallStringOptimization() )
        {
            ReAlloc( mString, 0, 0 );
        }
    }

    /*!
        \brief String concatenation

        Templatized so we can concatenate from strings with different policies too.
    */
    template < class PolicyRhs >
    BasicString& operator += ( const BasicString< Encoding, PolicyRhs >& rhs )
    {
        Insert( rhs.Begin(), rhs.End(), End() );
        return *this;
    }

    //! \todo;
    BasicString& operator += ( const C* rhs )
    {
        const C* end = rhs; while ( *end ) ++end;
        Insert( rhs, end, End() );
        return *this;
    }

    //! \todo;
    BasicString& operator += ( const C rhs )
    {
        PushBack( rhs );
        return *this;
    }

    /*!
        \brief Returns the length of the string (without the ending null-terminator);

        \warning Depending on the encoding scheme used by this string, this value may not be the
                    same as the actual number of characters in the string. Only for fixed-size encodings
                    like Ucs2 and Ascii, where each element 'C' corresponds with exactly one character
                    is the Size equal to the string length. For a Utf8 (for example) encoding however, the size
                    might not correspond to the length of the string;
    */
    int Size() const { return this->mSize; }

    /*!
        \brief Releases the string data and returns it in a character pointer;

        The callee becomes the owner. Make sure that the callee
        deallocates using the same allocator as this string uses;

        This string itself becomes empty;
    */
    C* Release()
    {
        if ( DoingSmallStringOptimization() )
        {
            C* s = ReAlloc( static_cast< const C* >( 0 ), 0, this->mSize+1 );
            Policy::Copy( mSmallString, mSmallString + this->mSize, s );
            s[ this->mSize ] = C();
            return s;
        }
        else
        {
            C* s = mString;
            mString = mSmallString;
            this->mSize = 0;
            return s;
        }
    }

    //! \todo;
    void Swap( BasicString& other )
    {
        int size = other.mSize; other.mSize = this->mSize; this->mSize = size;
        int capa = other.Capacity(); other.Capacity( this->Capacity() ); this->Capacity( capa );

        bool doingSmallString = DoingSmallStringOptimization();
        bool otherDoingSmallString = other.DoingSmallStringOptimization();

        C* otherString = other.mString;
        C* string = mString;

        if ( otherDoingSmallString ) mString = mSmallString; else mString = otherString;
        if ( doingSmallString ) other.mString = other.mSmallString; else other.mString = string;
        if ( otherDoingSmallString || doingSmallString )
        {
            C s[ SMALL_STRING_SIZE + 1 ];
            s[ SMALL_STRING_SIZE ] = C();
            Policy::Copy( mSmallString, mSmallString + SMALL_STRING_SIZE + 1, s );
            Policy::Copy( other.mSmallString, other.mSmallString + SMALL_STRING_SIZE + 1, mSmallString );
            Policy::Copy( s, s + SMALL_STRING_SIZE + 1, other.mSmallString );
        }
    }

// More public follows after this private section;
private:

    #if !defined _MSC_VER || _MSC_VER > 1200 // Debug iterators are not supported on MSVC 6;

        /*!
            \brief Base class for iterators.
            \internal

            \warning This class should NOT be used polymorphically. It merely acts as a place to put
                        common code shared by actual iterators. This base uses Copliens Curiously Recurring
                        Template Patterns (CRTP) to make sure operators return actual (derived) classes;

                \param DbgIterC     Underlying pointer type
                \param DerivedIterator Derived type (as per CRTP)
        */
        template < class DbgIterC, class DerivedIterator >
        class DbgIteratorBase
        {
        public:
            typedef BasicString< Encoding, Policy > StringType;
            typedef C                               CharT;

            const DbgIterC* operator -> () const { this->Validate(); return  mPtr; }
            const DbgIterC& operator *  () const { this->Validate(); return *mPtr; }

            DerivedIterator& operator ++ ()      { ++mPtr; return static_cast< DerivedIterator& >( *this ); }
            DerivedIterator& operator -- ()      { --mPtr; return static_cast< DerivedIterator& >( *this ); }
            DerivedIterator  operator ++ ( int ) { DerivedIterator tmp( static_cast< DerivedIterator& >( *this ) ); ++mPtr; return tmp; }
            DerivedIterator  operator -- ( int ) { DerivedIterator tmp( static_cast< DerivedIterator& >( *this ) ); --mPtr; return tmp; }

            DerivedIterator& operator += ( int delta ) { mPtr += delta; return static_cast< DerivedIterator& >( *this ); }
            DerivedIterator& operator -= ( int delta ) { mPtr -= delta; return static_cast< DerivedIterator& >( *this ); }

            bool operator == ( const DerivedIterator& other ) const { AssertSameRange( other ); return mPtr == other.mPtr; }
            bool operator != ( const DerivedIterator& other ) const { AssertSameRange( other ); return mPtr != other.mPtr; }

            // Todo: make non-members properly;
            DerivedIterator operator + ( int delta ) const { return DerivedIterator( mPtr + delta, mBegin, mEnd ); }
            DerivedIterator operator - ( int delta ) const { return DerivedIterator( mPtr - delta, mBegin, mEnd ); }

            int operator - ( const DbgIteratorBase& other ) const { AssertSameRange( other ); return static_cast< int >( mPtr - other.mPtr ); }

        public:

            // For internal (and debugging) purposes only;

            const DbgIterC* ConstRawPtr() const { return mPtr; }
            const DbgIterC* ConstBegin()  const { return mBegin; }
            const DbgIterC* ConstEnd()    const { return mEnd; }

            void Validate() const { Policy::Assert( (mPtr >= mBegin) && (mPtr < mEnd), "Trying to dereference an invalid iterator" );  }
            void AssertSameRange( const DbgIteratorBase& other ) const { Policy::Assert( mBegin == other.mBegin && mEnd == other.mEnd, "Iterators are not in same range" ); }

        protected:

            // Only derived classes can construct this;
            DbgIteratorBase( DbgIterC* ptr, DbgIterC* begin, DbgIterC* end ) : mPtr( ptr ), mBegin( begin ), mEnd( end ) {}

            DbgIterC* mPtr;
            DbgIterC* mBegin;
            DbgIterC* mEnd;
        };

    public:

        /*!
            \brief This implements the debug iterator for non-const characters;
        */
        struct DbgIterator : public DbgIteratorBase< C, DbgIterator >
        {
            typedef typename ::EA::String::BasicString< Encoding, Policy >::template DbgIteratorBase< C, DbgIterator > Base;

            DbgIterator() : Base( 0, 0, 0 ) {}
            DbgIterator( C* ptr, C* begin, C* end ) : Base( ptr, begin, end ) {}

            C* operator -> () const { this->Validate(); return  this->mPtr; }
            C& operator *  () const { this->Validate(); return *this->mPtr; }
        };

        /*!
            \brief This implements the debug iterator for const characters;
        */
        struct ConstDbgIterator : public DbgIteratorBase< const C, ConstDbgIterator >
        {
            typedef typename ::EA::String::BasicString< Encoding, Policy >::template DbgIteratorBase< const C, ConstDbgIterator > Base;

            ConstDbgIterator() : Base( 0, 0, 0 ) {}
            ConstDbgIterator( const DbgIterator& iter ) : Base( iter.ConstRawPtr(), iter.ConstBegin(), iter.ConstEnd() ) {} //!< non-explicit on purpose;
            ConstDbgIterator( const C* ptr, const C* begin, const C* end ) : Base( ptr, begin, end ) {}
        };

        /*!
            \brief This factory class typedef's the underlying iterator types used by
                    the client  and provides routines to create such iterators.

                    This version of the factory makes debug iterators.
        */
        struct DbgIteratorFactory
        {
            typedef ConstDbgIterator ConstIterator;
            typedef DbgIterator      Iterator;

            DbgIteratorFactory( C* begin, C* end ) : mBegin( begin ), mEnd( end ) {}
            Iterator Begin() { return DbgIterator( mBegin, mBegin, mEnd ); }
            Iterator End()   { return DbgIterator( mEnd, mBegin, mEnd ); }
            ConstIterator Begin() const { return ConstDbgIterator( mBegin, mBegin, mEnd ); }
            ConstIterator End()   const { return ConstDbgIterator( mEnd, mBegin, mEnd ); }
        private:
            C* mBegin;
            C* mEnd;
        };

    #endif

public:


    /*!
        \brief Another version of an \a IteratorFactory that
                makes non-debug versions of the iterators. In other words
                it makes regular pointers.
    */
    struct RawPtrIteratorFactory
    {
        typedef const C* ConstIterator;
        typedef C* Iterator;

        RawPtrIteratorFactory( C* begin, C* end ) : mBegin( begin ), mEnd( end ) {}
        ConstIterator Begin() const { return mBegin; }
        ConstIterator End() const { return mEnd; }
        Iterator Begin() { return mBegin; }
        Iterator End()   { return mEnd; }
    private:
        C* mBegin;
        C* mEnd;
    };

    //! Define the actual iterator factory used;
    #if !defined _MSC_VER || _MSC_VER > 1200 // Debug iterators are not supported on MSVC 6;
        typedef typename EA::Meta::If< Policy::USE_DEBUG_ITERATORS, DbgIteratorFactory, RawPtrIteratorFactory >::type IteratorFactory;
    #else
        typedef RawPtrIteratorFactory IteratorFactory;
    #endif

    //! Defines the type that is used for const iterators;
    typedef typename IteratorFactory::ConstIterator ConstIterator;

    //! Defines the type that is used for iterators;
    typedef typename IteratorFactory::Iterator Iterator;

    //! Returns a non-const iterator to the beginning of the string;
    Iterator Begin() { return IteratorFactory( mString, mString + this->mSize ).Begin(); }

    //! Returns a non-const iterator to the end of the string (a past-the-last-element iterator);
    Iterator End() { return IteratorFactory( mString, mString + this->mSize ).End(); }

    //! Returns a const iterator to the beginning of the string;
    ConstIterator Begin() const { const IteratorFactory factory( mString, mString + this->mSize ); return factory.Begin(); }

    //! Returns a const iterator to the end of the string (a past-the-last-element iterator);
    ConstIterator End() const { const IteratorFactory factory( mString, mString + this->mSize ); return factory.End(); }

    /*!
        \brief Returns a const pointer to the array of character data.
        The string stays owner ofcourse.
    */
    const C* ConstRawPtr() const { return mString; }

    //! Returns a non const pointer into the string;
    C* RawPtr() { return mString; }

    /*!
        \brief Const indexing operator;
    */
    const C& operator [] ( int i ) const
    {
        if ( Policy::BOUNDS_CHECKING )
        {
            Policy::Assert( i >= 0,    "BasicString index out of bounds" );
            Policy::Assert( i < this->mSize, "BasicString index out of bounds" );
        }

        return mString[ i ];
    }

    /*!
        \brief Non-const indexing operator;
    */
    C& operator [] ( int i )
    {
        if ( Policy::BOUNDS_CHECKING )
        {
            Policy::Assert( i >= 0,    "BasicString index out of bounds" );
            Policy::Assert( i < this->mSize, "BasicString index out of bounds" );
        }

        return mString[ i ];
    }

    /*!
        \brief Erases the part of the string from begin to end;
    */
    Iterator Erase( const Iterator& begin, const Iterator& end )
    {
        int distance = static_cast< int >( begin - Begin() );
        Policy::Copy( end, End(), begin );
        for ( int i = this->mSize - static_cast< int >( end - begin ); i < this->mSize; ++i )
            mString[ i ] = C();
        this->mSize = this->mSize - static_cast< int >( end - begin );
        return Begin() + distance;
    }

    //! Clears the entire vector;
    void Clear() { Erase( Begin(), End() ); }

    //! Resizes the string to the given number of elements;
    //! \todo: currently not a very efficient implementation, mwiahuahuuahuaha;
    void Resize( int size )
    {
        Policy::Assert( size >= 0, "invalid size" );
        if ( size > this->mSize )
        {
            int growth = size - this->mSize;
            const int randomNumber = 128;
            C array[ randomNumber ] = { C() };
            while ( growth > randomNumber )
            {
                Insert( static_cast< C* >( array ), array + randomNumber, End() );
                growth -= randomNumber;
            }
            Insert( static_cast< C* >( array ), array + growth, End() );
        }
        else
        {
            this->mSize = size;
            mString[ this->mSize ] = C();
        }
    }

    //! Increases the capacity, but not the size;
    void Reserve( int capacity )
    {
        if ( capacity <= this->Capacity() ) return;
        else
        {
            if ( capacity <= Policy::SMALL_STRING_SIZE )
            {
                this->Capacity( capacity );
            }
            else
            {
                C* string = ReAlloc( static_cast< const C* >( 0 ), 0, capacity+1 );
                Policy::Copy( Begin(), End(), string );
                this->Capacity( capacity );
                if ( ! DoingSmallStringOptimization() )
                    ReAlloc( mString, 0, 0 );
                mString = string;
                mString[ this->mSize ] = C();
            }
        }
    }

    //! Adds an element to the string;
    void PushBack( const C& c )
    {
        Insert( &c, &c + 1, End() );
    }

    /*!
        \brief Inserts the given range into the given point
    */
    template < class SrcConstIterator >
    void Insert( const SrcConstIterator& begin, const SrcConstIterator& end, const Iterator& at )
    {
        int distance = static_cast< int >( end - begin );
        Policy::Assert( distance >= 0, "Can't pass negative ranges, c&b." );
        if ( distance == 0 ) return;
        int newSize = this->mSize + distance;
        if ( newSize < this->Capacity() )
        {
            if ( at == End() )
            {
                C* currentEnd = mString + this->mSize;
                Policy::Copy( begin, end, currentEnd );
                mString[ newSize ] = C();
            }
            else
            {
                Policy::Move( at, End(), mString + (at - Begin()) + distance );
                Policy::Copy( begin, end, at );
                mString[ newSize ] = C();
            }

            this->mSize = newSize;
        }
        else
        {
            int newCapacity = static_cast< int >( static_cast< float >( this->Capacity() ) * Policy::GrowthFactor() );
            if ( newCapacity < newSize ) newCapacity = newSize;

            int insertPos = static_cast< int >( at - Begin() );
            Reserve( newCapacity ); // This might invalidate the at iterator, hence the insertPos;
            Insert( begin, end, Begin() + insertPos );
        }
    }

    template < bool > struct BoolToType {};

    /*!
        \brief Conversion operator, converts to a different encoding;
    */
    template < class BasicStringOther >
    BasicStringOther To( const BasicStringOther* deductionHelper = 0 ) const
    {
        BoolToType< Meta::IsSame< Encoding, typename BasicStringOther::Encoding >::value > overloadResolver;
        return To< BasicStringOther >( deductionHelper, overloadResolver );
    }

    template < class BasicStringOther >
    BasicStringOther To( const BasicStringOther* deductionHelper, BoolToType< true > ) const
    {
        return BasicStringOther( *this );
    }

    template < class BasicStringOther >
    BasicStringOther To( const BasicStringOther* deductionHelper, BoolToType< false > ) const
    {
        ConstIterator srcIter = Begin();
        ConstIterator srcEnd  = End();
        int dstSize = Size();
        if ( ! Encoding::SizeIsLength() || ! BasicStringOther::Encoding::SizeIsLength() )
        {
            ConstIterator i = srcIter;
            dstSize = 0;
            while ( i != srcEnd )
            {
                dstSize += BasicStringOther::Encoding::NumCharactersFor( Encoding::ToUcs4( i ) );
            }
        }

        BasicStringOther ret( dstSize );
        typename BasicStringOther::Iterator dstIter = ret.Begin();
        while ( srcIter != srcEnd )
            BasicStringOther::Encoding::FromUcs4( dstIter, Encoding::ToUcs4( srcIter ) );
        *dstIter = C();
        return ret;
    }

private:

    //! Local constant, so we won't have to type the whole thing (also avoids an MSVC problem);
    enum { SMALL_STRING_SIZE = Policy::SMALL_STRING_SIZE };

    //! Returns true if the string is currently stored inside the small-string;
    bool DoingSmallStringOptimization() const { return mString == mSmallString; }

    // Attributes;
    typename Encoding::CharT* mString; //!< Actual string;
    C   mSmallString[ SMALL_STRING_SIZE + 1 ]; //!< Storage for the small string optimization;
};

/*!
    \brief String concatation operator.

    This is templatized so we can concatenate strings of different policies;

    \warning The return value will have the same policy as the left-hand-side string.
                This is a somewhat arbitrary choice, but since string concatenation is not
                commutative anyway, and since we have implicit conversions from one policy
                to another, we don't care so much.
*/
template < class Encoding, class PolicyLhs, class PolicyRhs >
BasicString< Encoding, PolicyLhs >
operator + ( const BasicString< Encoding, PolicyLhs >& lhs,
                const BasicString< Encoding, PolicyRhs >& rhs )
{
    BasicString< Encoding, PolicyLhs > tmp( lhs );
    return tmp += rhs;
}

//! \todo;
template < class Encoding, class PolicyLhs >
BasicString< Encoding, PolicyLhs >
operator + ( const BasicString< Encoding, PolicyLhs >& lhs,
                const typename Encoding::CharT* rhs )
{
    BasicString< Encoding, PolicyLhs > tmp( lhs );
    BasicString< Encoding, PolicyLhs > s( rhs ); //!< \todo optimize;
    return tmp += s;
}

//! \todo;
template < class Encoding, class PolicyRhs >
BasicString< Encoding, PolicyRhs >
operator + ( const typename Encoding::CharT* lhs,
                const BasicString< Encoding, PolicyRhs >& rhs )
{
    BasicString< Encoding, PolicyRhs > tmp( lhs );
    return tmp += rhs;
}

/*!
    \brief String comparison operator;
*/
template < class Encoding, class PolicyLhs, class PolicyRhs >
bool operator == ( const BasicString< Encoding, PolicyLhs >& lhs,
                    const BasicString< Encoding, PolicyRhs >& rhs )
{
    bool equal = lhs.Size() == rhs.Size();
    int i = 0;
    while ( equal && (i < lhs.Size()) )
    {
        equal = lhs[ i ] == rhs[ i ];
        ++i;
    }
    return equal;
}

/*!
    \brief String comparison operator that takes just a C ptr;
    \warning make sure the rhs is null-terminated;
*/
template < class Encoding, class PolicyLhs >
bool operator == ( const BasicString< Encoding, PolicyLhs >& lhs, const typename Encoding::CharT* rhs )
{
    int i = 0;
    while ( (i < lhs.Size()) && (*rhs != EA_STRING_DEDUCED_TYPENAME Encoding::CharT()) )
    {
        if ( lhs[ i ] != *rhs ) return false;
        ++i;
        ++rhs;
    }
    if ( (i == lhs.Size()) && (*rhs == EA_STRING_DEDUCED_TYPENAME Encoding::CharT()) )
        return true;
    else
        return false;
}

/*!
    \brief Swapped version of the above version;
    \warning make sure the lhs is null-terminated;
*/
template < class Encoding, class PolicyRhs >
bool operator == ( const typename Encoding::CharT* lhs, const BasicString< Encoding, PolicyRhs >& rhs )
{
    return rhs == lhs;
}

/*!
    \brief String not-equal operator
*/
template < class Encoding, class PolicyLhs, class PolicyRhs >
bool operator != ( const BasicString< Encoding, PolicyLhs >& lhs,
                    const BasicString< Encoding, PolicyRhs >& rhs )
{
    return !(lhs == rhs);
}

/*!
    \brief String not-equal operator that takes a C ptr;
    \warning make sure the lhs is null-terminated;
*/
template < class Encoding, class PolicyRhs >
bool operator != ( const typename Encoding::CharT* lhs, const BasicString< Encoding, PolicyRhs >& rhs )
{
    return !(rhs == lhs);
}

/*!
    \brief String not-equal operator that takes a C ptr;
    \warning make sure the rhs is null-terminated;
*/
template < class Encoding, class PolicyLhs >
bool operator != ( const BasicString< Encoding, PolicyLhs >& lhs, const typename Encoding::CharT* rhs )
{
    return !(lhs == rhs);
}

//! Lexical less-than operator
template < class Encoding, class PolicyLhs, class PolicyRhs >
bool operator < ( const BasicString< Encoding, PolicyLhs >& lhs,
                    const BasicString< Encoding, PolicyRhs >& rhs )
{
    if ( lhs.Size() == 0 )
    {
        if ( rhs.Size() == 0 )
        {
            return false;
        }
        else
        {
            return true;
        }
    }

    int i = 0;
    for ( ; (i < lhs.Size()) && (i < rhs.Size()); ++i )
    {
        if  ( lhs[ i ] < rhs[ i ] )
            return true;
        else
            if ( lhs[ i ] > rhs[ i ] )
                return false;
    }
    if ( i == lhs.Size() && i != rhs.Size() ) return true;
    return false;
}

//! Lexical greater-than-or-equals operator
template < class Encoding, class PolicyLhs, class PolicyRhs >
bool operator >= ( const BasicString< Encoding, PolicyLhs >& lhs,
                    const BasicString< Encoding, PolicyRhs >& rhs )
{
    return !(lhs < rhs);
}

//! Lexical greater-than operator
template < class Encoding, class PolicyLhs, class PolicyRhs >
bool operator > ( const BasicString< Encoding, PolicyLhs >& lhs,
                    const BasicString< Encoding, PolicyRhs >& rhs )
{
    return (rhs < lhs);
}

//! Lexical less-than-or-equals operator
template < class Encoding, class PolicyLhs, class PolicyRhs >
bool operator <= ( const BasicString< Encoding, PolicyLhs >& lhs,
                    const BasicString< Encoding, PolicyRhs >& rhs )
{
    return rhs >= lhs;
}

// -----------------------------------------------------------------------

    }
}

#undef EA_STRING_DEDUCED_TYPENAME

#if defined _MSC_VER && _MSC_VER <= 1300
    #pragma warning( pop )
#endif

#endif
