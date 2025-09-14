// (c) 1998-2010 Electronic Arts Inc.

#ifndef EAMemberSelect_h
#define EAMemberSelect_h

#include <eastl/functional.h>

// FUNCTION ABSTRACT //
// EAmemberSelect - a function object that selects class member data
//
// SUMMARY //
// #include <EAMemberSelect.h>
//
// EAmemberSelect( &Class::member )
//
// DESCRIPTION //
// EAmemberSelect( member_ptr ) returns a function object whose operator()
// takes a class object and returns the member of that object specified by
// "member_ptr."
//
// EXAMPLE //
// struct Foo {
//   EAString name;
//   // other stuff
// };
//
// list<Foo> objs;
//
// // print x.name for all x in objs
// transform( objs.begin(), objs.end(), ostream_iterator<EAString>( cout ),
//   EAmemberSelect( &Foo::name ) );
//
// SEE ALSO //
// EAmemberCall
//
// END ABSTRACT //
///

	template< class Class, class MemberType >
class EAMemberSelect : public eastl::unary_function< Class, MemberType > {
	public:
		EAMemberSelect( MemberType Class::*m ) : member( m ) {
			}
		MemberType operator()( const Class& obj ) const {
			return obj.*member;
			}

	protected:
		MemberType Class::*member;
	};

	template< class Class, class MemberType >
EAMemberSelect< Class, MemberType > EAmemberSelect( MemberType Class::*m ) {
	return EAMemberSelect< Class, MemberType >( m );
	}

#endif // EAMemberSelect_h
