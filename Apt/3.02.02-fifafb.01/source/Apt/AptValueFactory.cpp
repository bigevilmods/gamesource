/*H************************************************************************************************/
/*!

    \File
        AptValueFactory.cpp

    \Description
        This source contains the methods definition to create Apt Value Factory.

    \Notes
        None.

    \Copyright
        (c) 2004 Electronic Arts Inc.

    \Version
        1.0        12/20/04 (DUG) First Version
        1.1        04/30/08 ($senzee) Added locking for these external API functions for when called
                            on other threads.

*/
/************************************************************************************************H*/
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "_Apt.h"
#include "AptValueFactory.h"
#include "MainInline.h"

/*F************************************************************************************************/
/*!
\Function    AptValueFactory::CreateString

\Description
Insert function's purpose here

Insert your long description here.

\Input      szValue         -

\Output
None.

\Notes
None.

\Version    1.0        12/20/04 (DUG) First Version

*/
/************************************************************************************************F*/
AptValue * AptValueFactory::CreateString(const char *szValue)
{
    APT_ASSERT(szValue != NULL);

    AptUpdateAutoLock lock;

    AptValue *pValue = AptString::Create(szValue);

    return pValue;

}


/*F************************************************************************************************/
/*!
\Function    AptValueFactory::CreateInteger

\Description
Insert function's purpose here

Insert your long description here.

\Input      szFormat, ...          -

\Output
None.

\Notes
None.

\Version    1.0        12/20/04 (DUG) First Version

*/
/************************************************************************************************F*/
AptValue * AptValueFactory::CreateInteger(int nValue)
{
    AptUpdateAutoLock lock;

    AptInteger *pValue = AptInteger::Create(nValue);

    return pValue;
}

/*F************************************************************************************************/
/*!
\Function    AptValueFactory::CreateFloat

\Description
Insert function's purpose here

Insert your long description here.

\Input      szFormat, ...          -

\Output
None.

\Notes
None.

\Version    1.0        12/20/04 (DUG) First Version

*/
/************************************************************************************************F*/
AptValue * AptValueFactory::CreateFloat( float fValue)
{
    AptUpdateAutoLock lock;

    AptValue *pValue = (AptValue *) AptFloat::Create(fValue);

    return pValue;
}

/*F************************************************************************************************/
/*!
\Function    AptValueFactory::CreateBoolean

\Description
Insert function's purpose here

Insert your long description here.

\Input      bValue         -

\Output     AptValue *


\Notes
None.

\Version    1.0        12/20/04 (DUG) First Version

*/
/************************************************************************************************F*/
AptValue * AptValueFactory::CreateBoolean( bool bValue)
{
    AptUpdateAutoLock lock; // don't really need a lock here in boolean..

    AptValue *pValue = AptBoolean::Create(bValue);

    return pValue;

}

/*F************************************************************************************************/
/*!
\Function    AptValueFactory::CreateArray

\Description
Insert function's purpose here

Insert your long description here.

\Input      nElements           -
            pAptValues          -

\Output     AptValue *


\Notes
None.

\Version    1.0        12/20/04 (DUG) First Version

*/
/************************************************************************************************F*/
AptValue * AptValueFactory::CreateArray(int nElements, AptValue ** pAptValue)
{
    AptUpdateAutoLock lock;

    if (nElements)
    {
        APT_ASSERT(pAptValue != NULL);
    }

    AptValue *pValue = new AptArray(nElements, pAptValue);

    return pValue;
}

/******************************************************************************/
/*!
    \brief  Create and return an empty array

    \return         - A new AptArray containing nothing
*/
/******************************************************************************/
AptValue * AptValueFactory::CreateArray()
{
    AptUpdateAutoLock lock;
    return new AptArray();
}

/******************************************************************************/
/*!
    \brief  Create and return an empty object

    \return         - A new object with no properties
*/
/******************************************************************************/
AptValue * AptValueFactory::CreateObject()
{
    AptUpdateAutoLock lock;
    return (new AptObject(AptVFT_Object));
}


/*F************************************************************************************************/
/*!
\Function    AptValueFactory::CreateStringFormatted

\Description
Insert function purpose here

Insert your long description here.

\Input      szFormat, ...          -

\Output
None.

\Notes
To be added later

\Version    1.0        12/20/04 (DUG) First Version

*/
/************************************************************************************************F*/
AptValue * AptValueFactory::CreateStringFormatted(const char *szFormat, ...)
{
    APT_ASSERT(szFormat != NULL);

    AptUpdateAutoLock lock;

    AptString *pValue = AptString::Create("");

    va_list Args;

    va_start(Args, szFormat);   //  Initialize variable arguments.

    AptNativeString *ans = pValue->GetInternalString();
    ans->vsFormat(szFormat, Args);

    va_end(Args);
    return pValue;
}


/*F************************************************************************************************/
/*!
\Function    AptValueFactory::CreateUndefined

\Description
This function will return an AptValue * which points to gpUndefinedValue

\Input
None.

\Output     AptValue *        -

\Notes
Game code might want to pass back an UndefinedValue in some cases to Apt.
#FR862 AptValueFactory - need to create undefined Value

\Version    1.0        03/07/05 (ASK) First Version

*/
/************************************************************************************************F*/
AptValue * AptValueFactory::CreateUndefined()
{
    return gpUndefinedValue ;
}



/*H*************************************************************************************************
    EOF
*************************************************************************************************H*/

