/*H************************************************************************************************/
/*!

    \File    Apt\AptObject\AptXmlMembers.h

    \Description
        Simple description of main purpose of file.

    \Notes
        None.

    \Copyright
        (c) 2005 Electronic Arts Inc.

    \Version    1.0        09/06/05 (LEO) First Version

*/
/************************************************************************************************H*/


#ifndef _Apt_AptObject_AptXmlMembers_h
#define _Apt_AptObject_AptXmlMembers_h

#if defined(_MSC_VER)
#pragma once
#endif

/*** Include files ********************************************************************************/

/*** Defines **************************************************************************************/

/*** Macros ***************************************************************************************/

/*** Type Definitions *****************************************************************************/
/* C++ code produced by gperf version 2.7.2 */
/* Command-line: '..\\..\\bin\\util\\gperf.exe' -t -K szName -E -L C++ -Z XmlMemberIndex -k 2,6,8 xml.gperf  */
struct XmlMembers { char *szName; int nIndex; };
/* maximum key range = 73, duplicates = 0 */

class XmlMemberIndex
{
private:
    static inline unsigned int hash (const char *str, unsigned int len);
public:
    static struct XmlMembers *in_word_set (const char *str, unsigned int len);
};

/*** Variables ************************************************************************************/

/*** Functions ************************************************************************************/

#endif // _Apt_AptObject_AptXmlMembers_h
