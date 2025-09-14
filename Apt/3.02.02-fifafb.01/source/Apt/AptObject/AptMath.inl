/*H************************************************************************************************/
/*!

    \File
        AptMath.inl

    \Description
        Inline methods for AptMath.

    \Notes
        These are inlined only if !defined(APT_3D).

    \Copyright
        (c) 2007 Electronic Arts Inc.

    \Version
        1.0        10/09/07 (RB) First Version

*/
/************************************************************************************************H*/
#ifndef _AptMath_inl_
#define _AptMath_inl_

#include "Apt.h"
#include "AptDefine.h"

#if !defined(APT_3D)

AptMath::ClipTransformT *AptMath::ClipStackPush() 
{
    APT_ASSERT((m_nStackCount+1) < m_nStackCapacity);
    return &m_pStackBase[++m_nStackCount];
}
AptMath::ClipTransformT *AptMath::ClipStackPop()  
{ 
    APT_ASSERT(m_nStackCount < m_nStackCapacity);
    return &m_pStackBase[--m_nStackCount];
}
AptMath::ClipTransformT *AptMath::_ClipStackGetTop()  
{
    APT_ASSERT(m_nStackCount < m_nStackCapacity);
    return (&m_pStackBase[m_nStackCount]);
}
bool AptMath::ClipStackIsEmpty() 
{ 
    APT_ASSERT(m_nStackCount < m_nStackCapacity);
    return (m_nStackCount==0); 
}

#endif // !APT_3D

#endif // _AptMath_inl_

