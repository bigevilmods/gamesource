/*H************************************************************************************************/
/*!
    \File  AptMovieClipLoader.cpp

    \Description  Implements the Flash class MovieClipLoader

    \Notes     None.

    \Copyright   (c) 2008 Electronic Arts Inc.

    \Version    1.0        06/02/08 (JP) First Version
    \Version    1.0        07/29/08 (RB) Pass NULL metrics structs to mListenerSet and mLoadingSet
*/
/************************************************************************************************H*/

#include "_Apt.h"
#include "MainInline.h"
#include "AptObject/AptMovieClipLoader.h"
#include "AptTarget.h"

APT_DEFINE_OBJECT_METRICS(AptMovieClipLoader, AptObject);

// Listener notifications that we are sending
const char *AptMovieClipLoader::maListenerFunctions[LISTENER_FUNCTIONS_TOT] =
{
    "onLoadStart",
    "onLoadComplete",
    "onLoadInit",
    "onLoadError"
};

int32_t AptMovieClipLoader::mMaxListeners = 64;  // Default value in Apt for iListenerSetSize
int32_t AptMovieClipLoader::mMaxLoading = 364;  // Default value in Apt for iMaxNewMovieClipsPerFrame

/*F************************************************************************************************/
/*!
    AptMovieClipLoader::AptMovieClipLoader: Constructor

    \Input      None

    \Output     None

    \Notes      None.

    \Version    1.0     06/12/08 (JP) First Version
*/
/************************************************************************************************F*/

AptMovieClipLoader::AptMovieClipLoader()
: AptObject(AptVFT_MovieClipLoader),
  mListenersSet(mMaxListeners, NULL),
  mLoadingSet(mMaxLoading, NULL)
{
}

/*F************************************************************************************************/
/*!
    Attaches a listener that receives a notification when the movie is loaded

    \Input      pListener         - Listener to be added

    \Output     None

    \Notes      None.

    \Version    1.0     06/12/08 (JP) First Version
*/
/************************************************************************************************F*/

NATIVE_MEMBER_FUNCTION(AptMovieClipLoader, addListener)
{
    if (nParams > 0)
    {
        AptValue *pListener = gAptActionInterpreter.stackAt(0);
        if (pListener->isObject())
        {
            AptMovieClipLoader *pLoader = (AptMovieClipLoader *)pThis;
            if(!pLoader->mListenersSet.has(pListener))
            {
                pLoader->mListenersSet.add(pListener) ;
            }
        }
    }
    return gpUndefinedValue;
}

/*F************************************************************************************************/
/*!
    Removes a listener previously attached with addListener

    \Input      pListener         - Listener to be removed

    \Output     None

    \Notes      None.

    \Version    1.0     06/12/08 (JP) First Version
*/
/************************************************************************************************F*/

NATIVE_MEMBER_FUNCTION(AptMovieClipLoader, removeListener)
{
    if (nParams > 0)
    {
        AptValue *pParam = gAptActionInterpreter.stackAt(0);
        if (pParam->getIsDefined())
        {
            AptMovieClipLoader *pLoader = (AptMovieClipLoader *)pThis;
            if(pLoader->mListenersSet.has(pParam))
            {
                pLoader->mListenersSet.remove(pParam) ;
            }
        }
    }
    return gpUndefinedValue;
}

/*F************************************************************************************************/
/*!
    Loads a movieclip

    \Input      pMovie      - Name of the movie file
                pHolder     - Movieclip (or level number) to hold the movie

    \Output     True if success, false if failure

    \Notes      None.

    \Version    1.0     06/12/08 (JP) First Version
*/
/************************************************************************************************F*/

NATIVE_MEMBER_FUNCTION(AptMovieClipLoader, loadClip)
{
    if (nParams < 2)
    {
        return AptBoolean::Create(false);
    }

    AptMovieClipLoader *pLoader = (AptMovieClipLoader *)pThis;

    AptValue *pMovie = gAptActionInterpreter.stackAt(0);    // Movieclip file name
    AptValue *pTarget =  GetTargetParameter(1);

    if(pTarget == NULL || pMovie == NULL)
    {
        return AptBoolean::Create(false);
    }

    AptNativeString sMovieName;
    AptNativeString sTarget;
    pMovie->toString(sMovieName);
    pTarget->toString(sTarget);

    // List of strings that contains the name of the movie + the name of the target (separated with a asterix that cannot be part of a filename)
    // The same movie can be being loaded in several targets but the notification of complete loaded is only sent once
    AptString *pComposed = AptString::Create(sMovieName);
    pComposed->cat("*");
    pComposed->cat(sTarget);
    pLoader->mLoadingSet.add(pComposed);

    pLoader->SendToListeners(maListenerFunctions[LOAD_START], 1, pTarget);

    GetTargetSim()->GetLinker()->Load(sMovieName, sTarget, pLoader);

    return AptBoolean::Create(true);
}

