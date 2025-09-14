/*H************************************************************************************************/
/*!

    \File
        AptFilter.h

    \Description
        Defines low-level data structures for the flash.filters.* classes.

    \Copyright
        (c) 2007 Electronic Arts Inc.

    \Version 1.0        04/29/08 (RB) First version

*/
/************************************************************************************************H*/
#include <EABase/eabase.h>
#include "AptObject/AptFilter.h"
#include "AptObject/AptObject.h"
#include "MainInline.h"
#include "string/StringPool.h"

/// A private helper class that has methods for common conversions that we need to perform here
struct AptFilterHelper
{
    /// Converts a 16.16 fixed point value to a 32-bit float.
    static float Fixed16ToFloat( uint32_t nFixed )
    {
        return int16_t(nFixed >> 16) + float(nFixed & 0xFFFF) / 65535.0f;
    }

    /// Converts an 8.8 fixed point value to a 32-bit float.
    static float Fixed8ToFloat( uint16_t nFixed )
    {
        return char8_t(nFixed >> 8) + float(nFixed & 0xFF) / 255.0f;
    }

    /// Converts a 16.16 fixed point value in radians to a 32-bit float value in degrees.
    static float Fixed16RadToDeg( uint32_t nFixed )
    {
        const float APPROX_PI = 3.14159265f;
        return (Fixed16ToFloat(nFixed) * 180.0f) / APPROX_PI;
    }

    /// Converts a 0-based filter ID to the corresponding StringPool enum.
    static StringCode GetFilterClassStringCode( uint32_t nFilterID )
    {
        const uint32_t MAX_ID = 7;
        const StringCode scStringCodes[] =
        {
            SC_DropShadowFilterClass,
            SC_BlurFilterClass,
            SC_GlowFilterClass,
            SC_BevelFilterClass,
            SC_GradientGlowFilterClass,
            SC_ConvolutionFilterClass,
            SC_ColorMatrixFilterClass,
            SC_GradientBevelFilterClass
        };
        if ( nFilterID <= MAX_ID )
            return scStringCodes[ nFilterID ];

        APT_ASSERT( false && "Unknown filter ID" );
        return SC_LAST;
    }
};

/// Drop Shadow Filter

// Takes memory-mapped data and creates entries in a script object's hash table.
void AptFilterDropShadow::SetProperties( const AptFilter* pFilter, AptNativeHash* pHash )
{
        // get pointer to memory-mapped data
        const AptFilterDropShadow* pObj = static_cast<const AptFilterDropShadow*>(pFilter);

        // For every field in the memory-mapped data, create an AptValue and insert it in the hash table.
        // This allows the values to be accessed as properties in ActionScript.  This is how we build the filter object
        // so that it acts like flash.filters.DropShadowFilter.
        pHash->Set( StringPool::GetString(SC_BlendAlpha), AptFloat::Create( float(pObj->mnARGB >> 24) / 255.0f ) );
        pHash->Set( StringPool::GetString(SC_angle), AptFloat::Create( AptFilterHelper::Fixed16RadToDeg(pObj->mnAngle) ) );
        pHash->Set( StringPool::GetString(SC_blurX), AptFloat::Create( AptFilterHelper::Fixed16ToFloat(pObj->mnBlurX) ) );
        pHash->Set( StringPool::GetString(SC_blurY), AptFloat::Create( AptFilterHelper::Fixed16ToFloat(pObj->mnBlurY) ) );
        pHash->Set( StringPool::GetString(SC_Color), AptInteger::Create( pObj->mnARGB & 0xFFFFFF ) );
        pHash->Set( StringPool::GetString(SC_distance), AptFloat::Create( AptFilterHelper::Fixed16ToFloat(pObj->mnDistance) ) );
        pHash->Set( StringPool::GetString(SC_strength), AptFloat::Create( AptFilterHelper::Fixed8ToFloat(pObj->mnStrength) ) );
        pHash->Set( StringPool::GetString(SC_inner), AptBoolean::Create( ((pObj->mnFlags >> 5) & 0x4) != 0 ) );
        pHash->Set( StringPool::GetString(SC_knockout), AptBoolean::Create( ((pObj->mnFlags >> 5) & 0x2) != 0) );
        pHash->Set( StringPool::GetString(SC_hideObject), AptBoolean::Create( ((pObj->mnFlags >> 5) & 0x1) == 0) );  // opposite from other flags
        pHash->Set( StringPool::GetString(SC_quality), AptInteger::Create( pObj->mnFlags & 0x1F ) );
}

