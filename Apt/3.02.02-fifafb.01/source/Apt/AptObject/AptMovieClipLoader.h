/*H************************************************************************************************/
/*!
    \File   AptMovieClipLoader.h

    \Description  This file defines a the actionscript "MovieClipLoader" object handlers.

    \Notes

    \Copyright   (c) 2008 Electronic Arts Inc.

    \Version 1.0        06/12/08 (JP)
*/
/************************************************************************************************H*/

#ifndef __APTMOVIECLIPLOADER_H__
#define __APTMOVIECLIPLOADER_H__

#include "AptObject/AptObject.h"

enum MovieClipLoaderNotifications
{
    LOAD_START = 0,
    LOAD_COMPLETE,
    LOAD_INIT,
    LOAD_ERROR,
    LISTENER_FUNCTIONS_TOT
};


class AptMovieClipLoader : public AptObject
{
public:
    APT_VALUE_GC_NEW_DELETE_OPERATORS
    APT_DECLARE_OBJECT_METRICS

    AptMovieClipLoader();

    NATIVE_MEMBER_FUNCTION_DECL(loadClip);
    NATIVE_MEMBER_FUNCTION_DECL(addListener);
    NATIVE_MEMBER_FUNCTION_DECL(getProgress);
    NATIVE_MEMBER_FUNCTION_DECL(removeListener);
    NATIVE_MEMBER_FUNCTION_DECL(unloadClip);

    virtual AptValue    *objectMemberLookup     (AptValue * const pContext, const AptNativeString * const pName) const;

    static void         Initialize              (const AptInitParmsT &aptInitParms);
    static void         CleanNativeFunctions    ();

    void                NotifyListeners         (const AptNativeString &sMovie, int32_t notification);

protected:
    virtual             ~AptMovieClipLoader     () {}

private:
    void                SendToListeners         (const char  *szName, int32_t nNumParams, ...);
    static AptValue     *GetTargetParameter     (int32_t idxParameter);

    static const char *maListenerFunctions[];
    static int32_t    mMaxListeners;
    static int32_t    mMaxLoading;

    AptValueSet<AptValue *>  mListenersSet;
    AptValueSet<AptString *>  mLoadingSet;
};

#endif  //  __APTMOVIECLIPLOADER_H__

/*H*************************************************************************************************
    EOF
*************************************************************************************************H*/
