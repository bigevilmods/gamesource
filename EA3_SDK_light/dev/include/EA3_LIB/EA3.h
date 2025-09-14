#ifndef EA3_H
#define EA3_H
// (c) 1998-2007 Electronic Arts Inc.
// This code is for use by employees of Electronic Arts only, 
// unless otherwised licensed from Electronic Arts.

#include <eabase/eabase.h>

// This needs to be automatically generated somehow?
#define EA3_MAJOR_VERSION 3
#define EA3_MINOR_VERSION 29
#define EA3_INTERM_VERSION 15

#ifndef SWIG

#define EA3_LABEL __DATE__ " " __TIME__


#define EA3_RCS_VERSION "$Id: //depot/packages/EA3_SDK_pc/dev/include/EA3_LIB/EA3.h#7 $"
#define EA3_RCS_HEADER "***VERSION*** $Header: //depot/packages/EA3_SDK_pc/dev/include/EA3_LIB/EA3.h#7 $"

static const char *ea3version = EA3_RCS_HEADER;
inline const char *_getEA3Version() {
  return(ea3version);
}
#endif    // ifndef SWIG

#include "ea3regexp.h"

#ifdef _MSC_VER
// disable some warnings (like truncated template names that are too long)
#pragma warning( disable : 4097 4100 4251 4510 4512 4610 4710 4786) 
#endif

#ifndef SWIG
#define MAX_LINE_LEN 100000
#endif

#ifndef EA3_EXPORT
#  if defined(EA_PLATFORM_WINDOWS) && defined(EA_DLL)
#    ifdef EA3_EXPORTS
#      define EA3_EXPORT __declspec(dllexport)
#    else
#      define EA3_EXPORT __declspec(dllimport)
#    endif
#  else
#    define EA3_EXPORT
#  endif
#endif

EA3_EXPORT void getEA3Version(int &major,int &minor,int &interm);
EA3_EXPORT const char *getEA3Label();
// size is number of 4 byte values.  Can be from 2 to 6 or 16 currently.
EA3_EXPORT void *EA3HeapAlloc(int size);
//! size is number of 4 byte values.  Can be from 2 to 6 or 16 currently.  Returns true iff ptr was in the heap of that size.
EA3_EXPORT bool EA3HeapFree(void *ptr,int size);
#ifndef SWIG
extern EA3_EXPORT int ea3asciiVerbose;
#endif

/*!
 ********************************************************
 * \file
 */
/*!
 *  
 */
/*!
 *  
 *  
 */
/*!
 * \page       ea3classesOverview Overview of the EA3 Classes
 *  
 * 
 * EA3 uses a set of classes to represent an EA3 file.
 * To represent low level data like floats, ints and strings 
 * the class EA3Field is used.  It contains an array of EA3FieldData (which is a union of 
 * basic types).
 * To group fields together to represent higher level data structures, 
 * the EA3Block or EA3Array are used. EA3Blocks can contain lists
 * of any other object derived from EA3Base.  EA3Arrays contain lists of
 * the same type of object derived from EA3Base (all blocks, all fields, etc).
 * An EA3Reference is used to hold a list of pointers to any other object
 * in the scene.  For example a polygon may reference the coordinates that are
 * stored with EA3Fields in an EA3Array.  To create an EA3Scene, you must follow 
 * the grammar.  Every EA3 object
 * has a type and a name (which may be empty).  The grammar defines which types
 * can be created in which other types.  For example, the grammar says that
 * an EA3Reference of type "coord" can be created in an EA3Block of type "polygon"
 * For more detailed information please read:
 * - EA3 Overview
 * - EA3 Grammar
 * - EA3 Conventions
 */
/*!
 ********************************************************
 * \brief           get the EA3 version
 *  
 * 
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef SWIG
#include "XbPArray.h"
#endif
//#include <malloc.h>
#include <assert.h>

#include <stdarg.h>

#if 0
void fnVarArgs( const char *format, ... )
{
    va_list args;
    va_start( args, format );

    char *arg = va_arg( args, char * );

    while( arg )
    {
        arg = va_arg( args, char * );
        printf( "arg = %s\n", arg );
    }
}
#endif

#ifndef SWIG
extern EA3_EXPORT int ea3verbose;

class EA3_EXPORT EA3Verbose {
public:
  EA3Verbose() {
    if (ea3verbose > 0) printf("%s\n",EA3_RCS_VERSION);
  }
};
#endif  // #ifndef SWIG

#ifndef SWIG
typedef float Float4[4];
typedef float Float3[3];
typedef float Float2[2];
#endif
typedef unsigned int ModeBits;
typedef uint32_t U32;
typedef int32_t I32;
typedef unsigned char U8;

// Internal classes
class EA3Chunk;
class gzReader;
class EA3BaseInfoList;
class EA3BaseInfo;
class EA3_EXPORT EA3BaseLRU;
class XbRefName;
class EA3FieldDataString;
class EA3Pointer;

// Class Hierarchy (external)
class EA3_EXPORT XbString;
  class EA3_EXPORT XbIdentifier;
  class EA3_EXPORT XbQString;
class EA3_EXPORT XbColour;

class EA3_EXPORT EA3Base;
  class EA3_EXPORT EA3Field;
  class EA3_EXPORT EA3Reference;
  class EA3_EXPORT EA3Block;
    class EA3_EXPORT EA3Grammar;
    class EA3_EXPORT EA3Array;
    class EA3_EXPORT EA3Scene;



// FIFA_BEGIN | SENDBACK | mbarr@ea.com | 2018-10-22 | Change typedef to match zlib.h.
typedef struct gzFile_s *gzFile;
// FIFA_END
#define GZWRITER_MAX_SIZE 1048576
class gzWriter : public EA3Mem {
#ifndef SWIG
public:
  gzWriter() : ok(true) {
    flushCB = NULL;
    CBdata = NULL;
    data.setSize(GZWRITER_MAX_SIZE+10);
    data.setSize(0);
  }
  void append(const char *_str) {
    while(*_str) {
      data.append(*_str);
      _str++;
      if (data.getSize() >= GZWRITER_MAX_SIZE && flushCB) {
        (*flushCB)(data,CBdata);
        data.setSize(0);
      }
    }
  }
  const char *str() const {
    return(data.buffer);
  }
  int getLength() const {
    return data.getSize();
  }
  bool fail() const {
    return (!ok);
  }
  bool bad() const {
    return(!ok);
  }
  gzWriter &operator<<(int i) {
    char buf[20];
    sprintf(buf,"%d",i);
    append(buf);
    return(*this);
  };
  gzWriter &operator<<(unsigned int i) {
    char buf[20];
    sprintf(buf,"%#x",i);
    append(buf);
    return(*this);
  };
  gzWriter &operator<<(float f) {
    char buf[30];
    sprintf(buf,"%.8g",f);
    append(buf);
    return(*this);
  };
  gzWriter &operator<<(const char *_str) {
    append(_str);
    return(*this);
  }
  gzWriter &operator<<(char c) {
    data.append(c);
    return(*this);
  }
  typedef void FlushCB(XbPArray<char> &,void *CBdata);
  void *CBdata;
  FlushCB *flushCB;
private:
  bool ok;
  XbPArray<char> data;
  
#endif      // ifndef SWIG
};


//!  A simple string class.
/*!
   
  An XbString class holds a pointer to a char* that is in a hash table. This
  allows XbStrings to be compared for equality by simply comparing pointers.
  \sa stringMatch
 */

class XbString : public EA3Mem {
  public:

    //! Default constructor.
    XbString();

    virtual ~XbString() {}

#ifndef SWIG
    //! A constructor.
    /*!
      \param other (in) XbString to be copied from.
    */
    XbString(const XbString &other);

    //! A constructor.
    /*!
      \param str (in) char string to be copied from.
    */
    XbString(const char *str);
    XbString(const EA3FieldDataString &str);
#endif

    //~XbString();

