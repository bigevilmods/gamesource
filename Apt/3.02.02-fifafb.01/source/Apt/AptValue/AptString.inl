/*H************************************************************************************************/
/*!

    \File
        AptString.inl

    \Description
        has the inline functions of AptString.cpp

    \Notes
        None.

    \Copyright
        (c) 2004 Electronic Arts Inc.

    \Version
        1.0        03/29/04 (ONA) First Version

*/
/************************************************************************************************H*/


#include "AptString.h"

APT_INLINE bool AptString::IsConstantString()
{
    return str.IsConstantString();
}
