/*H************************************************************************************************/
/*!

    \File    AptValue.inl

    \Description
        Simple description of main purpose of file.

    \Notes
        None.

    \Copyright
        (c) 2004 Electronic Arts Inc.

    \Version    1.0        03/30/04 (JSH) First Version
    \Version    1.1        11/01/04 (DC)  Updated to use bit-field members instead of explicit bit
                                          operations (which were confusing and poorly maintained at 
                                          best).
    \Version    1.2        06/12/08 (JP)  Adding support for the MovieClipLoader class 
    \Version    1.3        08/14/08 ($senzee) Adding support for the AptUtil class 

*/
/************************************************************************************************H*/

/*** Include files ********************************************************************************/
#include "_Apt.h"
#include "AptValue/AptValue.h"
#include "AptValue/AptValueVector.h"
#include "AptGlobal.h"

/*** Defines **************************************************************************************/

/*** Macros ***************************************************************************************/

/*** Type Definitions *****************************************************************************/

/*** Variables ************************************************************************************/

/*** Functions ************************************************************************************/

/*F************************************************************************************************/
/*!
    \Function    *AptValue::c_lookup
    
    \Description
        Insert function's purpose here
        
    \Input
        None.
        
    \Output     AptLookup         * - 
    
    \Notes
        None.
        
    \Version    1.0        03/30/04 (JSH) First Version
    
*/
/************************************************************************************************F*/
AptLookup *AptValue::c_lookup() const
{
	APT_ASSERT(isLookup());
	return (AptLookup *)this;
}

/*F************************************************************************************************/
/*!
    \Function    *c_integer
    
    \Description
        Insert function's purpose here
        
    \Input
        None.
        
    \Output     AptInteger        * - 
    
    \Notes
        None.
        
    \Version    1.0        03/30/04 (JSH) First Version
    
*/
/************************************************************************************************F*/
AptInteger *AptValue::c_integer() const
{
	APT_ASSERT(isInteger());
	return (AptInteger *)this;
}

/*F************************************************************************************************/
/*!
    \Function    *c_register
    
    \Description
        Insert function's purpose here
        
    \Input
        None.
        
    \Output     AptRegister       * - 
    
    \Notes
        None.
        
    \Version    1.0        03/30/04 (JSH) First Version
    
*/
/************************************************************************************************F*/
AptRegister *AptValue::c_register() const
{
	APT_ASSERT(isRegister());
	return (AptRegister *)this;
}

/*F************************************************************************************************/
/*!
    \Function    *AptValue::c_float
    
    \Description
        Insert function's purpose here
        
    \Input
        None.
        
    \Output     AptFloat          * - 
    
    \Notes
        None.
        
    \Version    1.0        03/30/04 (JSH) First Version
    
*/
/************************************************************************************************F*/
AptFloat *AptValue::c_float() const
{
	APT_ASSERT(isFloat());
	return (AptFloat *)this;
}

/*F************************************************************************************************/
/*!
    \Function    *AptValue::c_boolean
    
    \Description
        Insert function's purpose here
        
    \Input
        None.
        
    \Output     AptBoolean        * - 
    
    \Notes
        None.
        
    \Version    1.0        03/30/04 (JSH) First Version
    
*/
/************************************************************************************************F*/
AptBoolean *AptValue::c_boolean() const
{
	APT_ASSERT(isBoolean());
	return (AptBoolean *)this;
}

/*F************************************************************************************************/
/*!
    \Function    *AptValue::c_scriptfunction
    
    \Description
        Returns this pointer casted to AptScriptFunction
        
    \Input
        None.
        
    \Output     AptScriptFunction  -  *   
    
    \Notes
        None.
        
    \Version    1.0        03/30/04 (JSH) First Version
    
*/
/************************************************************************************************F*/
AptScriptFunctionBase *AptValue::c_scriptfunction() const
{
	APT_ASSERT(isScriptFunction());
	return (AptScriptFunctionBase *)this;
}

/******************************************************************************/
/*!
    \brief  Return this item as an AptNativeFunction

    \return         - Cast pointer to this
*/
/******************************************************************************/
AptNativeFunction * AptValue::c_nativefunction() const
{
    APT_ASSERT(isNativeFunction());
    return (AptNativeFunction *)this;
}

/******************************************************************************/
/*!
    \brief  Return this item as an AptExternalFunction

    \return         - Cast pointer to this
*/
/******************************************************************************/
AptExternalFunction * AptValue::c_externalfunction() const
{
    APT_ASSERT(isExternalFunction());
    return (AptExternalFunction *)this;
}

