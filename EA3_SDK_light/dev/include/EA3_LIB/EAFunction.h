// (c) 1998-2010 Electronic Arts Inc.

#ifndef EAFunction_h
#define EAFunction_h

#include <eastl/functional.h>

template< class First, class Second >
class EAFirst : public eastl::binary_function< First, Second, First > {

    // ABSTRACT //
    // projection function object
    //
    // SUMMARY //
    // #include <EAFunction.h>
    //
    // DESCRIPTION //
    // This is a function object whose operator() returns its first
    // argument, i.e. a function First x Second -> First which performs
    // the projection (x,y) -> x.
    //
    // SEE ALSO //
    // EASecond
    ///

public:
    // OPERATORS //
    // returns first argument
    result_type operator()( first_argument_type first,
        second_argument_type ) const
    {
        return first;
    }

    // END ABSTRACT //
    ///
};

template< class First, class Second >
class EASecond : public eastl::binary_function< First, Second, Second > {

    // ABSTRACT //
    // projection function object
    //
    // SUMMARY //
    // #include <EAFunction.h>
    //
    // DESCRIPTION //
    // This is a function object whose operator() returns its second
    // argument, i.e. a function First x Second -> Second which performs
    // the projection (x,y) -> y.
    //
    // SEE ALSO //
    // EAFirst
    ///

public:
    // OPERATORS //
    // returns first argument
    result_type operator()( first_argument_type,
        second_argument_type second ) const
    {
        return second;
    }

    // END ABSTRACT //
    ///
};

template< class T >
class EAIdentity : public eastl::unary_function< T, T > {

    // ABSTRACT //
    // unary identity function object
    //
    // SUMMARY //
    // #include <EAFunction.h>
    //
    // DESCRIPTION //
    // EAIdentity<T>() is an identity function from T -> T.
    //
    // SEE ALSO //
    // EAFunction
    ///

public:
    // OPERATORS //
    result_type operator()( const argument_type& t ) const
    {
        return t;
    }

    // END ABSTRACT //
    ///
};


// "class X" is bogus, because Micro-big-soft big-ass compiler won;t work without it.
template< class T, class X >
class EAConstant : public eastl::unary_function< T, T >
{
    // ABSTRACT //
    // constant function object
    //
    // SUMMARY //
    // #include <EAFunction.h>
    //
    // DESCRIPTION //
    // EAConstant<T>(v) is a constant function from X -> T whose
    // value is v, where X is any type at all (i.e. "...").
    // Thus it will work as a unary, binary or other type of function.
    //
    // SEE ALSO //
    // EAFunction
    ///

public:
    // CONSTRUCTORS //
    EAConstant( const T& t ) : value( t )
    {
    }

    // OPERATORS //
    //result_type operator()( ... ) const {
    // DB Aug/97 - changed ... to X to get around Microsoft Compiler error
    result_type operator()( X ) const {

        return value;
    }

    // END ABSTRACT
    ///

protected:
    const T& value;
    EAConstant const &operator=( EAConstant const &src );
};


// FUNCTION ABSTRACT //
// EAconstant - create constant function object
//
//
// SUMMARY //
// #include <EAFunction.h>
//
// EAconstant(x);
//
//
// DESCRIPTION //
// EAconstant() simply constructs an EAConstant function object with
// the type determined by the type of x.
//
//
// SEE ALSO //
// EAFunction
// 
// FUNCTION DECLARATIONS //

// DB Aug/97 - added class X to get around Microsoft Compiler error
template< class T, class X >
EAConstant<T,X> EAconstant( const T& t, const X& ) {
    return EAConstant<T,X>( t );
}
// before
//	template< class T >
//EAConstant<T> EAconstant( const T& t ) {
//	return EAConstant<T>( t );
//	}


// END ABSTRACT //