    //! Returns the length of the string.
    int length() const;

#ifndef SWIG
    //! Copy from a char string.
    /*!
      \param str (in) char string to be copied from.
     */
    XbString operator =(const char *str);

    //! Copy from a XbString.
    /*!
      \param str (in) XbString to be copied from.
     */
    XbString operator =(const XbString &str);
    XbString operator =(const EA3FieldDataString &str);

    //! Concatenate this string with a XbString.
    /*!
      \param str (in) XbString to be added.
     */
    XbString & operator +=(const XbString &str);

    //! Concatenate this string with a char string.
    /*!
      \param str (in) char string to be added.
     */
    XbString & operator +=(const char *str);

    friend XbString EA3_EXPORT operator +(XbString str1, const XbString &str2);
    friend int EA3_EXPORT operator ==(const XbString &str, const char *s);
    friend int EA3_EXPORT operator ==(const char *s, const XbString &str);
    friend int EA3_EXPORT operator ==(const XbString &str1, const XbString &str2);
    friend int EA3_EXPORT operator !=(const XbString &str, const char *s);
    friend int EA3_EXPORT operator !=(const char *s, const XbString &str);
    friend int EA3_EXPORT operator !=(const XbString &str1, const XbString &str2);
    friend int EA3_EXPORT operator <(const XbString &str1, const XbString &str2);

    //! Return a character from the string.
    char operator[](int _i) const {return data[_i];}

    //! Is the string empty?
    int operator!() const {return data[0]==0;}
#endif

    //! Returns the length of the string.
    int getLength() const {return static_cast<int>(strlen(data));}

    //! Find index of character - returns negative if not found.
    int index( char c ) const;

    //! Find the last index of character - returns negative if not found.
    int rindex( char c ) const;

    //! Return a substring, delimited by startChar and endChar.
    XbString getSubString(int startChar, int endChar = -1) const;

    //! Set a substring, delimited by begin and end.
    void setSubString( const XbString& s, int begin, int end = -1 );

    //! Delete a substring, delimited by startChar and endChar.
    void deleteSubString(int startChar, int endChar = -1);
 
    //! Return prefix.
    /*! This is everything before the first occurrence of <I>sep</I>. */
    XbString prefix( char sep ) const;

    //! Return suffix.
    /*! This is everything after the last occurrence of <I>sep</I>. */
    XbString suffix( char sep ) const;

    //! Remove prefix.
    /*!
      Including the separator character, <I>sep</I>, returns what was removed.
     */
    XbString removePrefix( char sep );

    //! Remove suffix.
    /*!
      Including the separator character, <I>sep</I>, returns what was removed.
     */
    XbString removeSuffix( char sep );

    //! Change prefix.
    /*! The prefix is removed if present, then the given string is prepended. */
    void changePrefix( char sep, const XbString& );

    //! Change suffix.
    /*! The suffix is removed if present, then the given string is appended. */
    void changeSuffix( char sep, const XbString& );

    //! Return the string as a double.
    double getDouble() const {return atof(data);}

    // change special characters to escape sequences, e.g. tab to \t
    // and vice versa - does not handle numeric escapes like \012
    // XbString escape( const char *special = "\n\t\v\b\r\f\a" ) const;
    // XbString unescape( const char *special = "ntvbrf" ) const;

    virtual char *read(char *is);
    //virtual istream& read( istream& is );
    virtual gzWriter& write( gzWriter& os ) const;

#ifndef SWIG
    //! set the string
    virtual void setString(const XbString &str);
#endif
    //! set the string
    virtual void setString(const char *str);
    
    //! get the string as a const char *
    virtual inline const char *getString() const {return data;}
    //! convert the string to lower case
    virtual XbString tolower() const;

#ifndef SWIG
    static XbPArray<char> linebuf;
#endif
    //! duplicate the string
    static char *_strdup(const char *);

  private:
#ifndef SWIG
    friend class EA3Base;
    friend class EA3Block;
	  friend class EA3Chunk;
    friend class EA3FieldDataString;
#endif
    char *data;
};

/*!
  \brief return true if string matches pattern
   
  \verbatim
  *       Matches any sequence of characters in string, including a null string. 
  ?       Matches any single character in string. 
  [chars] Matches any character in the set given by chars.  
          If a sequence of the form x-y appears in chars, then any character between x and y, 
          inclusive, will match. 
  \x	  Matches the single character x.  This provides a way of avoiding the special 
          interpretation of the characters *?[]\ in pattern. 
  \endverbatim
  */
inline int stringMatch(const XbString &str,const char *pattern) {
  const char *s = str.getString();
  return stringMatch(s,pattern);
}

/*!
   
  \brief return true if string matches the regular expression in pattern
  \par REGULAR EXPRESSIONS
Regular expressions are implemented using Henry Spencer's package (thanks, Henry!), 
and much of the description of regular expressions below is copied verbatim from his manual entry. 
- A regular expression is zero or more branches, separated by "|". It matches anything that 
matches one of the branches. 
- A branch is zero or more pieces, concatenated. It matches a match for the first, followed by a match for the second, etc. 
- A piece is an atom possibly followed by "*", "+", or "?". An atom followed by "*" matches a sequence of 0 or more matches of the atom. An atom followed by "+" matches a sequence of 1 or more matches of the atom. An atom followed by "?" matches a match of the atom, or the null string. 
- An atom is a regular expression in parentheses (matching a match for the regular expression), a range (see below), "." (matching any single character), "^" (matching the null string at the beginning of the input string), "$" (matching the null string at the end of the input string), a "\" followed by a single character (matching that character), or a single character with no other significance (matching that character). 
- A range is a sequence of characters enclosed in "[]". It normally matches any single character from the sequence. If the sequence begins with "^", it matches any single character not from the rest of the sequence. If two characters in the sequence are separated by "-", this is shorthand for the full list of ASCII characters between them (e.g. "[0-9]" matches any decimal digit). To include a literal "]" in the sequence, make it the first character (following a possible "^"). To include a literal "-", make it the first or last character.  
*/
EA3_EXPORT bool regexpMatch(const XbString &str,char *pattern);

//! A string with a restricted set of characters.
/*!
  Strings that can only have letters, digits and _'s. 
  Can not start with a digit.
 */

class XbIdentifier : public XbString {
  public:
    //! constructor
    XbIdentifier();
#ifndef SWIG
    //! constructor
    XbIdentifier(const char *str);
    //! constructor
    XbIdentifier(const XbString &other);
    //! constructor
    XbIdentifier(const EA3FieldDataString &str);
    //! assignment
    XbIdentifier operator =(const char *str);
    //! assignment
    XbIdentifier operator =(const XbIdentifier &str);
    //! assignment
    XbIdentifier operator =(const EA3FieldDataString &str);
    virtual char *read(char *is);
//    virtual istream& read( istream& is );
#endif
    //! set the string
    virtual void setString(const char *str);
#ifndef SWIG
    //! set the string
    virtual void setString(const XbIdentifier &str);
    //! set the string
    virtual void setString(const XbString &str);
#endif
};

//! The .str member in the EA3FieldData
class EA3FieldDataString {
public:
#ifndef SWIG
  //! cast
  operator const char *() const {
    return (str);
  }
  //! assignment
  const char *operator =(const char *s) {
    str = XbString(s).getString();
    return (str);
  }
  //! assignment
  const char *operator =(const XbString &s) {
    str = s.getString();
    return (str);
  }
  //! assignment
  const char *operator =(const XbIdentifier &s) {
    str = s.getString();
    return (str);
  }
  //! index
  char operator[](int ix) const {
    return str[ix];
  }
  //! equality
  bool operator==(const char *other) const {
    return(str==other);
  }
  //! equality
  bool operator==(const EA3FieldDataString &other) const {
    return(str==other.str);
  }
  //! equality
  bool operator==(const XbString &other) const {
    return(str==other.data);
  }
  //! inequality
  bool operator!=(const char *other) const {
    return(!(str==other));
  }
  //! inequality
  bool operator!=(const EA3FieldDataString &other) const {
    return(!(*this==other));
  }
  //! inequality
  bool operator!=(const XbString &other) const {
    return(!(str==other.data));
  }
#endif      // ifndef SWIG

private:
  const char *str;
};

