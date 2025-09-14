/*H************************************************************************************************/
/*!

    \File
        AptAnimation.cpp

    \Description
        Simple description of main purpose of file.

    \Notes
        None.

    \Copyright
        (c) 2005 Electronic Arts Inc.

    \Version    1.0        2004-03-07 (ONA) First Version
    \Version    1.1        2004-07-22 (rrv:1) enhancements for AS2 support
    \Version    1.2        11/01/04   (DC)  Changed to use allocation Macro to remove #if defines
                                            everywhere. There was not change to the logic.
    \Version    1.3        09/05/07   (RB)  Added back code that was removed during lockless implementation.
                                            This code used to be wrapped by #ifdef APT_DECOUPLED_RENDERING,
                                            and was thought to be unnecessary.  However it actually contained
                                            useful bugfixes.
                                            Fixes .swf-remaining assert and AptCharacter refcount assert.
                                            Fixes crash during unresolve.
    \Version    1.4        09/07/07 (psenzee) Modified for lock less Apt
    \Version    1.5        09/07/07 (ASK)   Commented call to pfnLoadTexture to delay it till actually rendering the shape
    \Version    1.6        12/13/07 (RB)    Handle PlaceObject3 in ExecuteInitActions
    \Version    1.7        06/25/08 (RB)    In Unresolve, reset mbNotLoaded flag and nFontID.
                                            Ensures our legacy regression tests pass cleanly.
    \Version    1.8        07/29/08 (RB)    Changed gpFreeTextureArray to SafeQueueFixed.
    \Version    1.9        09/14/08 ($senzee) UnmapCharacter function returned -1 when unresolved something that was never fully resolved, this value is not checked against, it is inserted as is back into the file, which causes issues on reload.  Instead, modified to insert offset provided to it.
    \Version    2.0        10/08/08 (RB)    Accept AptCharacterType_None to handle packed textures
    \Version    2.1        06/01/09 (RB)    Removed case AptCharacterType_Bitmap in AptCharacterAnimation::Link(): bugfix to prevent invoking pfnBindTexture
                                            before pfnLoadTexture.
*/
/************************************************************************************************H*/

#include "_Apt.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "Display/AptDisplayListState.h"
#include "AptTarget.h"
#include "AptRenderableGeometry.h"
#include APT_INC_THREAD_H
#include APT_INC_THREAD_MUTEX_H
#include "AptSafeQueueFixed.h"


#include "MainInline.h"

#define CHECK_DEQUE_BOUNDS( pCur ) APT_ASSERT( pCur >= &aActionPool[0] ); \
                                   APT_ASSERT( pCur < &aActionPool[ m_iActionPoolSize ] )

#define APT_SAVEINPUTS_RECORD_SIZE       8

#if (APT_DEBUG_LEVEL > APT_DEBUG_NONE)
        static bool IsBuiltForDecoupling(void *pAptData)
        {
            char* sAptTag = static_cast<char *>(pAptData);

            if(sAptTag[8]==':')
            {
                return (sAptTag[9] == '1');
            }
            return false;
        }
        static bool CheckCorrectPtrSize(void *pAptData)
        {
            char* sAptTag = static_cast<char *>(pAptData);

            if(sAptTag[12]==':')
            {
                return ((sAptTag[13] - '0') == EA_PLATFORM_PTR_SIZE);
            }
            return (EA_PLATFORM_PTR_SIZE == 4);
        }
        #define APT_CHECK_FILE_BUILD_TYPE(_)    \
            if(!IsBuiltForDecoupling(_))        \
            {                                   \
                APT_DEBUGPRINT(APT_DEBUG_MSG_WARNING_LVL, "FILE[%s.swf] IS NOT BUILT FOR DECOUPLING, PLEASE REBUILD THIS FILE USING THE --decouple OPTION IN SWFC\n", pFile.pData->GetName().c_str());    \
            }                                   \
            APT_ASSERT(IsBuiltForDecoupling(pAptData) && "THIS FILE IS NOT BUILT FOR DECOUPLING, PLEASE REBUILD THIS FILE USING THE --decouple OPTION IN SWFC");    \
            if(!CheckCorrectPtrSize(_))         \
            {                                   \
                APT_DEBUGPRINT(APT_DEBUG_MSG_WARNING_LVL, "FILE[%s.swf] IS NOT BUILT WITH CORRECT POINTER SIZE, PLEASE REBUILD THIS FILE USING --ptrsize=%i IN SWFC\n", pFile.pData->GetName().c_str(), EA_PLATFORM_PTR_SIZE);    \
            }                                   \
            APT_ASSERT(CheckCorrectPtrSize(pAptData) && "THIS FILE IS NOT BUILT WITH CORRECT POINTER SIZE, PLEASE REBUILD THIS FILE USING THE --ptrsize OPTION IN SWFC");

#else
    #define APT_CHECK_FILE_BUILD_TYPE(_)
#endif


