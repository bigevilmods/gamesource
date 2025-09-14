/*H************************************************************************************************/
/*!

    \File
    AptRenderingContext.cpp

    \Description
        AptRenderingContext class implementation.

    \Notes
    None.

    \Copyright
    (c) 2005 Electronic Arts Inc.

    \Version    1.0        01/04/05 (MCB) First Version

*/
/************************************************************************************************H*/

#include "_Apt.h"
#include "Display/AptRenderingContext.h"
#include "MainInline.h"

APT_DEFINE_OBJECT_METRICS_EXPLICIT(AptRenderingContext, NULL);

/*F************************************************************************************************/
/*!
    \Function    AptRenderingContext::AptRenderingContext

    \Description
        Constructor

    \Input
        None.

    \Output

    \Notes
        None.

    \Version    1.0        07/18/05 (LEO) First Version
    \Version    1.1        12/12/05 (LEO Added Apt_3D code

*/
/************************************************************************************************F*/
AptRenderingContext::AptRenderingContext()
{
    AptColorHelperScale& vColorMul4 = curCXForm.scale;
    AptColorHelperTranslate& vColorAdd4 = curCXForm.translate;

    vColorMul4.SetValuef(AptColorHelper::Red, 255.f);
    vColorMul4.SetValuef(AptColorHelper::Green, 255.f);
    vColorMul4.SetValuef(AptColorHelper::Blue, 255.f);
    vColorMul4.SetValuef(AptColorHelper::Alpha, 255.f);
    vColorAdd4.SetValuef(AptColorHelper::Red, 0.f);
    vColorAdd4.SetValuef(AptColorHelper::Green, 0.f);
    vColorAdd4.SetValuef(AptColorHelper::Blue, 0.f);
    vColorAdd4.SetValuef(AptColorHelper::Alpha, 0.f);

#if !defined(APT_3D)
    curVertexMatrix.a = 1.f;
    curVertexMatrix.b = 0.f;
    curVertexMatrix.c = 0.f;
    curVertexMatrix.d = 1.f;
    curVertexMatrix.tx = 0.f;
    curVertexMatrix.ty = 0.f;
#else
    AptMath::MatMakeIdentity(&curVertexMatrix);
#endif
#if (APT_DEBUG_LEVEL >= APT_DEBUG_NORMAL)
    nCXFormStack = 0;
    nVertexMatrixStack = 0;
#endif
    pVertexMatrixInterator = aVertexMatrixStack;
    pCXFormInterator = aCXFormStack;
}

/*F************************************************************************************************/
/*!
    \Function    AptRenderingContext::pushColourTransform

    \Description
        Pushes a new color transform onto the color transform stack

    \Input
        None.

    \Output
        None.

    \Notes
        None.

    \Version    1.0        07/18/05 (LEO) First Version

*/
/************************************************************************************************F*/
void AptRenderingContext::pushColourTransform(void)
{
    APT_ASSERT_RENDER_THREAD();

#if (APT_DEBUG_LEVEL >= APT_DEBUG_NORMAL)
    APT_ASSERT(nCXFormStack < APT_ARRAYSIZE(aCXFormStack));
    nCXFormStack++;
#endif
    *(pCXFormInterator++) = curCXForm;
}

/*F************************************************************************************************/
/*!
    \Function    AptRenderingContext::popColourTransform

    \Description
        Pops the top of the color transform stack

    \Input
        None.

    \Output
        None.

    \Notes
        None.

    \Version    1.0        07/18/05 (LEO) First Version

*/
/************************************************************************************************F*/
void AptRenderingContext::popColourTransform(void)
{
    APT_ASSERT_RENDER_THREAD();
#if (APT_DEBUG_LEVEL >= APT_DEBUG_NORMAL)
    APT_ASSERT(nCXFormStack > 0);
    --nCXFormStack;
#endif
    curCXForm = *(--pCXFormInterator);
}

