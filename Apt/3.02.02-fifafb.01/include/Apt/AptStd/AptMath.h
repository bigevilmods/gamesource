/*H************************************************************************************************/
/*!

    \File    AptMath.h

    \Description
        Defines basic AptMath class that contains some math types used by Apt like
        Mat44_t, ClipTransform_t

    \Notes
        None.

    \Copyright
        (c) 2004 Electronic Arts Inc.

    \Version    1.0        03/26/04  (JH) First Version
    \Version    1.1        11/01/04  (DC) fixed issues with the allocation size of the buffer and
                                          added checking to ensure that the buffer was not blown.
                                          Also, changed declarations to use APT_INLINE macro.
    \Version    1.2        12/15/05 (LEO) Added 3D support
    \Version    1.3        08/31/07 (RB)  Removed APTSTRUCT_ALIGN16 macros to fix alignment issue.
                                          The macro prevented compilers from generating a usable copy
                                          constructor.  Alignment is now handled explicitly in code.
    \Version    1.4        10/08/07 (RB)  Fix backwards compatibility with pfnSetVertexMatrix
    \Version    1.5        10/28/08 (JP)  New function for 3d rotation
    \Version    1.6        11/05/08 (JP)  Adding information about 3d rotation to the ClipTransform
    \Version    1.7        11/11/08 (JP)  Adding the z to the ClipTransform
    \Version    1.7        03/06/09 (RB)  Changed xrotation and yrotation from int16 to float
*/
/************************************************************************************************H*/


#ifndef _AptMath_h
#define _AptMath_h

/*** Include files ********************************************************************************/

#include <string.h>     // for memset

/*** Defines **************************************************************************************/

/*** Macros ***************************************************************************************/

/*** Type Definitions *****************************************************************************/

struct AptColorHelperScale;
struct AptColorHelperTranslate;

class AptMath
{

public:
    AptMath();

    // 4x4 matrix enum
    typedef enum MatIndex_t
    {
        MAT_A, MAT_B, MAT_C,   MAT_P,
        MAT_D, MAT_E, MAT_F,   MAT_Q,
        MAT_G, MAT_H, MAT_I,   MAT_R,

        MAT_X, MAT_Y, MAT_Z,   MAT_W,

        MAT_NUMELEMENTS
    } MatIndexT;

    // 4x4 matrix type
    typedef struct Mat44_t
    {
        float m[MAT_NUMELEMENTS];
        Mat44_t()
        {
            // Opt Note: We know there's 16 floats in a Mat44 so
            // let's just explicitly list all the stores.
            m[MAT_A] = 0.0f;
            m[MAT_B] = 0.0f;
            m[MAT_C] = 0.0f;
            m[MAT_P] = 0.0f;

            m[MAT_D] = 0.0f;
            m[MAT_E] = 0.0f;
            m[MAT_F] = 0.0f;
            m[MAT_Q] = 0.0f;

            m[MAT_G] = 0.0f;
            m[MAT_H] = 0.0f;
            m[MAT_I] = 0.0f;
            m[MAT_R] = 0.0f;

            m[MAT_X] = 0.0f;
            m[MAT_Y] = 0.0f;
            m[MAT_Z] = 0.0f;
            m[MAT_W] = 0.0f;
        }

    } Mat44T;

    // 1x4 vector type
    typedef struct Vec4_t
    {
        float vx,vy,vz,vw;
    } Vec4T;

    // Clip transform: position transform, color scale, color add
    typedef struct ClipTransform_t
    {
        ClipTransform_t() :
            Pos44(),
            vColorMul4(),
            vColorAdd4()
        {
        }

        ClipTransform_t& operator=(const ClipTransform_t& other)
        {
            Pos44 = other.Pos44;
            vColorMul4 = other.vColorMul4;
            vColorAdd4 = other.vColorAdd4;
            return *this;
        }

        Mat44T          Pos44;
        AptColorHelperScale      vColorMul4;
        AptColorHelperTranslate  vColorAdd4;
#if defined(APT_3D)
        // We need to store this values out of Pos44
        float          xrotation;
        float          yrotation;
        float          zposition;
#endif
    } ClipTransformT;

    // clip transform stack
    static void ClipStackInit(uint32_t uiSize);
    static void ClipStackShutdown();
    static void ClipStackPushUnit();
    static void ClipStackMakeUnit();

#if defined (APT_3D)
    static ClipTransformT *  ClipStackPush();
    static ClipTransformT *  ClipStackPop();
    static ClipTransformT *  _ClipStackGetTop();
    static bool              ClipStackIsEmpty();
#else
    static APT_INLINE ClipTransformT * ClipStackPush();
    static APT_INLINE ClipTransformT * ClipStackPop();
    static APT_INLINE ClipTransformT * _ClipStackGetTop();
    static APT_INLINE bool ClipStackIsEmpty();
#endif // APT_3D

    //! 2D matrix multiply
    static void MatMul2d(AptMath::Mat44T *pOut, const AptMath::Mat44T *pA, const AptMath::Mat44T *pB);
    //! 2d apt matrix to standard 4x4
    static void MatConvert(AptMath::Mat44T *pAptMat44, const AptMatrix *pAptMat);

#if defined(APT_3D)
    //! 3D matrix multiply
    static void MatMul3d(AptMath::Mat44T *pOut, const AptMath::Mat44T *pA, const AptMath::Mat44T *pB);
    static void MatRotate3d(AptMath::Mat44T *pMatrix, const float xRot, const float yRot, const float z, const float zScale = 1.f);

    static void MatMakeIdentity(AptMath::Mat44T *pOut);
    static void MatRotateYAxis(AptMath::Mat44T *pOut, const float angle);
    static void MatRotateXAxis(AptMath::Mat44T *pOut, const float angle);
#endif  // #if defined(APT_3D)

    //! standard 4x4 to 2d apt matrix
    static void MatConvert(AptMatrix *pAptMat, const AptMath::Mat44T *pAptMat44);


private:

    // DC - re-factored members, saves 4 bytes and allows for easier bounds checking.
    static AptMath::ClipTransformT        *m_pStackBase;      // This pointer is aligned.
    static AptMath::ClipTransformT        *m_pAllocatedBase;
    const static uint8_t                   m_nMinimumAlignment = 0x10;   // MUST be a power of two.
    static uint16_t                        m_nStackCapacity;
    static uint16_t                        m_nStackCount;

}; // end AptMath

/*** Variables ************************************************************************************/

/*** Functions ************************************************************************************/

#endif // _AptAtomic_h
