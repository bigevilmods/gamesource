/*H************************************************************************************************/
/*!

    \File
        AptCharacter.cpp

    \Description
        Simple description of main purpose of file.

    \Notes
        None.

    \Copyright
        (c) 2005 Electronic Arts Inc.

    \Version    1.0     03/07/04    (ONA)   First Version
    \Version    1.1     2004-07-22  (rrv)   corrections to autosize multi-line field
    \Version    1.2     11/01/04    (DC)   Changed to use allocation Macro to remove #if defines
                                           everywhere. There was a minor logic change in that
                                           previously we put an empty string as the default text
                                           of each text field, now we just use a NULL pointer to
                                           signify there is no default value.
    \Version    1.3     09/05/07    (RB)   Added back methods removed during lockless Apt implementation.
                                           Fixes .swf-remaining assert and AptCharacter refcount assert.

    \Version    1.4     09/07/07 (ASK) Added LoadResourcesFromCharacter function to load textures
    \Version    1.5     05/27/09 (DG)  Modified LoadResourcesFromCharacter() to avoid thread problem caused by using AptFilePtr's refcount.
*/
/************************************************************************************************H*/

#include "_Apt.h"
#include "Display/AptRenderingContext.h"
#include "Display/AptDisplayListState.h"

#include "AptRenderList.h"
#include "MainInline.h"

APT_DEFINE_OBJECT_METRICS_EXPLICIT(AptCharacter, NULL);

/*F************************************************************************************************/
/*!
    \Function    AptCharacter::SetupCharacter

    \Description
        Sets up each character

    This function basically sets ref counts to zero and copies over the AptFilePtr is needed.  If eType
    is a shape, it checks if there is a texture associated with it and sets a flag to indicate the texture needs
    to be loaded and binded before rendering.  These don't need to be set, but they are just in case.

    \Input      pFile             -

    \Output
        None.

    \Notes
        None.

    \Version    1.0        04/12/05 (LEO) First Version

*/
/************************************************************************************************F*/
void AptCharacter::SetupCharacter()
{
    m_pAnimFile = AptFilePtr(0);

    switch(eType)
    {
    case AptCharacterType_Shape:
        m_shapeData.m_nRef = 0;
        if(m_shapeData.m_textID != 0)
            m_shapeData.m_bNotLoaded = 1;
        else
            m_shapeData.m_bNotLoaded = 0;
        break;
    case AptCharacterType_Bitmap:
        m_bitmapData.m_nRef = 0;
        m_bitmapData.m_bLoaded = 0;
        m_bitmapData.m_bBinded = 0 ;
        break;
    default:
        APT_ASSERT(false);
        //lint -fallthrough
    case AptCharacterType_Font:
    case AptCharacterType_Sound:
    case AptCharacterType_Morph:
    case AptCharacterType_StaticText:
    case AptCharacterType_Video:
    case AptCharacterType_Text:
    case AptCharacterType_Button:
    case AptCharacterType_Sprite:
    case AptCharacterType_Animation:
    case AptCharacterType_None:
        m_data.m_nRef = 0;
        break;
    case AptCharacterType_Image:
        m_data.m_nRef = 0;
        image.texture = NULL;
        break;
    }
}

/*F************************************************************************************************/
/*!
    \Function    AptCharacter::ReleaseAnimationFile

    \Description
        Releases the AptFilePtr smart pointer reference

    Since rendered data now contains reference counts that in turn hold references to files, we need to release
    the file reference when the character is deleted.

    \Input
        None.

    \Output
        None.

    \Notes
        None.

    \Version    1.0        04/12/05 (LEO) First Version

*/
/************************************************************************************************F*/
void AptCharacter::ReleaseAnimationFile() const
{
    m_pAnimFile = AptFilePtr(0);
}