/*F************************************************************************************************/
/*!
    \Function    AptCharacterAnimation::Fixup

    \Description
        Memory maps the .apt file to the AptCharacterAnimation structure

    Insert your long description here.

    \Input      *pAptData         -
    \Input      *pConstFile       -
    \Input      *pUserData        -
    \Input      pFile             -

    \Output
        None.

    \Notes
        None.

    \Version    1.0        04/12/05 (LEO) Comment Header Added
    \Version    1.1        10/08/08 (RB)  Accept AptCharacterType_None to handle packed textures

*/
/************************************************************************************************F*/
void AptCharacterAnimation::Fixup(void *pAptData, AptConstFile *pConstFile, void *pUserData, AptFilePtr pFile)
{
    APT_CHECK_FILE_BUILD_TYPE(pAptData);
    int i;
    unsigned char *pBase = (unsigned char *)pAptData;

    APT_ASSERT((uintptr_t)apCharacters < 65536 && "file already fixed up!!");

    APT_RESOLVE(apCharacters);

    // resolve exports first to pass down export names to pfnLoadSound
    // added in 0.16.00 as per request from Nascar team
    APT_RESOLVE(aExports);
    for (i = 0; i < nExports; i++)
    {
        APT_RESOLVE(aExports[i].szName);
    }
    //-------------

    for (i = 0; i < nCharacters; i++)
    {
        APT_RESOLVE(apCharacters[i]);
        if (apCharacters[i])
        {
            APT_ASSERT((uintptr_t)apCharacters[i]->pParentAnim == 0x09876543);
            apCharacters[i]->pParentAnim = apCharacters[0];
            switch (apCharacters[i]->eType)
            {
                case AptCharacterType_Shape:
                    {
                        APT_ASSERT((intptr_t)apCharacters[i]->shape.pRenderUnit == i);
                        APT_PROFILE_AUXILIARY(pfnLoadRenderingUnit);
                        apCharacters[i]->shape.pRenderUnit = new AptRenderableGeometry(gAptFuncs.pfnLoadRenderingUnit(pUserData, i), pFile.Get());
                    }

                    break;
                    case AptCharacterType_Text:

                    {
                        APT_RESOLVE(apCharacters[i]->text.szInitialText);
                        APT_RESOLVE(apCharacters[i]->text.szVariable);
                    }

                    break;
                    case AptCharacterType_Font:

                    {
                        APT_RESOLVE(apCharacters[i]->font.szName);
                        APT_RESOLVE(apCharacters[i]->font.apGlyphs);
                    }

                    break;
                    case AptCharacterType_Button:

                    {
#if defined APT_USE_BUTTONS
                        int j;

                        APT_RESOLVE(apCharacters[i]->button.mHitTestVertexTable);
                        APT_RESOLVE(apCharacters[i]->button.mHitTestIndexTable);

                        APT_RESOLVE(apCharacters[i]->button.aButtonRecords);
                        for (j = 0; j < apCharacters[i]->button.nButtonRecords; j++)
                        {
                            apCharacters[i]->button.aButtonRecords[j].pCharacter = apCharacters[(uintptr_t)apCharacters[i]->button.aButtonRecords[j].pCharacter];
                        }
                        APT_RESOLVE(apCharacters[i]->button.aActionConditions);
                        for (j = 0; j < apCharacters[i]->button.nActionConditions; j++)
                        {
                            APT_RESOLVE(apCharacters[i]->button.aActionConditions[j].actions.aActionStream);
                            AptActionInterpreter::resolveStream(apCharacters[i]->button.aActionConditions[j].actions.aActionStream, pBase, pConstFile, &nCurrentConstantIndex);
                        }
                        APT_RESOLVE(apCharacters[i]->button.pButtonSound);
#else
                        APT_ASSERT(0 && "DEFINE APT_USE_BUTTONS TO USE BUTTONS");
#endif
                    }

                    break;
                    case AptCharacterType_Sprite:

                    {
                        apCharacters[i]->sprite.movie.resolve(pBase, pConstFile, &nCurrentConstantIndex);
                    }

                    break;
                    case AptCharacterType_Sound:

                    {
#if defined(APT_USE_SOUND_OBJECT)
                        // added in 0.16.00 to pass down the name of linkage.
                        char * szLinkageName = NULL;

                        APT_ASSERT((intptr_t)apCharacters[i]->sound.zID == i);

                        // Note by SI- 04/07/04, pass down the linkage name to pfnLoadSound
                        // this allows auxiliary library to do sound management using meaningful linkage names
                        // rather than abstract asset IDs
                        // note that this only works for sounds that are "attached" and thus have a linkage name
                        // parse the export list to find the linkage name for the asset
                        for (int iExport = 0; iExport < nExports; iExport++)
                        {
                            if (aExports[iExport].nID == i)
                            {
                                szLinkageName = aExports[iExport].szName;
                            }
                        }
                        // Note by SI- the assert below is invalid for sounds that are not exported for linkage
                        // This is valid in Flash, so we can't assert =(
                        //SysAssert(szLinkageName != NULL);
                        APT_PROFILE_AUXILIARY(pfnLoadSound);
                        apCharacters[i]->sound.zID = gAptFuncs.pfnLoadSound(pUserData, i, szLinkageName);

                        // commented in 0.16.00
                        //APT_ASSERT((int)apCharacters[i]->sound.zID == i);
                        //apCharacters[i]->sound.zID = gAptFuncs.pfnLoadSound(pUserData, i);
#else
                        APT_ASSERT(0 && "DEFINE APT_USE_SOUND_OBJECT TO USE SOUND");
#endif
                    }

                    break;
                    case AptCharacterType_Bitmap:

                    {
                        APT_ASSERT((intptr_t)apCharacters[i]->bitmap.zID == i);
                        // commented out call for pfnLoadTexture from here as it should be
                        // done from render thread like it used to be for Decoupled rendering code
                        //APT_PROFILE_AUXILIARY(pfnLoadTexture);
                        //apCharacters[i]->bitmap.zID = gAptFuncs.pfnLoadTexture(pUserData, i);
                    }

                    break;
                    case AptCharacterType_Morph:

                    {
                    }

                    break;
                    case AptCharacterType_Animation:

                    {
                        apCharacters[i]->animation.movie.resolve(pBase, pConstFile, &nCurrentConstantIndex);
                        APT_ASSERT(i == 0);
                    }

                    break;
                    case AptCharacterType_StaticText:

                    {
                        int j;

                        APT_RESOLVE(apCharacters[i]->statictext.aRecords);
                        for (j = 0; j < apCharacters[i]->statictext.nFontRecords; j++)
                        {
                            APT_RESOLVE(apCharacters[i]->statictext.aRecords[j].aGlyphs);
                        }
                    }
                    break;
                    case AptCharacterType_Video:
                    {
                    }
                    break;
                    case AptCharacterType_None: // might be a packed texture
                    {
                    }
                    break;
                    default:
                    {
                        APT_ASSERT(NOT_REACHED);
                    }
            }
            apCharacters[i]->SetupCharacter();       // Set up each character after being resolved
        }
    }

    APT_RESOLVE(aImports);
    for (i = 0; i < nImports; i++)
    {
        APT_RESOLVE(aImports[i].szFile);
        APT_RESOLVE(aImports[i].szName);
        aImports[i].file = GetTargetSim()->GetLoader()->Load(AptNativeString(aImports[i].szFile), AptFileType_Animation);
    }
}