/*F************************************************************************************************/
/*!
    \Function    AptRenderingContext::appendColourTransform

    \Description
        Multiplies out a color matrix and sends it to the aux layer for rendering with

    \Input      *pCXForm          -

    \Output
        None.

    \Notes
        None.

    \Version    1.0        07/18/05 (LEO) First Version

*/
/************************************************************************************************F*/
void AptRenderingContext::appendColourTransform(const AptCXForm *pCXForm)
{
    APT_ASSERT_RENDER_THREAD();

    if(pCXForm != &gIdentityCXForm)     // Do not do this if pCXForm is the idenity CXForm
    {
        curCXForm.scale.SetValuef(AptColorHelper::Alpha, (curCXForm.scale.GetValuef(AptColorHelper::Alpha) * pCXForm->scale.GetValuef(AptColorHelper::Alpha)) / AptColorHelperScale::SCALE_FACTOR);
        curCXForm.scale.SetValuef(AptColorHelper::Red,   (curCXForm.scale.GetValuef(AptColorHelper::Red) * pCXForm->scale.GetValuef(AptColorHelper::Red)) / AptColorHelperScale::SCALE_FACTOR);
        curCXForm.scale.SetValuef(AptColorHelper::Green, (curCXForm.scale.GetValuef(AptColorHelper::Green) * pCXForm->scale.GetValuef(AptColorHelper::Green)) / AptColorHelperScale::SCALE_FACTOR);
        curCXForm.scale.SetValuef(AptColorHelper::Blue,  (curCXForm.scale.GetValuef(AptColorHelper::Blue) * pCXForm->scale.GetValuef(AptColorHelper::Blue)) / AptColorHelperScale::SCALE_FACTOR);
        curCXForm.translate.SetValuef(AptColorHelper::Alpha, curCXForm.translate.GetValuef(AptColorHelper::Alpha) + pCXForm->translate.GetValuef(AptColorHelper::Alpha));
        curCXForm.translate.SetValuef(AptColorHelper::Red,   curCXForm.translate.GetValuef(AptColorHelper::Red) + pCXForm->translate.GetValuef(AptColorHelper::Red));
        curCXForm.translate.SetValuef(AptColorHelper::Green, curCXForm.translate.GetValuef(AptColorHelper::Green) + pCXForm->translate.GetValuef(AptColorHelper::Green));
        curCXForm.translate.SetValuef(AptColorHelper::Blue,  curCXForm.translate.GetValuef(AptColorHelper::Blue) + pCXForm->translate.GetValuef(AptColorHelper::Blue));

        // The following code has been commented out because MultiplyColors and AddColors have not been fully tested yet..  but it seems to work
        //curCXForm.scale.SetValue(AptColorHelper::Alpha, AptColorHelper::MultiplyColors(curCXForm.scale.GetA(), pCXForm->scale.GetA()));
        //curCXForm.scale.SetValue(AptColorHelper::Red,   AptColorHelper::MultiplyColors(curCXForm.scale.GetR(), pCXForm->scale.GetR()));
        //curCXForm.scale.SetValue(AptColorHelper::Green, AptColorHelper::MultiplyColors(curCXForm.scale.GetG(), pCXForm->scale.GetG()));
        //curCXForm.scale.SetValue(AptColorHelper::Blue,  AptColorHelper::MultiplyColors(curCXForm.scale.GetB(), pCXForm->scale.GetB()));

        //curCXForm.translate.SetValue(AptColorHelper::Alpha, AptColorHelper::AddColors(curCXForm.translate.GetA(), pCXForm->translate.GetA()));
        //curCXForm.translate.SetValue(AptColorHelper::Red,   AptColorHelper::AddColors(curCXForm.translate.GetR(), pCXForm->translate.GetR()));
        //curCXForm.translate.SetValue(AptColorHelper::Green, AptColorHelper::AddColors(curCXForm.translate.GetG(), pCXForm->translate.GetG()));
        //curCXForm.translate.SetValue(AptColorHelper::Blue,  AptColorHelper::AddColors(curCXForm.translate.GetB(), pCXForm->translate.GetB()));
        #if (APT_DEBUG_LEVEL >= APT_DEBUG_NORMAL) && defined(APT_CXFORM_MATRIX_PROFILE)
        gCXFormMult++;
        #endif
    }
    #if (APT_DEBUG_LEVEL >= APT_DEBUG_NORMAL) && defined(APT_CXFORM_MATRIX_PROFILE)
    else
    {
        gCXFormNoMult++;
    }
    #endif
    /* $senzee this functionality has been moved to AptRenderList.cpp
    APT_PROFILE_AUXILIARY(pfnSetColourTransform);
    gAptFuncs.pfnSetColourTransform(&curCXForm);
    */
}