// FUNCTION ABSTRACT //
// EAFunction - operations on unary function objects
//
//
// SUMMARY //
// #include <EAFunction.h>
//
// EA_UNARY_OP1( combine_operation, function_name );
// EA_BINARY_OP1( combine_operation, function_name );
//
//
// DESCRIPTION //
// The macros listed above expand into function templates which can
// instantiate a function called "function_name" that takes one (in
// the case of EA_UNARY_OP1) or two (in the case of EA_BINARY_OP1)
// unary function objects as arguments and returns a unary function
// object created by applying "combine_operation" to the argument(s).
// When "function_name" is an operator, this yields a particularly
// clear and concise way of building up function objects.
//
//
// EXAMPLE //
// EA_BINARY_OP1( plus, operator+ ) is a template for an operator+ such
// that you can add functions f and g to create a new function f+g.
//
// Suppose you want to select from a container items that are greater
// than three and less than seven.  There are a few ways to go about
// it.  You could write a specialized predicate like this:
//
// bool predicate( int x ) { return x < 3 && x > 7; }
//
// I think it's bad to write a lot of little special purpose predicates,
// especially when a simple function isn't enough and you have to define
// a function class.  Also, when you're reading code, you might have to
// go hunting for the predicate definition to understand what the code
// does.  The alternative is to construct predicates on the fly from
// STL-defined functions:
//
// compose2( logical_and<bool>,
//   bind2nd( less<int>, 3 ), bind2nd( greater<int>, 7 ) )
//
// In general these are not particulary easy to write nor to decipher.
// However the same thing can be written more clearly using overloaded
// operators to combine functions:
//
// EA_BINARY_OP1( less, operator< );
// EA_BINARY_OP1( greater, operator> );
// EA_BINARY_OP1( logical_and, operator&& );
//
// EAIdentity<int>() < EAconstant(3) && EAIdentity<int>() > EAconstant(7)
//
// In this approach the building blocks are EAIdentity, which represents
// the function argument, EAconstant for constants, and the familiar
// operators.  Additional examples follow.
//
// To square all the elements of a container:
//
// EA_BINARY_OP1( times, operator* );
//
// C c;
// transform( c.begin(), c.end(), c.begin(),
//   EAIdentity<C::value_type>() * EAIdentity<C::value_type>() );
//
// To count the number which are divisible by n:
//
// EA_UNARY_OP1( logical_not, operator! );
// EA_BINARY_OP1( modulus, operator% );
//
// int k = 0;
// count_if( c.begin(), c.end(),
//   ! ( EAIdentity<C::value_type>() % EAconstant(n) ), k );
//
//
// SEE ALSO //
// EAIdentity, EAConstant, EAconstant
//
// CAVEAT //
// Operator templates created by these macros tend to cause
// ambiguities with STL-defined operator templates and overloads.
// That is why the header file does not simply define all the
// templates.  Instead you should use just the macros you need,
// and just where you need them, thus hopefully avoiding ambiguities.
// You can also use a regular function name instead of an operator
// function name, but then you lose the notational benefits of
// using operators.
//
// FUNCTION DECLARATIONS //
#define EA_UNARY_OP1( OP, NAME ) \
    template< class F > \
    eastl::unary_compose< OP< typename F::result_type >, F > NAME( const F& f ) { \
    return compose1( OP< F::result_type >(), f ); \
    }

#if (defined(_MSC_VER) && (_MSC_VER < 1300)) 
#define EA_BINARY_OP1( OP, NAME ) \
    template< class F, class G > \
    eastl::binary_compose< OP< typename F::result_type >, F, G > NAME( const F& f, const G& g ) { \
    return compose2( OP< F::result_type >(), f, g ); \
    }
#else
#define EA_BINARY_OP1( OP, NAME ) \
    template< class F, class G > \
    eastl::binary_compose< OP< typename F::result_type >, F, G > NAME( const F& f, const G& g ) { \
    return compose2( OP< typename F::result_type >(), f, g ); \
    }
#endif

// END ABSTRACT //
#endif // EAFunction_h
