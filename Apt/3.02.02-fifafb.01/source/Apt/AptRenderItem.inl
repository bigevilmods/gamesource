/*H************************************************************************************************/
/*!

    \File    AptRenderItem.cpp

    \Description
        This is a placeholder file to implement functions for the AptRenderITem Class as needed.

    \Notes
        None.

    \Copyright
        (c) 2005 Electronic Arts Inc.

    \Version    1.0        03/30/05 (DC) Bringin' it to ya
    \Version    1.1        06/09/06 (ASK) Moved functions from header file to this file and added headers for AptRenderItem base class functions. 
    \Version    1.2        07/10/09 (RB)  Made some methods inline: GetPositionMatrixConst, GetColorMatrixConst, AddReference, ReleaseReference
*/
/************************************************************************************************H*/


/*** Include files ********************************************************************************/

/*** Defines **************************************************************************************/

/*** Macros ***************************************************************************************/

/*** Type Definitions *****************************************************************************/

/*** Function Prototypes **************************************************************************/

/*** Variables ************************************************************************************/

// Private variables

// Public variables


/*** Private Functions ****************************************************************************/

/*** Public Functions *****************************************************************************/

const AptCXForm * AptRenderItem::GetColorMatrixConst()    const
{
    if(mpColorMatrix == NULL)
    {
        return &gIdentityCXForm;
    }
    return mpColorMatrix;
}

const AptMatrix * AptRenderItem::GetPositionMatrixConst() const
{
    if(mpPositionMatrix == NULL)
    {
        return &gIdentityMatrix;
    }
    return mpPositionMatrix;
}




