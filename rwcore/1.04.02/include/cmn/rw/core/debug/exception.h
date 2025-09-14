// (c) Electronic Arts. All Rights Reserved.

/// \file
/// Exception handling class.

#ifndef RW_EXCEPTION_HPP
#define RW_EXCEPTION_HPP
#include <rw/core/base/ostypes.h>
#include <rw/core/debug/debug.h>
// NB: This header should never have a
//     dependency on any other header
//     in the debug system other than debug.h
//     this is to allow users to pull in this
//     header without having a dependency on
//     the debug manager.


// ***********************************************************************************************************
// Includes

// ***********************************************************************************************************
// Forward Declarations

// ***********************************************************************************************************
// Typedefs

// ***********************************************************************************************************
// Defines + Enums + Consts

namespace rw
{

// ***********************************************************************************************************
// Global Variables


// ***********************************************************************************************************
// External Functions Prototypes


// ***********************************************************************************************************
// Structs + Unions + Classes


// ***********************************************************************************************************
//                                                Exception CLASS
// ***********************************************************************************************************

/**
Renderware exception handling class.

You may throw exceptions based directly on this class, or inherit
from it to provide your own exception handling functionality.

In addition to the standard what() API, a where() method is provided which
specifies the location at which the exception was thrown.

*/
class Exception
{
public:
    // Construct exception with given reason in given location.
    Exception(const RwChar *msg, const RwChar *file = "",
              const uint32_t line = 0, const RwChar *func = "") throw();

    // Destroy exception.
    virtual ~Exception() throw();

    // construct by copying _Right
    Exception(const Exception& _Right) throw();

    // assign _Right
    Exception& operator = (const Exception& _Right) throw();

    // Return the reason for the exception.
    const RwChar *What() const throw();

    // Return the location of the exception.
    const RwChar *Where() throw();

private:

    // Default constructor.
    Exception() throw() {}

    // Exception message.
    RwChar *    m_pMsg;

    // File in which exception was thrown.
    RwChar *    m_pFile;

    // Function in which exception was thrown.
    // \note Only supported on certain platforms.
    RwChar *    m_pFunc;

    // Line on which exception was thrown.
    int32_t     m_Line;

    // Where in the code the exception occured
    RwChar *    m_pWhere;
};

} // namespace rw

/**
Throws the specified object (if exceptions are enabled) or creates an assert if not.
\param object The object to be thrown.

*/
#ifdef RWEXCEPTION
#define rwTHROW(object) throw(object);
#else
#define rwTHROW(object) rwASSERTM(FALSE, (#object))
#endif

/**
Throws an rw::Exception object with the passed in message. The file and line of the
exception are also shown. If exceptions are turned off, this will assert instead.

\param msgArgs The message to display, has file and line number added to it. This must be formatted as a
call to a printf type function, ie rwTHROWERROR(("error")) or rwTHROWERROR(("error %d", localErrNum)).

*/
#ifdef RWEXCEPTION
#define rwTHROWERROR(msgArgs)                                                   \
    throw(rw::Exception(rw::rwDebugPrintHelper().WriteToBuffer msgArgs, __FILE__, __LINE__))
#else
#define rwTHROWERROR(msgArgs) rwASSERTM(FALSE, msgArgs)
#endif

/**
Similar as rwTHROWERROR, but tests to see if the condition is FALSE (ZERO) before firing.

\param condition The condition to test.
\param msgArgs The message to display, has file and line number added to it. This must be formatted as a
call to a printf type function, ie rwTHROWERROR(("error")) or rwTHROWERROR(("error %d", localErrNum)).

*/
#ifdef RWEXCEPTION
#define rwENFORCE(condition, msgArgs) if (!(condition)) rwTHROWERROR(msgArgs)
#else   // RWEXCEPTION
#define rwENFORCE(condition, msgArgs) rwASSERTM(condition, msgArgs)
#endif  // RWEXCEPTION

#endif // RW_EXCEPTION_HPP
