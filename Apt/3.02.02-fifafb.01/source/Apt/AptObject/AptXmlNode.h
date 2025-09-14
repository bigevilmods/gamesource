/*H************************************************************************************************/
/*!

    \File
        AptXmlNode.h

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

#ifndef __APTXMLNODE_H__
#define __APTXMLNODE_H__

#include "AptObject/AptObject.h"
#include "_AptInternalDefines.h"

class IAptXmlNode;
class AptArray;

class AptXmlNode : public AptObject
{
public:
#if defined (APT_USE_XML_OBJECT)

    AptXmlNode(AptVirtualFunctionTable_Indices eType, IAptXmlNode * pIXmlNodeParam = NULL);

    virtual void        PreDestroy();

    virtual AptValue *  objectMemberLookup(AptValue * const pContext, const AptNativeString * const pName) const;
    virtual bool        objectMemberSet(AptValue * const pContext, const AptNativeString * const pName, AptValue * const pValue);

    NATIVE_MEMBER_FUNCTION_DECL(appendChild) ;
    NATIVE_MEMBER_FUNCTION_DECL(cloneNode) ;
    NATIVE_MEMBER_FUNCTION_DECL(hasChildNodes) ;
    NATIVE_MEMBER_FUNCTION_DECL(insertBefore) ;
    NATIVE_MEMBER_FUNCTION_DECL(removeNode) ;
    NATIVE_MEMBER_FUNCTION_DECL(toString) ;

    IAptXmlNode *   pIXmlNode;      // this object could be pointing to IAptXml also
    AptArray *      paChildNodes;
#else
    AptXmlNode(AptVirtualFunctionTable_Indices eType, IAptXmlNode * pIXmlNodeParam = NULL):AptObject(AptVFT_XmlNode)
    {
        APT_ASSERT(0 && "APT-Warning-XML object cannot be used as it is compiled out, make sure to compile with APT_USE_XML_OBJECT defined") ;
    }
#endif
    static void         CleanNativeFunctions();
protected:
    virtual ~AptXmlNode();

    APT_VALUE_GC_NEW_DELETE_OPERATORS
    // DC - Added Metric defines
    APT_DECLARE_OBJECT_METRICS
};

#endif  //  __APTXMLNODE_H__

/*H*************************************************************************************************
    EOF
*************************************************************************************************H*/

