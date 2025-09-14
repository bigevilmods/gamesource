// (c) Electronic Arts. All Rights Reserved.

/// \file
/// Windows host specific types for messages that are internationalized and managed by the docs team.

#ifndef RW_HOSTDOCMSG_HPP
#define RW_HOSTDOCMSG_HPP
#include <rw/core/base/ostypes.h>
#include <stdio.h>
#include <stdarg.h>

#ifdef RWENABLEDOCMESSAGE

/*************************************************************************************************************
 Defines + Enums + Consts
 */

/**
A windows host uses rwWinDocMessageHandler to process it's DocMessage(s)
*/
#define rwHOSTSPECIFICDOCMESSAGEHANDLERFN WinDocMessageHandler

/*************************************************************************************************************
 External Function Prototypes
 */

namespace rw
{

extern void
rwHOSTSPECIFICDOCMESSAGEHANDLERFN(const RwChar *file,
                                  uint32_t line,
                                  uint32_t messageId,
                                  const RwChar *format,
                                  va_list args);

} // namespace rw

#endif /* RWENABLEDOCMESSAGE */

#endif /* RW_HOSTDOCMSG_HPP */
