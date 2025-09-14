// (c) Electronic Arts. All Rights Reserved.

/// \file
/// Messages that are internationalized and managed by the docs team.

#ifndef RW_DOCMESSAGE_HPP
#define RW_DOCMESSAGE_HPP
#include <rw/core/base/ostypes.h>

#include <stdio.h>
#include <stdarg.h>

// Set DocMessage system to always be on
#ifndef RWENABLEDOCMESSAGE
#define RWENABLEDOCMESSAGE
#endif

namespace rw
{

/*************************************************************************************************************
 Defines
 */

// Doc message ID for progress system statistics output.
#define RWDOCMESSAGE_PROGRESS_STATS 2

// Doc message ID range for exporter & associated apps output.
#define RWDOCMESSAGE_EXPORTER_MESSAGES_SIZE  1000
#define RWDOCMESSAGE_EXPORTER_MESSAGES_BEGIN 1000
#define RWDOCMESSAGE_EXPORTER_MESSAGES_END (RWDOCMESSAGE_EXPORTER_MESSAGES_BEGIN + RWDOCMESSAGE_EXPORTER_MESSAGES_SIZE)

// note The DocMessage system is compiled in if the RWENABLEDOCMESSAGE macro is defined.

/*************************************************************************************************************
 Typedefs
 */

typedef void (*DocMessageHandlerCallBack)(const RwChar *file,
                                          uint32_t line,
                                          uint32_t messageId,
                                          const RwChar *format,
                                          va_list args);


/* ***********************************************************************************************************
 Includes
 */

} // namespace rw

namespace rw
{

/*************************************************************************************************************
 Classes
 */

class DocMessageData
{
public:
    #if defined(RWENABLEDOCMESSAGE)
    DocMessageHandlerCallBack m_messageHandler;
    #endif

    DocMessageData(void);
};

class DocMessage
{
private:

    #if defined(RWENABLEDOCMESSAGE)
    const RwChar *m_file;
    const uint32_t m_line;
    #endif /* defined(RWENABLEDOCMESSAGE) */

    // default constructor is private to appease W4 warning
    // but avoid users ever using it.
    #if defined(RWENABLEDOCMESSAGE)
    DocMessage() : m_file(0), m_line(0)
    {
        rwFUNCTION();
        rwRETURNVOID();
    }
    #else
    DocMessage()
    {
        rwFUNCTION();
        rwRETURNVOID();
    }
    #endif /* defined(RWENABLEDOCMESSAGE) */
    DocMessage & operator=(const DocMessage & RWUNUSED(other))
    {
        rwFUNCTION();
        rwRETURN(*this);
    }

public:

    #if defined(RWENABLEDOCMESSAGE)
    DocMessage(const RwChar *file, uint32_t line) : m_file(file), m_line(line)
    {
        rwFUNCTION();
        rwRETURNVOID();
    }
    #else
    DocMessage(const RwChar* RWUNUSED(file), uint32_t RWUNUSED(line))
    {
        rwFUNCTION();
        rwRETURNVOID();
    }
    #endif /* defined(RWENABLEDOCMESSAGE) */

    #if defined(RWENABLEDOCMESSAGE)
    void
    Send(uint32_t messageId, const RwChar *format, ...);
    #endif /* defined(RWENABLEDOCMESSAGE) */

    static void
    SetTarget(DocMessageHandlerCallBack callback);

    static DocMessageData &Data();
};


/*************************************************************************************************************
 Defines
 */

// Usage rwDOCMESSAGE((uint32_t messageId, const RwChar *format, ...))
#if defined(RWENABLEDOCMESSAGE)
#define rwDOCMESSAGE(params)    rw::DocMessage(__FILE__, __LINE__).Send params
#else
#define rwDOCMESSAGE(params)
#endif

} // namespace rw

#endif /* RW_DOCMESSAGE_HPP */