/*F************************************************************************************************/
/*!
    \Function    AptCharacterAnimation::Resolve

    \Description
        Calls the Fixup function to resolve the apt data from the newly loaded file

    Insert your long description here.

    \Input      *pAptData         -
    \Input      *pConstFile       -
    \Input      *pUserData        -
    \Input      pFile             -

    \Output
        None.

    \Notes
        None.

    \Version    1.0        04/12/05 (LEO) Comment Header Added

*/
/************************************************************************************************F*/
void AptCharacterAnimation::Resolve(void *pAptData, AptConstFile *pConstFile, void *pUserData, AptFilePtr pFile)
{
    unsigned char *pBase = (unsigned char *)pConstFile;

    APT_RESOLVE(pConstFile->aConstants);

    nCurrentConstantIndex = 0;
    Fixup(pAptData, pConstFile, pUserData, pFile);

    APT_UNRESOLVE(pConstFile->aConstants);
}


intptr_t AptCharacterAnimation::UnmapCharacter(AptCharacter *mpCharacter)
{
    for (int i = 0; i < nCharacters; i++)
    {
        if (apCharacters[i] == mpCharacter) return i;
    }
    APT_ASSERT(NOT_REACHED);
    return (intptr_t)mpCharacter;
}

/*F************************************************************************************************/
/*!
    \Function    AptCharacterAnimation::isImport

    \Description
        This function will run through the animation and see if the passed in character id is imported.

    see above

    \Input      nID               -

    \Output     uint32_t          - index of imported item, -1 if none found.

    \Notes
        - (LEO) Changed to return the index in the import array for bug657: Nested Components Not Firing InitClip     // $BUG657  2/7

    \Version    1.0        08/19/04 (LEO) First Version

*/
/************************************************************************************************F*/
int32_t AptCharacterAnimation::IsImport(int nID)
{
    for (int i = 0; i < nImports; i++)
    {
        if (aImports[i].nID == nID)
        {
            return i;
        }
    }
    return -1;
}