/// Blur Filter

// Takes memory-mapped data and creates entries in a script object's hash table.
void AptFilterBlur::SetProperties( const AptFilter* pFilter, AptNativeHash* pHash )
{
        const AptFilterBlur* pObj = static_cast<const AptFilterBlur*>(pFilter);

        pHash->Set( StringPool::GetString(SC_blurX), AptFloat::Create( AptFilterHelper::Fixed16ToFloat(pObj->mnBlurX) ) );
        pHash->Set( StringPool::GetString(SC_blurY), AptFloat::Create( AptFilterHelper::Fixed16ToFloat(pObj->mnBlurY) ) );
        pHash->Set( StringPool::GetString(SC_quality), AptInteger::Create( (pObj->mnFlags >> 3) & 0x1F ) );
}

/// Glow Filter

void AptFilterGlow::SetProperties( const AptFilter* pFilter, AptNativeHash* pHash )
{
        const AptFilterGlow* pObj = static_cast<const AptFilterGlow*>(pFilter);

        pHash->Set( StringPool::GetString(SC_BlendAlpha), AptFloat::Create( float(pObj->mnARGB >> 24) / 255.0f ) );
        pHash->Set( StringPool::GetString(SC_blurX), AptFloat::Create( AptFilterHelper::Fixed16ToFloat(pObj->mnBlurX) ) );
        pHash->Set( StringPool::GetString(SC_blurY), AptFloat::Create( AptFilterHelper::Fixed16ToFloat(pObj->mnBlurY) ) );
        pHash->Set( StringPool::GetString(SC_Color), AptInteger::Create( pObj->mnARGB & 0xFFFFFF ) );
        pHash->Set( StringPool::GetString(SC_strength), AptFloat::Create( AptFilterHelper::Fixed8ToFloat(pObj->mnStrength) ) );
        pHash->Set( StringPool::GetString(SC_inner), AptBoolean::Create( ((pObj->mnFlags >> 5) & 0x4) != 0 ) );
        pHash->Set( StringPool::GetString(SC_knockout), AptBoolean::Create( ((pObj->mnFlags >> 5) & 0x2) != 0) );
        pHash->Set( StringPool::GetString(SC_hideObject), AptBoolean::Create( ((pObj->mnFlags >> 5) & 0x1) == 0) );  // opposite from other flags
        pHash->Set( StringPool::GetString(SC_quality), AptInteger::Create( pObj->mnFlags & 0x1F ) );

}

/// Bevel Filter

