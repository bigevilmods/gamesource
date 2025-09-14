/////////////////////////////////////////////////////////////
//
// XbPArray.h
//
//  Sean Halliday
//
// (c) 1998-2010 Electronic Arts Inc.
//  XbPArray Template
//  operators
//    []  access array elements
//
//  functions
//
//    getSize
//    append
//    insert
//    remove
//    clear 
//    setSize
//    qsort
//


#ifndef  __XbPArray
#define  __XbPArray
#ifdef EA_PLATFORM_WINDOWS
  #ifdef EA3_EXPORTS
	  #define CLASS_EXPORT __declspec(dllexport)
	#else
		#define CLASS_EXPORT __declspec(dllimport)
  #endif
  #pragma warning( push )
  #pragma warning( disable: 4710 4514)
#else
	#define CLASS_EXPORT
#endif

#ifndef EA_PLATFORM_WINDOWS
//#include <bstring.h>
#endif
#include <stdlib.h>
#include "ea3mem.h"

// type should be a pointer, no news or deletes are done
// on Type only malloc and free

template <class Type> class XbPArray {
  public:
// constructors
    XbPArray( void) {
      allocated = 0;
      size = 0;
      buffer = NULL;
    };
    XbPArray( int preAllocate) {
      allocated = preAllocate;
      size = 0;
      buffer = (Type *)EA3Malloc(sizeof(Type)*allocated);
    };
    XbPArray( const XbPArray<Type>& a) {
      int   i;
      allocated = a.allocated;
      if ( allocated == 0 ) {
        size = 0;
        buffer = NULL;
      } else {
        size = a.size;
        buffer = (Type *)EA3Malloc(sizeof(Type)*allocated);
        for ( i = 0; i < a.size; i++ ) buffer[i] = a.buffer[i];
      }
    };
// destructor
    ~XbPArray( void) {
      if (allocated == 0) {
        EA3Free(buffer);
      } else {
        EA3Free(buffer);
      }
      allocated = 0;
      size = 0;
      buffer = NULL;
    };
// operators
    inline Type& operator []( int i) {
      return( buffer[i]);
    };
    inline const Type& operator []( int i) const {
      return( buffer[i]);
    };
    XbPArray<Type> operator=( const XbPArray<Type>& a) {
      int   i;
      allocated = a.allocated;
      size = a.size;
      buffer = (Type *)EA3Realloc(buffer,sizeof(Type)*allocated);
      for ( i = 0; i < a.size; i++ ) buffer[i] = a.buffer[i];
      return( *this);
    }
    XbPArray<Type> &operator+=( const Type n) {
      append(n);
      return(*this);
    }
    XbPArray<Type> &operator+=( const XbPArray<Type>& n) {
      append(n);
      return(*this);
    }
// methods
    void append( const Type n) {
      size++;
      if ( allocated == 0 )
      {
        allocated = 1;
        buffer = (Type *)EA3Realloc(buffer,sizeof(Type)*allocated);
      } else if ( size > allocated ) {
        allocated *= 2;
        buffer = (Type *)EA3Realloc(buffer,sizeof(Type)*allocated);
      }
      (*this)[size - 1] = n;
    }
    void append( const XbPArray<Type>& n) {
      int  i;
      size += n.size;
      if ( allocated == 0 )
      {
        allocated = n.size;
        buffer = (Type *)EA3Realloc(buffer,sizeof(Type)*allocated);
      }
      else if (size >= allocated )
      {
        allocated = size;
        buffer = (Type *)EA3Realloc(buffer,sizeof(Type)*allocated);
      }
      for ( i = 0; i < n.size; i++ )
        buffer[i + (size - n.size)] = n.buffer[i];
    }
    void insert( int location, const Type n) {
      int   i;
      size++;
      if ( allocated == 0 ) {
        allocated = 1;
        buffer = (Type *)EA3Realloc(buffer,sizeof(Type)*allocated);
        buffer[0] = n;
      } else if ( size > allocated ) {
        allocated*=2;
        buffer = (Type *)EA3Realloc(buffer,sizeof(Type)*allocated);
      }
      for ( i = size-1; i > location; i-- )
        buffer[i] = buffer[i - 1];
      buffer[location] = n;
    }
/*
not tested!
    void insert( int location, const XbPArray<Type>& n) {
      int  i;
      size += n.size;
      if ( allocated == 0 )
      {
        buffer = new Type[n.size];
        allocated = n.size;
        for ( i = 0; i < n.size; i++ )
          buffer[i] = n.buffer[i];
      } else if ( size > allocated ) {
        Type*  newBuffer;
        allocated = size;
        newBuffer = new Type[allocated];
        for ( i = 0; i < location; i++ )
          newBuffer[i] = buffer[i];
        for ( i = 0; i < n.size; i++ )
          newBuffer[i + location] = n.buffer[i];
        for ( i = location; i < (size - n.size); i++ )
          newBuffer[i + n.size] = buffer[i];
        delete[] buffer;
        buffer = newBuffer;
      } else {
        int  i;
        for ( i = location + n.size; i < size; i++ )
          buffer[i] = buffer[i - n.size];
        for ( i = 0; i < n.size; i++ )
          buffer[i + location] = n.buffer[i];
      }
    }
*/
    void remove( int location) {
      remove( location, 1);
    }
    void remove( int location, int _num) {
      int i;
      for ( i = (location + _num);i < size; i++ )
        buffer[i - _num] = buffer[i];
      size -= _num;
    }
    void clear( void) {
      size = 0;
    }

    inline int getSize( void) const {return(size);}

    void setSize(int s) {
      size = s;
      if(allocated < s) {
        if (allocated == 0) allocated = s;
        while(allocated < s) allocated *= 2;
        buffer = (Type *)EA3Realloc(buffer,sizeof(Type)*allocated);
      }
	};

    void allocateSize(int a) {
      if(allocated < a) {
        if (allocated == 0) allocated = a;
        while(allocated < a) allocated *= 2;
        buffer = (Type *)EA3Realloc(buffer,sizeof(Type)*allocated);
      }
	};

    //
    //  Invoke standard library's qsort() routine.
    //
    //  keyCompare() should return "<0", "0", or ">0" if the first key is
    //  less than, equals, or more than the second key, respectively.
    //
    void qsort( int (*keyCompare)( Type *, Type * ) )
    {
	::qsort(
	    (void *)buffer,
	    size,
	    sizeof( Type ),
	    (int (*)(const void *,const void *))keyCompare );
    }

// data
// DANGER: NOT private for optimization in other classes
    Type  *buffer;
    int  allocated;
    int  size;
};
#ifdef EA_PLATFORM_WINDOWS
#pragma warning( pop )
#endif
#endif  // __XbPArray


