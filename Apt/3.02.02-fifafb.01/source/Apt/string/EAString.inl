/*H************************************************************************************************/
/*!

    \File
        EAString.inl

    \Description
        Simple description of main purpose of file.

    \Notes
        None.

    \Copyright
        (c) 2004 Electronic Arts Inc.

    \Version
        1.0        02/23/04 (ONA) First Version
        1.1        06/05/09 (RB)  Moved UTF8_IsValid into header because it's marked static inline.
                                  Moved MemInitialize and MemUninitialize into .cpp because they shouldn't be marked inline.

*/
/************************************************************************************************H*/

#ifndef _EAString_inl_
#define _EAString_inl_

#include "string.h"
#include "Apt.h"
#include "AptDefine.h"
#include "string/EAString_platform.h"


APT_DECLARE_CUSTOM_METRICS(AptNativeStringBuffers)

/*F************************************************************************************************/
/*!
    Insert your long description here.

    \Input      uSize             - 

    \Output     None.

    \Notes
        None.

    \Version
        1.0        06/30/03 (ONA) First Version

*/
/************************************************************************************************F*/
void    EAStringC::SetInternalSize(uint32_t uSize)
{
    APT_ASSERT(uSize <= GetInternalMaxSize());
    if ( uSize > GetInternalMaxSize() )
    {
        uSize = GetInternalMaxSize();
        APT_ASSERT(uSize <= 0xffff);
    }

    m_pData->m_uSize = static_cast<uint16_t>(uSize);
}

/*F************************************************************************************************/
/*!
    Insert your long description here.

    \Input      None.

    \Output     uint32_t            - 

    \Notes
        None.

    \Version
        1.0        06/30/03 (ONA) First Version

*/
/************************************************************************************************F*/
uint32_t    EAStringC::GetInternalMaxSize() const
{
    return  (m_pData->m_uMaxSize);
}

/*F************************************************************************************************/
/*!
    Insert your long description here.

    \Input      uMaxSize          - 

    \Output     None.

    \Notes
        None.

    \Version
        1.0        06/30/03 (ONA) First Version

*/
/************************************************************************************************F*/
void    EAStringC::SetInternalMaxSize(uint32_t uMaxSize)
{
    APT_ASSERT(uMaxSize <= 0xffff);
    m_pData->m_uMaxSize = static_cast<uint16_t>(uMaxSize);
}

/*F************************************************************************************************/
/*!
    Insert your long description here.

    \Input      None.

    \Output     uint32_t            - 

    \Notes
        None.

    \Version
        1.0        06/30/03 (ONA) First Version

*/
/************************************************************************************************F*/
uint32_t    EAStringC::GetInternalRefCount() const
{
    return  (m_pData->m_uRefCount);
}

/*F************************************************************************************************/
/*!
    Insert your long description here.

    \Input      uRefCount         - 

    \Output     None.

    \Notes
        None.

    \Version
        1.0        06/30/03 (ONA) First Version

*/
/************************************************************************************************F*/
void    EAStringC::SetInternalRefCount(const uint32_t uRefCount)
{
    APT_ASSERT(uRefCount <= 0xffff);
    m_pData->m_uRefCount = static_cast<uint16_t>(uRefCount);
}


/*F************************************************************************************************/
/*!
    Insert your long description here.

    \Input
        None.

    \Output
        bool

    \Notes
        None.

    \Version
        1.0        04/25/03 (ONA) First Version

*/
/************************************************************************************************F*/
bool    EAStringC::IsEmpty() const
{
    return  (m_pData == (DebugDataC *)&s_EmptyInternalData);
}

/*F************************************************************************************************/
/*!
    Insert your long description here.

    \Input      None.

    \Output     uint16_t          - 

    \Notes
        None.

    \Version
        1.0        03/29/04 (ONA) First Version

*/
/************************************************************************************************F*/
uint16_t    EAStringC::GetHashValue() const
{
    APT_ASSERT(m_pData->m_uHash != 0);
    return  (m_pData->m_uHash);
}

