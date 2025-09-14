// (c) 2000-2007 Electronic Arts Inc.
#ifndef _LBTYPES_H
#define _LBTYPES_H

//***************************************************************************
/* Standard types : Base types

	These following are #defines rather than typedefs so that the compiler 
	can convert freely between them. Even though the ANSI C++ specification
	says that all typedefs should be treated as their basde types, at the
	time of going to press, Microsoft Visual C++ did not do this with
	templates.
	
	Upon examination, it should be noted that these #defines behave
	EXACTLY as would corresponding typedefs.

Base types for particular attention:

TBOOL : Always a TRUE or FALSE value.

CHAR : The type used for textual strings. The compiler will always	evaluate 
	code such as: "Hello" to a const CHAR * for example. It must be noted
	also, that for international language support this may not be a single
	byte, but a word.

UINT and int : Guaranteed to be at least 32 bits in size. This type
	is the natural size for the compiled processor. For example, under
	32-bit DOS (DOS4gw) it would be a 32 bits, Windows Win32 (80386) it 
	would be 32 bits.
*/
//***************************************************************************
#ifndef __cplusplus
	#error "This library is designed for use with a C++ compiler"
#endif

// When this is defined, the headers used to change packing will also disable the warning.
// comment this out to help find where packing is going wrong.
#define _LB_DISABLE_PACK_WARNING

// Unicode Stuff.
// To be used when we go over to NT
// typedef short SHORT;
// typedef int  LONG; 
//
//#ifdef _UNICODE
//	#ifndef UNICODE
//		#define UNICODE
//	#endif
//#endif
//
//#ifdef UNICODE
//	#ifndef _UNICODE
//		#define _UNICODE
//	#endif
//#endif
//
// Include tchar.h to give unicode compatability if required.
//#include <tchar.h>

// is this an mfc build?
#if defined(__AFX_H__)
	#if !defined(__MFC__)
		#define __MFC__
	#endif
#endif

#ifndef __MFC__
#ifndef VOID
	// <t>>VOID<<
	typedef void		VOID;	// VOID #defined to void in MFC
//	#define 	VOID	void	// VOID #defined to void in MFC
#endif
#endif

// Unicode Stuff.
// To be used when we go over to NT
 typedef short SHORT;
 typedef int  LONG; 

//
//#ifdef _UNICODE
//	#ifndef UNICODE
//		#define UNICODE
//	#endif
//#endif
//
//#ifdef UNICODE
//	#ifndef _UNICODE
//		#define _UNICODE
//	#endif
//#endif
//
// Include tchar.h to give unicode compatability if required.

#include <tchar.h>

//#define _LB_UNICODE
//#ifdef _LB_UNICODE
//	#define _LBSTR(x)	_T(x)
//#else
	#define _LBSTR(x)	x
//#endif

//
// Bullfrog types
//
// <t>>UINT<<
typedef unsigned int	UINT;
// <t>>SINT<<
typedef int				SINT;

// <t>>SLONG<<
typedef signed int		SLONG;
// <t>>ULONG<<
typedef unsigned int	ULONG;

// <t>>SWORD<<
typedef signed short	SWORD;
// <t>>UWORD<<
typedef unsigned short	UWORD;

// <t>>SBYTE<<
typedef signed char		SBYTE;
// <t>>UBYTE<<
typedef	unsigned char	UBYTE;

//#ifdef _UNICODE
//	typedef	TCHAR	TBCHAR;
//	typedef char	CHAR;
//#else
// <t>>CHAR<<
typedef	char			TBCHAR;
// <t>>TBCHAR<<
typedef TBCHAR			CHAR;
//#endif

// <t>>UNICHAR<<
typedef UWORD			UNICHAR;


// types for pointing to const strings
typedef const TBCHAR*	LPCTBCHAR;
typedef const CHAR*		LPCCHAR;
typedef const UNICHAR*	LPCUNICHAR;


// <t>>BOOL<<
typedef int	BOOL;

#ifndef NULL
	// <t>>NULL<<
	#define NULL 0
#endif

#ifndef TRUE
	// <t>>TRUE<<
	#define TRUE 1
#endif

#ifndef FALSE
	// <t>>FALSE<<
	#define FALSE 0
#endif

// <t>>TbHandle<<
typedef void*			TbHandle;

//	Watcom and Windows, make sure all the defines we expect are set
//***************************************************************************
#if defined(_MSC_VER) && defined(_LB_WINDOWS)
	#ifndef	WIN32
		#define	WIN32
	#endif
	#ifndef	_WIN32
		#define	_WIN32
	#endif
#endif


//	Microsoft
//***************************************************************************
#ifdef _MSC_VER	
	#define __MSC__
#endif


//	Microsoft
//***************************************************************************
#ifdef __MSC__	
	// DEBUG or not DEBUG that is the question?
	// By default, the MSVC Project Wizard sets _DEBUG and NDEBUG
	//***************************************************************************
	#if defined(_DEBUG) && !defined(_LB_DEBUG)
		#define _LB_DEBUG
	#elif defined (_LB_DEBUG) && !defined(_DEBUG)
		#define _DEBUG
	#endif
	#if defined(NDEBUG) && !defined(_LB_NDEBUG)
		#define _LB_NDEBUG
	#elif defined(_LB_NDEBUG) && !defined(NDEBUG)
		#define NDEBUG
	#endif
	#if !defined(_LB_CMDLINE) && defined(_CONSOLE)
		#define _LB_CMDLINE
	#elif !defined(_CONSOLE) && defined(_LB_CMDLINE)
		#define _LB_CONSOLE
	#endif
	//	are we using the right runtime library?
	//***************************************************************************
#endif


