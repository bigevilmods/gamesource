// (c) Electronic Arts. All Rights Reserved.

/// \file
/// An exception that is internationalized and managed by the docs team.

#include "rwsharedheaders.hpp"
#include "rw/core/docmessage/docexception.h"

namespace rw
{

// ***********************************************************************************************************
// Local Functions

/**
\internal The default 'throw' function
*/
namespace
{

#ifdef RWEXCEPTION
void
DefaultThrow(const RwChar *file, const RwChar *function, uint32_t line,
             uint32_t messageId, const RwChar *format, va_list args) RWNORETURN;
void
DefaultThrow(const RwChar *file,
             const RwChar *function,
             uint32_t line,
             uint32_t messageId,
             const RwChar *format,
             va_list args)
{
    #define workspaceSize 256
    RwChar workspace[workspaceSize];

    rwFUNCTION();

    RwVsnprintf(workspace, workspaceSize, format, args);

    // note: my base Exception class allocates space for and copies 'workspace',
    // so when I leave this function, there will be no reference to my local
    // 'workspace'
    throw(DocException(messageId, workspace, file, line, function));
}
#else
void
DefaultThrow(const RwChar* RWUNUSED(file),
             const RwChar* RWUNUSED(function),
             uint32_t RWUNUSED(line),
             uint32_t RWUNUSED(messageId),
             const RwChar* RWUNUSED(format),
             va_list RWUNUSED(args))
{
    rwFUNCTION();
    /* empty */
    rwRETURNVOID();
}
#endif

} // namespace

// ***********************************************************************************************************
// Statics

#if defined(RWENABLEDOCMESSAGE)
/*static*/ ThrowDocExceptionCallBack ThrowDocException::m_messageHandler = DefaultThrow;
#endif


// ***********************************************************************************************************
// Class Member Functions

#if defined(RWENABLEDOCMESSAGE)

void
ThrowDocException::Throw(uint32_t messageId, const RwChar *format, ...)
{
    va_list args;
    rwFUNCTION();

    if((ThrowDocExceptionCallBack)NULL != m_messageHandler)
    {
        va_start(args, format);
        (*m_messageHandler)(m_file, m_function, m_line, messageId, format, args);
        va_end(args);
    }

    rwRETURNVOID();
}
#endif

/**
Construction

\param messageId Internationalized message identifier.
\param msg Default message. Used if no internationalizeable message is found.
\param file Source file that the exception was thrown from.
\param line Line number that the exception was thwown from.
\param func Function that the exception was thrown from.
*/
DocException::DocException(uint32_t messageId,
                           const RwChar *msg,
                           const RwChar *file,
                           const uint32_t line,
                           const RwChar *func) throw()
            : Exception(msg, file, line, func), m_messageId(messageId)
{
    rwFUNCTION();

    rwRETURNVOID();
}

/**
Copy construction
*/
DocException::DocException(const DocException &source) throw()
            : Exception(source), m_messageId(source.m_messageId)
{
    rwFUNCTION();

    rwRETURNVOID();
}


/**
Assignment
*/
DocException&
DocException::operator = (const DocException &other) throw()
{
    rwFUNCTION();

    // use the Exception class copy functionality
    *static_cast<Exception*>(this) = *static_cast<const Exception*>(&other);

    // copy my stuff
    m_messageId = other.m_messageId;

    rwRETURN(*this);
}

} // namespace rw