/*F************************************************************************************************/
/*!
    \Function    AptCharacterAnimation::unresolve

    \Description
        This function will unresolve a animation

    This function will run through the character animation information and unresolve the memory allocated for it when it was loaded.

    \Input
        None.

    \Output
        None.

    \Notes
        - Added code to take affect for the change done in the isImport function above.

    \Version    1.0        08/19/04 (LEO) First Version
    \Version    1.1        06/22/05 (LEO) $bug931 Multi-threaded crash in AptCharacter::render() for some swf.
    \Version    1.2        07/18/06 (RB)  $bug105123   - free textures in non-decoupled mode
    \Version    1.3        07/29/08 (RB)  Changed gpFreeTextureArray to SafeQueueFixed.
*/
/************************************************************************************************F*/
void AptCharacterAnimation::Unresolve(void *pAptData)
{
    intptr_t i;
    unsigned char *pBase = (unsigned char *)pAptData;

    nCurrentConstantIndex = 0;

    for (i = 0; i < nCharacters; i++)
    {
        if(apCharacters[i] == NULL)
        {
            continue;
        }
        if (apCharacters[i] && IsImport(i) == -1)           // (LEO)    $BUG657  3/7
        {
            switch (apCharacters[i]->eType)
            {
                case AptCharacterType_Morph:
                {
                    apCharacters[i]->morph.pStartCharacter = (AptCharacter *)UnmapCharacter(apCharacters[i]->morph.pStartCharacter);
                    apCharacters[i]->morph.pEndCharacter = (AptCharacter *)UnmapCharacter(apCharacters[i]->morph.pEndCharacter);
                }
                break;
                case AptCharacterType_Button:
                {
#if defined APT_USE_BUTTONS
                    AptCharacterButtonSound *pBS = apCharacters[i]->button.pButtonSound;
                    if (pBS)
                    {
                        if (pBS->pIdleToOverUp) pBS->pIdleToOverUp = (AptCharacter *)UnmapCharacter(pBS->pIdleToOverUp);
                        if (pBS->pOverDownToOverUp) pBS->pOverDownToOverUp = (AptCharacter *)UnmapCharacter(pBS->pOverDownToOverUp);
                        if (pBS->pOverUpToIdle) pBS->pOverUpToIdle = (AptCharacter *)UnmapCharacter(pBS->pOverUpToIdle);
                        if (pBS->pOverUpToOverDown) pBS->pOverUpToOverDown = (AptCharacter *)UnmapCharacter(pBS->pOverUpToOverDown);
                    }
                    APT_UNRESOLVE(apCharacters[i]->button.pButtonSound);
#else
                    APT_ASSERT(0 && "DEFINE APT_USE_BUTTONS TO USE BUTTONS");
#endif
                }
                break;
                default:
                {
                }
            }
        }
        else
        {
            apCharacters[i]->ReleaseCharacterReference();
        }
    }


    for (i = 0; i < nCharacters; i++)
    {
        if (apCharacters[i] && IsImport(i) == -1)              // (LEO)    $BUG657  4/7
        {
            switch (apCharacters[i]->eType)
            {
                case AptCharacterType_Sprite:
                {
                    apCharacters[i]->sprite.movie.unresolve(pBase, &nCurrentConstantIndex);
                }

                break;
                case AptCharacterType_Shape:

                {
                    AptRenderableGeometry *pGeom = apCharacters[i]->shape.pRenderUnit;
                    if (pGeom)
                        pGeom->Release();
                    apCharacters[i]->shape.pRenderUnit = (AptRenderableGeometry *)i;
                    apCharacters[i]->m_shapeData.m_bNotLoaded = 0;
                }

                break;
                case AptCharacterType_Button:

                {
#if defined APT_USE_BUTTONS
                    int j;

                    APT_UNRESOLVE(apCharacters[i]->button.mHitTestVertexTable);
                    APT_UNRESOLVE(apCharacters[i]->button.mHitTestIndexTable);

                    for (j = 0; j < apCharacters[i]->button.nButtonRecords; j++)
                    {
                        apCharacters[i]->button.aButtonRecords[j].pCharacter = (AptCharacter *)UnmapCharacter(apCharacters[i]->button.aButtonRecords[j].pCharacter);
                    }
                    APT_UNRESOLVE(apCharacters[i]->button.aButtonRecords);

                    for (j = 0; j < apCharacters[i]->button.nActionConditions; j++)
                    {
                        AptActionInterpreter::unresolveStream(apCharacters[i]->button.aActionConditions[j].actions.aActionStream, pBase, &nCurrentConstantIndex);
                        APT_UNRESOLVE(apCharacters[i]->button.aActionConditions[j].actions.aActionStream);
                    }
                    APT_UNRESOLVE(apCharacters[i]->button.aActionConditions);
#else
                    APT_ASSERT(0 && "DEFINE APT_USE_BUTTONS TO USE BUTTONS");
#endif
                }

                break;
                case AptCharacterType_Font:

                {
                    APT_UNRESOLVE(apCharacters[i]->font.szName);

                    for (int j = 0; j < apCharacters[i]->font.nGlyphs; j++)
                    {
                        apCharacters[i]->font.apGlyphs[j] = (AptCharacter *)UnmapCharacter(apCharacters[i]->font.apGlyphs[j]);
                    }
                    APT_UNRESOLVE(apCharacters[i]->font.apGlyphs);
                }

                break;
                case AptCharacterType_Sound:

                {
#if defined(APT_USE_SOUND_OBJECT)
                    APT_PROFILE_AUXILIARY(pfnFreeSound);
                    gAptFuncs.pfnFreeSound(apCharacters[i]->sound.zID);
                    apCharacters[i]->sound.zID = (AptAssetSound)i;
#else
                    APT_ASSERT(0 && "DEFINE APT_USE_SOUND_OBJECT TO USE SOUND");
#endif
                }

                break;
                case AptCharacterType_Text:

                {
                    int nFontID = apCharacters[i]->text.nFontID;
                    if ( nFontID < 0 )
                    {
                        apCharacters[i]->text.nFontID = -nFontID;
                    }
                    APT_UNRESOLVE(apCharacters[i]->text.szInitialText);
                    APT_UNRESOLVE(apCharacters[i]->text.szVariable);
                }

                break;
                case AptCharacterType_Bitmap:

                {
                    if (apCharacters[i]->m_bitmapData.m_bLoaded)
                    {
                        APT_ASSERT( gAptFuncs.pfnFreeTexture );
                        if ( gAptFuncs.pfnFreeTexture )
                        {
                            gAptFuncs.pfnFreeTexture(apCharacters[i]->bitmap.zID);
                        }
                    }
                    apCharacters[i]->m_bitmapData.m_bLoaded = 0;
                    apCharacters[i]->m_bitmapData.m_bBinded = 0;
                    apCharacters[i]->bitmap.zID = (AptAssetTexture)i;
                }

                break;
                case AptCharacterType_Animation:

                {
                    apCharacters[i]->animation.movie.unresolve(pBase, &nCurrentConstantIndex);
                    APT_ASSERT(i == 0);
                }

                break;
                case AptCharacterType_StaticText:

                {
                    int j;

                    for (j = 0; j < apCharacters[i]->statictext.nFontRecords; j++)
                    {
                        APT_UNRESOLVE(apCharacters[i]->statictext.aRecords[j].aGlyphs);
                    }
                    APT_UNRESOLVE(apCharacters[i]->statictext.aRecords);
                }

                break;
                default:
                {
                }
            }
        }
    }

    for (i = 0; i < nImports; i++)
    {
        aImports[i].file = AptFilePtr(0);
        apCharacters[aImports[i].nID] = 0;
    }


    for (i = 0; i < nCharacters; i++)
    {
        if (apCharacters[i])
        {
            apCharacters[i]->pParentAnim = (AptCharacter *)0x09876543;
            APT_UNRESOLVE(apCharacters[i]);
        }
    }

    APT_UNRESOLVE(apCharacters);

    for (i = 0; i < nImports; i++)
    {
        APT_UNRESOLVE(aImports[i].szFile);
        APT_UNRESOLVE(aImports[i].szName);
    }

    for (i = 0; i < nExports; i++)
    {
        APT_UNRESOLVE(aExports[i].szName);
        if(aExports[i].nID < 0)
        {
            aExports[i].nID = -aExports[i].nID ;
        }
    }

    APT_UNRESOLVE(aImports);
    APT_UNRESOLVE(aExports);

    nCurrentConstantIndex = 0;
}