void AptFilterBevel::SetProperties( const AptFilter* pFilter, AptNativeHash* pHash )
{
        const AptFilterBevel* pObj = static_cast<const AptFilterBevel*>(pFilter);

        pHash->Set( StringPool::GetString(SC_shadowColor), AptInteger::Create( pObj->mnARGBShadow & 0xFFFFFF ) );
        pHash->Set( StringPool::GetString(SC_shadowAlpha), AptFloat::Create( float(pObj->mnARGBShadow >> 24) / 255.0f ) );
        pHash->Set( StringPool::GetString(SC_highlightColor), AptInteger::Create( pObj->mnARGBHighlight & 0xFFFFFF ) );
        pHash->Set( StringPool::GetString(SC_highlightAlpha), AptFloat::Create( float(pObj->mnARGBHighlight >> 24) / 255.0f ) );
        pHash->Set( StringPool::GetString(SC_blurX), AptFloat::Create( AptFilterHelper::Fixed16ToFloat(pObj->mnBlurX) ) );
        pHash->Set( StringPool::GetString(SC_blurY), AptFloat::Create( AptFilterHelper::Fixed16ToFloat(pObj->mnBlurY) ) );
        pHash->Set( StringPool::GetString(SC_angle), AptFloat::Create( AptFilterHelper::Fixed16RadToDeg(pObj->mnAngle) ) );
        pHash->Set( StringPool::GetString(SC_distance), AptFloat::Create( AptFilterHelper::Fixed16ToFloat(pObj->mnDistance) ) );
        pHash->Set( StringPool::GetString(SC_strength), AptFloat::Create( AptFilterHelper::Fixed8ToFloat(pObj->mnStrength) ) );
        pHash->Set( StringPool::GetString(SC_knockout), AptBoolean::Create( ((pObj->mnFlags >> 4) & 0x4) != 0) );
        pHash->Set( StringPool::GetString(SC_quality), AptInteger::Create( pObj->mnFlags & 0xF ) );

        // The 'type' property needs to be set based on the value of the flags bitfield.
        uint32_t nFlags = pObj->mnFlags >> 4;
        bool bInnerShadow = ((nFlags & 0x8) != 0);
        bool bOnTop = ((nFlags & 0x1) != 0);

        AptString* pTypeString = AptString::Create();
        if ( !bInnerShadow && !bOnTop )
        {
            // outer
            pTypeString->cpy(StringPool::GetString(SC_outer));
        }
        else if ( !bInnerShadow && bOnTop )
        {
            // full
            pTypeString->cpy(StringPool::GetString(SC_full));
        }
        else
        {
            // inner
            APT_ASSERT(( bInnerShadow && !bOnTop ));
            pTypeString->cpy(StringPool::GetString(SC_inner));
        }

        if ( pTypeString )
        {
            pHash->Set( StringPool::GetString(SC_type), pTypeString );
        }
}

/// Gradient Glow Filter.
/// Note that the exact same data structure is used for the Gradient Bevel Filter.

void AptFilterGradientGlow::SetProperties( const AptFilter* pFilter, AptNativeHash* pHash )
{
        const AptFilterGradientGlow* pObj = static_cast<const AptFilterGradientGlow*>(pFilter);

        pHash->Set( StringPool::GetString(SC_blurX), AptFloat::Create( AptFilterHelper::Fixed16ToFloat(pObj->mnBlurX) ) );
        pHash->Set( StringPool::GetString(SC_blurY), AptFloat::Create( AptFilterHelper::Fixed16ToFloat(pObj->mnBlurY) ) );
        pHash->Set( StringPool::GetString(SC_angle), AptFloat::Create( AptFilterHelper::Fixed16RadToDeg(pObj->mnAngle) ) );
        pHash->Set( StringPool::GetString(SC_distance), AptFloat::Create( AptFilterHelper::Fixed16ToFloat(pObj->mnDistance) ) );
        pHash->Set( StringPool::GetString(SC_strength), AptFloat::Create( AptFilterHelper::Fixed8ToFloat(pObj->mnStrength) ) );
        pHash->Set( StringPool::GetString(SC_knockout), AptBoolean::Create( ((pObj->mnFlags >> 4) & 0x4) != 0) );
        pHash->Set( StringPool::GetString(SC_quality), AptInteger::Create( pObj->mnFlags & 0xF ) );

        // The 'type' property needs to be set based on the value of the flags bitfield.
        uint32_t nFlags = pObj->mnFlags >> 4;
        bool bInnerShadow = ((nFlags & 0x8) != 0);
        bool bOnTop = ((nFlags & 0x1) != 0);
        AptString* pTypeString = AptString::Create();
        if ( !bInnerShadow && !bOnTop )
        {
            // outer
            pTypeString->cpy(StringPool::GetString(SC_outer));
        }
        else if ( !bInnerShadow && bOnTop )
        {
            // full
            pTypeString->cpy(StringPool::GetString(SC_full));
        }
        else
        {
            // inner
            APT_ASSERT(( bInnerShadow && !bOnTop ));
            pTypeString->cpy(StringPool::GetString(SC_inner));
        }

        if ( pTypeString )
        {
            pHash->Set( StringPool::GetString(SC_type), pTypeString );
        }

        // colors, alphas
        AptArray* pColorsArray = new AptArray;
        AptArray* pAlphasArray = new AptArray;
        for ( uint32_t i=0; i<pObj->mnNumColors; ++i )
        {
            pColorsArray->set( static_cast<int>(i), AptInteger::Create(pObj->mpGradColors[i]&0xFFFFFF) );
            pAlphasArray->set( static_cast<int>(i), AptFloat::Create( (pObj->mpGradColors[i]>>24)/255.0f ) );
        }
        pHash->Set( StringPool::GetString(SC_colors), pColorsArray );
        pHash->Set( StringPool::GetString(SC_alphas), pAlphasArray );
}

