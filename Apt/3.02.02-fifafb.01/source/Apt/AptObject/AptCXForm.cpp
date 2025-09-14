/*H************************************************************************************************/
/*!

    \File           AptCXForm.cpp

    \Description    Implements Flash color transform functionality.

    \Notes          If APT_USE_FLASH_COLOR_RANGE is defined, color transforms
                    behave exactly as they do in Flash (provided a correct shader).
                    Otherwise, color transforms are somewhat simplified.
                    See AptCXForm.h for details.

    \Copyright      (c) 2004 Electronic Arts Inc.

    \Version
                    1.0        03/07/04 (ONA) First Version
                    1.01       08/08/07 (RB)  EATech 110673: Minor fix in CopyToFloatArray4.

*/
/************************************************************************************************H*/
#include "_Apt.h"
#include "AptStd/AptCXForm.h"
#include "MainInline.h"         // added for psp-release build

/*F************************************************************************************************/
/*!
    \Function    AptColorHelper::AptColorHelper

    \Description
        Constructor

    \Input
        None.

    \Output     A                 -

    \Notes
        None.

    \Version    1.0        07/15/05 (LEO) First Version

*/
/************************************************************************************************F*/
AptColorHelper::AptColorHelper()
{
#if defined(APT_USE_FLASH_COLOR_RANGE)
    mfVal[0] = 0.f;
    mfVal[1] = 0.f;
    mfVal[2] = 0.f;
    mfVal[3] = 0.f;
#else
    mnVal = 0x00000000;
#endif
}

void AptColorHelper::CopyFromFloatArray(const float *paCXForm)
{
#if defined(APT_USE_FLASH_COLOR_RANGE)
    memcpy(mfVal, paCXForm, sizeof(float) * 4);
#else
    uint32_t nTmp = (uint32_t)(paCXForm[0] * 255.f);
    mnVal = (mnVal & 0x00ffffff) | (nTmp << 24);
    nTmp = (uint32_t)(paCXForm[1] * 255.f);
    mnVal = (mnVal & 0xff00ffff) | (nTmp << 16);
    nTmp = (uint32_t)(paCXForm[2] * 255.f);
    mnVal = (mnVal & 0xffff00ff) | (nTmp << 8);
    nTmp = (uint32_t)(paCXForm[3] * 255.f);
    mnVal = (mnVal & 0xffffff00) | (nTmp << 0);
#endif
}


/*F************************************************************************************************/
/*!
\Function    AptColorHelper::CopyToFloatArray4

\Description
Copies the 32bit color to a array of four floats.  Assumptions are that fColorArray is an array of four floats

\Input      *fColorArray      -

\Output
None.

\Notes
None.

\Version    1.0        07/15/05 (LEO) First Version
            1.01       08/08/07 (RB)  EATech 110673: Copy all 4 floats if using APT_USE_FLASH_COLOR_RANGE.

*/
/************************************************************************************************F*/
void AptColorHelper::CopyToFloatArray4(float *fColorArray) const
{
#if defined(APT_USE_FLASH_COLOR_RANGE)
    memcpy(fColorArray, mfVal, sizeof(float) * 4);
#else
    fColorArray[0] = GetValuef(Alpha);
    fColorArray[1] = GetValuef(Red);
    fColorArray[2] = GetValuef(Green);
    fColorArray[3] = GetValuef(Blue);
#endif
}


/*F************************************************************************************************/
/*!
\Function    AptColorHelper::GetValuef

\Description
Returns a ARGB value from 0.0-1.0 where A = 0, R = 1, G = 2, B = 3

\Input      eColor              -

\Output     float             -

\Notes
-range will be -1-1 if compiled with APT_USE_FLASH_COLOR_RANGE

\Version    1.0        07/15/05 (LEO) First Version

*/
/************************************************************************************************F*/
float AptColorHelper::GetValuef(AptColorValue eColor) const
{
#if defined(APT_USE_FLASH_COLOR_RANGE)
	//PcLint warning Warning 568: non-negative quantity is never less than zero
	int32_t eColorTest = (int32_t)eColor;
    if(eColorTest >= 0 && eColorTest < 4)
    {
	    return mfVal[(int32_t)eColor];
    }

    return 0.f;
#else
    return GetValue(eColor) / 255.f;
#endif
}


/*F************************************************************************************************/
/*!
\Function    AptColorHelper::GetValue

\Description
Returns a ARGB value from 0-255 where A = 0, R = 1, G = 2, B = 3

\Input      eColor              - color wanted

\Output     float             - color

\Notes
-range will be -255-255 if compiled with APT_USE_FLASH_COLOR_RANGE
eColor << 3 = fast multiply by 8 to convert bytes into bits, we use this to select the nth byte into mnVal.

\Version    1.0        07/15/05 (LEO) First Version

*/
/************************************************************************************************F*/
int32_t AptColorHelper::GetValue(AptColorValue eColor) const
{
#if defined(APT_USE_FLASH_COLOR_RANGE)
	//PcLint warning Warning 568: non-negative quantity is never less than zero
	int32_t eColorTest = (int32_t)eColor;
    if(eColorTest >= 0 && eColorTest < 4)
    {
        return (uint32_t)(mfVal[(int32_t)eColor] * 255.f);
    }

    return 0;
#else
    return (mnVal & (0xff000000u >> ((int32_t)eColor << 3))) >> ((3 - (int32_t)eColor) << 3);
#endif
}