/*F************************************************************************************************/
/*!
    \Function    AptRenderingContext::getVertexMatrix

    \Description
        Returns the top item of the matrix rendering stack

    \Input      *pMatrix          -

    \Output
        None.

    \Notes
        None.

    \Version    1.0        07/18/05 (LEO) First Version
    \Version    1.1        12/12/05 (LEO) Added Apt_3D code,  changed prototype
*/
/************************************************************************************************F*/
#if !defined(APT_3D)
void AptRenderingContext::getVertexMatrix(AptMatrix *pMatrix) const
#else
void AptRenderingContext::getVertexMatrix(AptMath::Mat44T *pMatrix) const
#endif
{
    APT_ASSERT_RENDER_THREAD();

    APT_ASSERT(pMatrix);
    *pMatrix = curVertexMatrix;
}

/*F************************************************************************************************/
/*!
    \Function    AptRenderingContext::pushVertexMatrix

    \Description
        Adds a new item to the top of the matrix stack for rendering

    \Input
        None.

    \Output
        None.

    \Notes
        None.

    \Version    1.0        07/18/05 (LEO) First Version

*/
/************************************************************************************************F*/
void AptRenderingContext::pushVertexMatrix(void)
{
    APT_ASSERT_RENDER_THREAD();

#if (APT_DEBUG_LEVEL >= APT_DEBUG_NORMAL)
    APT_ASSERT(nVertexMatrixStack < APT_ARRAYSIZE(aVertexMatrixStack));
    nVertexMatrixStack++;
#endif
    *(pVertexMatrixInterator++) = curVertexMatrix;
}

/*F************************************************************************************************/
/*!
    \Function    AptRenderingContext::setVertexMatrix

    \Description
        Sets the top rendering matrix

    \Input      pSetMatrix        -

    \Output
        None.

    \Notes
        None.

    \Version    1.0        07/18/05 (LEO) First Version
    \Version    1.1        12/12/05 (LEO) Added Apt_3D code, changed prototype
*/
/************************************************************************************************F*/
#if !defined(APT_3D)
void AptRenderingContext::setVertexMatrix(AptMatrix * pSetMatrix)
#else
void AptRenderingContext::setVertexMatrix(AptMath::Mat44T * pSetMatrix)
#endif
{
    APT_ASSERT_RENDER_THREAD();
    curVertexMatrix = *pSetMatrix;
}

/*F************************************************************************************************/
/*!
    \Function    AptRenderingContext::popVertexMatrix

    \Description
        Pops the top matrix off the stack

    \Input
        None.

    \Output
        None.

    \Notes
        None.

    \Version    1.0        07/18/05 (LEO) First Version

*/
/************************************************************************************************F*/
void AptRenderingContext::popVertexMatrix(void)
{
    APT_ASSERT_RENDER_THREAD();

#if (APT_DEBUG_LEVEL >= APT_DEBUG_NORMAL)
    APT_ASSERT(nVertexMatrixStack > 0);
    --nVertexMatrixStack;
#endif
    curVertexMatrix = *(--pVertexMatrixInterator);
    /* $senzee this functionality has been moved to AptRenderList.cpp
    APT_PROFILE_AUXILIARY(pfnSetVertexMatrix);
    gAptFuncs.pfnSetVertexMatrix(&curVertexMatrix);
    */
}

/*F************************************************************************************************/
/*!
    \Function    AptRenderingContext::multMatrix

    \Description
        Multiplies a matrix and returns in as pOut

    Added some optimizations to check for identity matrices

    \Input      *pA               -
    \Input      *pB               -
    \Input      *pOut             -

    \Output
        None.

    \Notes
        None.

    \Version    1.0        07/18/05 (LEO) First Version

*/
/************************************************************************************************F*/
void AptRenderingContext::multMatrix(const AptMatrix *pA, const AptMatrix *pB, AptMatrix *pOut)
{
    if(pA != &gIdentityMatrix && pB != &gIdentityMatrix)            // If pA and pB are not identity, we have to do work
    {
        AptMatrix temp = *pA;
        AptMatrix temp2 = *pB;

        pOut->a  = temp2.a  * temp.a + temp2.b  * temp.c;
        pOut->b  = temp2.a  * temp.b + temp2.b  * temp.d;
        pOut->c  = temp2.c  * temp.a + temp2.d  * temp.c;
        pOut->d  = temp2.c  * temp.b + temp2.d  * temp.d;
        pOut->tx = temp2.tx * temp.a + temp2.ty * temp.c + temp.tx;
        pOut->ty = temp2.tx * temp.b + temp2.ty * temp.d + temp.ty;

        #if (APT_DEBUG_LEVEL >= APT_DEBUG_NORMAL) && defined(APT_CXFORM_MATRIX_PROFILE)
        gMatrixMult++;
        #endif
    }
    else                                                            // Here we are multiplying against a idenity matrix, so simply return the other matrix
    {
        memcpy(pOut, pA == &gIdentityMatrix ? pB : pA, sizeof(AptMatrix));
        #if (APT_DEBUG_LEVEL >= APT_DEBUG_NORMAL) && defined(APT_CXFORM_MATRIX_PROFILE)
        gMatrixNoMult++;
        #endif
    }
}

