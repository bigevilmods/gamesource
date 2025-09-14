// (c) 1998-2010 Electronic Arts Inc.

#ifndef EAHandle_h
#define EAHandle_h

#include <assert.h>

//#include <function.h>

//
// This class is the base for EAHandle and EARef.
// It contains the reference count and common operations on it.
//
	template< class T >
class EARefBase {
	public:
		typedef T Type;

		bool attached( void ) const {
			return rc != 0;
			}

	protected:
		struct RefCount {
			RefCount( T *t ) {
				ptr = t;
				count = 1;
				}
			T *ptr;
			int count;
			};

		// reference <- nothing
		EARefBase( void ) {
			rc = 0;
			}

		virtual ~EARefBase( void ) {
			// derived classes should take care of deleting rc
			assert( ! attached() );
			}

		// reference <- nothing
		void ref( void ) {
			unref();
			rc = 0;
			}

		// reference <- object t
		void ref( T *t ) {
			unref();
			rc = new RefCount( t );
			}

		// reference <- object referenced by x
		void ref( const EARefBase& x ) {
			unref();
			rc = x.rc;
			addref();
			}

		// access object
		T& obj( void ) {
			assert( attached() );
			return *rc->ptr;
			}
		const T& obj( void ) const {
			assert( attached() );
			return *rc->ptr;
			}

		// is object being shared?
		bool shared( void ) const {
			return rc->count > 1;
			}

	private:
		RefCount *rc;

		void addref( void ) {
			if ( rc ) {
				++rc->count;
				}
			}

		void unref( void ) {
			if ( rc && --rc->count == 0 ) {
				delete rc->ptr;
				delete rc;
				}
			}

		EARefBase( const EARefBase& ) {
			// derived classes should take care of setting rc
			assert( false );
			}
	};

	template< class T >
class EAHandle : public EARefBase<T> {

	// ABSTRACT //
	// smart pointers for storage management
	//
	// SUMMARY //
	// #include <EAHandle.h>
	//
	// DESCRIPTION //
	// The bigger your objects are, the more work the copy constructors
	// and assignment operators have to do.  To avoid lots of memory
	// copying, you might want to use pointers to big chunks of data,
	// rather than including the data itself in your class, but then
	// you need to keep track of all the references to that storage
	// so that it isn't deleted prematurely or more than once, but it
	// is deleted when it is no longer needed.  EAHandle greatly
	// simplifies this storage management.  The only rules to follow
	// are these: use handles only to dynamically allocated data, and
	// let the handles take care of the deletion of that data.  This
	// implies it is not wise to refer to the data except through a
	// handle, because the handles can delete it.
	//
	// EXAMPLE //
	// class BigObj {
	//   int data[99999];
	// };
	// 
	// // the problem
	// class Heavy {
	//   BigObj x; // this makes the class "heavy"
	// };
	// 
	// // the solution
	// class Light {
	//   Light() {
	//     x = new BigObj; // you must set up the handle
	//   }
	//   EAHandle<BigObj> x; // handles are "light"
	// };
	// 
	// // Handles have * and -> defined, just like pointers
	// Light feather;
	// BigObj& x = *feather.x;
	// int *data = feather.x->data;
	//
	// // If you want to stop sharing the data, call copy()
	// EAHandle<Foo> f1, f2;
	// f1 = new Foo;
	// f2 = f1;   // f2 refers to the same Foo as f1
	// f2.copy(); // f2 now has its own copy of the original Foo
	// f1.copy(); // f1 makes a copy and the original Foo is deleted
	///

	public:
		// CONSTRUCTORS //
		// create a handle that points at nothing
		EAHandle( void ) {
			}

		// copying a handle creates another reference to the data
		EAHandle( const EAHandle& x ) {
			ref( x );
			}

		// create handle that points to the data at the given address
		EAHandle( T *t ) {
			ref( t );
			}

		// DESTRUCTOR //
		// destroying a handle removes a reference to the data - if it
		// was the last reference, the data is deleted
		virtual ~EAHandle( void ) {
			ref();
			}

		// OPERATORS //
		// a reference (if there were any) is removed from data of
		// the left side handle, and one is created for the data of
		// the right side
		EAHandle& operator=( const EAHandle& x ) {
			ref( x );
			return *this;
			}
		EAHandle& operator=( T *t ) {
			ref( t );
			return *this;
			}

		// use like a pointer - access data with * or ->
		T& operator*( void ) {
			return obj();
			}
		const T& operator*( void ) const {
			return obj();
			}
		T* operator->( void ) {
			return & obj();
			}
		const T* operator->( void ) const {
			return & obj();
			}

		// METHODS //
		// make our own copy of the data
		void copy( void ) {
			ref( new T( obj() ) );
			}

	// END ABSTRACT //
	///
	};

	template< class T >
class EARef : public EARefBase<T> {

	// ABSTRACT //
	// smart pointers for use in heterogeneous containers
	//
	// SUMMARY //
	// #include <EAHandle.h>
	//
	// DESCRIPTION //
	//
	// When you want to store pointers in containers (so they can be
	// heterogeneous), store EARef's instead.
	// EARef is like EAHandle, except there can be no sharing of the
	// referenced data (otherwise the contents of your container could
	// be changed indirectly).
	// Unlike EAHandle, EARef relies on a copy function, so that
	// references to a base class copy correctly when they refer to a
	// derived object.
	//
	// EXAMPLE //
	// class Base {
	//   virtual Base *copy( void ) const {
	//     return new Base( *this );
	//   }
	// };
	// class Derived : public Base {
	//   virtual Base *copy( void ) const {
	//     return new Derived( *this );
	//   }
	// };
	// // container to hold anything derived from Base
	// list< EARef< Base > > c;
	// // mix objects in container
	// // (EARef constructor should be called automatically)
	// c.push_back( Base() );
	// c.push_back( Derived() );
	// // you'll have to decide when it's safe to cast
	// Derived& d = (Derived &) c.back();
	//
	// SEE ALSO //
	// EAHandle
	///

	public:
		// CONSTRUCTORS //
		// reference to nothing
		EARef( void ) {
			}

		// creates another reference to the data
		EARef( const EARef& x ) {
			ref( x );
			}

		// reference to copy of t
		EARef( const T& t ) {
			ref( t.copy() );
			}

		// DESTRUCTOR //
		// destroying removes a reference to the data - if it
		// was the last reference, the data is deleted
		virtual ~EARef( void ) {
			ref();
			}

		// OPERATORS //
		// a reference (if there were any) is removed on the left side
		// of the assignment, and one is added on the right side
		EARef& operator=( const EARef& x ) {
			ref( x );
			return *this;
			}

		// access the data
		T& operator*( void ) {
			noshare();
			return obj();
			}
		const T& operator*( void ) const {
			return obj();
			}

		// compare underlying objects
		bool operator==( const EARef& x ) const {
			return obj() == x.obj();
			}
		bool operator<( const EARef& x ) const {
			return obj() < x.obj();
			}

		// METHODS //
		// I/O of underlying object
		EAistream& read( EAistream& is ) {
			noshare();
			return is >> obj();
			}
		EAostream& write( EAostream& os ) const {
			return os << obj();
			}

	// END ABSTRACT //
	///

	protected:
		void noshare( void ) {
			if ( shared() ) {
				ref( obj().copy() );
				}
			}
	};

#endif // EARef_h
