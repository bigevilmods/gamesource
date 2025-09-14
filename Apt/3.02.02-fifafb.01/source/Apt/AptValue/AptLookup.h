/*H************************************************************************************************/
/*!

    \File
        AptLookup.h

    \Description
        Simple description of main purpose of file.

    \Notes
        None.

    \Copyright
        (c) 2004 Electronic Arts Inc.

    \Version
        1.0        03/06/04 (ONA) First Version
        1.1        09/06/05 (ASK) Added code to have global AptLookups. Saves lot of memory.

*/
/************************************************************************************************H*/

#ifndef __APTLOOKUP_H__
#define __APTLOOKUP_H__

#include "AptValue/AptValue.h"
#include "_AptInternalDefines.h"

class AptLookup : public AptValueNoGC
{
public:
    APT_VALUE_NEW_DELETE_OPERATORS

    APT_DECLARE_OBJECT_METRICS

    int nLookup;
    static AptLookup * Create(int32_t nVal) ;

    static void Initialize();

    static void Shutdown();

    // Now we are creating global lookup objects so do not do anything in Add/Release
#if defined(APT_INC_DEC_MESSAGES)
    virtual void    AddRef(const char * szFuncName, const char * szFileName, int nLineNumber)  { }   //  Do nothing...
    virtual void    Release(const char * szFuncName, const char * szFileName, int nLineNumber) { }   //  Do nothing...
#else
    virtual void    AddRef()  { }   //  Do nothing...
    virtual void    Release() { }   //  Do nothing...
#endif

    static AptLookup *s_LookupArray ;
    static int32_t      s_nMaxLookups ;
private:
    virtual ~AptLookup()
    {
        //  Do nothing...
    }

    AptLookup(int32_t _nLookup) : AptValueNoGC(AptVFT_Lookup), nLookup(_nLookup)
    {
        setIsDefined(1);
        setRefCount(MAX_REFCOUNT);
    }

    AptLookup() : AptValueNoGC(AptVFT_Lookup), nLookup(0)
    {
        setIsDefined(1);
        setRefCount(MAX_REFCOUNT);
    }

};

#endif  //  __APTLOOKUP_H__

/*H*************************************************************************************************
    EOF
*************************************************************************************************H*/