//////////////////////////////////////////////////////////////////////////
// AptCXFrom
/*F************************************************************************************************/
/*!
    \Function    AptCXForm::AptCXForm

    \Description
        Copy Constructor

    \Input      *pCXForm          -

    \Output     A                 -

    \Notes
        None.

    \Version    1.0        07/21/05 (LEO) First Version

*/
/************************************************************************************************F*/
AptCXForm::AptCXForm(AptCXForm *pCXForm)
{
    AptCXFormCopy(pCXForm);
}

/*F************************************************************************************************/
/*!
    \Function    AptCXForm::AptCXForm

    \Description
        Copy Constructor for AptFloatArrayCXForm

    \Input      pFloatCXForm      -

    \Output     A                 -

    \Notes
        None.

    \Version    1.0        07/21/05 (LEO) First Version

*/
/************************************************************************************************F*/
AptCXForm::AptCXForm(AptFloatArrayCXForm * pFloatCXForm)
{
    AptFloatArrayCXFormCopy(pFloatCXForm);
}

/*F************************************************************************************************/
/*!
    \Function    AptCXForm::AptCXForm

    \Description
    Copy Constructor for AptUint32CXForm

    \Input      pUintCXForm       -

    \Output     A                 -

    \Notes
        None.

    \Version    1.0        07/21/05 (LEO) First Version

*/
/************************************************************************************************F*/
AptCXForm::AptCXForm(AptUint32CXForm * pUintCXForm)
{
    AptUint32CXFormCopy(pUintCXForm);
}


/*F************************************************************************************************/
/*!
    \Function    AptCXForm::AptCXFormCopy

    \Description
        Copies color data from a AptCXFormCopy object

    \Input      *pCXForm          -

    \Output
        None.

    \Notes
        None.

    \Version    1.0        07/21/05 (LEO) First Version

*/
/************************************************************************************************F*/
void AptCXForm::AptCXFormCopy(const AptCXForm *pCXForm)
{
    if(pCXForm != NULL)
    {
        memcpy(this, pCXForm, sizeof(AptCXForm));
    }
}


/*F************************************************************************************************/
/*!
    \Function    AptCXForm::AptFloatArrayCXFormCopy

    \Description
        Copies color data from a AptFloatArrayCXForm object

    \Input      *pCXForm          -

    \Output
        None.

    \Notes
        None.

    \Version    1.0        07/21/05 (LEO) First Version

*/
/************************************************************************************************F*/
void AptCXForm::AptFloatArrayCXFormCopy(const AptFloatArrayCXForm *pCXForm)
{
    if(pCXForm != NULL)
    {
        scale.CopyFromFloatArray(pCXForm->scale);
        translate.CopyFromFloatArray(pCXForm->translate);
    }
}

/*F************************************************************************************************/
/*!
    \Function    AptCXForm::AptUint32CXFormCopy

    \Description
        Copies color data from a AptUint32CXForm object

    \Input      *pCXForm          -

    \Output
        None.

    \Notes
        None.

    \Version    1.0        07/21/05 (LEO) First Version

*/

/************************************************************************************************F*/
void AptCXForm::AptUint32CXFormCopy(const AptUint32CXForm *pCXForm)
{
    if(pCXForm != NULL)
    {
#if defined(APT_USE_FLASH_COLOR_RANGE)
        uint32_t uScale = pCXForm->nScale;
        uint32_t uBias = pCXForm->nBias;
        //EATech#105290
        scale.SetValuef(AptColorHelper::Alpha, (((uScale >> 24) & 0xFF) / 254.f) * AptColorHelperScale::SCALE_FACTOR );
        scale.SetValuef(AptColorHelper::Red, (((uScale >> 16) & 0xFF) / 254.f) * AptColorHelperScale::SCALE_FACTOR );
        scale.SetValuef(AptColorHelper::Green, (((uScale >> 8) & 0xFF) / 254.f) * AptColorHelperScale::SCALE_FACTOR );
        scale.SetValuef(AptColorHelper::Blue, (((uScale >> 0) & 0xFF) / 254.f) * AptColorHelperScale::SCALE_FACTOR );

        translate.SetValuef(AptColorHelper::Alpha, (uBias >> 24) & 0xFF );
        translate.SetValuef(AptColorHelper::Red, (uBias >> 16) & 0xFF );
        translate.SetValuef(AptColorHelper::Green, (uBias >> 8) & 0xFF );
        translate.SetValuef(AptColorHelper::Blue, (uBias >> 0) & 0xFF );
#else
        scale.CopyFromUint32ARGB(&pCXForm->nScale);
        translate.CopyFromUint32ARGB(&pCXForm->nBias);
#endif
    }
}
