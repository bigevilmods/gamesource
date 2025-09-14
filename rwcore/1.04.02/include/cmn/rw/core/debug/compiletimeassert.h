// (c) Electronic Arts. All Rights Reserved.

/// \file
/// Definition for rwCOMPILETIMEASSERT.

#ifndef RW_COMPILETIMEASSERT_H
#define RW_COMPILETIMEASSERT_H

/**
Compile time assert generic, when assert evaluates to
false it evaluates to this templated forward declaration
which is not defined for \e  failure. Thus it will not
compile.
*/
template<bool>
class rwASSERTATCOMPILETIME_FAILURE;

/**
Compile time assert pass, when assert evaluates to
true it evaluates to this templated empty class which is
defined
*/
template<>
class rwASSERTATCOMPILETIME_FAILURE<true>
{
};

/**
Compile time assert, will stop compilation with a hopefully
suitable error message when test fails
*/
#define rwASSERTATCOMPILETIME(test, errormsg)       \
    for (rwASSERTATCOMPILETIME_FAILURE< ((test) != 0) > ERROR_##errormsg; ; (void)ERROR_##errormsg) break

#endif /* RW_COMPILETIMEASSERT_H */
