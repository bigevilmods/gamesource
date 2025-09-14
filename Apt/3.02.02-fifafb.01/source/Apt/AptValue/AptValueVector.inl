/*H************************************************************************************************/
/*!

    \File
        AptValueVector.inl

    \Description
        Simple description of main purpose of file.

    \Notes
        None.

    \Copyright
        (c) 2004 Electronic Arts Inc.

    \Version
        1.0        03/29/04 (ONA) First Version

*/
/************************************************************************************************H*/

#ifndef __APTVALUEVECTOR_INL__
#define __APTVALUEVECTOR_INL__

#include "AptValue/AptValueVector.h"


/*F************************************************************************************************/
/*!
    Insert your long description here.

    \Input      None.

    \Output     AptValue *        - 

    \Notes
        None.

    \Version
        1.0        03/29/04 (ONA) First Version

*/
/************************************************************************************************F*/
AptValue *    AptValueVector::PopValue()
{
    APT_ASSERT(mCurrentNum > 0);
    return (mpValues[--mCurrentNum]);
}

/*F************************************************************************************************/
/*!
    Insert your long description here.

    \Input      None.

    \Output     int32_t           - 

    \Notes
        None.

    \Version
        1.0        03/29/04 (ONA) First Version

*/
/************************************************************************************************F*/
int32_t    AptValueVector::GetNumValues() const
{
    
    return mCurrentNum;
}


/*F************************************************************************************************/
/*!
    \Function    AptValueVector::IsVectorFull
    
    \Description
        This returns if the vector is full.
    
    \Input
        None.
        
    \Output     int32_t           - 
    
    \Notes
        None.
        
    \Version    1.0        05/12/04 (ASK) First Version
    
*/
/************************************************************************************************F*/
int32_t    AptValueVector::IsVectorFull() const
{
    if (mCurrentNum >= mCapacity)
    {
        return (true);
    }
    return (false);
}


#if defined(EA_DEBUG) || defined(APT_ENABLE_ZOMBIE_OUTPUT)
/*F************************************************************************************************/
/*!
    Insert your long description here.

    \Input      None.

    \Output     int32_t           - 

    \Notes
        None.

    \Version
        1.0        03/29/04 (ONA) First Version

*/
/************************************************************************************************F*/
int32_t    AptValueVector::GetHighWaterValues() const
{
    return mHighWaterNum;
}
#endif

#endif  //  __APTVALUEVECTOR_INL__

/*H*************************************************************************************************
    EOF
*************************************************************************************************H*/

