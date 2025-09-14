/*H************************************************************************************************/
/*!

    \File
        AptMatrix.h

    \Description
        This structure maintains certain attributes of an object's position on the screen with
        relation to rotation, scale, and translation. A good reference for these topics can be
        found at http://www.siggraph.org/education/materials/HyperGraph/modeling/mod_tran/2dconc.htm.
        Basically, this matrix structure stores all of this information as such:

        A reference about the inverse matrix can be found at http://www.senocular.com/flash/tutorials/transformmatrix/

        When rotation is zero:
            a=Sx
            b=0
            c=0
            d=Sy
        When rotation is non-zero, these values take rotation into account as follows:
            a=Sx*cos(rotation)
            b=Sx*sin(rotation)
            c=Sy*(-sin(rotation))
            d=Sy*cos(rotation)
        Note:
            tx and ty are the x and y values respectively for translation.

        Again, refer to the webpage mentioned above to follow how these equations were derived.
        Instead of doing each operation separately everywhere in the code when they're needed, this
        is a more efficient method of combining all factors and doing one calculation to take
        all factors into consideration.

    \Notes
        None.

    \Copyright
        (c) 2005 Electronic Arts Inc.

    \Version
        1.0        01/07/05 (MCB) - Figured out what the hell this is used for.
        2.0        03/04/05 (JP)  - Inverse matrix method
*/
/************************************************************************************************H*/

#ifndef __APTMATRIX_H__
#define __APTMATRIX_H__

#include "AptDefine.h"

struct AptMatrix
{
    void AptMatrixCopy(const AptMatrix *pMatrix)
    {
        if(pMatrix != NULL)
        {
            a = pMatrix->a;
            b = pMatrix->b;
            c = pMatrix->c;
            d = pMatrix->d;
            tx = pMatrix->tx;
            ty = pMatrix->ty;
        }
    }

    bool AptMatrixInverse(const AptMatrix *pMatrix)
    {
        bool bHasInverse = (pMatrix != NULL);
        if(bHasInverse)
        {
            // The determinant can be different than 1 if we have an homotece
            float determinant = pMatrix->a * pMatrix->d - pMatrix->b * pMatrix->c;
            bHasInverse = (determinant != 0.f);
            if(bHasInverse)
            {
                a  = pMatrix->d  / determinant;
                b  = - pMatrix->b  / determinant;
                c  = - pMatrix->c  / determinant;
                d  = pMatrix->a  / determinant;
                tx = (- pMatrix->tx * pMatrix->d + pMatrix->ty * pMatrix->c) / determinant;
               ty =   (pMatrix->tx * pMatrix->b - pMatrix->ty * pMatrix->a) / determinant ;
            }
        }
        return bHasInverse;  // Returns false if the inverse matrix cannot be calculated
    }

    float a, b, c, d;
    float tx, ty;

    APT_NEW_DELETE_OPERATORS

    // DC - Added Metric defines
    APT_DECLARE_OBJECT_METRICS
};

#endif  //  __APTMATRIX_H__

/*H*************************************************************************************************
    EOF
*************************************************************************************************H*/

