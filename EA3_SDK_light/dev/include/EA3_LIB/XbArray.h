/////////////////////////////////////////////////////////////
//
// XbArray.h
//
//  Sean Halliday
//
// (c) 1998-2010 Electronic Arts Inc.
//  XbArray Template
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

#ifndef  __XbArray
#define  __XbArray

//#include <malloc.h>


template <class Type> class XbArray {
  public:
// constructors
    XbArray( void) {
      allocated = 0;
      size = 0;
      buffer = NULL;
    };
    XbArray( int preAllocate) {
      allocated = preAllocate;
      size = 0;
      buffer = new Type[preAllocate];
    };
    XbArray( const XbArray<Type>& a) {
      int   i;
      allocated = a.allocated;
      if ( allocated == 0 ) {
        size = 0;
        buffer = NULL;
      } else {
        size = a.size;
        buffer = new Type[allocated];
        for ( i = 0; i < a.size; i++ ) buffer[i] = a.buffer[i];
      }
    };
// destructor
    virtual ~XbArray( void) {
      if ( allocated > 0 ) delete[] buffer;
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
    XbArray<Type> operator=( const XbArray<Type>& a) {
      int   i;
      if (buffer) delete [] buffer;
      allocated = a.allocated;
      size = a.size;
      buffer = new Type[allocated];
      for ( i = 0; i < a.size; i++ ) buffer[i] = a.buffer[i];
      return( *this);
    }
    XbArray<Type> &operator+=( const Type& n) {
      append(n);
      return(*this);
    }
    XbArray<Type> &operator+=( const XbArray<Type>& n) {
      append(n);
      return(*this);
    }
// methods
    void append( const Type& n) {
      size++;
      if ( allocated == 0 )
      {
        buffer = new Type[32];
        allocated = 32;
        (*this)[size - 1] = n;
      } else if ( size > allocated ) {
        Type copyOFn = n;
        Type*  newBuffer;
        int   i;
        allocated *= 2;
        newBuffer = new Type[allocated];
        for ( i = 0; i < (size - 1); i++ )
          newBuffer[i] = buffer[i];
        delete[] buffer;
        buffer = newBuffer;
        (*this)[size - 1] = copyOFn;
      } else {
        (*this)[size - 1] = n;
      }
      
    }
    void append( const XbArray<Type>& n) {
      int  i;
      size += n.size;
      if ( allocated == 0 )
      {
        buffer = new Type[n.size];
        allocated = n.size;
      }
      else if (size >= allocated )
      {
        Type*  newBuffer;
        allocated = size;
        newBuffer = new Type[allocated];
        for ( i = 0; i < (size - n.size); i++ )
          newBuffer[i] = buffer[i];
        delete[] buffer;
        buffer = newBuffer;
      }
      for ( i = 0; i < n.size; i++ )
        buffer[i + (size - n.size)] = n.buffer[i];
    }
    void insert( int location, const Type& n) {
      size++;
      if ( allocated == 0 )
      {
        buffer = new Type[1];
        allocated = 1;
        buffer[0] = n;
      } else if ( size > allocated ) {
        Type*  newBuffer;
        int   i;
        allocated++;
        newBuffer = new Type[allocated];
        for ( i = 0; i < location; i++ )
          newBuffer[i] = buffer[i];
        newBuffer[location] = n;
        for ( i = location; i < (size - 1); i++ )
          newBuffer[i + 1] = buffer[i];
        delete[] buffer;
        buffer = newBuffer;
      } else {
        int  i;
        for ( i = size-1; i > location; i-- )
          buffer[i] = buffer[i - 1];
        buffer[location] = n;
      }
    }
/*
not tested!
    void insert( int location, const XbArray<Type>& n) {
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
    void remove( int location, int _count) {
      int i;
      for ( i = (location + _count);i < size; i++ )
        buffer[i - _count] = buffer[i];
      size -= _count;
    }


    void clear( void) {
      size = 0;
    }

    inline int getSize( void) const {return(size);}

    void setSize(int s) {
      if (s <= size) {
        size = s;
        return;
      }
      Type*  newBuffer;
      int    i;
      if ( allocated == 0 ) {
        size = s;
        allocated = s;
        buffer = new Type[s];
      } else {
        newBuffer = new Type[s];
        for ( i = 0; i < size; i++ )
          newBuffer[i] = buffer[i];
        delete[] buffer;
        buffer = newBuffer;
        size = s;
        allocated = s;
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


#endif  // __XbArray