/*F************************************************************************************************/
/*!
    \Function    *AptValue::c_cih
    
    \Description
        Insert function's purpose here
    
    \Input      false             - 
    
    \Output     AptCIH            * - 
    
    \Notes
        None.
        
    \Version    1.0        03/30/04 (JSH) First Version
    
*/
/************************************************************************************************F*/
AptCIH *AptValue::c_cih(bool bUndefOK) const
{
	APT_ASSERT(isCIH(bUndefOK));
	return (AptCIH *)this;
}

#if defined(APT_USE_SOUND_OBJECT)
/*F************************************************************************************************/
/*!
    \Function    *AptValue::c_sound
    
    \Description
        Insert function's purpose here
        
    \Input
        None.
        
    \Output     AptSound          * - 
    
    \Notes
        None.
        
    \Version    1.0        03/30/04 (JSH) First Version
    
*/
/************************************************************************************************F*/
AptSound *AptValue::c_sound() const
{
	APT_ASSERT(isSound());
	return (AptSound *)this;
}
#endif

/*F************************************************************************************************/
/*!
    \Function    *AptValue::c_key
    
    \Description
        Insert function's purpose here
        
    \Input
        None.
        
    \Output     AptKey            * - 
    
    \Notes
        None.
        
    \Version    1.0        03/30/04 (JSH) First Version
    
*/
/************************************************************************************************F*/
AptKey *AptValue::c_key() const
{
	APT_ASSERT(isKey());
	return (AptKey *)this;
}

#if defined(APT_USE_MOUSE)
/*F************************************************************************************************/
/*!
    \Function    AptValue::*c_mouse
    
    \Description
        Insert function's purpose here
        
    \Input
        None.
        
    \Output     inline AptMouse   * - 
    
    \Notes
        None.
        
    \Version    1.0        03/30/04 (JSH) First Version
    
*/
/************************************************************************************************F*/
inline AptMouse *AptValue::c_mouse() const
{
	APT_ASSERT(isMouse());
	return (AptMouse *)this;
}
#endif

/*F************************************************************************************************/
/*!
    \Function    AptValue::*c_global
    
    \Description
        Insert function's purpose here
        
    \Input
        None.
        
    \Output     inline AptGlobal  * - 
    
    \Notes
        None.
        
    \Version    1.0        03/30/04 (JSH) First Version
    
*/
/************************************************************************************************F*/
inline AptGlobal *AptValue::c_global() const
{
	APT_ASSERT(isKey());
	return (AptGlobal *)this;
}

#if APT_USE_MATH_OBJECT
/*F************************************************************************************************/
/*!
    \Function    *AptValue::c_math
    
    \Description
        Insert function's purpose here
        
    \Input
        None.
        
    \Output     AptMathObj        * - 
    
    \Notes
        None.
        
    \Version    1.0        03/30/04 (JSH) First Version
    
*/
/************************************************************************************************F*/
AptMathObj *AptValue::c_math() const
{
	APT_ASSERT(isMath());
	return (AptMathObj *)this;
}
#endif

#if APT_USE_SCRIPTCOLOUR_OBJECT
/*F************************************************************************************************/
/*!
    \Function    *AptValue::c_scriptcolour
    
    \Description
        Insert function's purpose here
        
    \Input
        None.
        
    \Output     AptScriptColour   * - 
    
    \Notes
        None.
        
    \Version    1.0        03/30/04 (JSH) First Version
    
*/
/************************************************************************************************F*/
AptScriptColour *AptValue::c_scriptcolour() const
{
	APT_ASSERT(isScriptColour());
	return (AptScriptColour *)this;
}
#endif

/*F************************************************************************************************/
/*!
    \Function    *AptValue::c_object
    
    \Description
        Insert function's purpose here
        
    \Input
        None.
        
    \Output     AptObject         * - 
    
    \Notes
        None.
        
    \Version    1.0        03/30/04 (JSH) First Version
    
*/
/************************************************************************************************F*/
AptObject *AptValue::c_object() const
{
	APT_ASSERT(isObject());
	return (AptObject *)this;
}

/*F************************************************************************************************/
/*!
    \Function    *AptValue::c_prototype
    
    \Description
        Insert function's purpose here
        
    \Input
        None.
        
    \Output     AptPrototype      * - 
    
    \Notes
        None.
        
    \Version    1.0        03/30/04 (JSH) First Version
    
*/
/************************************************************************************************F*/
AptPrototype *AptValue::c_prototype() const
{
	APT_ASSERT(isPrototype());
	return (AptPrototype *)this;
}

/*F************************************************************************************************/
/*!
    \Function    *AptValue::c_date
    
    \Description
        Insert function's purpose here
        
    \Input
        None.
        
    \Output     AptDate           * - 
    
    \Notes
        None.
        
    \Version    1.0        03/30/04 (JSH) First Version
    
*/
/************************************************************************************************F*/
AptDate *AptValue::c_date() const
{
	APT_ASSERT(isDate());
	return (AptDate *)this;
}