/*F************************************************************************************************/
/*!
    \Function    AptCharacter::AddRef

    \Description
        Increments the reference count

    Insert your long description here.

    \Input
        None.

    \Output
        None.

    \Notes
        None.

    \Version    1.0        04/12/05 (LEO) First Version
*/
/************************************************************************************************F*/
void AptCharacter::AddCharacterReference() const
{
    APT_ASSERT(this != NULL);
#if defined(EA_DEBUG)
    if(eType != AptCharacterType_Animation && m_pAnimFile.pData == NULL)
    {
        if(eType == AptCharacterType_Image)
        {
            // Not sure what these checks are for!  Sorry for not understanding Apt too well!
            // - Colin C. 3/7/12
        }
        else if(eType == AptCharacterType_Shape || eType == AptCharacterType_Bitmap)
        {
            APT_ASSERTM(this->m_pAnimFile.pData != NULL, ("pData should not be NULL!"));
        }
        else if(m_data.m_nDynamic != 1)
        {
            APT_ASSERTM(this->m_pAnimFile.pData != NULL, ("pData should not be NULL!"));
        }
    }
#endif

    if(m_data.m_nRef >= APT_RENDER_MAX_REFCOUNT)
    {
        APT_FAIL("APT_RENDER_MAX_REFCOUNT hit for pCharacter");
        m_data.m_nRef = APT_RENDER_MAX_REFCOUNT;
    }
    else
    {
        ++m_data.m_nRef;
    }
}
/*F************************************************************************************************/
/*!
    \Function    AptCharacter::Release

    \Description
        Decrements the reference count

    Insert your long description here.

    \Input
        None.

    \Output
        None.

    \Notes
        None.

    \Version    1.0        04/12/05 (LEO) First Version
    \Version    1.1        05/27/05 (DC)  Ensured we used the ret value from the atomic operation
                                          in decoupled mode (ensures only one execution gets a ref count
                                          of 0).

*/
/************************************************************************************************F*/
void AptCharacter::ReleaseCharacterReference() const
{
    uint32_t uRefCountLocal;
    APT_ASSERT(this != NULL);

    APT_ASSERT(m_shapeData.m_nRef > 0 && "pCharacter ref count hitting negative values");

    uRefCountLocal = --m_data.m_nRef;

    //APT_ASSERT(m_data.m_nRef == uRefCountLocal);
    if(uRefCountLocal == 0)
    {
        // If the reference count is zero, we delete this instance.
        ReleaseAnimationFile();
    }
}

/*F************************************************************************************************/
/*!
    \Function    AptCharacter::render

    \Description
        Calls auxiliary libs to render shapes and static text

    \Input      *pRenderingContext- passed around internall in apt with rendering info
    \Input      eMaskOperation    - type of render operation
    \Input      nMaskDepth        - number of recursive masks below us
    \Input      *pMatrix          - New Matrix to be used if passed in.

    \Output
        None.

    \Notes
        None.

    \Version    1.0        03/18/05 (DC) Bringin' it to ya
    \Version    1.1        03/18/05 (DC) $fr840 - DC - Added Support for nested Masks.
    \Version    2.0        04/12/05 (LEO) Added code to load and bind textures at render time
    \Version    2.1        06/22/05 (LEO) $bug931 Multi-threaded crash in AptCharacter::render() for some swf.

*/
/************************************************************************************************F*/
void AptCharacter::render(AptRenderingContext *pRenderingContext, AptMaskRenderOperation eMaskOperation, int nMaskDepth, const AptRenderItem *pRenderItem)
{
    APT_PROFILE_RENDER(AptCharacter_render);
    switch (eType)
    {
        case AptCharacterType_Shape:
        {
            // $senzee-lockless we can probly get the matrix from pRenderContext instead (which would be cleaner)
            AptRenderInfo info;
            info.mnLevel     = AptRenderItem::suCurrentRenderAnimLevel;
            info.mTransform  = *AptMath::_ClipStackGetTop();
            info.meMaskOper  = eMaskOperation;
            info.mnMaskDepth = nMaskDepth;
            AptRenderList::Add(info, pRenderItem, shape.pRenderUnit);
            break;
        }

        break;
        case AptCharacterType_None:

        {
            // ignore
        }

        break;
        default:
        {
            APT_ASSERT(NOT_REACHED);
        }
    }
}


