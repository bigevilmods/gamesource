/*H************************************************************************************************/
/*!

    \File
        AptXml.h

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

#ifndef __APTXML_H__
#define __APTXML_H__

#include "AptObject/AptXmlNode.h"

class AptXml : public AptXmlNode
{
public:
#if defined (APT_USE_XML_OBJECT)
    APT_VALUE_GC_NEW_DELETE_OPERATORS
    APT_DECLARE_OBJECT_METRICS

    AptXml(IAptXmlNode * pIXmlParam);
    explicit AptXml(AptValue * pszSource = NULL);

    virtual void    PreDestroy();

    virtual AptValue *  objectMemberLookup(AptValue * const pContext, const AptNativeString * const pName) const;

//  NATIVE_MEMBER_FUNCTION_DECL(contentType) ;
////    NATIVE_MEMBER_FUNCTION_DECL(createElement) ;
////    NATIVE_MEMBER_FUNCTION_DECL(createTextNode) ;
//  NATIVE_MEMBER_FUNCTION_DECL(docTypeDecl) ;
    NATIVE_MEMBER_FUNCTION_DECL(getBytesTotal) ;
    NATIVE_MEMBER_FUNCTION_DECL(getBytesLoaded) ;
//  NATIVE_MEMBER_FUNCTION_DECL(ignoreWhite) ;
    NATIVE_MEMBER_FUNCTION_DECL(load) ;
//  NATIVE_MEMBER_FUNCTION_DECL(isLoaded) ;
    NATIVE_MEMBER_FUNCTION_DECL(parseXml) ;
////    NATIVE_MEMBER_FUNCTION_DECL(send) ;
////    NATIVE_MEMBER_FUNCTION_DECL(sendAndLoad) ;
//  NATIVE_MEMBER_FUNCTION_DECL(status) ;
#else
    AptXml(IAptXmlNode * pIXmlParam):AptXmlNode(AptVFT_Xml, pIXmlParam)
    {
        APT_ASSERT(0 && "APT-Warning-XML object cannot be used as it is compiled out, make sure to compile with APT_USE_XML_OBJECT defined") ;
    }
    explicit AptXml(AptValue * pszSource = NULL):AptXmlNode(AptVFT_Xml, NULL)
    {
        APT_ASSERT(0 && "APT-Warning-XML object cannot be used as it is compiled out, make sure to compile with APT_USE_XML_OBJECT defined") ;
    }
#endif

    static void     CleanNativeFunctions();
protected:
    virtual ~AptXml();
};
#endif  //  __APTXML_H__

/*H*************************************************************************************************
    EOF
*************************************************************************************************H*/