//! A quoted string class.
/*!
  It converts to a single line when writing and converts back to original
  string when reading.
 */

class XbQString : public XbString {
  public:
    XbQString();
#ifndef SWIG
    XbQString(const XbString &str);
    XbQString(const char *str);
    XbQString(const EA3FieldDataString &str);
    friend XbQString EA3_EXPORT operator +(XbQString str1, const XbQString &str2);
    virtual char *read(char *is);
//    virtual istream& read( istream& is );
    virtual gzWriter& write( gzWriter& os ) const;
#endif
};

//! A class to hold a 32 bit Colour color.

class XbColour : public EA3Mem {
  public:
    //! constructor
    XbColour();
#ifndef SWIG
    //! constructor
    XbColour( unsigned int c );
    //! constructor
    XbColour(
        unsigned char r,
        unsigned char g,
        unsigned char b,
        unsigned char a );
#endif
    virtual char *read(char *is);
    virtual gzWriter& write( gzWriter& os ) const;
    //! get color
    unsigned int getColor() { return(col); }
    //! set color
    void setColor(unsigned int c) {col = c;}
    unsigned int col;
};

typedef enum {
  EA3OK = 0,
  EA3CANT_OPEN,
  EA3UNKNOWN_ITEMS,
  EA3UNKNOWN_TYPE,
  EA3MISSING_REFERENCE,
  EA3WRONG_TYPE,
  EA3SYNTAX_ERROR,
  EA3ERROR_WRITING,
  EA3BAD_HEADER
} EA3Status;

/*! 
  \brief convert EA3Status to a string
   
*/
inline const char *EA3Status2String(EA3Status s) {
  switch(s) {
    case EA3OK: return("OK");
    case EA3CANT_OPEN: return("CANT_OPEN");
    case EA3UNKNOWN_ITEMS: return("UNKNOWN_ITEMS");
    case EA3UNKNOWN_TYPE: return("UNKNOWN_TYPE");
    case EA3MISSING_REFERENCE: return("MISSING_REFERENCE");
    case EA3WRONG_TYPE: return("WRONG_TYPE");
    case EA3SYNTAX_ERROR: return("SYNTAX_ERROR");
    case EA3ERROR_WRITING: return("ERROR_WRITING");
    case EA3BAD_HEADER: return("BAD_HEADER");
  }
  return("UNITILIALIZED_STATUS");
}


class EA3GrammarTable;
//! An abstract base class for all EA3 classes.
/*!
   
  All objects derived from EA3Base can only be created by an 
  EA3Base::EA3Block (except an EA3Base::EA3Scene).
 */

class EA3Base : public EA3Mem {
  public:

#ifndef SWIG
    inline void *operator new(size_t, void *v) {
      return(v);
    }
    void *operator new(size_t s);
    void operator delete(void *v,size_t);
    inline void operator delete(void *, void *) {
    }
    inline void *operator new[](size_t ) {
      void *ptr = 0;
      return(ptr);
    }
    inline void operator delete[](void *) {
    }
#endif

#ifdef SWIG
    //! set the name
    virtual void setName( const char *n );
#else
    //! set the name
    virtual void setName(const XbString &n);
#endif
    //! get the name
    XbString getName() const;
    //! get the type (like material, diffuse, skeleton, etc.)
    XbIdentifier getType() const;

    //! Returns a unique integer ID for this object.
    intptr_t getUniqueId() const;
   
    //! Returns the upper most container block.
    EA3Scene *getScene() const;

    //! Returns the block that contains this object
    //! (every object except a scene is inside a container (EA3Block) )
    virtual EA3Block *getContainer() const = 0;

    //! Returns the objects index within its container (eg. the array index).
    int getIndex() const;

    //! Increases the reference count of an object, and returns it.
    int ref(); 
    //! get the reference count
    int refCount() const {return static_cast<int>(flags.refCount);}

    //! Decreases the reference count of an object.  
    /*! When refcount==0, object is deleted.  Returns 1 if object was deleted, else 0. */
    int unref(); 

    //! Decrease the reference count and returns the refcount.
    int unrefNoDelete();
    //! get the grammar block
    EA3Base *getGrammar() const;
    //! set the touched bit
    void touch();
    //! clear the touched bit
    void untouch();
    //! check if touched
    bool isTouched();

    //! Returns whether this object is of type EA3Array.
    virtual int isArray() const {return 0;}

    //! Returns whether this object is of type EA3Grammar.
    virtual int isGrammar() const;

    //! If the object is an EA3Grammar, returns the casted object.  Else returns NULL.
    EA3Grammar *toGrammar() const;

    //! Returns whether this object is of type EA3Block.
    virtual int isBlock() const {return 0;}

    //! Returns whether this object is of type EA3Field
    virtual int isField() const {return 0;}

    //! Returns whether this object is of type EA3Reference
    virtual int isReference() const {return 0;}

    //! Returns whether this object is of type EA3Array
    virtual EA3Array *toArray() const {return 0;}

    //! If the object is an EA3Block, returns the casted object. Else returns NULL.
    virtual EA3Block *toBlock() const {return 0;}

    //! If the object is an EA3Field, returns the casted object. Else returns NULL.
    virtual EA3Field *toField() const {return 0;}

    //! If the object is an EA3Reference, returns the casted object. Else returns NULL.
    virtual EA3Reference *toReference() const {return 0;}

    EA3BaseInfo *getInfo() const;
    //! return the class type as a string ie: "EA3Field"
    virtual const char* getClassType() const {return("EA3Base");}
    int typeIs(const char *dtype) const;
    //int hasUniqueTypeAndName() const;

    // IO methods
    gzWriter& writeRefName(gzWriter& os) const;
    //int hasRefName() const;
    gzWriter& writeIndent(gzWriter& os) const;
    virtual char *read(char *is) = 0;
    virtual gzWriter& write( gzWriter& os ) const = 0;
    virtual EA3Status writeBinary(EA3Chunk *) const = 0;
    virtual EA3Status readBinary(EA3Chunk *, int);
    //! make a copy of this in the new container
    virtual EA3Base *copy(const EA3Block *root, EA3Block *newcontainer) const = 0;

#ifndef SWIG
    typedef void EA3BaseDeleteCB(EA3Base *b);
    static void setDeleteCB(EA3BaseDeleteCB *cb);
    static EA3BaseDeleteCB *getDeleteCB();
    static EA3BaseDeleteCB *deleteCB;
#endif
    //! set user data (void *) 
    virtual void setUserData(void *ptr);
    //! get user data (void *)
    virtual void *getUserData() const;
    struct Flags {
      unsigned int num:7;
      unsigned int touched:1;
      unsigned int binaryDataField:1;
      unsigned int needsReindex:1;
      unsigned int internalFieldDataAlloc:1;
      unsigned int fieldHasExtraData:1;
      unsigned int refCount:20;
    };

  protected:

    //! Constructor .
    EA3Base(EA3Block *con, EA3Grammar *g,
      int ind, const XbIdentifier &t, const XbString &n);
    EA3Base() {  // Used by EA3Null
      info = NULL;
    }

    //! Destructor.
    virtual ~EA3Base();
    EA3Base *findByRefName(XbRefName *refName,const EA3Base *exclude) const;
    void resolveCopyReferences();
    void nullifyClonePointers() const;
    void setIndex(int i);
    void setRefId(int id);
    int getRefId() const;
    void clearRefId();
    EA3Status resolveReferences();
    void clearReferences();
    void setClone(EA3Base *c) const;
    EA3Base* getClone() const;

  private: 
    virtual int setContainer(EA3Block *g) = 0;
    Flags flags;
    //int index;

    static int id;
    static intptr_t firstBase;

    EA3BaseInfo *info;
//  XbRefName *refname;
//  mutable EA3Base *clone;
#ifndef SWIG
    EA3Base& operator=(const EA3Base &) { return(*this); }