/*F************************************************************************************************/
/*!
    \Function    AptCharacterAnimationInst::AptCharacterAnimationInst

    \Description
        Constructor

    The reason IncCharacterList is done here and the Ref counts are not incremented when the file is
    being resolved is that there is no reason to increment the ref count until the animation is being used
    and this only occures in AptLinker:Update when a AptCharacterAnimationInst is newly created.  Another reason
    is that if a file is loaded multiple times without being unloaded, the file will only be resolved once.
    We need every character's ref count to be increment every time the file is attached to an animation which is also
    only when we construct a new animation inst.

    \Input      file              -

    \Output     A                 -

    \Notes
        None.

    \Version    1.0        04/12/05 (LEO) Comment Header Added

*/
/************************************************************************************************F*/
AptCharacterAnimationInst::AptCharacterAnimationInst(AptCharacter *pCharacter, AptFilePtr file) :
    AptCharacterSpriteInstBase(pCharacter)
{
    mnLeftoverTime = 0;
    pCharacter->m_pAnimFile = file;
    mpFile = file; // this must be stored to keep a reference to it
    GetCharacterConst()->animation.IncCharacterList(file);
}

AptCharacterAnimationInst::~AptCharacterAnimationInst()
{
    GetCharacterConst()->animation.ClearCharacterList(); // Decrement each character in the character list
    if(GetCharacterWritable() != NULL)
    {
        GetCharacterWritable()->animation.ResetInitIndicators();
    }
}

/*F************************************************************************************************/
/*!
    Get SWF file version identifier from Apt data

    \Input      None.

    \Output     returns an integer value, version of SWF file: 6 (default) or 7

    \Notes
        Function is dependant on the Apt file tag generated in the pipeline by swfc.pl
        Its format is: "Apt Data:1:X" where X is the Flash SWF version.

    \Version
        1.0     2004-07-30  (rrv:1)     added for AS2 support
        1.1     2009-04-16  (RB)        Fixed comment

*/
/************************************************************************************************F*/
//(1.2:rrv:1/1): AS2 begin
int32_t AptCharacterAnimationInst::GetSwfVersion()
{
    int     nRetVal;
    char*   sAptTag = (char*) AptAnimationFile::Cast(mpFile)->GetAptData();

    // expect "Apt Data:1:X", where X is the SWF version

    if(sAptTag[10]==':')
    {
        // convert from single digit to numeric value
        nRetVal = sAptTag[11] - '0';
    }
    else
    {
        // Assume SWF 6.
        nRetVal = 6;
    }
    return nRetVal;
}
//(1.2:rrv:1/1): AS2 end

