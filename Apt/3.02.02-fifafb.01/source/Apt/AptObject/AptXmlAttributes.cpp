/*H************************************************************************************************/
/*!

    \File
        AptXmlAttributes.cpp

    \Description
        Simple description of main purpose of file.

    \Notes
        None.

    \Copyright
        (c) 2004 Electronic Arts Inc.

    \Version
        1.0        03/07/04 (ONA) First Version

*/
/************************************************************************************************H*/

#include "AptXmlAttributes.h"
#include "AptXmlInterfaces.h"
#include "AptProf.h"
#include "AptValue/AptString.h"

#include "MainInline.h"

/*F************************************************************************************************/
/*!
    Destructor

    \Notes
        None.

    \Version
        1.0        03/07/04 (ONA) First Version

*/
/************************************************************************************************F*/
AptXmlAttributes::~AptXmlAttributes()
{
#if defined (APT_USE_XML_OBJECT)
    pIXmlNode = NULL;
#endif
}

#if defined (APT_USE_XML_OBJECT)
/*F************************************************************************************************/
/*!
    Constructor

    \Input      eType             -
    \Input      pIXmlNodeParam    -

    \Notes
        None.

    \Version
        1.0        03/07/04 (ONA) First Version

*/
/************************************************************************************************F*/
AptXmlAttributes::AptXmlAttributes(AptVirtualFunctionTable_Indices eType, IAptXmlNode * pIXmlNodeParam)
:
AptObject(eType),
pIXmlNode(pIXmlNodeParam)
{
    //  Do nothing...
}


/*F************************************************************************************************/
/*!
    Insert your long description here.

    \Input      None.

    \Output     None.

    \Notes
        None.

    \Version
        1.0        03/07/04 (ONA) First Version

*/
/************************************************************************************************F*/
void    AptXmlAttributes::PreDestroy()
{
    pIXmlNode = NULL ;
}

/*F************************************************************************************************/
/*!
    Insert your long description here.

    \Input      pContext          -
    \Input      szName            -
    \Input      pValue            -

    \Output     bool              -

    \Notes
        None.

    \Version
        1.0        03/26/04 (ONA) First Version

*/
/************************************************************************************************F*/
bool    AptXmlAttributes::objectMemberSet(AptValue * const pContext, const AptNativeString * const pName, AptValue * const pValue)
{
    APT_PROFILE(AptXmlAttributes_objectMemberSet);
    if (pContext->isXmlAttributes())
    {
        AptXmlAttributes * pXmlAttrib = pContext->c_xmlattributes() ;
        if (pValue->isString())
        {
            pXmlAttrib->pIXmlNode->setAttribute(pName->c_str(), pValue->c_string()->GetInternalString()->c_str() ) ;
        }
    }
    return  (true);
}
#endif  // #if defined (APT_USE_XML_OBJECT)
/*H*************************************************************************************************
    EOF
*************************************************************************************************H*/