/// Resolves the two arrays in the memory-mapped data
void AptFilterGradientGlow::Resolve( AptFilter* pFilter, uint8_t* pBase )
{
        AptFilterGradientGlow* pObj = static_cast<AptFilterGradientGlow*>(pFilter);

        APT_RESOLVE( pObj->mpGradColors );
        APT_RESOLVE( pObj->mpGradRatios );
}

/// Unresolves the two arrays in the memory-mapped data
void AptFilterGradientGlow::Unresolve( AptFilter* pFilter, uint8_t* pBase )
{
        AptFilterGradientGlow* pObj = static_cast<AptFilterGradientGlow*>(pFilter);

        APT_UNRESOLVE( pObj->mpGradRatios );
        APT_UNRESOLVE( pObj->mpGradColors );
}

/// Color Matrix Filter (a.k.a. the Adjust Color Filter)

void AptFilterColorMatrix::SetProperties( const AptFilter * pFilter, AptNativeHash *pHash)
{
        const int32_t kArraySize = 5 * 4;
        const AptFilterColorMatrix* pObj = static_cast<const AptFilterColorMatrix*>(pFilter);

        AptValue * pFloats[kArraySize];

        for (int32_t i=0; i < kArraySize; ++i)
        {
            pFloats[i] = AptFloat::Create( pObj->mafColorValues[i] );
        }
        AptArray * pColorValues = new AptArray(kArraySize, pFloats);
        pHash->Set( StringPool::GetString(SC_matrix), pColorValues);

}

/*F************************************************************************************************/
/*!
    Resolves indirect filter data from a memory-mapped structure

    \Input      pFilter - Pointer to memory-mapped data. Concrete class can be
                          determined from pFilter->mnFilterID.

    \Output     None.

    \Notes
        The data is resolved in-place within pFilter.

    \Version
        1.0        04/29/08 (RB) First Version

*/
/************************************************************************************************F*/
void AptFilter::Resolve( AptFilter* pFilter, uint8_t* pBase )
{
    StringCode scFilterClass = AptFilterHelper::GetFilterClassStringCode( pFilter->mnFilterID );
    switch( scFilterClass )
    {
    case SC_GradientGlowFilterClass:
    case SC_GradientBevelFilterClass:
        AptFilterGradientGlow::Resolve(pFilter, pBase); break;
    //case SC_ConvolutionFilterClass:
    //    AptFilterConvolution::Resolve(pFilter, pBase); break;
    default:
        break;
    }
}


/*F************************************************************************************************/
/*!
    Unresolves indirect filter data in a memory-mapped structure

    \Input      pFilter - Pointer to memory-mapped data. Concrete class can be
                          determined from pFilter->mnFilterID.

    \Output     None.

    \Notes
        The data is unresolved in-place within pFilter.

    \Version
        1.0        04/29/08 (RB) First Version

*/
/************************************************************************************************F*/
void AptFilter::Unresolve( AptFilter* pFilter, uint8_t* pBase )
{
    StringCode scFilterClass = AptFilterHelper::GetFilterClassStringCode( pFilter->mnFilterID );
    switch( scFilterClass )
    {
    case SC_GradientGlowFilterClass:
    case SC_GradientBevelFilterClass:
        AptFilterGradientGlow::Unresolve(pFilter, pBase); break;
    //case SC_ConvolutionFilterClass:
    //    AptFilterConvolution::Resolve(pFilter, pBase); break;
    default:
        break;
    }
}


