// (c) 1998-2010 Electronic Arts Inc.

/*!
* \file fileExpr.h
*
* Performs operations on files using expressions
*/

#ifndef _EA3_UTIL_FILE_EXPRESSION_H_
#define _EA3_UTIL_FILE_EXPRESSION_H_

#include "EA3Expr.h"

namespace EA3Expr{

#ifdef DOXYGEN
	/*!
	Apply expression 'expr' to each file in directories represented by expression 
	'path' (paths must be absolute, empty string will return current directory), 
	entering subdirectories if expression 'recurse' returns a non-zero number.
	Return 1 on success or 
	return 0 if 'path' does not represent a string which identifies a directory 
	(or sequence of directories) or 'recurse' does not represent a number or error 
	occurs during file I/O.
	NOTE: Each file will be visited at most once during recursion.
	*/
	Result ForEachFile(Expression &path, Expression &expr, Expression &recurse);
#else	
	EXPR_THREE_EXPR_CLASS_EXPORT(EA3_EXPORT,ForEachFile);
#endif // DOXYGEN

#ifdef DOXYGEN
	/*!
	Returns the filename of the current file, for use in expressions in ForEachFile().
	When used in the expression 'expr', CurrentFilename() returns the full path of the 
	file on which expr is to be evaluated; 
	when used in the expression 'recurse', CurrentFilename() returns the full path of 
	the directory currently under	consideration.
	*/
	Result CurrentFilename();
#else	
	EXPR_NO_ARGS_CLASS_EXPORT(EA3_EXPORT,CurrentFilename);
#endif // DOXYGEN

#ifdef DOXYGEN
	/*!
	Open file represented by expression 'filename' with access represented by 
	expression 'mode'.
	Return file struct pointer (FILE*) on success or 
	return 0 if 'filename' or 'mode' do not represent strings or error occurs 
	during file I/O.
	NOTE: Input parameters conform to those used in the C function 'fopen'.
	*/
	Result FileOpen(Expression &filename, Expression &mode);
#else	
	EXPR_TWO_EXPR_CLASS_EXPORT(EA3_EXPORT,FileOpen);
#endif // DOXYGEN

#ifdef DOXYGEN
	/*!
	Close file represented by expression 'filePtr'.
	Return 1 on success or 
	return 0 if 'filePtr' does not represent a FILE* or error occurs.
	NOTE: 'FileOpen' returns an expression appropriate for filePtr.
	*/
	Result FileClose(Expression &filePtr);
#else	
	EXPR_ONE_EXPR_CLASS_EXPORT(EA3_EXPORT,FileClose);
#endif // DOXYGEN

#ifdef DOXYGEN
	/*!
	Write string represented by expression 'str' to file associated with file
	struct pointer (FILE*) represented by expression 'filePtr'.
	Return 1 on success or 
	return 0 if 'filePtr' does not represent a FILE*, or 'str' does not represent 
	a string, or error occurs during file I/O.
	NOTE: 'FileOpen' returns an expression approriate for filePtr.
	*/
	Result FilePutS(Expression &filePtr, Expression &str);
#else	
	EXPR_TWO_EXPR_CLASS_EXPORT(EA3_EXPORT,FilePutS);
#endif // DOXYGEN

#ifdef DOXYGEN
	/*!
	Return the next character read as a string, or 
	return 0 if 'filePtr' does not represent a FILE* or EOF read or error occurs 
	during file I/O.
	NOTE: 'FileOpen' returns an expression approriate for filePtr.
	*/
	Result FileGetC(Expression &filePtr);
#else	
	EXPR_ONE_EXPR_CLASS_EXPORT(EA3_EXPORT,FileGetC);
#endif // DOXYGEN

#ifdef DOXYGEN
	/*!
	Return the next line read as a string, including the newline character (if read) or
	return 0 if 'filePtr' does not represent a FILE* or EOF reached or error occurs 
	during file I/O.
	Characters are read until newline, maxChars, or EOF is reached. 
	NOTE: 'FileOpen' returns an expression approriate for filePtr.
	*/
	Result FileGetLine(Expression &filePtr, Expression &maxChars);
#else	
	EXPR_TWO_EXPR_CLASS_EXPORT(EA3_EXPORT,FileGetLine);
#endif // DOXYGEN

#ifdef DOXYGEN
	/*!
	Return 1 after first read operation that attempts to read past end 
	of file represented by filePtr or 
	return 0 if 'filePtr' does not represent a FILE* or if current position in file is 
	not EOF.
	NOTE: 'FileOpen' returns an expression approriate for filePtr.
	*/
	Result FileEOF(Expression &filePtr);
#else	
	EXPR_ONE_EXPR_CLASS_EXPORT(EA3_EXPORT,FileEOF);
#endif // DOXYGEN

#ifdef DOXYGEN
	/*!
	Return time of last modification to file represented by filePtr (in number of seconds 
	elapsed since midnight, January 1, 1970) or
	return 0 if 'filePtr' does not represent a FILE* or if error occurs during file I/O.
	NOTE: 'FileOpen' returns an expression approriate for filePtr.
	*/
	Result FileMTime(Expression &filePtr);
#else	
	EXPR_ONE_EXPR_CLASS_EXPORT(EA3_EXPORT,FileMTime);
#endif // DOXYGEN

#ifdef DOXYGEN
	/*!
	Return size (in bytes) of file represented by filePtr or
	return 0 if 'filePtr' does not represent a FILE* or if error occurs during file I/O.
	NOTE: 'FileOpen' returns an expression approriate for filePtr.
	*/
	Result FileSize(Expression &filePtr);
#else	
	EXPR_ONE_EXPR_CLASS_EXPORT(EA3_EXPORT,FileSize);
#endif // DOXYGEN

} // namespace EA3Expr
#endif // _EA3_UTIL_FILE_EXPRESSION_H_