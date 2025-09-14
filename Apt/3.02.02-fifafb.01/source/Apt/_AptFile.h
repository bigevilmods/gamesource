/*H************************************************************************************************/
/*!

\File
_AptFile.h

\Description
Simple description of main purpose of file.

\Notes
None.

\Copyright
(c) 2005 Electronic Arts Inc.

\Version    1.0     01/04/05  (MCB)   First Version
*/
/************************************************************************************************H*/

#ifndef __APTFILE_H__
#define __APTFILE_H__

struct AptConstantTable
{
    AptVirtualFunctionTable_Indices eType;
    union {
        char *szString;
        float fFloat;
        int nInteger;
        int nRegister;
        int bBoolean;
        unsigned int nLookup;
    };
};
struct AptCharacter;
// !! this structure is created as a direct memory image by swfc
// !! the exporter and importer must be in sync
struct AptConstFile
{
    char aMagic[20];

    // relative to base of .apt file
    AptCharacter *pMainCharacter;

    // constants used in the streams
    int nConstants;
    AptConstantTable *aConstants;

    char* GetTimeStamp()
    {
        return aMagic + 4; // aMagic's format is here
        // byte1 byte2 byte3 byte4 byte5 byte 6 ... byte20
        // A     p     t     1     [5 - 20 are time stamp]
    }
    //void fixup(void *pData, void *pUserData, int *pnCurrentConstantIndex, int bUnresolve = 0);
};
#endif // __APTFILE_H__