/*F************************************************************************************************/
/*!
    PreDestroy the object, mainly to keep compatibility with old __dec behavior.

    \Input      None.

    \Output     None.

    \Notes
        None.

    \Version
        1.0        03/17/04 (ONA) First Version

*/
/************************************************************************************************F*/
void    AptCharacterAnimationInst::PreDestroy()
{
    mDisplayList.clear();                        // ### LEO added for nasty unload event bug, where file would unresolve before displaylist d-ctor
}

void AptCharacterAnimation::Link(AptCharacter *pParentAnim, void *pUserData)
{
    int i;

    for (i = 0; i < nImports; i++)
    {
        APT_ASSERT(apCharacters[aImports[i].nID] == NULL);
        apCharacters[aImports[i].nID] = AptAnimationFile::Cast(aImports[i].file)->FindExport(aImports[i].szName);
        if(apCharacters[aImports[i].nID])
        {
            apCharacters[aImports[i].nID]->m_pAnimFile = aImports[i].file;
            apCharacters[aImports[i].nID]->AddCharacterReference();
        }
    }

    for (i = 0; i < nCharacters; i++)
    {
        if (apCharacters[i])
        {
            switch (apCharacters[i]->eType)
            {
                case AptCharacterType_Morph:

                {
                    apCharacters[i]->morph.pStartCharacter = apCharacters[(uintptr_t)apCharacters[i]->morph.pStartCharacter];
                    apCharacters[i]->morph.pEndCharacter = apCharacters[(uintptr_t)apCharacters[i]->morph.pEndCharacter];
                }

                break;
                case AptCharacterType_Button:

                {
#if defined APT_USE_BUTTONS
                    AptCharacterButtonSound *pBS = apCharacters[i]->button.pButtonSound;
                    if (pBS)
                    {
                        if (pBS->pIdleToOverUp) pBS->pIdleToOverUp = apCharacters[(uintptr_t)pBS->pIdleToOverUp];
                        if (pBS->pOverDownToOverUp) pBS->pOverDownToOverUp = apCharacters[(uintptr_t)pBS->pOverDownToOverUp];
                        if (pBS->pOverUpToIdle) pBS->pOverUpToIdle = apCharacters[(uintptr_t)pBS->pOverUpToIdle];
                        if (pBS->pOverUpToOverDown) pBS->pOverUpToOverDown = apCharacters[(uintptr_t)pBS->pOverUpToOverDown];
                    }
#else
                    APT_ASSERT(0 && "DEFINE APT_USE_BUTTONS TO USE BUTTONS");
#endif
                }

                break;
                case AptCharacterType_Font:

                {
                    int j;
                    for (j = 0; j < apCharacters[i]->font.nGlyphs; j++)
                    {
                        apCharacters[i]->font.apGlyphs[j] = apCharacters[(uintptr_t)apCharacters[i]->font.apGlyphs[j]];
                    }
                }

                break;
                default:
                {
                }
            }
        }
    }
}

/*F************************************************************************************************/
/*!
    \Function    AptCharacterAnimation::ClearCharacterList

    \Description
        Decrement the ref count of each item in the character list.


    Decrement the ref count of each item in the character list to remove this animations dependency
    on its characters.

    \Input
        None.

    \Output
        None.

    \Notes
        None.

    \Version    1.0        04/12/05 (LEO) First Version

*/
/************************************************************************************************F*/
void AptCharacterAnimation::ClearCharacterList() const
{
    // If the reference count of the file goes to zero, then we will be working in deleted memory!
    int32_t storedCharacters = nCharacters;

    // Don't decrement the animation itself (which is at character[0])
    for (int32_t i = 1; i < storedCharacters; i++)
    {
        if(apCharacters[i] && apCharacters[i]->eType != AptCharacterType_Animation)
        {
            apCharacters[i]->ReleaseCharacterReference();
        }
    }
}
/*F************************************************************************************************/
/*!
    \Function    AptCharacterAnimation::IncCharacterList

    \Description
        Increment the ref count of each item in the character list.


    Increment the ref count of each item in the character list to create this animations dependency
    on its characters.

    \Input
        None.

    \Output
        None.

    \Notes
        None.

    \Version    1.0        04/12/05 (LEO) First Version

*/
/************************************************************************************************F*/
void AptCharacterAnimation::IncCharacterList(AptFilePtr pFile) const
{
    // Don't increment the animation itself (which is at character[0])
    for (int32_t i = 1; i < nCharacters; i++)
    {
        if(apCharacters[i])
        {
            if(apCharacters[i]->m_pAnimFile.pData == NULL)
                apCharacters[i]->m_pAnimFile = pFile;
            apCharacters[i]->AddCharacterReference();
        }
    }
}

