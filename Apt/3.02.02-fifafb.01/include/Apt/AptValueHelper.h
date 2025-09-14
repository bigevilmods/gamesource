/*H************************************************************************************************/
/*!

    \File    AptValueHelper.h

    \Description
        This is a helper class that exposes functions to mainly lookup members from hash tables inside
        AptValue

    \Notes
        None.

    \Copyright
        (c) 2005 Electronic Arts Inc.

    \Version    1.0        07/19/05 (ASK) First Version
    \Version    2.1        06/06/08 (RB)  Added IsMovieClipVisible
    \Version    2.2        10/01/08 (ASK) Added NumberToFloat, NumberToInteger

*/
/************************************************************************************************H*/


#ifndef _AptValueHelper_h
#define _AptValueHelper_h

/*** Include files ********************************************************************************/


#include "eabase/eabase.h"
#include "AptString\EAString.h"
// FIFA_BEGIN | SENDBACK | mwebb | 2015 - 05 - 26 | Adding Frostbite support
#include "Module.h"
// FIFA_END

/*** Defines **************************************************************************************/

/*** Macros ***************************************************************************************/

/*** Type Definitions *****************************************************************************/

/*** Variables ************************************************************************************/

/*** Functions ************************************************************************************/

// Forward Declarations
#include "AptValue\AptValue.h"
#include "AptValue\AptArray.h"


