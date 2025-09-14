/*H************************************************************************************************/
/*!

    \File    Apt\AptSpriteMembers.h

    \Description
        Simple description of main purpose of file.

    \Notes
        None.

    \Copyright
        (c) 2005 Electronic Arts Inc.

    \Version    1.0        09/06/05 (LEO) First Version
    \Version    1.1        08/25/08 ($senzee) Added loadMovieNum, unloadMovieNum, regenerated file

*/
/************************************************************************************************H*/


#ifndef _Apt_AptSpriteMembers_h
#define _Apt_AptSpriteMembers_h

#if defined(_MSC_VER)
#pragma once
#endif

/*** Include files ********************************************************************************/

/*** Defines **************************************************************************************/

/*** Macros ***************************************************************************************/

/*** Type Definitions *****************************************************************************/
/* C++ code produced by gperf version 2.7.2 */
/* Command-line: '..\\..\\bin\\util\\gperf' -t -K szName -E -D -L C++ -Z SpriteMembersIndex -k 2,6,8 sprite.gperf  */
struct SpriteMembers { char *szName; int nIndex; };
/* maximum key range = 238, duplicates = 1 */

class SpriteMembersIndex
{
private:
  static inline unsigned int hash (const char *str, unsigned int len);
public:
  static struct SpriteMembers *in_word_set (const char *str, unsigned int len);
};
/*** Variables ************************************************************************************/

/*** Functions ************************************************************************************/

#endif // _Apt_AptSpriteMembers_h