/*F************************************************************************************************/
/*!
    Insert your long description here.

    \Input      index             - 

    \Output     uint32_t          - 

    \Notes
        None.

    \Version
        1.0        02/24/04 (ONA) First Version

*/
/************************************************************************************************F*/
uint32_t    EAStringC::operator[](const int32_t index) const
{
    return  (*(GetBuffer() + index));
}

/*F************************************************************************************************/
/*!
    Insert your long description here.

    \Input
        const EAStringC & - 

    \Output
        bool

    \Notes
        None.

    \Version
        1.0        04/24/03 (ONA) First Version

*/
/************************************************************************************************F*/
bool    EAStringC::operator!=(const EAStringC & strText) const
{
    return  (operator==(strText) == false);
}

/*F************************************************************************************************/
/*!
    Insert your long description here.

    \Input
        const EAStringC & - 

    \Output
        bool

    \Notes
        None.

    \Version
        1.0        04/24/03 (ONA) First Version

*/
/************************************************************************************************F*/
bool    EAStringC::operator>(const EAStringC & strText) const
{
    return  (operator<=(strText) == false);
}

/*F************************************************************************************************/
/*!
    Insert your long description here.

    \Input
        const EAStringC & - 

    \Output
        bool

    \Notes
        None.

    \Version
        1.0        04/24/03 (ONA) First Version

*/
/************************************************************************************************F*/
bool    EAStringC::operator>=(const EAStringC & strText) const
{
    return  (operator<(strText) == false);
}

/*F************************************************************************************************/
/*!
    Insert your long description here.

    \Input
        const char * const - 

    \Output
        bool

    \Notes
        None.

    \Version
        1.0        04/25/03 (ONA) First Version

*/
/************************************************************************************************F*/
bool    EAStringC::operator==(const char * const pStrText) const
{
    return  (strcmp(GetBuffer(), pStrText) == 0);
}

/*F************************************************************************************************/
/*!
    Insert your long description here.

    \Input
        const char * const - 

    \Output
        bool

    \Notes
        None.

    \Version
        1.0        04/25/03 (ONA) First Version

*/
/************************************************************************************************F*/
bool    EAStringC::operator!=(const char * const pStrText) const
{
    return  (operator==(pStrText) == false);
}

/*F************************************************************************************************/
/*!
    Insert your long description here.

    \Input
        const char * const - 

    \Output
        bool

    \Notes
        None.

    \Version
        1.0        04/25/03 (ONA) First Version

*/
/************************************************************************************************F*/
bool    EAStringC::operator<(const char * const pStrText) const
{
    return  (strcmp(GetBuffer(), pStrText) < 0);
}

/*F************************************************************************************************/
/*!
    Insert your long description here.

    \Input
        const char * const - 

    \Output
        bool

    \Notes
        None.

    \Version
        1.0        04/25/03 (ONA) First Version

*/
/************************************************************************************************F*/
bool    EAStringC::operator<=(const char * const pStrText) const
{
    return  (strcmp(GetBuffer(), pStrText) <= 0);
}

/*F************************************************************************************************/
/*!
    Insert your long description here.

    \Input
        const char * const - 

    \Output
        bool

    \Notes
        None.

    \Version
        1.0        04/25/03 (ONA) First Version

*/
/************************************************************************************************F*/
bool    EAStringC::operator>(const char * const pStrText) const
{
    return  (operator<=(pStrText) == false);
}

/*F************************************************************************************************/
/*!
    Insert your long description here.

    \Input
        const char * const - 

    \Output
        bool

    \Notes
        None.

    \Version
        1.0        04/25/03 (ONA) First Version

*/
/************************************************************************************************F*/
bool    EAStringC::operator>=(const char * const pStrText) const
{
    return  (operator<(pStrText) == false);
}

