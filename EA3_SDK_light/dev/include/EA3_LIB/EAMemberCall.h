// (c) 1998-2010 Electronic Arts Inc.

#ifndef EAMemberCall_h
#define EAMemberCall_h

// FUNCTION ABSTRACT //
// EAmemberCall - function objects that call class member functions
//
// SUMMARY //
// #include <EAMemberCall.h>
//
// Class object;
//
// // bind object and function
// EAmemberCall( object, Class::func )
//
// // bind function and arguments
// EAmemberCall( Class::func )
// EAmemberCall( Class::func, arg1 )
// EAmemberCall( Class::func, arg1, arg2 )
//
// // bind const function and arguments
// EAconstMemberCall( Class::func )
// EAconstMemberCall( Class::func, arg1 )
// EAconstMemberCall( Class::func, arg1, arg2 )
//
// DESCRIPTION //
// You need three things to call a member function: an object, the member
// function, and the function arguments (if any).  EAmemberCall() returns
// a function object in which two of the three things are bound to the
// supplied arguments and the third thing is the argument of the operator()
// of the resulting function object.  The operator() performs the member
// function call.
//
// The first form shown above binds the object and the function.  The
// second form binds the function and the arguments.  A third form would
// bind the object and the arguments, but this seems to be of limited
// usefulness so it is not provided yet.
//
// There should not really be a need for EAconstMemberCall() when using
// const member functions, but the compiler doesn't seem able to to
// distinguish between const and non-const member functions in this case,
// so you must use EAconstMemberCall when the member function is const.
//
// EXAMPLE //
// Container args;
// Class object;
// // call object.func( x ) for all x in args
// for_each ( args.begin(), args.end(), EAmemberCall( object, Class::func ) );
//
// Container objs;
// // call x.func( arg ) for all x in objs
// for_each ( objs.begin(), objs.end(), EAmemberCall( Class::func, arg ) );
//
// CAVEAT //
// The compiler sometimes has trouble matching function templates.  If
// you run into this problem, insert a declaration of the function you
// need.  Then the compiler seems to go ahead and generate the definition
// from the template.
//
// END ABSTRACT //
///

#include <eastl/functional.h>

//////////////////////////////////////////////////////////////////////////////
// object bound //////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

// non-const object, one argument

	template< class Class, class Result, class Arg1 >
class object_bind1 : public eastl::unary_function< Arg1, Result > {
	public:
		object_bind1( Class& o, Result (Class::*f)( Arg1 ) ) : obj(o), mf(f) {
			}
		Result operator()( Arg1 a1 ) const {
			return (obj.*mf)( a1 );
			}

	protected:
		Class& obj;
		Result (Class::*mf)( Arg1 );
	};

	template< class Class, class Result, class Arg1 >
object_bind1< Class, Result, Arg1 >
		EAmemberCall( Class& obj, Result (Class::*mf)( Arg1 ) ) {
	return object_bind1< Class, Result, Arg1 >( obj, mf );
	}

// const object, one argument

	template< class Class, class Result, class Arg1 >
class const_object_bind1 : public eastl::unary_function< Arg1, Result > {
	public:
		const_object_bind1( const Class& o, Result (Class::*f)( Arg1 ) const )
				: obj(o), mf(f) {
			}
		Result operator()( Arg1 a1 ) const {
			return (obj.*mf)( a1 );
			}

	protected:
		const Class& obj;
		Result (Class::*mf)( Arg1 ) const;
	};

	template< class Class, class Result, class Arg1 >
const_object_bind1< Class, Result, Arg1 >
		EAmemberCall( const Class& obj, Result (Class::*mf)( Arg1 ) const ) {
	return const_object_bind1< Class, Result, Arg1 >( obj, mf );
	}

// non-const object, two arguments

	template< class Class, class Result, class Arg1, class Arg2 >
class object_bind2 : public eastl::binary_function< Arg1, Arg2, Result > {
	public:
		object_bind2( Class& o, Result (Class::*f)( Arg1, Arg2 ) )
				: obj(o), mf(f) {
			}
		Result operator()( Arg1 a1, Arg2 a2 ) const {
			return (obj.*mf)( a1, a2 );
			}

	protected:
		Class& obj;
		Result (Class::*mf)( Arg1, Arg2 );
	};

	template< class Class, class Result, class Arg1, class Arg2 >
object_bind2< Class, Result, Arg1, Arg2 >
		EAmemberCall( Class& obj, Result (Class::*mf)( Arg1, Arg2 ) ) {
	return object_bind2< Class, Result, Arg1, Arg2 >( obj, mf );
	}

// const object, two arguments

	template< class Class, class Result, class Arg1, class Arg2 >
class const_object_bind2 : public eastl::binary_function< Arg1, Arg2, Result > {
	public:
		const_object_bind2( const Class& o,
				Result (Class::*f)( Arg1, Arg2 ) const ) : obj(o), mf(f) {
			}
		Result operator()( Arg1 a1, Arg2 a2 ) const {
			return (obj.*mf)( a1, a2 );
			}

	protected:
		const Class& obj;
		Result (Class::*mf)( Arg1, Arg2 ) const;
	};

	template< class Class, class Result, class Arg1, class Arg2 >