#if defined(APT_3D)
void AptRenderingContext::multMatrix3D(const AptMath::Mat44T *pA, const AptMath::Mat44T *pB, AptMath::Mat44T *pOut)
{
    AptMath::MatMul3d(pOut, pA, pB);
}
#endif

/*F************************************************************************************************/
/*!
    \Function    AptRenderingContext::appendVertexMatrix

    \Description
        Multiplies the top rendering matrix with the passed in matrix

    An optimization was added to check for the identity matrix

    \Input      *pMatrix          -

    \Output
        None.

    \Notes
        None.

    \Version    1.0        07/18/05 (LEO) First Version
    \Version    1.1        12/12/05 (LEO) Added Apt_3D code, changed prototype
*/
/************************************************************************************************F*/
#if !defined(APT_3D)
void AptRenderingContext::appendVertexMatrix(const AptMatrix *pMatrix)
#else
void AptRenderingContext::appendVertexMatrix(const AptMath::Mat44T *pMatrix)
#endif
{
    APT_ASSERT_RENDER_THREAD();

#if !defined(APT_3D)
    if(pMatrix != &gIdentityMatrix)
        multMatrix(&curVertexMatrix, pMatrix, &curVertexMatrix);
#else
    multMatrix3D(&curVertexMatrix, pMatrix, &curVertexMatrix);
#endif

    /* $senzee this functionality has been moved to AptRenderList.cpp
    APT_PROFILE_AUXILIARY(pfnSetVertexMatrix);
    gAptFuncs.pfnSetVertexMatrix(&curVertexMatrix);
    */
}

/*F************************************************************************************************/
/*!
    \Function    AptRenderingContext::expandBoundingRect

    \Description
        Find the bounding box by expanding pNewRect by the passed in AptMatrix

    \Input      *pNewRect         -
    \Input      pCurrentTransform -
    \Input      *pBoundingRect    -

    \Output
        None.

    \Notes
        None.

    \Version    1.0        07/18/05 (LEO) First Version
    \Version    1.0        11/21/07 (JO) Removing un-needed auxiliary arrays of floats
*/
/************************************************************************************************F*/
void AptRenderingContext::expandBoundingRect(const AptRect *pNewRect, const AptMatrix * pCurrentTransform, AptRect *pBoundingRect)
{
    float x[4], y[4];
    float xfX[4], xfY[4];
    int i;

    xfX[0] = x[0] = pNewRect->fLeft;
    xfY[0] = y[0] = pNewRect->fTop;
    xfX[1] = x[1] = pNewRect->fRight;
    xfY[1] = y[1] = pNewRect->fTop;
    xfX[2] = x[2] = pNewRect->fRight;
    xfY[2] = y[2] = pNewRect->fBottom;
    xfX[3] = x[3] = pNewRect->fLeft;
    xfY[3] = y[3] = pNewRect->fBottom;

    if(pCurrentTransform != NULL)
    {
        if(pCurrentTransform == &gIdentityMatrix)        // We have an identity matrix
        {
            for (i = 0; i < 4; i++)     // xform new rect by current matrix
            {
                xfX[i] = x[i] + pCurrentTransform->tx;
                xfY[i] = y[i] + pCurrentTransform->ty;
            }
        }
        else
        {
            for (i = 0; i < 4; i++)     // xform new rect by current matrix
            {
                xfX[i] = pCurrentTransform->a * x[i] + pCurrentTransform->c * y[i] + pCurrentTransform->tx;
                xfY[i] = pCurrentTransform->b * x[i] + pCurrentTransform->d * y[i] + pCurrentTransform->ty;
            }

        }
    }

    for (i = 0; i < 4; i++)             // find bounding rect of xformed points and orig brect
    {
        if (xfX[i] < pBoundingRect->fLeft)
            pBoundingRect->fLeft = xfX[i];
        if (xfX[i] > pBoundingRect->fRight)
            pBoundingRect->fRight = xfX[i];
        if (xfY[i] < pBoundingRect->fTop)
            pBoundingRect->fTop = xfY[i];
        if (xfY[i] > pBoundingRect->fBottom)
            pBoundingRect->fBottom = xfY[i];
    }
}