/*F************************************************************************************************/
/*!
    Insert your long description here.

    \Input
        None.

    \Output
        None.

    \Notes
        None.

    \Version
        1.0        04/24/03 (ONA) First Version

*/
/************************************************************************************************F*/
void    EAStringC::Clear()
{
    DecreaseInternalRefCount(m_pData);
    m_pData = (DebugDataC *)&s_EmptyInternalData;
    IncreaseInternalRefCount();
}

/*! \Clears out the static memory set to m_pData */
void    EAStringC::ClearConstantValue()
{
    APT_ASSERT(m_pData->m_uSize > m_pData->m_uMaxSize && "TRYING TO USE INCORRECT CLEAR A NON STATIC STRING IN THE STRING POOL");
    m_pData = (DebugDataC *)&s_EmptyInternalData;
    return;
}

/*F************************************************************************************************/
/*!
    Insert your long description here.

    \Input      pStrText          - 

    \Output     bool              - 

    \Notes
        None.

    \Version
        1.0        02/25/04 (ONA) First Version

*/
/************************************************************************************************F*/
bool    EAStringC::Equal(const char * const pStrText) const
{
    APT_ASSERT(pStrText != NULL);
    return  (strcmp(GetBuffer(), pStrText) == 0);
}

/*F************************************************************************************************/
/*!
    Insert your long description here.

    \Input      strText           - 

    \Output     bool              - 

    \Notes
        None.

    \Version
        1.0        02/25/04 (ONA) First Version

*/
/************************************************************************************************F*/
bool    EAStringC::Equal(const EAStringC & strText) const
{
    return  (*this == strText);
}

/*F************************************************************************************************/
/*!
    Insert your long description here.

    \Input      pStrText          - 

    \Output     bool              - 

    \Notes
        None.

    \Version
        1.0        02/25/04 (ONA) First Version

*/
/************************************************************************************************F*/
bool    EAStringC::EqualNoCase(const char * const pStrText) const
{
    APT_ASSERT(pStrText != NULL);
    return  (APT_STRING_NOCASE_COMPARE(GetBuffer(), pStrText) == 0);
}

/*F************************************************************************************************/
/*!
    Insert your long description here.

    \Input
        const char * const - 

    \Output
        int32_t

    \Notes
        None.

    \Version
        1.0        04/24/03 (ONA) First Version

*/
/************************************************************************************************F*/
int32_t    EAStringC::Compare(const char * const pStrText) const
{
    APT_ASSERT(pStrText != NULL);
    return  (strcmp(GetBuffer(), pStrText));
}

/*F************************************************************************************************/
/*!
    Insert your long description here.

    \Input
        const EAStringC & - 

    \Output
        int32_t

    \Notes
        None.

    \Version
        1.0        04/24/03 (ONA) First Version

*/
/************************************************************************************************F*/
int32_t    EAStringC::Compare(const EAStringC & strText) const
{
    if	(m_pData == strText.m_pData)
    {
        //  Same string
        return  (0);
    }

    return  (strcmp(GetBuffer(), strText.GetBuffer()));
}

/*F************************************************************************************************/
/*!
    Insert your long description here.

    \Input
        const char * const - 

    \Output
        int32_t

    \Notes
        None.

    \Version
        1.0        04/24/03 (ONA) First Version

*/
/************************************************************************************************F*/
int32_t    EAStringC::CompareNoCase(const char * const pStrText) const
{
    APT_ASSERT(pStrText != NULL);
    return  (APT_STRING_NOCASE_COMPARE(GetBuffer(), pStrText));
}

/*F************************************************************************************************/
/*!
    Insert your long description here.

    \Input
        const EAStringC & - 

    \Output
        int32_t

    \Notes
        None.

    \Version
        1.0        04/24/03 (ONA) First Version

*/
/************************************************************************************************F*/
int32_t    EAStringC::CompareNoCase(const EAStringC & strText) const
{
    if	(m_pData == strText.m_pData)
    {
        //  Same string
        return  (0);
    }

    return  (APT_STRING_NOCASE_COMPARE(GetBuffer(), strText.GetBuffer()));
}

