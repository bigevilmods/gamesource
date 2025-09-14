// (c) Electronic Arts. All Rights Reserved.

/// \file
/// Messages that are internationalized and managed by the docs team.

/*************************************************************************************************************
 Includes
 */
#include "rwsharedheaders.hpp"
#include "rw/core/docmessage/docmessage.h"
#include "rw/core/docmessage/hostdocmsg.h"

namespace rw
{

#if defined(RWENABLEDOCMESSAGE)

/*************************************************************************************************************
 Statics
 */

/**
 If the host has set the rwHOSTSPECIFICDOCMESSAGEHANDLERFN define, then it
 has a host specific output function that it would like to use. If this has
 not been set then I will default to NULL
*/

#ifndef rwHOSTSPECIFICDOCMESSAGEHANDLERFN
    #define rwHOSTSPECIFICDOCMESSAGEHANDLERFN NULL
#endif

DocMessageData::DocMessageData() : m_messageHandler(rwHOSTSPECIFICDOCMESSAGEHANDLERFN)
{
    rwFUNCTION();
    // empty
    rwRETURNVOID();
}

/*************************************************************************************************************
 Member Functions
 */

/**
Generate a doc message. This will be internationalized if a translation is available.
\param messageId The internationalization identifier of this message.
\param format A printf style argument list.
*/
void
DocMessage::Send(uint32_t messageId, const RwChar *format, ...)
{
    va_list args;

    rwFUNCTION();

    /* if I don't have anywhere to send this message, then it's not worth processing it. */
    if((DocMessageHandlerCallBack)NULL != Data().m_messageHandler)
    {
        va_start(args, format);
        (*Data().m_messageHandler)(m_file, m_line, messageId, format, args);
        va_end(args);
    }

    rwRETURNVOID();
}

/**
Sets a callback for when a doc message is generated. The default callback on the Windows
platform does an OutputDebugString with the message.
\param fn The callback function. This may be NULL.
*/
void
DocMessage::SetTarget(DocMessageHandlerCallBack fn)
{
    rwFUNCTION();
    Data().m_messageHandler = fn;
    rwRETURNVOID();
}

/**
The shared data of the doc message system. This is used in concert with the SharedData
method to share my m_messageHandler pointer over multiple instances of this object in
different DLLs
*/
DocMessageData&
DocMessage::Data()
{
    rwFUNCTION();

    static rw::SharedGlobal<DocMessageData, 0x00000006> sData;

    rwRETURN(sData.Data());
}

#else /* !defined(RWENABLEDOCMESSAGE) */

void
DocMessage::SetTarget(DocMessageHandlerCallBack RWUNUSED(callback))
{
    rwFUNCTION();
    rwRETURNVOID();
}

#endif /* defined(RWENABLEDOCMESSAGE) */

} // namespace rw