/*F************************************************************************************************/
/*!
    \Function    AptCharacterAnimation::getIDFromImportFile

    \Description
        This function will return the character ID of an imported item from the imported items character animation.
        This ID is different from the ID in the importing character animation.

    Insert your long description here.

    \Input      nID               - ID for the character in the importing character animation

    \Output     uint32_t          - The ID found in the exporting character animation

    \Notes
        - Added for bug 657: Nested Components Not Firing InitClip          // (LEO) $BUG657  5/7

    \Version    1.0        08/17/04 (LEO) First Version

*/
/************************************************************************************************F*/
int32_t AptCharacterAnimation::GetIDFromImportFile(int32_t nID)
{
    AptCharacter *pImportChar = AptAnimationFile::Cast(aImports[nID].file)->GetMainCharacter();
    for(int32_t i = 0; i < pImportChar[0].animation.nExports; i++)
    {
        if(strcmp(aImports[nID].szName, pImportChar[0].animation.aExports[i].szName) == 0)
        {
            return pImportChar[0].animation.aExports[i].nID;
        }
    }
    return -1;
}

/*F************************************************************************************************/
/*!
    \Function    AptCharacterAnimation::ExecuteInitActions

    \Description
        This will execute any init actions for items that are being placed in the DisplayList.

    If the item being placed is imported, we have to search for its initAction as well as any items on the first frame
    of the imported item.

    \Input      *pInst            -
    \Input      nID               -

    \Output
        None.

    \Notes
        None.

    \Version    1.0        09/28/04 (LEO) First Version
    \Version    1.01       12/13/07 (RB)  Handle PlaceObject3

*/
/************************************************************************************************F*/
void AptCharacterAnimation::ExecuteInitActions(AptCIH *pInst, int32_t nID)
{
    AptCharacterAnimation *pTmpAnim = this;
    const AptMovie *pMov = &pInst->GetCharacterInst()->GetCharacterConst()->sprite.movie;
    int32_t nImportIndex = pTmpAnim->IsImport(nID);

    // If this object is imported, update the pMov and the pTmpAnim pointers
    if(nImportIndex != -1)                                          // Here we have a place object for an imported item.
    {
        nID = pTmpAnim->GetIDFromImportFile(nImportIndex);          // Get the ID from the imported character's animation;  this is different from the id found in the importing character's animation
        if(nID != -1)
        {
            pTmpAnim = &(AptAnimationFile::Cast(pTmpAnim->aImports[nImportIndex].file)->GetMainCharacter()->animation);   // Update the animation pointer to point to the imported character's animation
            pMov = &(pTmpAnim->apCharacters[nID]->sprite.movie);    // Set the pMov pointer equal to the sprite from the new pTmpAnim
        }
    }

    if(pMov->nFrames > 0)                               // If this movie has any place objects on the first frame, we need to execute their #initclip blocks first
    {
        for(int32_t i = 0; i < pMov->aFrames[0].nControls; i++)
        {
            const AptControl* pControl = pMov->aFrames[0].apControls[i];
            // Do we have an imported item?  If so, we need to run the #initclip (if it has one)
            if(pControl->eType == AptControlType_PlaceObject2 && pControl->placeObject2.nCharacterID != -1)
            {
                pTmpAnim->ExecuteInitAction(pInst, pControl->placeObject2.nCharacterID);
            }
            else if (pControl->eType == AptControlType_PlaceObject3 && pControl->placeObject3.nCharacterID != -1)
            {
                pTmpAnim->ExecuteInitAction(pInst, pControl->placeObject3.nCharacterID);
            }
        }
    }
    if(nID != -1)
    {
        pTmpAnim->ExecuteInitAction(pInst, nID);       // Now, finally, we go execute the initActionfor this instance if it has one.
    }
}

/*F************************************************************************************************/
/*!
    \Function    AptCharacterAnimation::ExecuteInitAction

    \Description
        Executes the initAction clip code block for movie clips if found

    Insert your long description here.

    \Input      *pInst            - The character context to execute from
    \Input      nID               - The ID of the character we want to do the initAction for

    \Output
        None.

    \Notes
        - Added for bug 657: Nested Components Not Firing InitClip          // (LEO) $BUG657  6/7

    \Version    1.0        08/17/04 (LEO) First Version
    \Version    1.1        04/05/08 (JP) Turning on the flag that indicates that we are executing init actions
    \Version    1.2        06/27/08 (ASK) Added a static variable to AptActionInterpreter to turn on/off zombie detection code added previously for initclip actions

*/
/************************************************************************************************F*/
void AptCharacterAnimation::ExecuteInitAction(AptCIH *pInst, int32_t nID)
{
    for(int32_t i = 0; i < movie.aFrames->nControls; i++)
    {
        AptControl *pInitTagControl = movie.aFrames->apControls[i];
        if(pInitTagControl->eType == AptControlType_DoInitAction && pInitTagControl->initAction.nSpriteID == nID)   // Check if is DoInitAction tag and if the IDs are the same
        {
            ExportClassDefinitionAssets(pInst);
            APT_DEFINE_ACTION_SETUP(pInst, NULL, "AptImported_Init_Actions", AptActionType_Initclip);
            void * pSavedValue   = gAptActionInterpreter.PrepareForExecution(&oActionSetup);
            AptCharacterInst * pCharacterInst = pInst ? pInst->GetRootAnimation()->GetAnimationInst() : NULL ;
            // During initialization, component classes are registered to the global space, not to the calling movie clip
            gAptActionInterpreter.bRunningInitActions = AptActionInterpreter::ENABLE_AGGRESIVE_ZOMBIE_CLEANUP;
            gAptActionInterpreter.runStream(movie.aFrames->apControls[i]->initAction.actions.aActionStream, pInst, -1, pCharacterInst); // Yes, execute the initAction for the item
            gAptActionInterpreter.bRunningInitActions = false;
            gAptActionInterpreter.CleanupAfterExecution(pSavedValue, &oActionSetup); //oActionSetup is defined by the macro
            pInitTagControl->initAction.nSpriteID = -pInitTagControl->initAction.nSpriteID;                         // Negate the ID so it only happens once.
            return;
        }
    }
}