    friend class EA3Field;
    friend class EA3Block;
    friend class EA3BaseInfoList;
    friend class EA3BaseInfo;
    friend class EA3BaseLRU;
    friend class EA3Reference;
    friend class EA3Grammar;
    friend class EA3BinaryReadWrite;
    friend int getIndex(EA3Block *con,const EA3Base *b);
#endif
};

//! An intermediate class for safely type casting pointers to EA3Base.
/*! Many methods that use to return EA3Base* now return an EA3Pointer.
The EA3Pointer safely casts itself to pointers to derived classes of EA3Base.
You should never need to work with this class directly, although you may want 
to write your functions to return them instead of EA3Base *.
For example:<br>
EA3Field *f = block->getByType("what");<br>
If the block finds an EA3Base* with type "what" it will return an EA3Pointer 
to hold this pointer.  The compiler then tries to convert an EA3Pointer to an
EA3Field *.  It can do this by calling operator EA3Field*().  This operator
safely converts the pointer to an EA3Field if and only if the pointer really
is an EA3Field *.  An EA3Pointer also has an -> operator.  Which returns the 
internal pointer if it is not NULL.  If it is null an EA3Null* is returned. 
The EA3Null is derived from an EA3Base and overrides the virtual functions
toBlock, toField, toArray, etc.  Which will all return NULL.  This lets you do 
the following:<BR>
EA3Field *f = model->getByType("animation")->toArray()->getByIndex(0)->getByType("fps");<br>
This is always safe now.  If model is null, the method getByType will return a null pointer inside
an EA3Pointer.  The -> operator before the toArray() will then be called on an EA3Null*, which 
will return an EA3Array* which is null.  getByIndex called with a null <b>this</b> pointer will return
null.  And so on...  the result is that f will be null.
*/

class EA3_EXPORT EA3Pointer
{
public:

    EA3Pointer(EA3Base *b) {
      ptr = b;
    }

#ifndef SWIG
    class EA3Null : public EA3Base {
    public:
      EA3Null() : EA3Base() {
      }
      virtual ~EA3Null() {
      }
      virtual int isArray() const {return 0;}
      virtual int isGrammar() const {return 0;}
      virtual int isBlock() const {return 0;}
      virtual int isField() const {return 0;}
      virtual EA3Array *toArray() const {return 0;}
      virtual EA3Block *toBlock() const {return 0;}
      virtual EA3Field *toField() const {return 0;}
      virtual EA3Reference *toReference() const {return 0;}
      virtual EA3Block *getContainer() const {return 0;}
    private:
      virtual char* read(char *) {return 0;}
      virtual gzWriter &write(gzWriter &gzw) const  {return gzw;}
      virtual EA3Status writeBinary(EA3Chunk *) const {
        return EA3OK;
      }
      virtual EA3Base *copy(const EA3Block *,EA3Block *) const {
        return(NULL);
      }
      
      virtual int setContainer(EA3Block *) {
        return(0);
      }

      EA3Null operator=(EA3Null &other) {
        return(other);
      }
    };

    inline operator bool() {
      return(ptr != NULL);
    }
    inline bool operator!() {
      return(ptr == NULL);
    }
    inline operator EA3Block *() {
      if (ptr) return(ptr->toBlock());
      return(NULL);
    }
    inline operator EA3Field *() {
      if (ptr) return(ptr->toField());
      return(NULL);
    }
    inline operator EA3Reference *() {
      if (ptr) return(ptr->toReference());
      return(NULL);
    }
    inline operator EA3Array *() {
      if (ptr) return(ptr->toArray());
      return(NULL);
    }
    inline operator EA3Base *() {
      return(ptr);
    }
    inline operator EA3Grammar *() {
      if (ptr) return(ptr->toGrammar());
      return(NULL);
    }
    inline EA3Base *operator ->() {
      if (!ptr) return(&ea3null);
      return(ptr);
    }

private:
    static EA3Null ea3null;
    EA3Base *ptr;
#endif    // ifndef SWIG
};

//! The union that is used by EA3Fields to hold data.
/*!
      A union of:<br>
        float f;<br>
        int i;<br>
        unsigned int ui;<br>
        EA3FieldDataString str;<br>
        char *bindata;<br>
     */
union EA3FieldData {
  float f;
  int i;
  unsigned int ui;
  EA3FieldDataString str;
  char* bindata;
};

/*!  An EA3 class that holds the actual data.
   
*/
class EA3Field : public EA3Base {

  public:

    //! Holds the data (use this to set and get values exactly)
    EA3FieldData *data;

    //! Returns the number of the items in data array.
    unsigned int getNum() const;

    //! Converts an element in the data array to a string.  
    /*!
      The user is responible for calling free on the string.
      \param index (in) index to an element in the data array to convert
      to a string.<br>
      You shoud use the data array directly when you can.<br>
      \code
      EA3Field *crd = coords->getByIndex(i);
      float x = crd->data[0].f;
      float y = crd->data[1].f;
      float z = crd->data[2].f;
      \endcode
     */
    char *get(int index) const;

    //! Sets the value(s) in the data array via a string.
    /*!
      For example,<br>
      a field of type "float" and size 3 could be set with the string
      "1.4 0.6 5.4".
      You should use the data array directly to store values when you can.<br>
      \code
      EA3Field *f=coords->createField("coord");
      f->data[0].f = 1.4f;
      f->data[1].f = 0.6f;
      f->data[2].f = 5.4f;
      \endcode
     */
    void set(const char *);
    //! get my container 
    virtual EA3Block *getContainer() const;

    virtual char *read(char *is);
    virtual gzWriter& write1( gzWriter& os, int index ) const;
    virtual gzWriter& write( gzWriter& os ) const;
    virtual EA3Status write1Binary(EA3Chunk *,int index) const;
    virtual EA3Status writeBinary(EA3Chunk *) const;
    virtual EA3Status readBinary(EA3Chunk *, int);
    //! make a copy of me in the new container.  Root is my container
    virtual EA3Base *copy(const EA3Block *root, EA3Block *newcontainer) const;
    virtual int isArray() const {return 0;}
    virtual int isBlock() const {return 0;}
    virtual int isField() const {return 1;}
    virtual int isReference() const {return 0;}
    virtual EA3Array *toArray() const {return 0;}
    virtual EA3Block *toBlock() const {return 0;}
    virtual EA3Field *toField() const {return(const_cast<EA3Field*>(this));}
    virtual EA3Reference *toReference() const {return 0;}


    //! Returns an char * that specifies the data type(s) of the field.
    /*!
      Example: "float", "int", "XbColour", "XbIdentifier", "XbQString"<br>
      <b>"f" 1 float, "ff" two floats, "fis" a float an int and a XbQString.</b><br>
      f = float, i = int, s = string, d = identifier, c = colour<br>
      This allows fields to contain multiple data types, for example a key in
      a curve could be ffffd (time, value, left slope, right slope,
      interpolation type).
    */
    const char *getDataType() const;

#ifndef SWIG
    //! returns a char to indicate the type of data at index i.
    /*! f=float, i=int, u=unsigned int, c=colour, s=string, d=identifier. 
    */
    char getDataType(int i) const;
#endif

    /*! see setExtraTypes
    */
    const char *getExtraTypes() const;

    /*! returns true on succcess.
     *  Used when you want to add extra data to a field.
     *  Stored after last data element, data[getNum()] is first extra data item
     *  You can not add binary types or add to a field that has a binary type 
     *  f=float, i=int, u=unsigned int, c=colour, s=string , n or d=identifier 
     *  (use n to give a name to the data).
     *  Example: to store foo = 9; bar = 5.0f;
     *  \code 
     *  int num = fld->getNum();
     *  fld->setExtraTypes("ninf");
     *  fld->data[num+0].str = "foo";
     *  fld->data[num+1].i = 9;
     *  fld->data[num+2].str = "bar";
     *  fld->data[num+3].f = 5.0f;
     *  \endcode
    */
    bool setExtraTypes(const char *types); 
    //! appends extra types.  see setExtraTypes
    bool appendExtraTypes( const char* );