// FIFA_BEGIN | SENDBACK | mwebb | 2015 - 05 - 26 | Adding Frostbite support
//class AptValueHelper
class FB_APT_API AptValueHelper
// FIFA_END
{
public:
    // All these functions return true only if pValue has a hash table inside it.
    // Last parameters are OUT parameters which will return right value back.
    static bool GetMember(AptValue * pValue, const char * szValue, int32_t * pOutInteger);
    static bool GetMember(AptValue * pValue, const char * szValue, float_t * pOutFloat);
    static bool GetMember(AptValue * pValue, const char * szValue, bool * pOutBool);
    static bool GetMember(AptValue * pValue, const char * szValue, char8_t * pOutString, uint32_t nMaxChars);

    static bool GetMember(AptValue * pValue, const AptNativeString * pStringKey, int32_t * pOutInteger);
    static bool GetMember(AptValue * pValue, const AptNativeString * pStringKey, float_t * pOutFloat);
    static bool GetMember(AptValue * pValue, const AptNativeString * pStringKey, bool * pOutBool);
    static bool GetMember(AptValue * pValue, const AptNativeString * pStringKey, char8_t * pOutString, uint32_t nMaxChars);

    static bool GetArrayMember(AptValue * pValue, int32_t nIndex, int32_t * pOutInteger);
    static bool GetArrayMember(AptValue * pValue, int32_t nIndex, float_t * pOutFloat);
    static bool GetArrayMember(AptValue * pValue, int32_t nIndex, bool * pOutBool);
    static bool GetArrayMember(AptValue * pValue, int32_t nIndex, char8_t * pOutString, uint32_t nMaxChars);

    // array index can be a string also.
    static bool GetArrayMember(AptValue * pValue, const char * szValue, int32_t * pOutInteger);
    static bool GetArrayMember(AptValue * pValue, const char * szValue, float_t * pOutFloat);
    static bool GetArrayMember(AptValue * pValue, const char * szValue, bool * pOutBool);
    static bool GetArrayMember(AptValue * pValue, const char * szValue, char8_t * pOutString, uint32_t nMaxChars);

    // these functions are added to AptValueHelper class just because some of the other libraries are
    // including _apt.h for creating and accessing AptArray class. But when we are giving out Apt libs to external developers
    // they do not get _Apt.h and because of that these other libs could not be built. So exposing required AptArray functions
    // through AptValueHelper Class.
    static AptValue *   GetFromAptArray(AptValue * pArray, int nIndex) ;
    static void         SetInAptArray(AptValue * pArray, int nIndex, AptValue * pValue) ;
    static int          GetArrayLength(AptValue *pArray) ;

    static AptValue *   GetMemberAsAptValue(AptValue * pValue, const char * szValue);
    static AptValue *   GetMemberAsAptValue(AptValue * pValue, const AptNativeString * pKey);

    // Indicates whether or not a MovieClip is visible. Returns true only if the _visible property of all
    // the MovieClip's parents, along with given MovieClip's own _visible property, is true. Otherwise
    // returns false; also returns false if pValue isn't a MovieClip.
    static bool IsMovieClipVisible(AptValue * pValue);

    // Generic translation functions to convert AptValues to various primitive types
    static int   NumberToInteger(AptValue *value);
    static float NumberToFloat  (AptValue *value);
    static bool  GetBoolean     (AptValue *value);
    static const char* GetString(AptValue *value, int* length);

    // Functions to pull values out of Apt Objects.
    // Usage:
    // for (AptHashItem* item = AptValueHelper::GetObjectItemFirst(object) ; item ; item = AptValueHelper::GetObjectItemNext(object, item))
    // {
    //     ...
    // }
    static AptHashItem* GetObjectItemFirst(AptValue* value);
    static AptHashItem* GetObjectItemNext(AptValue* value, AptHashItem* item);

    // Returns the rendering unit of the first shape found in the given MovieClip.
    // Returns NULL if no shape or rendering unit found.
    static AptAssetRenderingUnit GetGeometryFromMovieClip(AptValue* pValue);

    // Returns the texture ID of the first shape found in the given MovieClip.
    // Returns -1 if no shape or texture found.
    static int32_t GetTextureIdFromMovieClip(AptValue* pValue);


    //
    // These return an AptValue* (unlike AptGetInternalVariable). Call APT_DEC on *ppOut when you're done with it.
    //

    // Returns the "undefined" value
    // Call APT_DEC on *ppOut when you are done with it.
    static void GetUndefined(AptValue ** ppOut);

    // Returns _global
    // Call APT_DEC on *ppOut when you are done with it.
    static void GetGlobal(AptValue ** ppOut);

    // Get this context.
    // This function is only available when AptDebugger is enabled.
    static void GetThis(AptValue* currentContext, AptValue** output, int level);

    // Get super context
    // This function is only available when AptDebugger is enabled.
    static void GetSuper(AptValue* currentContext, AptValue** output, int callStackLevel);

    // Returns _leveln, where n is a number; returns NULL if _level n does not yet exist
    // Call APT_DEC on *ppOut when you are done with it.
    static void GetLevel(int n, AptValue ** ppOut);

    // Returns a child member of pValue, or NULL if it doesn't exist. Recurses;
    // member name szValue can have a period in it.
    // Unlike GetMemberAsAptValue, this method calls APT_INC on the return value before returning.
    // Call APT_DEC on *ppOut when you are done with it.
    static void GetMember(AptValue * pValue, const char * szValue, AptValue ** ppOut, int callStackLevel = 0);

    // Return variable value specified by name as it's member of "this".
    static AptValue* GetThisMember(AptValue* context, const char* strName, int isFunctionScope, int level);

    // Gets member names of pValue, if pValue is an AptCIH or AptObject.
    // Only returns names of members not intrinsic to Apt (e.g. _x and _y would be excluded).
    // - this function allocates no memory, so on input buf must be a buffer of at least size maxChars
    // - on return, buf will contain the members of pValue separated by a colon (:)
    // - returns the number of chars written to buf (excluding trailing null)
    static int  GetExtrinsicMemberNames(AptValue * pValue, char* buf, int maxChars, bool isSuper);

    // Gets the value of an intrinsic member of an AptCIH (e.g., _x, _y, _alpha).
    static bool GetIntrinsicMember(AptValue *pValue, const char *szValue, int *pOutInteger);
    static bool GetIntrinsicMember(AptValue *pValue, const char *szValue, float *pOutFloat);
    static bool GetIntrinsicMember(AptValue *pValue, const char *szValue, bool *pOutBool);
    static bool GetIntrinsicMember(AptValue *pValue, const char *szValue, char *pOutString, int nMaxChars);

    // Get all local variables' name list by specific function runtime context
    static int GetLocalNameList(AptValue* currentContext, char* buf, int maxChars);

    //
    // These set a child member of pValue. Member name szValue can have a period in it.
    //
    static void SetMember( AptValue * pValue, const char * szValue, int newVal );
    static void SetMember( AptValue * pValue, const char * szValue, float newVal );
    static void SetMember( AptValue * pValue, const char * szValue, bool newVal );
    static void SetMember( AptValue * pValue, const char * szValue, const char * newVal );
    static void SetMember( AptValue * pValue, const char * szValue, AptValue* newValue);

};  // end of AptValueHelper


#endif // _AptValueHelper_h
