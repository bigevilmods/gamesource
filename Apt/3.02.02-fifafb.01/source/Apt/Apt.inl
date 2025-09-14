/*H************************************************************************************************/
/*!

    \File    Apt\Apt.inl

    \Description
        Simple description of main purpose of file.

    \Notes
        None.

    \Copyright
        (c) 2005 Electronic Arts Inc.

    \Version    1.0        09/09/05 (LEO) First Version

*/
/************************************************************************************************H*/


#ifndef _Apt_Apt_inl
#define _Apt_Apt_inl

#if defined(_MSC_VER)
#pragma once
#endif

/*** Include files ********************************************************************************/

/*** Defines **************************************************************************************/

/*** Macros ***************************************************************************************/

/*** Type Definitions *****************************************************************************/

/*** Variables ************************************************************************************/

/*** Functions ************************************************************************************/
/*! \Returns the currently set sim target */
AptTarget *GetTargetSim()
{
    return gpCurrentTargetSim;
}

/*! \Returns the currently set render target */
AptTarget *GetTargetRender()
{
    return gpCurrentTargetRender;
}

#endif // _Apt_Apt_inl
