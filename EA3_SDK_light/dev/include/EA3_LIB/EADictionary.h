// (c) 1998-2010 Electronic Arts Inc.

#ifndef EADictionary_h
#define EADictionary_h

#include <eastl/map.h>
#include <eastl/algorithm.h>
using namespace eastl;

	template< class Key, class Value >
class EADictionary : public map< Key, Value, less<Key> > {

	// ABSTRACT //
	// STL map with added convenience methods
	//
	// SUMMARY //
	// #include <EADictionary.h>
	//
	// DESCRIPTION //
	// This class adds searching methods to an STL map.  The Key and
	// Value classes must have comparison operators.
	//
	// SEE ALSO //
	// map
	///

	protected:
		class Match {
			public:
				Match( const Value& v ) : value( v ) {
					}
				bool operator()( const_reference x ) const {
					return x.second == value ? true : false;
					}

			protected:
				const Value& value;
      private:
        const Match &operator=(const Match &);
			};
    

	public:
		// METHODS //
		// search by key to find a value - runs in log time
		virtual bool getValue( const Key& k, Value& v ) const {
			const_iterator i = find( k );
			if ( i == end() ) {
				return false;
				}
			v = (*i).second;
			return true;
			}

		// search by value to find a key - runs in linear time
		virtual bool getKey( const Value& v, Key& k ) const {
			const_iterator i = eastl::find_if( begin(), end(), Match( v ) );
			if ( i == end() ) {
				return false;
				}
			k = (*i).first;
			return true;
			}

	// END ABSTRACT //
	///

		virtual ~EADictionary( void ) {
			}
	};

	template< class Key, class Value >
class EADictionaryRW : public EADictionary< Key, Value > {

	// ABSTRACT //
	// STL map with added convenience methods
	//
	// SUMMARY //
	// #include <EADictionary.h>
	//
	// DESCRIPTION //
	// This class adds I/O to EADictionary.  The Key and Value classes
	// must have I/O operators.
	//
	// SEE ALSO //
	// EADictionary
	///

	public:
		// METHODS //
		// I/O 
		virtual EAistream& read( EAistream& is ) {
			erase( begin(), end() );
			Key k;
			Value v;
			while ( is >> k >> v ) {
				(*this)[k] = v;
				}
			if ( is.eof() ) {
				// if we made it to eof, assume everything is ok
				is.clear( EAios::eofbit );
				}
			return is;
			}

		virtual EAostream& write( EAostream& os ) const {
			for( const_iterator i = begin(); i != end(); ++i ) {
				os << (*i).first << ' ' << (*i).second << endl;
				}
			return os;
			}

	// END ABSTRACT //
	///

		~EADictionaryRW( void ) {
			}
	};

#endif // EADictionary_h