/*F************************************************************************************************/
/*!
    \Function    AptCharacter::LoadResourcesFromCharacter

    \Description
    Goes through all the characters in the .apt file and loads all the textures that are needed for this SWF file.

    \Input
    None

    \Output
    None.

    \Notes
    Before lockless Apt this code use to be part of AptCharacter::Render function. But now it is called from
    AptRenderItemShape::LoadResourcesFromCharacter just before very first time a shape character from this file is rendered.
    This makes sure that Apt still calls pfnLoadTexture callback from render thread.

    \Version    1.0        09/06/07 (ASK) First Version
     \Version   1.1        11/21/07 (JP) Checking that pTmp.pData is not NULL
     \Version   1.2        05/27/09 (DG) Use m_pAnimFile.pData directly in order to avoid touching the AptFile refcount value
*/
/************************************************************************************************F*/
void AptCharacter::LoadResourcesFromCharacter()
{
    APT_PROFILE_RENDER(AptCharacter_render);
    switch (eType)
    {
    case AptCharacterType_Shape:
        {
            APT_PROFILE_RENDER(AptCharacter_render_Shape);
            //APT_ASSERT(shape.zID);

            if(m_shapeData.m_bNotLoaded == 1)
            {
                APT_PROFILE_RENDER(AptCharacter_render_Shape_BitmapBinding);
                APT_ASSERT(pParentAnim->animation.apCharacters[m_shapeData.m_textID]->eType == AptCharacterType_Bitmap);

                // code to go through all the characters in the parent animation and load any bitmaps which are there.
                //  this is needed because a shape can have multiple bitmaps in it and we need to load and bind them
                // all before we render them.
                int32_t nTemp = pParentAnim->animation.nCharacters ;
                for (int32_t i = 0; i < nTemp; i++)
                {
                    if(pParentAnim->animation.apCharacters[i] == NULL)
                    {
                        continue;
                    }
                    if(pParentAnim->animation.apCharacters[i]->eType == AptCharacterType_Bitmap)
                    {
                        if(pParentAnim->animation.apCharacters[i]->m_bitmapData.m_bLoaded == 0)
                        {
                            int32_t nTempId = pParentAnim->animation.IsImport(i) ;
                            if (nTempId != -1 )
                            {
                                APT_PROFILE_AUXILIARY(pfnLoadTexture_and_BindTexture);
                                pParentAnim->animation.apCharacters[i]->bitmap.zID = gAptFuncs.pfnLoadTexture(AptAnimationFile::Cast(pParentAnim->animation.apCharacters[i]->m_pAnimFile)->GetUserData(), pParentAnim->animation.GetIDFromImportFile(nTempId));
                                gAptFuncs.pfnBindTexture(AptAnimationFile::Cast(m_pAnimFile)->GetUserData(), i, pParentAnim->animation.apCharacters[i]->bitmap.zID);
                                gAptFuncs.pfnBindTexture(AptAnimationFile::Cast(pParentAnim->animation.apCharacters[i]->m_pAnimFile)->GetUserData(), pParentAnim->animation.GetIDFromImportFile(nTempId), pParentAnim->animation.apCharacters[i]->bitmap.zID);
                                pParentAnim->animation.apCharacters[i]->m_bitmapData.m_bLoaded = 1;
                                pParentAnim->animation.apCharacters[i]->m_bitmapData.m_bBinded = 1 ;
                            }
                            else
                            {
                                // $bug944 - RB - If pAnimFile is NULL, we search for an imported item and uses that m_pAnimFile pointer.  This is safe
                                // $bug944 - RB - because if we reach this case, we know that at least one thing in the exporting character list is being used.
                                AptAnimationFile* pTmp = AptAnimationFile::Cast(m_pAnimFile);
                                if(pTmp == NULL)
                                {
                                    int32_t ii = 0;
                                    for (ii = 0; ii < pParentAnim->animation.nExports; ii++)
                                    {
                                        if(pParentAnim->animation.apCharacters[pParentAnim->animation.aExports[ii].nID] != NULL && pParentAnim->animation.apCharacters[pParentAnim->animation.aExports[ii].nID]->m_pAnimFile.pData != NULL)
                                        {
                                            pTmp = AptAnimationFile::Cast(pParentAnim->animation.apCharacters[pParentAnim->animation.aExports[ii].nID]->m_pAnimFile);
                                            break;
                                        }
                                    }
                                    APT_ASSERT(ii != pParentAnim->animation.nExports);
                                }
                                APT_PROFILE_AUXILIARY(pfnLoadTexture_and_BindTexture);
                                if(pTmp) // If pTmp continues being NULL the assert is triggered, but this avoids an eventual crash
                                {
                                    pParentAnim->animation.apCharacters[i]->bitmap.zID = gAptFuncs.pfnLoadTexture(pTmp->GetUserData(), i);
                                    gAptFuncs.pfnBindTexture(pTmp->GetUserData(), i, pParentAnim->animation.apCharacters[i]->bitmap.zID);
                                    pParentAnim->animation.apCharacters[i]->m_bitmapData.m_bLoaded = 1;
                                }
                            }
                        }
                        else if(pParentAnim->animation.apCharacters[i]->m_pAnimFile != m_pAnimFile && pParentAnim->animation.apCharacters[i]->m_pAnimFile.pData != NULL)
                        {
                            // that means this is a imported bitmap, now we have to bind it to all shapes in this animation.
                            if (pParentAnim->animation.apCharacters[i]->m_bitmapData.m_bBinded == 0  ||  m_shapeData.m_bNotLoaded == 1)
                            {
                                APT_PROFILE_AUXILIARY(pfnBindTexture);
                                gAptFuncs.pfnBindTexture(AptAnimationFile::Cast(m_pAnimFile)->GetUserData(), i, pParentAnim->animation.apCharacters[i]->bitmap.zID);
                                pParentAnim->animation.apCharacters[i]->m_bitmapData.m_bBinded = 1 ;
                            }
                        }
                        else if (pParentAnim->animation.apCharacters[i]->m_pAnimFile.pData == NULL  &&  m_shapeData.m_bNotLoaded == 1)
                        {
                            APT_PROFILE_AUXILIARY(pfnBindTexture);
                            gAptFuncs.pfnBindTexture(AptAnimationFile::Cast(m_pAnimFile)->GetUserData(), i, pParentAnim->animation.apCharacters[i]->bitmap.zID);
                        }
                    }
                    else if (pParentAnim->animation.apCharacters[i]->eType == AptCharacterType_Shape)
                    {
                        pParentAnim->animation.apCharacters[i]->m_shapeData.m_bNotLoaded = 0;
                    }
                }
            }
            break;
        }

        break;
    case AptCharacterType_None:

        {
            // ignore
        }

        break;
    default:
        {
            APT_ASSERT(NOT_REACHED);
        }
    }
}

/*F************************************************************************************************/
/*!
    \Function   AptCharacter::GetBoundingRect

    \Description
        returns the boundary box of shapes and static text in pRect

    Insert your long description here.

        NO END BRACKET

    \Output     NO RETURN         -

    \Notes
        None.

    \Version    1.0        05/04/04 (LEO) First Version

*/
/************************************************************************************************F*/

void AptCharacter::GetBoundingRect(AptRenderingContext *pRenderingContext, const AptMatrix *pMatrix, AptRect *pRect) const
{
    switch (eType)
    {
        case AptCharacterType_Shape:
        {
            pRenderingContext->expandBoundingRect( &shape.rBounds, pMatrix, pRect);
            break;
        }

        case AptCharacterType_None:
        {
            // ignore
            break;
        }

        case AptCharacterType_StaticText:
        {
            pRenderingContext->expandBoundingRect(&statictext.rBounds, pMatrix, pRect );
            break;
        }

        case AptCharacterType_Image:
        {
            pRenderingContext->expandBoundingRect(&image.bounds, pMatrix, pRect );
            break;
        }

        default:
        {
            APT_ASSERT(NOT_REACHED);
            break;
        }
    }
}
