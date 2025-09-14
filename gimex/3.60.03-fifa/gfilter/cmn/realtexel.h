// (c) 2002-2007 Electronic Arts Inc.

/*!
 * \file        realtexel.h
 * \brief       The realtexel public header of the RealShape library.
 *
 * \internal
 * Version      : $Version$
 * File         : $Header$
 * Platform     : Common
 * Maintained by:
 * Last Modified: $Author$ on $DateTime$
 */

#ifndef REALTEXEL_H
#define REALTEXEL_H

#if defined(_MSC_VER)
#pragma once
#endif

#include "realgraph.h"

/*!
* \ingroup g_shape_api
*
* \brief The RealShape namespace encompasses the entire realshape and realtexel APIs.
*
* RealShape contains all of the types, defines, enumerations and classes for both the
* realshape and realtexel APIs.  These APIs reside entirely inside the RealShape namespace.
* There are no global functions or variables.
*
* RealShape uses the RealShape namespace.
*
* \sa namespace RealShape
* \sa realshape.h
*
*/
namespace RealShape
{

/*!
 * \ingroup g_shape_api
 *
 * \brief   Enumerated values for all available texel formats.
 *
 * The TexelFormat enumeration lists all of the available texel formats for a Shape.
 * These enumerated formats are platform dependent.
 * They are used primarily by internal realshape library code to
 * decode shape files.
 *
 * Realtexel provides an additional set of cross-platform shape types by
 * defining a TEX_ type for each supported TexelFormat.
 * Users should use the TEX_ defines to keep their code platform-independent.
 * \n example:
 * \n #if   defined(PC) #define TEX_RGB_565  FSH_RGB_565
 * \n #elif defined(GC) #define TEX_RGB_565  GSH_RGB_565
 * \n If a particular TexelFormat is not supported for a particular platform, no
 * #define exists for that type.
 *
 * \n For example, dxt3 textures are not supported on GameCube.
 * So, if building for GameCube, TEX_DXT3 is not defined.
 *
 * \n TexelFormat naming convention:
 * \n A = Alpha, R = Red, G = Green, B = Blue
 * \n P  = Palettized           (AKA indexed texture)
 * \n L  = Luminance            (AKA intensity texture)
 * \n UV = 2D indirect textures (AKA ST textures or EMBM bumpmap)
 * \n DXT = compressed textures in dxt (s3) format (4x4 blocks with 2 indexed colours, 2 interpolated colours).
 * \n YUV = LCbCr textures (luminance, blue chrominance, red chrominance)
 * \n CLUT indicates the format of a palette.
 * \n numerical values indicate the corresponding number of bits per channel.
 * \n For example, FSH_RGB_565 means 5 bits of red, 6 bits of blue, 5 bits of green (and 0 bits of alpha).
 * \sa class Shape
 */
enum TexelType
{
    TEX_INVALID        = 0x00, //!< invalid texel type... Useful for returning an error
    SHP_NAME           = 0x70, //!< name element type
    SHP_HOT_SPOTS      = 0x7c, //!< hot spots element type
    SHP_COMMENT        = 0x6f, //!< comment element type
    SHP_CLIP           = 0x7a, //!< clip element type
    SHP_EAGL           = 0x69, //!< eagl element type
    SHP_USER           = 0x6b, //!< user element type
    SHP_HEAD           = 0x81, //!< head element type... Used as a place holder element (i.e. an element of this type will be ignored)
    TEX_ARGB           = 0x40, //!< platform independent argb value (\sa ARGB class)

   // SSH - PS2 shape types
    SSH_P_4            = 0x01, //!< \b (PS2) 4-bit indexed
    SSH_P_8            = 0x02, //!< \b (PS2) 8-bit indexed
    SSH_ARGB_1555      = 0x03, //!< \b (PS2) 16-bit argb with 1-bit alpha
    SSH_RGB_888        = 0x04, //!< \b (PS2) 24-bit rgb
    SSH_ARGB_8888      = 0x05, //!< \b (PS2) 32-bit argb
    SSH_DXT1           = 0x60, //!< \b (PS2) dxt1 format
    SSH_DXT3           = 0x61, //!< \b (PS2) dxt3 format
    SSH_DXT5           = 0x62, //!< \b (PS2) dxt5 format
    SSH_CLUT_ARGB_1555 = 0x20, //!< \b (PS2) 16-bit argb palette format with 1-bit alpha
    SSH_CLUT_ARGB_8888 = 0x21, //!< \b (PS2) 32-bit argb palette format