/*F************************************************************************************************/
/*!
    Returns the loading progress

    \Input      pHolder     - Movieclip (or level number) that holds the movie

    \Output     Object with properties bytesLoaded and bytesTotal

    \Notes      None

    \Version    1.0     06/12/08 (JP) First Version
*/
/************************************************************************************************F*/

NATIVE_MEMBER_FUNCTION(AptMovieClipLoader, getProgress)
{
    if (nParams < 1)
    {
        return gpUndefinedValue;
    }

    AptValue *pTarget =  GetTargetParameter(0);

    if(pTarget)
    {
        int32_t nBytesLoaded = 0;
        int32_t nBytesTotal = 0;
/*
//   pfnGetBytesLoaded and pfnGetBytesTotal are not implemented
        if (pTarget->isCIH() && pTarget->c_cih()->IsAnimationInst())
        {
            AptNativeString sBuf = pTarget->c_cih()->GetAnimationInst()->mpFile->GetName() ;
            if(gAptFuncs.pfnGetBytesLoaded)
            {
                nBytesLoaded = (int32_t) gAptFuncs.pfnGetBytesLoaded(sBuf.ConstRawPtr(), AptGetBytesEnum_MovieClip);
            }
            if(gAptFuncs.pfnGetBytesTotal)
            {
                nBytesTotal = (int32_t) gAptFuncs.pfnGetBytesTotal(sBuf.ConstRawPtr(), AptGetBytesEnum_MovieClip);
            }
        }
*/
        AptObject *pObject = new AptObject(AptVFT_Object);
        AptNativeString tempStr = "bytesLoaded";
        pObject->Set(&tempStr, AptInteger::Create(nBytesLoaded));
        tempStr = "bytesTotal";
        pObject->Set(&tempStr, AptInteger::Create(nBytesTotal));
        return pObject ;
    }

    return gpUndefinedValue;
}

/*F************************************************************************************************/
/*!
    Unloads the movie

    \Input      pHolder     - Movieclip (or level number) that holds the movie

    \Output     None

    \Notes      None

    \Version    1.0     06/12/08 (JP) First Version
*/
/************************************************************************************************F*/
NATIVE_MEMBER_FUNCTION(AptMovieClipLoader, unloadClip)
{
    if (nParams < 1)
    {
        return gpUndefinedValue;
    }

    AptValue *pTarget =  GetTargetParameter(0);

    if(pTarget)
    {
        // Unloading a movie is equivalent to loading an empty movie; pass the empty string in for
        // the movie name
        AptNativeString sMovie("");
        AptNativeString sTarget;
        pTarget->toString(sTarget);
        GetTargetSim()->GetLinker()->Load(sMovie, sTarget);
    }

    return gpUndefinedValue;
}

/*F************************************************************************************************/
/*!
    AptMovieClipLoader::CleanNativeFunctions: Static function that releases the AptNativeScript functions.

    \Input      None.

    \Output     None.

    \Notes     This function needs to be called at least during AptShutdown.

    \Version   1.0        06/12/08 (JP) First Version
*/
/************************************************************************************************F*/

void    AptMovieClipLoader::CleanNativeFunctions()
{
    NATIVE_MEMBER_FUNCTION_DESTROY(loadClip);
    NATIVE_MEMBER_FUNCTION_DESTROY(addListener);
    NATIVE_MEMBER_FUNCTION_DESTROY(getProgress);
    NATIVE_MEMBER_FUNCTION_DESTROY(removeListener);
    NATIVE_MEMBER_FUNCTION_DESTROY(unloadClip);
}

/*F************************************************************************************************/
/*!
    AptMovieClipLoader::Initialize: Static function that initializes the maximum size of the arrays

    \Input      aptInitParams   - Initialization parameters

    \Output     None

    \Notes      None.

    \Version    1.0     06/12/08 (JP) First Version
*/
/************************************************************************************************F*/

void AptMovieClipLoader::Initialize(const AptInitParmsT &aptInitParms)
{
    mMaxListeners = aptInitParms.iListenerSetSize;
    mMaxLoading = aptInitParms.iMaxNewMovieClipsPerFrame;
}


/*F************************************************************************************************/
/*!
    Returns the AptValue for the context and the member name.

    \Input      pContext          - The context passed in.
    \Input      pName             - The member name.

    \Output     AptValue *        - The AptValue of the member.
                                    gpUndefinedValue is the member is unknown.

    \Notes      None.

    \Version    1.0     06/12/08 (JP) First Version
*/
/************************************************************************************************F*/