    //! returns the number of extra types
    unsigned int getNumExtraTypes() const;

    virtual const char* getClassType() const {return("EA3Field");}

    template<typename T>
    void CopyFrom(const T* source, size_t size, size_t offset)
    {
        size /= 4; // assuming 32-bit data
        const uint32_t* source32 = reinterpret_cast<const uint32_t*>(source);
        for (size_t i = 0; i < size; ++i)
        {
            data[i + offset].ui = source32[i];
        }
    }

    template<typename T>
    void CopyFrom(const T* source, size_t size)
    {
        CopyFrom(source, size, 0);
    }

    template<typename T>
    void CopyTo(T* dest, size_t size, size_t offset)
    {
        size /= 4; // assuming 32-bit data
        uint32_t* dest32 = reinterpret_cast<uint32_t*>(dest);
        for (size_t i = 0; i < size; ++i)
        {
            dest32[i] = data[i + offset].ui;
        }
    }

    template<typename T>
    void CopyTo(T* dest, size_t size)
    {
        CopyTo(dest, size, 0);
    }

#ifndef SWIG
  protected:
    //! Constructor.
    /*! Can only be created in a block. */
    EA3Field(
        const XbIdentifier &dt,
        int nb,
        EA3Block *con,
        EA3Grammar *g,
        int index,
        const XbIdentifier &t,
        const XbString &nm );

    ~EA3Field();
    friend class EA3Block;
    friend int getIndex(EA3Block *con,const EA3Base *b);

  private:
    virtual int setContainer(EA3Block *g);
    EA3Field &operator=(EA3Field &other) {
      if (getNum() != other.getNum()) return(*this);
      int num = static_cast<int>(getNum());
      for(int i=0;i<num;i++) {
        data[i].i = other.data[i].i;
      }
      return(*this); 
    }
    int index;
#endif
};


class EA3ConstIterator;
class EA3Iterator;
//! A class that contains a list of pointers to EA3Base derived objects.
/*!
 EXAMPLE<br>
 \code
    EA3Block *poly = mesh->getByType("poly");
    EA3Reference *crd = poly->getByType("coord");
    int *coordindex = new int[crd->getNumReferences()];

    for(int i = 0; i < crd->getNumReferences() ; i++) {
      coordindex[i] = crd->getReference(i)->getIndex();
    }

    EA3Reference *mtlref = poly->getByType("material");
    EA3Block *mtl = mtlref->getReference(0);
 \endcode
 */

class EA3Reference : public EA3Base {
  public:

    EA3Pointer getReference(int ix) const;

    //! Replaces all occurances of (o) with (n).
    int replaceReference(EA3Base *o, EA3Base *n);
    //! get the number of references
    int getNumReferences() const;
    //! set by index
    int setReference(int index, EA3Base *n);
    //! insert by index
    int insertReference(int index,EA3Base *b);

    //! Appends a node to the reference list.
    /*!
      If <I>check</I> == 1, then it checks to see if the node is already in
      the list. If it is, nothing is done.
     */
    int addReference(EA3Base *b,int check = 0);

#ifndef SWIG
    //! Removes all occurences of b from the list (return number removed).
    int removeReference(EA3Base *b);
#endif

    //! Removes reference by index (return 1 if removed, 0 otherwise)
    int removeReference(int index);

    //! Empties the list.
    void removeAllReferences();

#ifndef SWIG
    //! Finds all indices in the list that match the reference.
    int find(const EA3Base *b, XbPArray<int> &inlist) const;
#endif
    //! get my container
    virtual EA3Block *getContainer() const;
    
    virtual char *read(char *is);
    virtual char *readAppend( char *is );
    virtual char *readRef( char *is, int append );
    virtual gzWriter& write( gzWriter& os ) const;
    virtual EA3Status writeBinary(EA3Chunk *) const;
    virtual EA3Status readBinary(EA3Chunk *, int);
    //! create a copy of me in the newcontainer.  root is my container.
    virtual EA3Base *copy(const EA3Block *root, EA3Block *newcontainer) const;
    void setCurRefName(XbRefName *rn);
    virtual int isArray() const {return 0;}
    virtual int isBlock() const {return 0;}
    virtual int isField() const {return 0;}
    virtual int isReference() const {return 1;}
    virtual EA3Array *toArray() const {return 0;}
    virtual EA3Block *toBlock() const {return 0;}
    virtual EA3Field *toField() const {return 0;}
    virtual EA3Reference *toReference() const {return const_cast<EA3Reference *>(this);}
    void addRefName(XbRefName *r);
    virtual const char* getClassType() const {return("EA3Reference");}

	// returns an iterator to the references in this EA3Reference
	virtual EA3Iterator begin() const;
	virtual EA3ConstIterator beginConst() const;


#ifndef SWIG
    EA3Base *operator[](int index);
    static EA3BaseLRU LRU;

  protected:

    //!0 Contructor. Can only be created from an EA3Block.
    EA3Reference(EA3Block *con,EA3Grammar *g,int index,const XbIdentifier &t,const XbString &nm);
    ~EA3Reference();

  private:
    virtual int setContainer(EA3Block *g);
    gzWriter& writeBlockRef(gzWriter& os) const;
    gzWriter& writeNormalRef(gzWriter& os) const;
    EA3Status resolveRefNames();
    void clearRefNames();
    EA3Base **members;
    EA3Block *container;
    //XbPArray<XbRefName *> refnames;
    friend class EA3Block;
    friend class EA3Base;
    friend class EA3BinaryReadWrite;
#endif
};

//!  A class that holds an EA3 object.
/*!
  It contains an array of members, which can be another EA3Block, an EA3Array,
  an EA3Field, or EA3Reference.
 */ 
class EA3Block : public EA3Base {
  public:
    //! get the number of members
    int getNumMembers() const;

    /*!
      \brief Returns a member in this EA3Block.
      Gets the first member that matches the type, the name and the dtype 
      The dtype is one of "EA3Block", "EA3Array", "EA3Reference" or "EA3Field" 
      If dtype == NULL, (the default) then the first match of type and name 
      is returned.
     */
#ifdef SWIG
    EA3Pointer get( const char *ftype, const char *fname, char *dtype=NULL ) const;
#else
    EA3Pointer get(const XbIdentifier &ftype,const XbString &fname,char *dtype = NULL) const;
#endif

    //! get a member by name
    /*!
       If dtype is given, it should be one of match the name of an EA3 class derived from EA3Base.
       example:  \code getByName("Bob","EA3Block"); \endcode
       It should rarely be necessary to specify dtype.
       \sa EA3Block::find, EA3Expr::Expression
    */
#ifdef SWIG
    EA3Pointer getByName( const char *fname, char *dtype=NULL ) const;
#else
    EA3Pointer getByName(const XbString &fname, char *dtype = NULL) const;
#endif

    //! get a member by type
    /*!
       If dtype is given, it should be one of match the name of an EA3 class derived from EA3Base.
       example:  \code getByType("coord","EA3Field"); \endcode
       It should rarely be necessary to specify dtype.
       \sa EA3Block::find, EA3Expr::Expression
    */
#ifdef SWIG
    EA3Pointer getByType( const char *ftype, char *dtype=NULL ) const;
#else
    EA3Pointer getByType(const XbIdentifier &ftype, char *dtype = NULL) const;
#endif

    //! get a member by index
    EA3Pointer getByIndex(int ix) const;

    //! Like EA3Block::get() but searches recursively when a member is another EA3Block.
#ifdef SWIG
    EA3Pointer find( const char *ftype, const char *fname, char *dtype=NULL ) const;
    EA3Pointer findByName( const char *fname, char *dtype=NULL ) const;
    EA3Pointer findByType( const char *ftype, char *dtype=NULL ) const;
#else
    EA3Pointer find(
	    const XbIdentifier &ftype,
	    const XbString &fname,
	    char *dtype = NULL) const;
    //! Like EA3Block::get() but searches recursively when a member is another EA3Block.
    EA3Pointer findByName(const XbString &fname,char *dtype = NULL) const;
    //! Like EA3Block::get() but searches recursively when a member is another EA3Block.
    EA3Pointer findByType(const XbIdentifier &ftype, char *dtype = NULL) const;
#endif