/*F************************************************************************************************/
/*!
    Creates a filter script object based on memory-mapped data from a PlaceObject3 tag.

    \Input      pFilter - Pointer to memory-mapped data. Concrete class can be
                          determined from pFilter->mnFilterID.
                pItem   - The movieclip or textfield that owns the filter.

    \Returns    AptObject - The new filter object, which is an instance of a class from flash.filters.*

    \Version
        1.0        04/29/08 (RB) First Version
        1.1        06/13/08 (ASK) Removed extra APT_INC as it was causing objects to stay behind.

*/
/************************************************************************************************F*/
AptObject* AptFilter::CreateFilterObject( const AptFilter* pFilter, AptValue* pItem )
{
    StringCode scFilterClass = AptFilterHelper::GetFilterClassStringCode( pFilter->mnFilterID );
    if ( scFilterClass == SC_LAST )
        return NULL;

    AptObject* pObject = NULL;

    // find out if a constructor is available for the filter class
    AptValue *pConstructor = gAptActionInterpreter.getVariable(pItem, NULL, StringPool::GetString(scFilterClass));

    if (!pConstructor->isUndefined() && pConstructor->CanCreateScriptObject() )
    {
        // instantiate new script object
        pObject = new AptObject(AptVFT_Object);

        // setup constructor and super constructor(s)
        AptNativeHash * pNativeHash = pConstructor->GetNativeHashVirtual();
        AptValue *pConstructorPrototype = pNativeHash->GetPrototype();

        if (pConstructorPrototype == NULL)
        {
            AptPrototype * pTemp = new AptPrototype();
            pTemp->SetSuperConstructor(pConstructor);
            pConstructorPrototype = pTemp;
            pNativeHash->SetPrototype(pConstructorPrototype);
        }

        pObject->SetHasClass(1);
        pObject->setInMainInst(1);
        pObject->Set__Proto__(pConstructorPrototype);

        //
        // set filter properties
        //
        AptNativeHash* pHash = pObject->GetNativeHashVirtual();
        pHash->Set( StringPool::GetString(SC_filterType), AptInteger::Create(pFilter->mnFilterID) );    // StringPool::GetString(SC_filters), pFilterArray );

        // note: we don't use polymorphism because we needed to APT_RESOLVE the AptFilter-derived objects,
        // and we didn't want to add a virtual function table
        switch( scFilterClass )
        {
        case SC_DropShadowFilterClass:
            AptFilterDropShadow::SetProperties(pFilter, pHash); break;
        case SC_BlurFilterClass:
            AptFilterBlur::SetProperties(pFilter, pHash); break;
        case SC_GlowFilterClass:
            AptFilterGlow::SetProperties(pFilter, pHash); break;
        case SC_BevelFilterClass:
            AptFilterBevel::SetProperties(pFilter, pHash); break;
        case SC_GradientGlowFilterClass:
        case SC_GradientBevelFilterClass:
            AptFilterGradientGlow::SetProperties(pFilter, pHash); break;
        //case SC_ConvolutionFilterClass:
        //  AptFilterConvolution::SetProperties(pFilter, pHash); break;
        case SC_ColorMatrixFilterClass:
            AptFilterColorMatrix::SetProperties(pFilter, pHash); break;
        default:
            APT_ASSERT(false); break;
        }

        pObject->setInMainInst(0);
    }
    else
    {
        APT_ASSERT( false && "Could not create filter" );
    }

    // pObject is a new instance of a class from the flash.filters.* namespace.
    return pObject;
}