const_object_bind2< Class, Result, Arg1, Arg2 >
		EAmemberCall( const Class& obj,
		Result (Class::*mf)( Arg1, Arg2 ) const ) {
	return const_object_bind2< Class, Result, Arg1, Arg2 >( obj, mf );
	}

//////////////////////////////////////////////////////////////////////////////
// arguments bound ///////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

// non-const object, no arguments

	template< class Class, class Result >
class arg_bind0 : public eastl::unary_function< Class, Result > {
	public:
		arg_bind0( Result (Class::*f)( void ) ) : mf(f) {
			}
		Result operator()( Class& obj ) const {
			return (obj.*mf)();
			}

	protected:
		Result (Class::*mf)( void );
	};

	template< class Class, class Result >
arg_bind0< Class, Result > EAmemberCall( Result (Class::*mf)( void ) ) {
	return arg_bind0< Class, Result >( mf );
	}

// const object, no arguments

	template< class Class, class Result >
class const_arg_bind0 : public eastl::unary_function< Class, Result > {
	public:
		const_arg_bind0( Result (Class::*f)( void ) const ) : mf(f) {
			}
		Result operator()( const Class& obj ) const {
			return (obj.*mf)();
			}

	protected:
		Result (Class::*mf)( void ) const;
	};

	template< class Class, class Result >
const_arg_bind0< Class, Result >
		EAconstMemberCall( Result (Class::*mf)( void ) const ) {
	return const_arg_bind0< Class, Result >( mf );
	}

// non-const object, one argument

	template< class Class, class Result, class Arg1 >
class arg_bind1 : public eastl::unary_function< Class, Result > {
	public:
		arg_bind1( Result (Class::*f)( Arg1 ), Arg1 a1 ) : mf(f), arg1(a1) {
			}
		Result operator()( Class& obj ) const {
			return (obj.*mf)( arg1 );
			}

	protected:
		Result (Class::*mf)( Arg1 );
		Arg1 arg1;
	};

	template< class Class, class Result, class Arg1 >
arg_bind1< Class, Result, Arg1 >
		EAmemberCall( Result (Class::*mf)( Arg1 ), Arg1 arg1 ) {
	return arg_bind1< Class, Result, Arg1 >( mf, arg1 );
	}

// const object, one argument

	template< class Class, class Result, class Arg1 >
class const_arg_bind1 : public eastl::unary_function< Class, Result > {
	public:
		const_arg_bind1( Result (Class::*f)( Arg1 ) const, Arg1 a1 )
				: mf(f), arg1(a1) {
			}
		Result operator()( const Class& obj ) const {
			return (obj.*mf)( arg1 );
			}

	protected:
		Result (Class::*mf)( Arg1 ) const;
		Arg1 arg1;
	};

	template< class Class, class Result, class Arg1 >
const_arg_bind1< Class, Result, Arg1 >
		EAconstMemberCall( Result (Class::*mf)( Arg1 ) const, Arg1 arg1 ) {
	return const_arg_bind1< Class, Result, Arg1 >( mf, arg1 );
	}

// non-const object, two arguments

	template< class Class, class Result, class Arg1, class Arg2 >
class arg_bind2 : public eastl::unary_function< Class, Result > {
	public:
		arg_bind2( Result (Class::*f)( Arg1, Arg2 ), Arg1 a1, Arg2 a2 )
				: mf(f), arg1(a1), arg2(a2) {
			}
		Result operator()( Class& obj ) const {
			return (obj.*mf)( arg1, arg2 );
			}

	protected:
		Result (Class::*mf)( Arg1, Arg2 );
		Arg1 arg1;
		Arg2 arg2;
	};

	template< class Class, class Result, class Arg1, class Arg2 >
arg_bind2< Class, Result, Arg1, Arg2 >
		EAmemberCall( Result (Class::*mf)( Arg1, Arg2 ),
		Arg1 arg1, Arg2 arg2 ) {
	return arg_bind2< Class, Result, Arg1, Arg2 >( mf, arg1, arg2 );
	}

// const object, two arguments

	template< class Class, class Result, class Arg1, class Arg2 >
class const_arg_bind2 : public eastl::unary_function< Class, Result > {
	public:
		const_arg_bind2( Result (Class::*f)( Arg1, Arg2 ) const,
				Arg1 a1, Arg2 a2 ) : mf(f), arg1(a1), arg2(a2) {
			}
		Result operator()( const Class& obj ) const {
			return (obj.*mf)( arg1, arg2 );
			}

	protected:
		Result (Class::*mf)( Arg1, Arg2 ) const;
		Arg1 arg1;
		Arg2 arg2;
	};

	template< class Class, class Result, class Arg1, class Arg2 >
const_arg_bind2< Class, Result, Arg1, Arg2 >
		EAconstMemberCall( Result (Class::*mf)( Arg1, Arg2 ) const,
		Arg1 arg1, Arg2 arg2 ) {
	return const_arg_bind2< Class, Result, Arg1, Arg2 >( mf, arg1, arg2 );
	}

#endif // EAMemberCall_h