    /*!
      Creates an EA3Reference member and appends it to the array
      of members.  If the grammar does not specify that this block can
      have a reference with the given type then an assertion will fail.
     */ 
#ifdef SWIG
    EA3Reference *createReference( const char *t, const char *nm = "",bool isGram = false  );
#else
    EA3Reference *createReference( const XbIdentifier &t, const XbString &nm = "",bool isGram = false );
#endif

    /*!
      Creates an EA3Block member and appends it to the array
      of members. If the grammar does not specify that this block can
      have a reference with the given type then null is returned.
     */ 
#ifdef SWIG
    EA3Block *createBlock( const char *t, const char *nm = "",bool isGram = false );
#else
    EA3Block *createBlock( const XbIdentifier &t, const XbString &nm = "",bool isGram = false  );
#endif

    /*!
      Creates a EA3Array member and appends it to the array
      of members. If the grammar does not specify that this block can
      have a reference with the given type then null is returned.
     */ 
#ifdef SWIG
    EA3Array *createArray( const char *t, const char *nm = "",bool isGram = false );
#else
    EA3Array *createArray(const XbIdentifier &t, const XbString &nm = "",bool isGram = false );
#endif

    /*!
      Creates a EA3Field member and appends it to the array
      of members. If the grammar does not specify that this block can
      have a reference with the given type null is returned.
     */ 
#ifdef SWIG
    EA3Field *createField( const char *t, const char *nm="",bool isGram = false  );
#else
    EA3Field *createField(const XbIdentifier &t,const XbString &nm = "",bool isGram = false );
#endif
    /*!
      It is undefined if you move elements from one EA3Scene to another.
      I could have made it impossible, but for effiency, it is not enforced.
      For the same reason, it is possible to move members to places that
      the grammar does not define.  Again, it is too expensive to check.
    */
    int setMember(int index,EA3Base *b);
    /*!
    It is undefined if you move elements from one EA3Scene to another.
    I could have made it impossible, but for effiency, it is not enforced.
    For the same reason, it is possible to move members to places that
    the grammar does not define.  Again, it is too expensive to check.
    */
    int insertMember(int index,EA3Base *b);
    /*!
    It is undefined if you move elements from one EA3Scene to another.
    I could have made it impossible, but for effiency, it is not enforced.
    For the same reason, it is possible to move members to places that
    the grammar does not define.  Again, it is too expensive to check.
    */
    int appendMember(EA3Base *b);
    //! remove a member - call ref() if you want a handle to it.  Otherwise, removeMember may cause it to be deleted
    int removeMember(int index);

    /*!
      Swap two members given by their indices within this EA3Block.
     */
    int swapMembers( int m1, int m2 );

    int setNum(int num);
    //! remove all members

    int removeAllMembers();
#ifndef SWIG
    //! remove specific members
    int removeMembers(const XbPArray<EA3Base *> &baseList,bool keep=false);
    int removeMember(EA3Base *b);
#endif
    int memberIndex(EA3Base *b);

#ifndef SWIG
    //! sort the members - you provide callback
    void sortMembers( int (*keyCompare)( EA3Base **, EA3Base ** ) );
#endif

	//! get an iterator for the members of this block.  iterator starts at the first child.  
	EA3Iterator      begin() const;
	EA3ConstIterator beginConst() const;

    virtual const char* getClassType() const {return("EA3Block");}

/*
    int canAddBlock() const {
      if (!isArray()) return(1);
      if (getNumMembers() == 0) return(1);
      EA3Base *b = members[0];
      if (t->isField()) {
        if(b->isField()) return(1);
        return(0);
      }
      if (t->isReference()) {
        if(b->isReference()) return(1);
        return(0);
      }
      if (t->isArray()) {
        if(b->isArray()) return(1);
        return(0);
      }
      if (t->isBlock()) {
        if (b->isBlock() && !b->isArray()) return(1);
        return(0);
      }
      return(0);
    }
#define EA3CanAddType(a,b) a->canAddType((b*)0)
*/
    virtual int isArray() const {return 0;}
    virtual int isBlock() const {return 1;}
    virtual int isField() const {return 0;}
    virtual int isReference() const {return 0;}
    virtual EA3Array *toArray() const {return 0;}
    virtual EA3Block *toBlock() const {return const_cast<EA3Block *>(this);}
    virtual EA3Field *toField() const {return 0;}
    virtual EA3Reference *toReference() const {return 0;}
    void real_reindex();
    void setInherits(const XbIdentifier &in);
    XbIdentifier getInherits() const;
    virtual EA3Base *copy(EA3Block *newcontainer) const {
      return(copy(this,newcontainer));
    }
    //! parse a string formated like an EA3 ASCII file.
    virtual EA3Status parse(const char *is);
    virtual gzWriter& write( gzWriter& os ) const;
    virtual EA3Status writeBinary(EA3Chunk *) const;
    virtual EA3Status readBinary(EA3Chunk *, int isGram);
    virtual EA3Block *getContainer() const;
    
#ifndef SWIG
  protected:
    virtual char *read(char *is );
    EA3Reference *realCreateReference(
	    const EA3Base *g,const XbIdentifier &t,const XbString &nm,int index);
    EA3Block *realCreateBlock(
	    const EA3Base *g,const XbIdentifier &t, const XbString &nm,int index);
    EA3Array *realCreateArray(
	    const EA3Base *g,const XbIdentifier &t, const XbString &nm,int index);
    EA3Field *realCreateField(
	    const EA3Base *g,const XbIdentifier &baseType,const XbIdentifier &t,const XbString &nm,int index);
    void reindex() {flags.needsReindex = 1;}
    virtual char *readMembers(char *is, int isGrammar);
    virtual EA3Base *copy(const EA3Block *root, EA3Block *newcontainer) const;

    //! Constructor.
    EA3Block(
	    EA3Block *con,EA3Grammar *g,int index,
	    const XbIdentifier &t, const XbString &nm);
    ~EA3Block();

  private:
    virtual int setContainer(EA3Block *g);
    EA3Base **members;
    EA3Block *container;

    friend class EA3Reference;
    //friend class EA3Field;
    friend class EA3Grammar;
    friend class EA3Array;
    friend class EA3Scene;
    friend class EA3BinaryReadWrite;
    friend int getIndex(EA3Block *con,const EA3Base *b);
#endif
};

//! A class for defining the EA3 grammar.

class EA3Grammar : public EA3Block {
  public:
    EA3Field *createFieldInstance(EA3Block *par,const XbIdentifier &type,const XbString &name, int isGrammar,int index=-1) const;
    EA3Block *createBlockInstance(EA3Block *par,const XbIdentifier &type,const XbString &name, int isGrammar,int index=-1) const;
    EA3Reference *createReferenceInstance(EA3Block *par,const XbIdentifier &type,const XbString &name, int isGrammar,int index=-1) const;
    EA3Array *createArrayInstance(EA3Block *par,const XbIdentifier &type,const XbString &name, int isGrammar,int index=-1) const;

    virtual int isArray() const {return 0;}
    virtual int isBlock() const {return 1;}
    virtual int isGrammar() const {return 1;}
    virtual int isField() const {return 0;}
    virtual int isReference() const {return 0;}

    virtual EA3Array *toArray() const {return 0;}
    virtual EA3Block *toBlock() const {return const_cast<EA3Block *>(static_cast<const EA3Block *>(this));}
    virtual EA3Field *toField() const {return 0;}
    virtual EA3Reference *toReference() const {return 0;}
    virtual const char* getClassType() const {return("EA3Grammar");}

#ifndef SWIG
  protected:
 