    // GSH - GameCube shape types
    GSH_P_4            = 0x18, //!< \b (GC) 4-bit indexed
    GSH_P_8            = 0x19, //!< \b (GC) 8-bit indexed
    GSH_5A3            = 0x15, //!< \b (GC) 16-bit rgb5a3 format
    GSH_RGB_565        = 0x14, //!< \b (GC) 16-bit rgb format
    GSH_ARGB_8888      = 0x16, //!< \b (GC) 32-bit argb format
    GSH_DXT1           = 0x1e, //!< \b (GC) dxt1 format
    GSH_L_4            = 0x10, //!< \b (GC) 4-bit intensity
    GSH_L_8            = 0x11, //!< \b (GC) 8-bit intensity
    GSH_AL_44          = 0x12, //!< \b (GC) 8-bit alpha/intensity
    GSH_AL_88          = 0x13, //!< \b (GC) 16-bit alpha/intensity
    GSH_YUV411         = 0x1c, //!< \b (GC) yuv format (advanced, requires shader)
    GSH_CLUT_5A3       = 0x32, //!< \b (GC) 16-bit rgb5a3 palette format
    GSH_CLUT_RGB_565   = 0x31, //!< \b (GC) 16-bit rgb palette format
    GSH_CLUT_AL_88     = 0x30, //!< \b (GC) 16-bit alpha/intensity palette format
    GSH_CLUT_ARGB_8888 = 0x33, //!< \b (GC) 32-bit argb palette format (advanced, requires 2 AL_88 cluts)

    // XSH - XBOX shape types (subset of FSH)
    XSH_P_4            = 0x79, //!< \b (XBOX) 4-bit indexed
    XSH_P_8            = 0x7b, //!< \b (XBOX) 8-bit indexed
    XSH_ARGB_4444      = 0x6d, //!< \b (XBOX) 16-bit argb format
    XSH_ARGB_1555      = 0x7e, //!< \b (XBOX) 16-bit argb with 1-bit alpha
    XSH_RGB_565        = 0x78, //!< \b (XBOX) 16-bit rgb format
    XSH_RGB_888        = 0x7f, //!< \b (XBOX) 24-bit rgb format
    XSH_ARGB_8888      = 0x7d, //!< \b (XBOX) 32-bit argb format
    XSH_DXT1           = 0x60, //!< \b (XBOX) dxt1 format
    XSH_DXT3           = 0x61, //!< \b (XBOX) dxt3 format
    XSH_DXT5           = 0x62, //!< \b (XBOX) dxt5 format
    XSH_A_8            = 0x64, //!< \b (XBOX) 8-bit alpha format
    XSH_AL_88          = 0x65, //!< \b (XBOX) 16-bit alpha/intensity
    XSH_YUY2           = 0x68, //!< \b (XBOX) 32-bit yuv format (2 pixels per 32-bit word)
    XSH_YUV411         = 0x6c, //!< \b (XBOX) 12-bit yuv format (6B per 4 pixels)
    XSH_LVU_655        = 0x67, //!< \b (XBOX) 16-bit bumpmap format with luminance
    XSH_CLUT_ARGB_8888 = 0x2a, //!< \b (XBOX) 32-bit argb palette format
    XSH_L_8            = 0x11, //!< \b (PC/GC) 8-bit intensity