/*F************************************************************************************************/
/*!
    Insert your long description here.

    \Input      None.

    \Output     int               - 

    \Notes
        None.

    \Version
        1.0        02/23/04 (ONA) First Version

*/
/************************************************************************************************F*/
int    EAStringC::Size() const
{
    return  (GetLength());
}

/*F************************************************************************************************/
/*!
    Insert your long description here.

    \Input      nSize             - 

    \Output     None.

    \Notes
        None.

    \Version
        1.0        02/23/04 (ONA) First Version

*/
/************************************************************************************************F*/
void    EAStringC::Reserve(int32_t nSize)
{
    ReserveSize(nSize);
}


/*F************************************************************************************************/
/*!
    Insert your long description here.

    \Input      None.

    \Output     None.

    \Notes
        None.

    \Version
        1.0        02/27/04 (ONA) First Version

*/
/************************************************************************************************F*/
void    EAStringC::Invalidate()
{
    APT_ASSERT(IsValid());
    DecreaseInternalRefCount(m_pData);
    m_pData = NULL;
}

/*F************************************************************************************************/
/*!
    Insert your long description here.

    \Input      None.

    \Output     None.

    \Notes
        None.

    \Version
        1.0        02/27/04 (ONA) First Version

*/
/************************************************************************************************F*/
void    EAStringC::Validate()
{
    APT_ASSERT(IsValid() == false);
    m_pData = (DebugDataC *)&s_EmptyInternalData;
    IncreaseInternalRefCount();
}

/*F************************************************************************************************/
/*!
    Insert your long description here.

    \Input      strText           - 

    \Output     None.

    \Notes
        None.

    \Version
        1.0        02/27/04 (ONA) First Version

*/
/************************************************************************************************F*/
void    EAStringC::Validate(const EAStringC & strText)
{
    APT_ASSERT(IsValid() == false);
    m_pData = strText.m_pData;
    IncreaseInternalRefCount();
}

/*F************************************************************************************************/
/*!
    Insert your long description here.

    \Input      None.

    \Output     bool              - 

    \Notes
        None.

    \Version
        1.0        02/27/04 (ONA) First Version

*/
/************************************************************************************************F*/
bool    EAStringC::IsValid() const
{
    return  (m_pData != NULL);
}

/*F************************************************************************************************/
/*!
    Insert your long description here.

    \Input      None.

    \Output     None.

    \Notes
        None.

    \Version
        1.0        03/31/04 (ONA) First Version

*/
/************************************************************************************************F*/
void    EAStringC::CalculateHashValue() const
{
    m_pData->m_uHash = CalculateHashValue(GetInternalBuffer());
}

/*F************************************************************************************************/
/*!
    Insert your long description here.

    \Input      None.

    \Output     uint16_t          - 

    \Notes
        None.

    \Version
        1.0        03/31/04 (ONA) First Version

*/
/************************************************************************************************F*/
uint16_t    EAStringC::UpdateHashValue() const
{
    if (m_pData->m_uHash == 0)
    {
        CalculateHashValue();
        APT_ASSERT(m_pData->m_uHash != 0);
    }
#if (APT_DEBUG_LEVEL >= APT_DEBUG_NORMAL)
    else
    {
        uint16_t    uOldHash = m_pData->m_uHash;
        CalculateHashValue();
        APT_ASSERT(m_pData->m_uHash == uOldHash);
    }
#endif
    return  (m_pData->m_uHash);
}

/*F************************************************************************************************/
/*!
    Insert your long description here.

    \Input      None.

    \Output     None.

    \Notes
        None.

    \Version
        1.0        03/29/04 (ONA) First Version

*/
/************************************************************************************************F*/
void    EAStringC::InvalidateHashValue() const
{
    m_pData->m_uHash = 0;
}


/*H*************************************************************************************************
    EOF
*************************************************************************************************H*/

#endif // _EAString_inl_

