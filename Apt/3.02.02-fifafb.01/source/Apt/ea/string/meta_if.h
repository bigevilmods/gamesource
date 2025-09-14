// -----------------------------------------------------------------------
// Copyright: (c) 2002 by Electronic Arts Canada. All ri
// -----------------------------------------------------------------------

/*! \file */

#ifndef EA_META_IF_H
#define EA_META_IF_H

namespace EA
{
    namespace Meta
    {
        #if defined  _MSC_VER && _MSC_VER <= 1300
            namespace WorkAround
            {
                template < bool Condition >
                struct IfImpl
                {
                    template < class A, class B >
                    struct Clauses
                    {
                        typedef A type;
                    };
                };

                template <>
                struct IfImpl< false >
                {
                    template < class A, class B >
                    struct Clauses
                    {
                        typedef B type;
                    };
                };
            } // Namespace WorkAround

            template < bool Condition, class A, class B >
            struct If
            {
                typedef typename WorkAround::IfImpl< Condition >::template Clauses< A, B >::type type;
            };
        #else
            template < bool Condition, class A, class B >
            struct If
            {
                typedef A type;
            };

            template < class A, class B >
            struct If< false, A, B >
            {
                typedef B type;
            };
        #endif

        // ------------------------------------------------------------
        namespace Impl
        {
            class TrueType { char m; };
            class FalseType { TrueType m[ 2 ]; };

            template < class T >
                TrueType IsSameTester( T*, T* );

            FalseType IsSameTester( ... );
        }

        // ------------------------------------------------------------
        template < class A, class B >
        struct IsSame
        {
            enum
            {
                value = sizeof( Impl::IsSameTester( (A*)0, (B*)0 ) ) == sizeof( Impl::TrueType )
            };
        };
    }
}

#endif