//	Microsoft - more defines
//***************************************************************************
#if !defined(_LB_WINDOWS) && (defined(_WIN32) || defined(WIN32) || defined(__WINDOW_386__) || defined(__WINDOWS__))
	#define _LB_WINDOWS
#endif


//	what platform exactly did you want to run on?
//***************************************************************************
#if !defined(_LB_WINDOWS) && !defined(_LB_DOS)
	#error Please specify which platform with one of the above defines.
#endif

#if defined(_LB_DOS)
	#error DOS no longer supported in library
#endif


//	Make your mine up time
//***************************************************************************
#ifdef _LB_DEBUG
	#ifdef _LB_NDEBUG
		#error Cant compile both _LB_DEBUG and _LB_NDEBUG at the same time
	#endif
#endif


// calltypes and externs
//***************************************************************************
#define LB_CALLTYPE		__cdecl	
#define LB_EXTERNTYPE	extern
#define	LB_EXTERN		extern


// callback
//***************************************************************************
#define LB_CALLBACK LB_CALLTYPE


// Get rid of the wanings
//***************************************************************************
#ifdef __MSC__
	//#pragma warning( disable : 4273 ) // 'lValue' : inconsistent dll linkage.  dllexport assumed.
	//#pragma warning( disable : 4275 ) // non dll-interface class 'className' used as base for dll-interface class 'baseClass'
	//#pragma warning( disable : 4251 ) // class 'className' needs to have dll-interface to be used by clients of class 'otherClass'
	//#pragma warning( disable : 4355 ) // 'this' : used in base member initializer list
#elif defined(__WATCOMC__)
	#pragma	warning	604 9
	#pragma	warning	594 9
	#pragma	warning	387 9
	#pragma	warning	11 9

	#ifdef	_LB_LIBRARY
		#pragma	warning	14 9
	#endif
#endif


//***************************************************************************
//	Structure and class member arrangement code
//***************************************************************************
#ifdef __cplusplus
	#define PRIVATE_MEMBERS			private:
	#define PROTECTED_MEMBERS		protected:
	#define PUBLIC_MEMBERS			public:
#else
	#define PRIVATE_MEMBERS
	#define PROTECTED_MEMBERS
	#define PUBLIC_MEMBERS
#endif
#if defined(_LB_LIBRARY) || !defined(_LB_DEBUG)
	#define LB_PRIVATE_MEMBERS
	#define LB_PROTECTED_MEMBERS
	#define LB_PUBLIC_MEMBERS		PUBLIC_MEMBERS
#else
	#define LB_PRIVATE_MEMBERS		PRIVATE_MEMBERS
	#define LB_PROTECTED_MEMBERS	PROTECTED_MEMBERS
	#define LB_PUBLIC_MEMBERS		PUBLIC_MEMBERS
#endif

#ifdef __cplusplus
	#define LB_INLINE	inline
	#define LB_FRIEND	friend
#else
	#define LB_INLINE
	#define LB_FRIEND	//
#endif

//***************************************************************************
// DLL and library call types : DLL imports, exports and externs.
//***************************************************************************
//
//	 Library Macros: Used for library-supplied functionality.
//	
//		LIBEXPORT : Used to export a function from the library.
//		LIBIMPORT : Used to export a variable from the library.
//		LIBCLASSEXPORT : Used to export a class from the library.
//	
//	 DLL-Only Macros: Only valid when using DLLs.
//	
//		DLLIMPORT : Import function or variable from a DLL. (Used in header files)
//	
//	 DLL-Only Macros: Only valid in DLL code.
//	
//		DLLEXPORT :	Export the given function or variable from a DLL. (Used in source code)
//		DLLCLASSEXPORT : Export the given class from a DLL.
//
//***************************************************************************
#ifdef __MSC__
	#define DLLEXPORT(ret)	__declspec(dllexport) ret
	#define DLLIMPORT(ret)	__declspec(dllimport) ret
	#define DLLVAREXPORT	__declspec(dllexport)
	#define DLLVARIMPORT	__declspec(dllimport)
	#define DLLCLASSEXPORT	__declspec(dllexport)
	#define DLLCLASSIMPORT	__declspec(dllimport)
#else
	#error Dont know how to declare exports for functions and classes
#endif

// Only set for building of the library
//***************************************************************************
/*#ifdef LB_DLL
	#ifdef _LB_LIBRARY
		#define LB_API(ret) LB_EXTERNTYPE DLLEXPORT(ret) LB_CALLTYPE
		#define LB_API_CLASS(classname) class DLLCLASSEXPORT classname
		#define LB_API_VAR_IMPORT	extern DLLVAREXPORT
		#define LB_API_VAR_EXPORT	DLLVAREXPORT
	#else
		#define LB_API(ret) LB_EXTERNTYPE DLLIMPORT(ret) LB_CALLTYPE
		#define LB_API_CLASS(classname) class DLLCLASSIMPORT classname
		#define LB_API_VAR_IMPORT	extern DLLVARIMPORT
		#define LB_API_VAR_EXPORT	DLLVAREXPORT
	#endif
#else*/
	#ifdef _LB_LIBRARY
		#define LB_API(ret) LB_EXTERNTYPE ret LB_CALLTYPE
		#define LB_API_CLASS(classname) class classname
		#define LB_API_VAR_IMPORT	extern
		#define LB_API_VAR_EXPORT	
	#else
		#define LB_API(ret) LB_EXTERNTYPE ret LB_CALLTYPE
		#define LB_API_CLASS(classname) class classname
		#define LB_API_VAR_IMPORT	extern
		#define LB_API_VAR_EXPORT	
	#endif
//#endif

// Used by TbPtrList
// <t>>TBLISTPOS<<
typedef void* TBLISTPOS;

//***************************************************************************
#endif // _LBTYPES_H