    // FSH - PC shape types
    FSH_P_4            = 0x79, //!< \b (PC) 4-bit indexed
    FSH_P_8            = 0x7b, //!< \b (PC) 8-bit indexed
    FSH_ARGB_4444      = 0x6d, //!< \b (PC) 16-bit argb format
    FSH_ARGB_1555      = 0x7e, //!< \b (PC) 16-bit argb with 1-bit alpha
    FSH_RGB_565        = 0x78, //!< \b (PC) 16-bit rgb format
    FSH_RGB_888        = 0x7f, //!< \b (PC) 24-bit rgb format (bgr ordering)
    FSH_ARGB_8888      = 0x7d, //!< \b (PC) 32-bit argb format
    FSH_ARGB_6666      = 0x66, //!< \b (PC) 24-bit argb format (advanced)
    FSH_ARGB_2101010   = 0x6a, //!< \b (PC) 32-bit argb format with 2-bit alpha (advanced)
    FSH_DXT1           = 0x60, //!< \b (PC) dxt1 format
    FSH_DXT3           = 0x61, //!< \b (PC) dxt3 format
    FSH_DXT5           = 0x62, //!< \b (PC) dxt5 format
    FSH_A_8            = 0x64, //!< \b (PC) 8-bit alpha format
    FSH_AL_88          = 0x65, //!< \b (PC) 16-bit alpha/intensity
    FSH_YUY2           = 0x68, //!< \b (PC) 32-bit yuv format (2 pixels per 32-bit word)
    FSH_YUV411         = 0x6c, //!< \b (PC) 12-bit YUV format (6B for 4 pixels)
    FSH_LVU_655        = 0x67, //!< \b (PC) 16-bit bumpmap format with luminance
    FSH_CLUT_ARGB_1555 = 0x2d, //!< \b (PC) 16-bit argb palette format with 1-bit alpha
    FSH_CLUT_RGB_565   = 0x29, //!< \b (PC) 16-bit rgb palette format
    FSH_CLUT_ARGB_8888 = 0x2a, //!< \b (PC) 32-bit argb palette format
    FSH_CLUT_RGB_888   = 0x2f, //!< \b (PC) 24-bit rgb palette format (bgr ordering)
    FSH_L_8            = 0x11, //!< \b (PC/GC) 8-bit intensity
    FSH_5A3            = 0x15, //!< \b (PC/GC) 16-bit rgb5a3 format advanced - for Gamecube testing purposes only
    FSH_CLUT_5A3       = 0x32  //!< \b (PC/GC) 16-bit rgb5a3 clut format advanced - for Gamecube testing purposes only
};

/*!
 * \ingroup g_shape_api
 *
 * \brief a packed 32-bit texel.
 *
 * A Texel is a typedef for a 32-bit unsigned int.  It represents the packed colour channels of a texel.
 * The interpretation of the bits is dependent on the TexelType of the Texel and on the target platform.
 * \n For example, a Texel of type FSH_ARGB_8888 represents an argb colour value with 8 bits per
 * channel, packed as 0xAARRGGBB on PC.
 * \n A Texel may use fewer than 32-bits depending on its TexelType.
 * \n For example, a Texel of type FSH_RGB_565 (a 16-bit texel format) uses only the lower 16 bits of the 32-bit range.
 *
 * \sa enum TexelType
 */
typedef unsigned int Texel;

// texel mover function

/*!
 * \brief Typedef for realshape's texel conversion functions.
 * A Mover is a function that converts a row of Texels from one TexelType to another.
 * \n realshape contains an internal list of Movers for interconversion between TexelTypes.
 * \n These Movers are used whenever possible to avoid an extra conversion to a common
 * intermediate TexelType.
 * \n Movers are used internally by the texture conversion functions of the Shape class.
 * \n Shape also provides a function for the user to retrieve one of the internal Movers for
 * use in their own conversion routines.
 *
 * 'd' is a pointer to the destination texels.
 * 's' is a pointer to the source texels.
 * 'pix' is the number of pixels to convert.
 *
 * \sa TexelType
 * \sa Shape::GetConverter
 */
typedef void (Mover)(unsigned char *d, const unsigned char *s, int pix);

/*!
 * \ingroup g_shape_api
 * \name TEX_ #defines for cross-platform TexelTypes.
 * \brief Cross-platform #defines for TexelTypes.
 *
 * The TEX_ defines are a cross-platform method of referring to a TexelType.
 * TEX_ defines are mapped to platform-specific TexelTypes depending on the
 * build target.
 * \n Users should use these in place of the FSH_, SSH_ etc. TexelTypes to preserve platform-independence.
 * \n For example:
 * \n #if defined(PS2)
 * \n #define TEX_P_8       SSH_P_8
 * \n #elif defined(GC)
 * \n #define TEX_P_8       GSH_P_8
 *
 * \sa enum TexelType
 */
//@{
#if defined(PS2)
#define TEX_P_4            SSH_P_4
#define TEX_P_8            SSH_P_8
#define TEX_ARGB_1555      SSH_ARGB_1555
#define TEX_RGB_888        SSH_RGB_888
#define TEX_ARGB_8888      SSH_ARGB_8888
#define TEX_DXT1           SSH_DXT1
#define TEX_DXT3           SSH_DXT3
#define TEX_DXT5           SSH_DXT5
#define TEX_CLUT_ARGB_1555 SSH_CLUT_ARGB_1555
#define TEX_CLUT_ARGB_8888 SSH_CLUT_ARGB_8888
#elif defined(GC) || defined(SNGC)
#define TEX_P_4            GSH_P_4
#define TEX_P_8            GSH_P_8
#define TEX_5A3            GSH_5A3
#define TEX_RGB_565        GSH_RGB_565
#define TEX_ARGB_8888      GSH_ARGB_8888
#define TEX_DXT1           GSH_DXT1
#define TEX_L_4            GSH_L_4
#define TEX_L_8            GSH_L_8
#define TEX_AL_44          GSH_AL_44
#define TEX_AL_88          GSH_AL_88
#define TEX_CLUT_5A3       GSH_CLUT_5A3
#define TEX_CLUT_RGB_565   GSH_CLUT_RGB_565
#define TEX_CLUT_AL_88     GSH_CLUT_AL_88
#define TEX_CLUT_ARGB_8888 GSH_CLUT_ARGB_8888

// MSH_ (from Real5 and earlier) types are no longer supported.
// They have been remapped to existing GC formats
#elif defined(SUN) || defined(OSX) || defined(SGI)

#define TEX_P_4            GSH_P_4
#define TEX_P_8            GSH_P_8
#define TEX_RGB_565        GSH_RGB_565
#define TEX_ARGB_8888      GSH_ARGB_8888
#define TEX_CLUT_ARGB_8888 GSH_CLUT_ARGB_8888,

#elif defined(XBOX)
#define TEX_P_4            XSH_P_4
#define TEX_P_8            XSH_P_8
#define TEX_ARGB_4444      XSH_ARGB_4444
#define TEX_ARGB_1555      XSH_ARGB_1555
#define TEX_RGB_565        XSH_RGB_565
#define TEX_RGB_888        XSH_RGB_888
#define TEX_ARGB_8888      XSH_ARGB_8888
#define TEX_DXT1           XSH_DXT1
#define TEX_DXT3           XSH_DXT3
#define TEX_DXT5           XSH_DXT5
#define TEX_A_8            XSH_A_8
#define TEX_AL_88          XSH_AL_88
#define TEX_YUY2           XSH_YUY2
#define TEX_CLUT_ARGB_8888 XSH_CLUT_ARGB_8888
#define TEX_L_8            XSH_L_8

#else // PC/Linux
#define TEX_P_4            FSH_P_4
#define TEX_P_8            FSH_P_8
#define TEX_ARGB_4444      FSH_ARGB_4444
#define TEX_ARGB_6666      FSH_ARGB_6666
#define TEX_ARGB_2101010   FSH_ARGB_2101010
#define TEX_ARGB_1555      FSH_ARGB_1555
#define TEX_RGB_565        FSH_RGB_565
#define TEX_RGB_888        FSH_RGB_888
#define TEX_ARGB_8888      FSH_ARGB_8888
#define TEX_DXT1           FSH_DXT1
#define TEX_DXT3           FSH_DXT3
#define TEX_DXT5           FSH_DXT5
#define TEX_A_8            FSH_A_8
#define TEX_AL_88          FSH_AL_88
#define TEX_YUY2           FSH_YUY2
#define TEX_CLUT_ARGB_8888 FSH_CLUT_ARGB_8888
#define TEX_CLUT_ARGB_1555 FSH_CLUT_ARGB_1555
#define TEX_CLUT_RGB_565   FSH_CLUT_RGB_565
#define TEX_CLUT_RGB_888   FSH_CLUT_RGB_888
#define TEX_5A3            FSH_5A3
#define TEX_CLUT_5A3       FSH_CLUT_5A3
#define TEX_L_8            FSH_L_8
#endif

#define TEX_ARGB           TEX_ARGB
//@}

const int TEXEL_TYPE_STR_LEN = 24;

/*!
 * \ingroup g_shape_api
 *
 * \brief   The TexelType Helper class.
 *
 * The TexelTypeHelper class is a static class that contains functions that work with the TexelType enum.
 * Among these functions are debug functions, a function to get obtain a texel mover from one type to another, and
 * functions that that query a TexelType's characteristics.
 *
 * \sa      class TexelHelper
 * \sa      enum  TexelType
 * \sa      class Shape
 */
class REALAPI TexelTypeHelper
{
public:
    static void             ToString( char* buf, const TexelType type );
    static void             Print( const TexelType type, char* whiteSpace = NULL );
    static Mover*           GetMover( const TexelType targetType, const TexelType sourceType );
    static TexelType        DepthToTextureType( int depth );
    static TexelType        DepthToClutType( int clutDepth );
    static inline bool      IsTextureType( const TexelType type );
    static inline bool      IsClutType( const TexelType type );
    static inline int       GetDepth( const TexelType type );
    static inline bool      IsSwizzleable( const TexelType type );
    static inline bool      IsCompressed( const TexelType type );
    static inline bool      IsIndexed( const TexelType type );
    static inline bool      HasAlpha( const TexelType type );
    static inline TexelType GetTextureType( const TexelType type );
    static inline TexelType GetClutType( const TexelType type );
};

/*!
 * \ingroup g_shape_api
 *
 * \brief   The cross-platform TexelHelper helper class.
 *
 * The TexelHelper class provides functions that work with a platform-independent representation of a colour.
 * TexelHelper also provides convenience functions to create texels in a number of common colours.
 *
 * \sa      class ARGB
 * \sa      class Shape
 * \sa      class ARGBHelper
 * \sa      class TexelTypeHelper
 */
class REALAPI TexelHelper
{
public:
    static void Print( const Texel texel, const TexelType type, char* whiteSpace = NULL );
};

// inlined function implementations:
#include "texeltype.inl"

} // end namespace RealShape

#endif /* REALTEXEL_H */