/*F************************************************************************************************/
/*!
    \Function    *AptValue::c_movieClipLoader
    
    \Description Casts to MovieClipLoader
        
    \Input      None.
        
    \Output     MovieClipLoader pointer 
    
    \Notes      None.
        
    \Version    1.0        06/16/08 (JP) First Version
    
*/
/************************************************************************************************F*/
AptMovieClipLoader *AptValue::c_movieClipLoader() const
{
	APT_ASSERT(isMovieClipLoader());
	return (AptMovieClipLoader *)this;
}


/*F************************************************************************************************/
/*!
    \Function    AptValue::*c_textformat
    
    \Description
        Insert function's purpose here
        
    \Input
        None.
        
    \Output     inline AptTextFormat  * - 
    
    \Notes
        None.
        
    \Version    1.0        03/30/04 (JSH) First Version
    
*/
/************************************************************************************************F*/
AptTextFormat *AptValue::c_textformat() const
{
	APT_ASSERT(isTextFormat());
	return (AptTextFormat *)this;
}

/*F************************************************************************************************/
/*!
    \Function    *AptValue::c_movieClip
    
    \Description
        Insert function's purpose here
        
    \Input
        None.
        
    \Output     AptMovieClip      * - 
    
    \Notes
        None.
        
    \Version    1.0        03/30/04 (JSH) First Version
    
*/
/************************************************************************************************F*/
AptMovieClip *AptValue::c_movieClip() const
{
	APT_ASSERT(isMovieClip());
	return (AptMovieClip *)this;
}

/*F************************************************************************************************/
/*!
    \Function    AptValue::c_xmlnode
    
    \Description
        Insert function's purpose here
        
    \Input
        None.
        
    \Output     AptXmlNode*       - 
    
    \Notes
        None.
        
    \Version    1.0        03/30/04 (JSH) First Version
    
*/
/************************************************************************************************F*/
AptXmlNode* AptValue::c_xmlnode() const
{
    APT_ASSERT(isXmlNode());
    return (AptXmlNode *) this ;
}

/*F************************************************************************************************/
/*!
    \Function    AptValue::c_xml
    
    \Description
        Insert function's purpose here
        
    \Input
        None.
        
    \Output     AptXml*           - 
    
    \Notes
        None.
        
    \Version    1.0        03/30/04 (JSH) First Version
    
*/
/************************************************************************************************F*/
AptXml* AptValue::c_xml() const
{
    APT_ASSERT(isXml());
    return (AptXml *) this;
}

/*F************************************************************************************************/
/*!
    \Function    AptValue::c_xmlattributes
    
    \Description
        Insert function's purpose here
        
    \Input
        None.
        
    \Output     AptXmlAttributes*  -    
    
    \Notes
        None.
        
    \Version    1.0        03/30/04 (JSH) First Version
    
*/
/************************************************************************************************F*/
AptXmlAttributes* AptValue::c_xmlattributes() const
{
    APT_ASSERT(isXmlAttributes());
    return (AptXmlAttributes *) this;
}

#if APT_USE_LOADVARS_OBJECT
/*F************************************************************************************************/
/*!
    \Function    *AptValue::c_loadvars
    
    \Description
        Insert function's purpose here
        
    \Input
        None.
        
    \Output     AptLoadVars       - 
    
    \Notes
        None.
        
    \Version    1.0        03/30/04 (JSH) First Version
    
*/
/************************************************************************************************F*/
AptLoadVars *AptValue::c_loadvars() const
{
    APT_ASSERT(isLoadVars());
    return (AptLoadVars *) this ;
}
#endif

/*F************************************************************************************************/
/*!
    \Function    *AptValue::c_stage
    
    \Description
        Insert function's purpose here
        
    Insert your long description here.
    
    \Input
        None.
        
    \Output     AptStage          - 
    
    \Notes
        None.
        
    \Version    1.0        06/28/04 (LEO) First Version
    
*/
/************************************************************************************************F*/
AptStage *AptValue::c_stage() const
{
    APT_ASSERT(isStage());
    return (AptStage *)this;
}

#if APT_USE_UTILITY
/*F************************************************************************************************/
/*!
    \Function    AptValue::c_util
    
    \Description
        Cast to an AptUtil instance
        
    \Input
        None.
        
    \Output     AptUtil           * - 
    
    \Notes
        None.
        
    \Version    1.0        07/03/08 ($senzee) first version
    
*/
/************************************************************************************************F*/
AptUtil *AptValue::c_util() const
{
    APT_ASSERT(isAptUtil());
    return (AptUtil *)this;
}
#endif