AptValue *    AptMovieClipLoader::objectMemberLookup(AptValue * const pContext, const AptNativeString * const pName) const
{
    char c = *pName->c_str();
    switch(c)
    {
        case 'l':
            if(strcmp(pName->c_str(),"loadClip")==0)
            {
                NATIVE_MEMBER_FUNCTION_DISPATCH(loadClip);
            }
            break;
        case 'a':
            if(strcmp(pName->c_str(),"addListener")==0)
            {
                NATIVE_MEMBER_FUNCTION_DISPATCH(addListener);
            }
            break;
        case 'g':
            if(strcmp(pName->c_str(),"getProgress")==0)
            {
                NATIVE_MEMBER_FUNCTION_DISPATCH(getProgress);
            }
            break;
        case 'r':
            if(strcmp(pName->c_str(),"removeListener")==0)
            {
                NATIVE_MEMBER_FUNCTION_DISPATCH(removeListener);
            }
            break;
        case 'u':
            if(strcmp(pName->c_str(),"unloadClip")==0)
            {
                NATIVE_MEMBER_FUNCTION_DISPATCH(unloadClip);
            }
            break;
    }
    return 0;
}

/*F************************************************************************************************/
/*!
    \Function    AptMovieClipLoader::NotifyListeners

    \Description Received when a load has been completed or aborted. Notifies the listeners

    \Input      sMovie   - Movie name
                notification - Desired notification

    \Output     None.

    \Notes

    \Version    1.0        06/16/08 (JP) First Version
*/
/************************************************************************************************F*/

void AptMovieClipLoader::NotifyListeners(const AptNativeString &sMovie, int32_t notification)
{
    int nCount = mLoadingSet.mnElements;
    int nFile = sMovie.GetLength();

    for(int32_t i = 0; i < mLoadingSet.capacity(); ++i)
    {
        AptString *pLoading = mLoadingSet.maElements[i];
        // The movie name is the first part of the composed string
        if(pLoading)
        {
            AptNativeString *pString = pLoading->GetInternalString();
            if(pString->UTF8_CharAt(nFile)=='*' && strncmp(pString->GetBuffer(), sMovie, nFile) == 0)
            {
                bool bRemove = false;
                // The target is the last part of the composed string (after the asterix)
                AptValue *pTarget = AptString::Create(pString->Right(pString->GetLength() - nFile - 1));
                switch(notification)
                {
                    case LOAD_ERROR:
                    {
                        // In the error message we are sending the name of the movie
                        bRemove = true;
                        AptValue *pMovie = AptString::Create(sMovie.GetBuffer());
                        SendToListeners(maListenerFunctions[notification], 2, pTarget, pMovie);
                        break;
                    }
                    case LOAD_COMPLETE:
                        SendToListeners(maListenerFunctions[notification], 1, pTarget);
                        break;
                    case LOAD_INIT:
                        bRemove = true;
                        SendToListeners(maListenerFunctions[notification], 1, pTarget);
                        break;
                }
                if(bRemove)
                {
                    APT_DEC(mLoadingSet.maElements[i]);
                    mLoadingSet.mnElements--;
                    mLoadingSet.maElements[i] = 0;
                }
            }
            nCount--;
            if(nCount == 0)
            {
                break;
            }
        }
    }
}


/*F************************************************************************************************/
/*!
    \Function    AptMovieClipLoader::SendToListeners

    \Description Execute the listener functions

    \Input      saFunctionName    - Function name
    \Input      nNumParams        - Number of arguments
    \Input      Arguments

    \Output     None.

    \Notes

    \Version    1.0        06/16/08 (JP) First Version
*/
/************************************************************************************************F*/

void AptMovieClipLoader::SendToListeners(const char *szFunctionName, int32_t nNumParams, ...)
{
    int32_t nCount = mListenersSet.mnElements;
    AptNativeString strName(szFunctionName);

    for (int32_t i = 0; i < mListenersSet.capacity() && nCount > 0; ++i)
    {
        AptValue *pListener = mListenersSet.maElements[i];
        if(pListener)
        {
            va_list varargs;
            va_start( varargs, nNumParams );
            AptCallMemberFunctionV(szFunctionName, NULL, pListener, nNumParams, varargs);
            va_end(varargs);
            nCount--;
        }
    }
}

/*F************************************************************************************************/
/*!
    \Function    AptMovieClipLoader::GetTargetParameter

    \Description Deals with the "tatget" parameter

    \Input      idxParameter - Parameter index

    \Output     Target parameter (converted if it comes as a level number)

    \Notes      The "target" parameter can be a movieclip or a level number

    \Version    1.0        06/16/08 (JP) First Version
*/
/************************************************************************************************F*/

AptValue *AptMovieClipLoader::GetTargetParameter(int32_t idxParameter)
{
    AptValue *pTarget =   gAptActionInterpreter.stackAt(idxParameter);

    if(pTarget->isInteger())  // It's a level number
    {
        pTarget = _AptGetAnimationAtLevel(pTarget->toInteger());
    }

    return pTarget;
}
