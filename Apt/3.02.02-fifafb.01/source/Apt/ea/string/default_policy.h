//   -----------------------------------------------------------------------
// Copyright: (c) 2002 by Electronic Arts Canada. All ri
// -----------------------------------------------------------------------

/*! \file */

#ifndef EA_STRING_DEFAULT_POLICY_H
#define EA_STRING_DEFAULT_POLICY_H

#include <string.h>
#include <assert.h>

namespace EA
{
    namespace String
    {
        struct DefaultPolicy
        {
            enum
            {
                SMALL_STRING_SIZE                = 32,
                ALLOW_LARGE_STRING               = true,
                USE_CAPACITY                     = true,
                ALLOW_NULL_STRING                = true,

                #if defined(EA_DEBUG)
                    BOUNDS_CHECKING                  = false,
                    USE_DEBUG_ITERATORS              = false
                #else
                    BOUNDS_CHECKING                  = false,
                    USE_DEBUG_ITERATORS              = false
                #endif
            };

            static float GrowthFactor()
            {
                return 2.0f;
            }

            template < class T >
            static T* ReAlloc( const T* ptr, int oldNum, int newNum )
            {
                if ( !ptr ) return new T[ newNum ];
                T* ret = 0;
                if ( newNum )
                {
                    ret = new T[ newNum ];
                    int numElem = (newNum < oldNum ? newNum : oldNum) * sizeof( T );
                    T* dst = ret;
                    while ( numElem-- ) *dst++ = *ptr++;
                }
                delete [] const_cast< T* >( ptr );
                return ret;
            }

            #if defined(EA_DEBUG)
                static void Assert( bool expression, const char* message )
                {
                    (void)expression;
                    (void)message;
                    assert( expression && message );
                }
            #else
                static void Assert( bool, const char* ) {}
            #endif

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

            static wchar_t* Copy( const wchar_t* srcBegin, const wchar_t* srcEnd, wchar_t* dstBegin )
            {
                memcpy( dstBegin, srcBegin, static_cast< int >( srcEnd - srcBegin ) * sizeof( short ) );
                return dstBegin + (srcEnd - srcBegin);
            }

            template < class ForwardIterator, class OutputIterator >
            static OutputIterator Move( ForwardIterator srcBegin, ForwardIterator srcEnd, OutputIterator dstBegin )
            {
                int distance = static_cast< int >( srcEnd - srcBegin );
                dstBegin += distance;
                --dstBegin;
                --srcEnd;
                while ( distance-- )
                {
                    *dstBegin = *srcEnd;
                    --srcEnd;
                    --dstBegin;
                }
                return dstBegin;
            }

            static char* Move( const char* srcBegin, const char* srcEnd, char* dstBegin )
            {
                memmove( dstBegin, srcBegin, static_cast< int >( srcEnd - srcBegin ) );
                return dstBegin + (srcEnd - srcBegin);
            }
        };
    }
}

#endif
