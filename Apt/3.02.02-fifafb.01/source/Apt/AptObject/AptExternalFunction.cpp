/******************************************************************************/
/*!
    Copyright 2012 Electronic Arts Inc.

    \file       AptExternalFunction.cpp

    \brief      Function class, extendable outside of Apt
*/
/******************************************************************************/

/*** Includes *****************************************************************/

#include "AptObject/AptExternalFunction.h"

#include "_AptValue.h"

/*** Macros *******************************************************************/

/*** Namespaces ***************************************************************/

/*** Constants ****************************************************************/

/*** Declarations *************************************************************/

/*** Variables ****************************************************************/

/*** Implementation ***********************************************************/

/******************************************************************************/
/*!
    \brief  Constructor

    \param  classSize    Size of the class, eg. sizeof(Blah)
*/
/******************************************************************************/
AptExternalFunction::AptExternalFunction(uint32_t classSize)
    : AptObject(AptVFT_ExternalFunction)
    , mClassSize(classSize)
{
    // I'm guessing this should be the same as AptNativeFunction.
    SetAllowDelayedDeletion(false);
}

/******************************************************************************/
/*!
    \brief  Destructor
*/
/******************************************************************************/
AptExternalFunction::~AptExternalFunction()
{
    // Don't mess with mClassSize - believe it or not, we need to access that
    // value AFTER the destructor is called (super sigh)
}

/******************************************************************************/
/*!
    \brief  Return the size of this class

    \return         - Required for a place in Apt where we need to get the size
                      of each object.  Most objects look in some global lookup
                      table but that doesn't make sense for extensions
*/
/******************************************************************************/
uint32_t AptExternalFunction::GetClassSize() const
{
    return mClassSize;
}

/******************************************************************************/

