/*H************************************************************************************************/
/*!

    \File
        _AptVector.h

    \Description
        Simple description of main purpose of file.

    \Notes
        None.

    \Copyright
        (c) 2004 Electronic Arts Inc.

    \Version    1.0        03/07/04 (ONA) First Version
    \Version    1.1        11/01/04 (DC)  Changed to use allocation Macro to remove #if defines
                                          everywhere. There was not change to the logic.


*/
/************************************************************************************************H*/

#ifndef ___APTVECTOR_H__
#define ___APTVECTOR_H__

#include <stddef.h>
#include "ea/string/basic_string.h"
#include "ea/string/meta_if.h"

namespace EA
{
    namespace Meta
    {
        template < class T > struct IsFloat         { enum { value = false }; };
        template <>          struct IsFloat< float >{ enum { value = true  }; };
        template < class T > struct IsInteger       { enum { value = false }; };
        template <>          struct IsInteger< int >{ enum { value = true  }; };
        template < class T > struct IsVoid          { enum { value = false }; };
        template <>          struct IsVoid<   void >{ enum { value = true  }; };
        template < class T > struct IsChar          { enum { value = false }; };
        template <>          struct IsChar<   char >{ enum { value = true  }; };

        template < bool T > struct BoolToType { enum { value = T }; };

        template < int  T > struct IntToType {  enum { value = T }; };

        template < class T > struct IsBuiltIn                    { enum { value = false }; };
        template <>          struct IsBuiltIn <          int   > { enum { value = true  }; };
        template <>          struct IsBuiltIn < unsigned int   > { enum { value = true  }; };
        template <>          struct IsBuiltIn <          char  > { enum { value = true  }; };
        template <>          struct IsBuiltIn < unsigned char  > { enum { value = true  }; };
        template <>          struct IsBuiltIn < signed   char  > { enum { value = true  }; };
        template <>          struct IsBuiltIn <          short > { enum { value = true  }; };
        template <>          struct IsBuiltIn < unsigned short > { enum { value = true  }; };
        template <>          struct IsBuiltIn <          long  > { enum { value = true  }; };
        template <>          struct IsBuiltIn < unsigned long  > { enum { value = true  }; };
        template <>          struct IsBuiltIn < bool           > { enum { value = true  }; };
        template <>          struct IsBuiltIn < float          > { enum { value = true  }; };
        template <>          struct IsBuiltIn < double         > { enum { value = true  }; };
    }
}



#if defined _MSC_VER && _MSC_VER <= 1400
    #pragma warning( push )
    #pragma warning( disable: 4291 )
#endif

// -----------------------------------------------------------------------
template < class T >
struct StringAsVectorEncoding
{
    typedef T CharT;
};

// -----------------------------------------------------------------------
struct StringAsVectorPolicy : public EA::String::DefaultPolicy
{
    enum
    {
        SMALL_STRING_SIZE                = 1,
        ALLOW_LARGE_STRING               = true,
        USE_CAPACITY                     = true,
        ALLOW_IMPLICIT_CONVERSION        = false,
        ALLOW_NULL_STRING                = false,

        #if defined(EA_RELEASE_DEBUGGABLE)
            BOUNDS_CHECKING                  = false,
            USE_DEBUG_ITERATORS              = false
        #else
            BOUNDS_CHECKING                  = true,
            USE_DEBUG_ITERATORS              = true
        #endif
    };

    static float GrowthFactor()
    {
        return 2.0f;
    }

    template < class T >
    static T* ReAlloc( const T* ptr, int oldNum, int newNum )
    {
        T* ret = 0;

        if ( !ptr )
        {
            if ( EA::Meta::IsBuiltIn< T >::value )
            {
                return APT_MALLOC_ARRAY( T, newNum);
            }
            else
            {
                return new T[ newNum ];
            }
        }

        if ( newNum )
        {
            if ( EA::Meta::IsBuiltIn< T >::value )
            {
                ret = APT_MALLOC_ARRAY( T, newNum);
            }
            else
            {
                ret = new T[ newNum ];
            }

            int numElem = newNum < oldNum ? newNum : oldNum;
            T* dst = ret;
            while ( numElem-- )
                *dst++ = *ptr++;
        }

        if ( EA::Meta::IsBuiltIn< T >::value )
        {
            APT_FREE_ARRAY( ptr, T, oldNum);
        }
        else
        {
            delete [] ptr;
        }

        return ret;
    }

    template < class ForwardIterator, class OutputIterator >
    static OutputIterator Copy( ForwardIterator srcBegin, ForwardIterator srcEnd, OutputIterator dstBegin )
    {
        while ( srcBegin != srcEnd )
        {
            *dstBegin++ = *srcBegin++;
        }
        return dstBegin;
    }

    static char* Copy( const char* srcBegin, const char* srcEnd, char* dstBegin )
    {
        memcpy( dstBegin, srcBegin, static_cast< int >( srcEnd - srcBegin ) );
        return dstBegin + (srcEnd - srcBegin);
    }
};

// -----------------------------------------------------------------------
template < class T >
struct VectorFactory
{
    typedef EA::String::BasicString< StringAsVectorEncoding< T >, StringAsVectorPolicy > type;
};

#if defined _MSC_VER && _MSC_VER <= 1400
    #pragma warning( pop )
#endif

#endif  //  ___APTVECTOR_H__

/*H*************************************************************************************************
    EOF
*************************************************************************************************H*/

