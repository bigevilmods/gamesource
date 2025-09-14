// (c) Electronic Arts. All Rights Reserved.

/// \file
/// An exception that is internationalized and managed by the docs team.

#ifndef RW_DOCEXCEPTION_HPP
#define RW_DOCEXCEPTION_HPP
#include <rw/core/base/ostypes.h>
#include <rw/core/debug/debug.h>
#include <rw/core/debug/exception.h>
#include <rw/core/docmessage/docmessage.h>  // defines RWENABLEDOCMESSAGE

// ***********************************************************************************************************
// Includes

// ***********************************************************************************************************
// Defines + Enums + Consts

namespace rw
{

// ***********************************************************************************************************
//                                                DocException CLASS
// ***********************************************************************************************************

typedef void (*ThrowDocExceptionCallBack)(const RwChar *file,
                                          const RwChar *function,
                                          uint32_t line,
                                          uint32_t messageId,
                                          const RwChar *format,
                                          va_list args);

// A class that throws DocExceptions
class ThrowDocException
{
private:

    #if defined(RWENABLEDOCMESSAGE)
    static ThrowDocExceptionCallBack m_messageHandler;
    #endif

    const RwChar *m_file;
    const RwChar *m_function;
    const uint32_t m_line;

    // default constructor is private to appease the compiler but force users
    // to use the appropriate constructor.
    ThrowDocException() : m_file(0), m_function(0), m_line(0) {}
    ThrowDocException & operator=(const ThrowDocException & RWUNUSED(other))
    {
        rwFUNCTION();
        rwRETURN(*this);
    }

public:

    ThrowDocException(const RwChar *file, const RwChar *function, const uint32_t line)
        : m_file(file), m_function(function), m_line(line)
    {
        rwFUNCTION();
        rwRETURNVOID();
    }


    #if defined(RWENABLEDOCMESSAGE)
    void
    Throw(uint32_t messageId, const RwChar *format, ...);
    #endif

    #if defined(RWENABLEDOCMESSAGE)
    static void
    SetCallBack(ThrowDocExceptionCallBack fn)
    {
        rwFUNCTION();
        m_messageHandler = fn;
        rwRETURNVOID();
    }
    #else
    static void
    SetCallBack(ThrowDocExceptionCallBack RWUNUSED(fn))
    {
        rwFUNCTION();
        rwRETURNVOID();
    }
    #endif
};

/**
An exception that has an internationalized error
*/
class DocException : public Exception
{
public:

    // Construction
    DocException(uint32_t messageId,
                 const RwChar *msg,
                 const RwChar *file = "",
                 const uint32_t line = 0,
                 const RwChar *func = "") throw();

    // Copy construction
    DocException(const DocException &source) throw();

    // assignment
    DocException& operator = (const DocException &other) throw();

    // \return The internationalized error message id.
    uint32_t GetMessageId() const throw()
    {
        rwFUNCTION();
        rwRETURN(m_messageId);
    }

private:

    // ID of the internationalized error message.
    uint32_t m_messageId;
};

/**
Sets a callback that is used to throw a DocException in rwDOCTHROWERROR. This can be set to NULL. There is a default
implementation of this method that throws a DocException object.
*/
inline void
SetDocExceptionCallBack(ThrowDocExceptionCallBack fn)
{
    rwFUNCTION();
    ThrowDocException::SetCallBack(fn);
    rwRETURNVOID();
}

/**
Throws an rw::DocException object with the passed in message. The file and line of the
exception are also shown. If exceptions are turned off, this will assert instead.

\param msgArgs a bracketed printf style statement in the form: (messageId, "Error in English %d", someNumber)
where: messageId is he localization identifier, and the rest is a printf style statement for a default error message.
\param "" A printf style statement of the message to display, e.g.
\note e.g. rwDOCTHROWERROR((messageId, "Error in English %d", someNumber));
*/
#ifdef RWEXCEPTION

  #ifdef RWENABLEDOCMESSAGE

    #ifdef __FUNCTION__
      #define rwDOCTHROWERROR(msgArgs) \
              rw::ThrowDocException(__FILE__, __FUNCTION__, __LINE__).Throw msgArgs;
    #else  /* __FUNCTION__ */
      #define rwDOCTHROWERROR(msgArgs) \
              rw::ThrowDocException(__FILE__, "", __LINE__).Throw msgArgs;
    #endif /* !__FUNCTION__ */

  #else  /* !RWENABLEDOCMESSAGE */

       #define rwDOCTHROWERROR(msgArgs) \
              rwASSERT(FALSE)

  #endif /* RWENABLEDOCMESSAGE */

#else  /* !RWEXCEPTION */

  #define rwDOCTHROWERROR(msgArgs) \
                rwASSERT(FALSE)

#endif /* RWEXCEPTION */

/**
Similar to rwDOCTHROWERROR, but tests to see if the condition is FALSE (ZERO) before firing.
\param condition The condition to test.
\param msgArgs a bracketed printf style statement in the form: (messageId, "Error in English %d", someNumber)
where: messageId is he localization identifier, and the rest is a printf style statement for a default error message.
\note e.g. rwDOCENFORCE(condition, (messageId, "Error in English %d", someNumber));
*/
#ifdef RWEXCEPTION
#define rwDOCENFORCE(condition, msgArgs) if (!(condition)) \
                rwDOCTHROWERROR(msgArgs)
#else
#define rwDOCENFORCE(condition, msgArgs) \
                rwASSERT(condition)
#endif

} // namespace rw

#endif // DOCEXCEPTION
