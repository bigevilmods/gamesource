/*H************************************************************************************************/
/*!

    \File
        MainInline.h

    \Description
        Simple description of main purpose of file.

    \Notes
        None.

    \Copyright
        (c) 2004 Electronic Arts Inc.

    \Version
        1.0        03/18/04 (ONA) First Version
        1.1        10/08/07 (RB)  Include AptMath.inl
        1.2        06/08/09 (RB)  Removed StringPool.inl

*/
/************************************************************************************************H*/

#ifndef __MAININLINE_H__
#define __MAININLINE_H__

#if defined(APT_ENABLE_INLINE)

#include "AptValue/AptValue.inl"
#include "AptNativeHash.inl"
#include "AptActionInterpreter.inl"
#include "AptValue/AptValueVector.inl"
#include "AptValue/AptString.inl"
#include "AptValue/AptBoolean.inl"
#include "AptObject/AptObject.inl"
#include "AptObject/AptGlobalObject.inl"
#include "AptObject/AptGlobalExtensionObject.inl"
#include "AptObject/AptMath.inl"
#include "string/EAString.inl"

#include "AptRenderItem.inl"
#include "AptCharacterInst.inl"
#include "AptCIH.inl"

#include "Apt.inl"
#endif

#endif  //  __MAININLINE_H__

/*H*************************************************************************************************
    EOF
*************************************************************************************************H*/