/*F************************************************************************************************/
/*!
    \Function    AptCharacterAnimation::ResetInitIndicators

    \Description
        Reset initialization indicator so that the clip will not be presumed init'ed.

    \Input
        None.

    \Output
        None.

    \Notes

    \Version    1.0        07/18/05 (PDS) First Version

*/
/************************************************************************************************F*/
void AptCharacterAnimation::ResetInitIndicators()
{
    AptCharacter            *pChar;
    AptCharacterSprite      *pSpriteChar;
    AptFrame                *aFrames;
    AptControl              *pControl,
                            **apControls;
    AptControlDoInitAction  *initAction;
    for (int i = 0, sz = nCharacters; i < sz; i++)
    {
        if ((pChar = apCharacters[i]) != NULL)
        {
            switch (pChar->eType)
            {
            case AptCharacterType_Animation:
                pSpriteChar = &pChar->animation;
                break;
            case AptCharacterType_Sprite:
                pSpriteChar = &pChar->sprite;
                break;
            default:
                pSpriteChar = NULL;
                break;
            }
            if (pSpriteChar &&
                (aFrames = pSpriteChar->movie.aFrames) != NULL &&
                (apControls = aFrames->apControls)     != NULL)
            {
                for (int32_t i = 0, sz = aFrames->nControls; i < sz; i++)
                {
                    pControl = apControls[i];
                    if (pControl && pControl->eType == AptControlType_DoInitAction &&
                        (initAction = &(pControl->initAction))->nSpriteID < 0)// Check if is DoInitAction tag
                    {
                        initAction->nSpriteID = -initAction->nSpriteID;   // Un-negate the ID so that it can be re-init'ed.
                        break;
                    }
                }
            }
        }
    }
    AptExport *pExport;
    for (int i = 0, sz = nExports; i < sz; i++)
    {
        if ((pExport = &(aExports[i])) != NULL)
        {
            if (pExport->nID < 0)
            {
                pExport->nID = -pExport->nID;   // Un-negate the ID so that it can be re-init'ed.
            }
        }
    }
}

void AptCharacterAnimation::ExportClassDefinitionAssets(AptCIH *pInst)
{
    for(int32_t j = 0; j < nExports; j++)
    {
        if(aExports[j].nID < 0)
        {
            return;
        }
        if(strstr(aExports[j].szName, "__Packages.") != NULL)
        {
            int32_t nID = aExports[j].nID;
            for(int32_t k = 0; k < movie.aFrames->nControls; k++)
            {
                AptControl *pInitTC = movie.aFrames->apControls[k];
                if(pInitTC->eType == AptControlType_DoInitAction && pInitTC->initAction.nSpriteID == nID)
                {
                    //void * pSavedValue = gAptActionInterpreter.PrepareForExecution("Imported Init Actions");
                    APT_DEFINE_ACTION_SETUP(pInst, NULL, "AptImported_Init_Actions", AptActionType_Initclip);
                    void * pSavedValue   = gAptActionInterpreter.PrepareForExecution(&oActionSetup);
                    AptCharacterInst * pCharacterInst = pInst ? pInst->GetRootAnimation()->GetAnimationInst() : NULL ;
                    // JP - During initialization, component classes are registered to the global space, not to the calling movie clip
                    gAptActionInterpreter.bRunningInitActions = AptActionInterpreter::ENABLE_AGGRESIVE_ZOMBIE_CLEANUP;
                    gAptActionInterpreter.runStream(movie.aFrames->apControls[k]->initAction.actions.aActionStream, pInst, -1, pCharacterInst); // Yes, execute the initAction for the item
                    gAptActionInterpreter.bRunningInitActions = false;
                    gAptActionInterpreter.CleanupAfterExecution(pSavedValue, &oActionSetup);  //oActionSetup is defined by the macro
                    //gAptActionInterpreter.CleanupAfterExecution("Imported Init Actions", pSavedValue);
                    break;
                }
            }
            aExports[j].nID = -aExports[j].nID;
        }
    }
}