    //! Constructor.
    EA3Grammar(
	    EA3Block* con, int ind,const XbIdentifier &t, const XbString &nm);
    ~EA3Grammar();
    mutable EA3GrammarTable *gt;
    friend class EA3Scene;
#endif
};

//! An EA3 array class.
/*!
  An EA3Array is just like an EA3Base::EA3Block except that all it may have
  duplicate members of the same type.
*/

class EA3Array : public EA3Block {
  public:
    virtual int isArray() const {return 1;}
    virtual int isBlock() const {return 1;}
    virtual int isField() const {return 0;}
    virtual int isReference() const {return 0;}
    virtual EA3Array *toArray() const {return const_cast<EA3Array *>(this);}
    virtual EA3Block *toBlock() const {return const_cast<EA3Block *>(static_cast<const EA3Block *>(this));}
    virtual EA3Field *toField() const {return 0;}
    virtual EA3Reference *toReference() const {return 0;}
    virtual const char* getClassType() const {return("EA3Array");}

#ifndef SWIG
    friend class EA3Block;
#endif

  protected:

    //! Constructor.
    EA3Array(
	    EA3Block *con,EA3Grammar *g,int index,
	    const XbIdentifier &t,const XbString &nm);
 
    //! Destructor.
    ~EA3Array();
};
class EA3GrammarTable;

//! An EA3 scene class.
/*!
  An EA3Scene is what you need to start doing anything with EA3.
 
  EXAMPLE
  \code
    int main(int argc, char **argv) {
      if (argc != 3) {
        fprintf(stderr,"usage: ea3parser <infile> <outfile>\n");
      }
      EA3Scene scene;
      if (scene.readFile(argv[1])!=0) {
        fprintf(stderr,"error reading %s\n",argv[1]);
        return(-1);
      }
      EA3Base *b;
      int i;
      for(i=0;i<scene.getNumMembers();i++) {
        b = scene.getByIndex(i);
        if (b->getType() == "model") {
          parseModel(b->toBlock());
        }
      }
      return(1);
    }
  \endcode
 */


class EA3Scene : public EA3Block {
  public:
    //! constructor.  
    EA3Scene();
    //! destructor 
    /*! 
       When a scene is deleted, all nodes will be deleted (even if they
       have been removed from the scene.  Reference counts are ignored.
    */
    ~EA3Scene();

    //! Reads a file from disk based on a filename.
    /*!
      File can be compressed or not. Files can be compressed via the
      write methods or by using gzip on the disk file.
     */
    EA3Status readFile(const char *filename);

    EA3Status readFile(char *buffer, int size);

    //! Writes a file to disk based on a filename.
    /*!
      if compress==1 then file will be saved compressed via a 
      gzip (zlib) compressor.  Files can be viewed by piping thru gzcat.
      \retval 0 on success
      \retval -1 on open failure
      \retval -2 on write failure
     */
    EA3Status writeFile(const char *filename,int compress);
    //! write our a binary file
    EA3Status writeBinaryFile(const char *filename,int compress) const;

    //! Write a file to a file descriptor (eg. fileno(stdout))
    //EA3Status writeFD(int fd,int compress);

//    virtual istream& readStream( istream& is );
    virtual gzWriter& write( gzWriter& os ) const;

    //!  Appends information to the history block in the EA3 file.
    /*!
      This command automatically fills in the "user" and "date" 
      fields of the history block.
     */
    void appendHistory(const XbString &toolName,const XbString &comment);

    static int lineCheck(char *line);
    EA3Grammar *newGrammar();

    //! Use one of the precompiled grammars by index.
    /*!
      You cannot change the grammar if the scene has data in it.
      \return 1 on success
     */
    int useGrammar(int index);

    int getGrammarIndex() const {return grammarIndex;}
    void grammarModified();
    void rebuildGrammarTable();
    EA3Grammar *findGrammar(
	    const XbIdentifier &type,const XbIdentifier &childType,
	    bool is_array) const;

#ifndef SWIG
    /*!
      typedef void ErrorCallback(char *msg);<br>
      Set this to point to a function that will be called when errors occur 
      (like read errors).
     */
    typedef void ErrorCallback(char *msg);
    static ErrorCallback *errorCallback;

    /*!
      typedef void ProgressCallback(int which, int numDone, int total);<br>
      Set this to point to a function that will be called when:<br>
      which == 1 Each line of the file is read.<br>
        \param which (in) 1 when each line of the file is read;
	2 when references are being resolved.
        \param numDone (in) is bytes into file when which==1;
	# references currently resolved when which==2.
	\param total (in) is fileSize when which==1;
	# references in scene when which==2.

	If you call readStream() or readFD(), you are responsible for
	initializing fileSize and filePtr. Set fileSize to -1 to get no
	progress.  (ex: reading from a string)
        Calling readFile() will initialize fileSize and filePtr.
     */
    typedef void ProgressCallback(intptr_t which, int numDone, int total);
    static ProgressCallback* progressCallback;
    void getUncontained(XbPArray<EA3Base *> &unconList) const;
#endif

    virtual const char* getClassType() const {return("EA3Scene");}
    int fileSize;
    FILE *filePtr;

    virtual int isArray() const {return 0;}
    virtual int isBlock() const {return 1;}
    virtual int isGrammar() const {return 0;}
    virtual int isField() const {return 0;}
    virtual int isReference() const {return 0;}

    virtual EA3Array *toArray() const {return 0;}
    virtual EA3Block *toBlock() const
    	{return const_cast<EA3Block *>(static_cast<const EA3Block *>(this));}
    virtual EA3Field *toField() const {return 0;}
    virtual EA3Reference *toReference() const {return 0;}

    char *getLine();
    bool lastReadWasBinary() {return wasBinary;}
    bool lastReadWasCompressed() {return wasCompressed;}


#ifndef SWIG
    static int lineNum;
    static char errorData[MAX_LINE_LEN];
    static EA3Status readStatus;

    // Used for progress callback
    static int numReferences;
    static int numResReferences;

    friend class EA3Block;
    friend class EA3Base;
    friend class EA3Reference;
    friend class EA3Field;
    friend class EA3Grammar;

  private:
	//! Read a file from file descriptor.
    EA3Status readInternal(const char* filename);
    EA3Status readInternal(char* buffer, int size);

    EA3GrammarTable *grammarTable;
    EA3Status readAll();
    //istream *sceneIs;
    gzReader *gzr;
    //istream *_is;
    XbString version;
    int grammarIndex;
    bool wasBinary;
    bool wasCompressed;
    // used to keep track of elements with no container.  
    // They will get deleted when the scene is deleted.
    void *uncontained; 
#endif
};

class EA3BaseInfo : public EA3Mem {
// Data that can be shared by multiple EA3Base's
// (Attempt to make EA3Base lighter)
// For example, most items in an array can probably share the 
// same EA3BaseInfo
  public:
    EA3BaseInfo(EA3Block *con,EA3Base *g, const char *t,const char *n);
    inline EA3Scene *getScene() {
      EA3Block *b = grammar->getContainer();
      if (!b) {
        return((EA3Scene*)const_cast<EA3Base *>(grammar));
      }
      EA3Block *con=b;
      while(b) {
        con = b;
        b=b->getContainer();
      }
      return((EA3Scene*)con);
    }
    EA3Block *const container;
    EA3Base *const grammar;
    const char *type;
    const char *name;
    void ref();
    void unref();

    inline void *operator new(size_t s);
    inline void operator delete(void *v);

  private:
    int refCount;
    EA3BaseInfo &operator=(EA3BaseInfo &) {return *this;}
};

void EA3_EXPORT EA3_cleanUp();

class EA3IteratorTable {
public:
	inline EA3IteratorTable() : _iTable(500), s(0) {};
	inline ~EA3IteratorTable(){};

	inline void updateIterators(EA3Base** oldStart, EA3Base** oldEnd, EA3Base** newStart, EA3Base** newEnd ) const;

	inline void addIterator( EA3Iterator* it );

	inline void removeIterator( EA3Iterator* it );

	void dumpTable() const;

	inline void clear() { _iTable.clear(); };

	inline int size() { return _iTable.getSize(); };

private:
	XbPArray<EA3Iterator*> _iTable;
	EA3Iterator* _a[100];
	int s;
};

extern EA3_EXPORT EA3IteratorTable masterTable;

class EA3ConstIterator {
friend class EA3IteratorTable;
friend class EA3Block;
friend class EA3Reference;
friend class EA3Iterator;
public:
	inline ~EA3ConstIterator() { };
	
	// pre-increments the iterator to the next EA3Pointer
	inline EA3ConstIterator& operator++() { 
		_currPtr = ((EA3Base**)_currPtr) + 1;
		return *this;
	};

	// dereferences the iterator, returns an EA3Pointer object that the iterator
	// currently points to
	inline EA3Pointer operator*() {
		return EA3Pointer(*_currPtr);          
	};

	// dereferences the iterator, allows for 
	// syntax such as: EA3Field* f = iterator->getByType(Xbcoord); 
	inline EA3Base* operator->() {
		return *_currPtr;		
	};

	inline bool operator !=(EA3Base** other) { 
		return this->_currPtr != other; 
	};

	inline bool operator !=(const EA3ConstIterator& other) {
		return this->_currPtr != other._currPtr;  
	};

	inline bool operator ==(const EA3ConstIterator& other) {
		return this->_currPtr == other._currPtr;
	};

	inline bool operator ==(EA3Base** other) {
		return this->_currPtr == other;
	};

	inline EA3Base** end() {
		return _endPtr;
	};

	inline EA3ConstIterator( const EA3ConstIterator& other ) { 
		_currPtr = other._currPtr;
	};

	inline EA3ConstIterator& operator=(const EA3ConstIterator& other) 
	{ 
		_currPtr = other._currPtr;
		_endPtr = other._endPtr;
		return *this;
	};

private:
	// pointers to the current EA3Base, and last EA3Base in the array.
	EA3Base** _currPtr;
	EA3Base** _endPtr;

	// can't create iterators by yourself
	inline EA3ConstIterator() :
		_currPtr(NULL),
		_endPtr(NULL)
	{};
	
	inline EA3ConstIterator( EA3Base** currPtr, EA3Base** endPtr ) : 
		_currPtr(currPtr),
		_endPtr(endPtr)
	{};	
};

class EA3Iterator {
friend class EA3IteratorTable;
friend class EA3Block;
friend class EA3Reference;
friend class EA3ConstIterator;
public:
	inline ~EA3Iterator() { 
		masterTable.removeIterator(this); 
	};
	
	// pre-increments the iterator to the next EA3Pointer
	inline EA3Iterator& operator++() { 
		_currPtr = ((EA3Base**)_currPtr) + 1;
		return *this;
	};

	// dereferences the iterator, returns an EA3Pointer object that the iterator
	// currently points to
	inline EA3Base* operator*() {
		return *_currPtr;          
	};

	// dereferences the iterator, allows for 
	// syntax such as: EA3Field* f = iterator->getByType(Xbcoord); 
	inline EA3Base* operator->() {
		return *_currPtr;		
	};

	inline bool operator !=(EA3Base** other) { 
		return this->_currPtr != other; 
	};

	inline bool operator !=( EA3Iterator& other) {
		return this->_currPtr != other._currPtr;  
	};

	inline bool operator ==(const EA3Iterator& other) {
		return this->_currPtr == other._currPtr;
	};

	inline bool operator ==(EA3Base** other) {
		return this->_currPtr == other;
	};

	inline EA3Base** end() {
		return _endPtr;
	};

	EA3Iterator& operator=( EA3Iterator& other) 
	{ 
		_currPtr = other._currPtr;
		_endPtr = other._endPtr;
		return *this;
	};

	inline EA3Iterator( const EA3Iterator& other ) { 
		_currPtr = other._currPtr;
		masterTable.addIterator( this );  
	};

private:
	// pointers to the current EA3Base, and last EA3Base in the array.
	EA3Base** _currPtr;
	EA3Base** _endPtr;

	// can't create iterators by yourself
	inline EA3Iterator() :
		_currPtr(NULL),
		_endPtr(NULL)
	{};
	
	inline EA3Iterator( EA3Base** currPtr, EA3Base** endPtr ) : 
		_currPtr(currPtr),
		_endPtr(endPtr)
	{ 
		masterTable.addIterator( this );  
	};
};

inline void EA3IteratorTable::addIterator( EA3Iterator* it ) {
	_iTable.append(it);
};

inline void EA3IteratorTable::removeIterator( EA3Iterator* it ) {
	int _size = _iTable.getSize() - 1;
	if(_iTable[_size] == it) {
		_iTable.remove(_size); //playing on the odds that iterators get added/removed in a stack order...
		return;
	}

	for(int i = _size - 1; i > -1; --i) {
		if( _iTable[i] == it ) {
			_iTable.remove(i);
			return;
		}
	} 
	
	return;
};

inline void EA3IteratorTable::updateIterators(EA3Base** oldStart, EA3Base** oldEnd, EA3Base** newStart, EA3Base** newEnd ) const
{
	for (int i = _iTable.getSize() - 1; i > -1; --i)
    {
		if( _iTable[i]->_endPtr == oldEnd )
        { 
				// found a match 

				// figure out the old offset
			 int offset = static_cast<int>(_iTable[i]->_currPtr - oldStart );

				newStart + offset >= newEnd ?                   // make sure dont go out of bounds. 
					_iTable[i]->_currPtr = newEnd :
					_iTable[i]->_currPtr = newStart + offset;

				_iTable[i]->_endPtr = newEnd;
		}
	}

	return;
};

// dump the contents of the iterator table
inline void EA3IteratorTable::dumpTable() const 
{
	for (int i = 0; i < _iTable.getSize(); ++i)
    {
		printf( "%i: %p %p\n", i, (void *)_iTable[i]->_currPtr, (void *)_iTable[i]->_endPtr);
	}
	return;
}



#ifndef SWIG
inline gzWriter& operator<<( gzWriter& os, const XbString& x ) { return x.write( os ); }
inline gzWriter& operator<<( gzWriter& os, const XbQString& x ) { return x.write( os ); }
inline gzWriter& operator<<( gzWriter& os, const XbIdentifier& x ) { return x.write( os ); }
inline gzWriter& operator<<( gzWriter& os, const EA3Base& x ) { return x.write( os ); }
inline gzWriter& operator<<( gzWriter& os, const XbColour& x ) { return x.write( os ); }


typedef void EA3SetSelectCB(const EA3Base *b,bool state);
typedef bool EA3IsSelectedCB(const EA3Base *b);
typedef void EA3GetSelectedCB(XbPArray<EA3Base *> &inlist);

extern EA3_EXPORT EA3SetSelectCB *ea3setSelectCB;
extern EA3_EXPORT EA3IsSelectedCB *ea3isSelectedCB;
extern EA3_EXPORT EA3GetSelectedCB *ea3getSelectedCB;

EA3_EXPORT void EA3SetSelect(const EA3Base *b,bool state);
EA3_EXPORT bool EA3IsSelected(const EA3Base *b);
EA3_EXPORT void EA3GetSelected(XbPArray<EA3Base *> &sel);

EA3_EXPORT void copyFieldValues(const EA3Field *src, EA3Field *dst);
EA3_EXPORT int getStringHashMemSize();
EA3_EXPORT int getStringHashSize();
#endif      // ifndef SWIG

// MB: moved from EA3.cpp and EA3Chunk.cpp

#include <eastl/hash_map.h>
using namespace eastl;

template <class TYPE>
struct myhash : public unary_function<TYPE,size_t>  {
    size_t operator()(TYPE b) const {
        return((size_t)b);
    }
};
typedef hash_map< const char *,unsigned int,myhash<const char *>,equal_to<const char *>, StlAllocator > StringHash;


#endif   // ifndef EA3_H
