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

#ifndef REALARGB_H
#define REALARGB_H

#if defined(_MSC_VER)
#pragma once
#endif

#include "realgraph.h"
#include "realtexel.h"

namespace RealShape
{

/*!
 * \ingroup g_shape_api
 *
 * \brief   The cross-platform ARGB class.
 *
 * The ARGB class provides a platform-independent representation of a 4-channel colour.
 * This class provides methods to get and set both the full argb value and
 * individual colour channels, and to convert an ARGB to a Texel.
 *
 * \sa      class Texel
 * \sa      class Shape
 */
class REALAPI ARGB
{
public:

  /*!
   * \brief Creates an ARGB from a Texel and a TexelType.
   *
   * \param texel  Texel containing the source colour value.
   * \param type   format of the texel parameter.
   * \return an initialized ARGB object.
   */
   static inline ARGB CreateARGB( const Texel texel, const TexelType type ) { return( FromTexel( texel, type ) ); }

   /*!
    * \brief Creates an ARGB from 4 colour values.
    *
    * \param alpha  8-bit (0 to 255) alpha value.
    * \param red    8-bit (0 to 255) red value.
    * \param green  8-bit (0 to 255) green value.
    * \param blue   8-bit (0 to 255) blue value.
    * \return an initialized ARGB object.
    */
    static inline ARGB CreateARGB( unsigned char alpha, unsigned char red, unsigned char green, unsigned char blue )
    {
        ARGB argb;
        #if defined(PS2)
        *reinterpret_cast<unsigned int *>(&argb) = (static_cast<unsigned int>(alpha)<<24)|(static_cast<unsigned int>(blue)<<16)|(static_cast<unsigned int>(green)<<8)|static_cast<unsigned int>(red);
        #elif defined(GC)
        *reinterpret_cast<unsigned int *>(&argb) = (static_cast<unsigned int>(red)<<24)|(static_cast<unsigned int>(green)<<16)|(static_cast<unsigned int>(blue)<<8)|(static_cast<unsigned int>(alpha)<<0);
        #else
        *reinterpret_cast<unsigned int *>(&argb) = (static_cast<unsigned int>(alpha)<<24)|(static_cast<unsigned int>(red)<<16)|(static_cast<unsigned int>(green)<<8)|static_cast<unsigned int>(blue);
        #endif
        return( argb );
    }

   /*!
    * \brief Creates an ARGB from an int.
    *
    * As the order of the colour channels within an ARGB is platform-dependent,
    * this constructor should only be used to initialize an ARGB to 0.
    * \param intargb  32-bit argb value.
    * \return an initialized ARGB object.
    */
    static inline ARGB CreateARGB(unsigned int intargb)
    {
        ARGB argb;
        *reinterpret_cast<unsigned int *>(&argb) = intargb;
        return( argb );
    }

    void Print( char* whiteSpace = NULL) const;

    // 'get' methods

   /*!
    * \brief Convert an ARGB to an unsigned int.
    *
    * \param none
    * \return the ARGB value as a 32-bit unsigned integer.  The ordering of the a,r,g,b channels within this
    * unsigned int is platform-dependent.
    */
    unsigned int AsUInt(void) const
    {
        return( *reinterpret_cast<const unsigned int *>(this) );
    }

   /*!
    * \brief Returns the alpha channel of an ARGB.
    *
    * \param none
    * \return the alpha channel as an unsigned char (0 to 255).
    */
    unsigned char GetA(void) const  { return(mA); }

   /*!
    * \brief Returns the red channel of an ARGB.
    *
    * \param none
    * \return the red channel as an unsigned char (0 to 255).
    */
    unsigned char GetR(void) const  { return(mR); }

   /*!
    * \brief Returns the green channel of an ARGB.
    *
    * \param none
    * \return the green channel as an unsigned char (0 to 255).
    */
    unsigned char GetG(void) const  { return(mG); }

   /*!
    * \brief Returns the blue channel of an ARGB.
    *
    * \param none
    * \return the blue channel as an unsigned char (0 to 255).
    */
    unsigned char GetB(void) const  { return(mB); }


   /*!
    * \brief Sets the arguments to the corresponding ARGB colour channel.
    *
    * This function is useful to retrieve a number of the ARGB colour channels at once.
    * \param alpha  address of an unsigned char that will be set to the ARGB's alpha value.  If NULL, this parameter is ignored.
    * \param red    address of an unsigned char that will be set to the ARGB's red value.  If NULL, this parameter is ignored.
    * \param green  address of an unsigned char that will be set to the ARGB's green value.  If NULL, this parameter is ignored.
    * \param blue   address of an unsigned char that will be set to the ARGB's blue value.  If NULL, this parameter is ignored.
    * \return none.
    */
    void Get(unsigned char *alpha, unsigned char *red, unsigned char *green, unsigned char *blue) const
    {
        if (alpha != NULL)  *alpha = mA;
        if (red   != NULL)  *red   = mR;
        if (green != NULL)  *green = mG;
        if (blue !=  NULL)  *blue  = mB;
    }


    // 'set' methods

   /*!
   * \brief Sets an ARGB from an unsigned int.
   *
   * caution: This function is not cross-platform as the arrangement of the colour channels
   * within an ARGB depends on the platform.  It is faster than setting individual channels,
   * but should be used only with platform-specific code where the order of the channels is known.
   *
   * \n On PC, XBOX and LINUX, (little-endian argb), order is (B0=B, B1=G, B2=R, B3=A) where B0 is the byte with the lowest address.
   * \n As an int, pack as AARRGGBB.
   *
   * \n On PS2, (little-endian, abgr), order is (B0=R, B1=G, B2=B, B3=A). where B0 is the byte with the lowest address.
   * \n As an int, pack as AABBGGRR.  Typically, 128 is the maximum value for alpha on this platform.
   *
   * \n On GC, (big-endian argb), order is (B0=A, B1=R, B2=G, B3=B) where B0 is the byte with the lowest address.
   * \nAs an int, pack as AARRGGBB.
   *
   * \param intARGB source unsigned int value used to set ARGB internal values
   * \return none
   */
    void FromUInt(unsigned int uIntARGB)
    {
        *reinterpret_cast<unsigned int *>(this) = uIntARGB;
    }

    /*!
    * \brief Sets a single colour channel of an ARGB.
    *
    * \param alpha,red,green,blue the colour value to set for the specified channel
    * \return none.
    */
    //@{
    void SetA(unsigned char alpha)  { mA = alpha; }  //!< Sets the alpha channel of an ARGB
    void SetR(unsigned char red)    { mR = red;   }  //!< Sets the red channel of an ARGB
    void SetG(unsigned char green)  { mG = green; }  //!< Sets the green channel of an ARGB
    void SetB(unsigned char blue)   { mB = blue;  }  //!< Sets the blue channel of an ARGB
    //@}

   /*!
    * \brief Set an ARGB from 4 colour channels.
    *
    * Each of the ARGB's internal colour channels is set to the corresponding argument.
    * The valid range for a colour is 0 to 255.
    * This function is useful to set a number of the ARGB colour channels at once.
    * \param alpha  unsigned char used to set the ARGB's alpha channel.
    * \param red    unsigned char used to set the ARGB's red channel.
    * \param green  unsigned char used to set the ARGB's green channel.
    * \param blue   unsigned char used to set the ARGB's blue channel.
    * \return none.
    */
    void Set(unsigned char alpha, unsigned char red, unsigned char green, unsigned char blue )
    {
        mA = alpha;
        mR = red;
        mG = green;
        mB = blue;
    }

    // operators

   /*!
   * \brief Test two ARGBs for equality.
   *
   * \param argb ARGB to compare against.
   * \return true if the ARGBs are equal; false if they are not equal.
   */
    bool operator==( ARGB argb ) const { return( AsUInt() == argb.AsUInt() ); }

  /*!
   * \brief Test two ARGBs for inequality.
   *
   * \param argb ARGB to compare against.
   * \return true if the ARGBs are not equal; false if they are equal.
   */
    bool operator!=( ARGB argb ) const { return( !(operator==(argb)) ); }


  /*!
   * \brief Multiply two ARGBs for shading.
   *
   * Rounding is performed and the result is ensured to not underflow or overflow, as this operator
   * is limited to multiplying two argb values.
   *
   * \param argb ARGB to multiply by.
   * \return ARGB values multiplied together.
   */

    const ARGB operator*( ARGB argb ) const
    {
        ARGB r;
        r.mA = static_cast<unsigned char>( (static_cast<int>(mA) * static_cast<int>(argb.mA) + 128) >> 8 );
        r.mR = static_cast<unsigned char>( (static_cast<int>(mR) * static_cast<int>(argb.mR) + 128) >> 8 );
        r.mG = static_cast<unsigned char>( (static_cast<int>(mG) * static_cast<int>(argb.mG) + 128) >> 8 );
        r.mB = static_cast<unsigned char>( (static_cast<int>(mB) * static_cast<int>(argb.mB) + 128) >> 8 );
        return r;
    }

  /*!
   * \brief Add two ARGBs for brightening
   *
   * Saturation to 255 is performed, so the values must be treated as unsigned.
   *
   * \param argb ARGB to add.
   * \return ARGB values added together.
   */

    const ARGB operator+( ARGB argb ) const
    {
        ARGB r;
        r.mA = static_cast<unsigned char>( (static_cast<int>(mA) + (static_cast<int>(argb.mA))<255)?(static_cast<int>(mA) + static_cast<int>(argb.mA)):255);
        r.mR = static_cast<unsigned char>( (static_cast<int>(mR) + (static_cast<int>(argb.mR))<255)?(static_cast<int>(mR) + static_cast<int>(argb.mR)):255);
        r.mG = static_cast<unsigned char>( (static_cast<int>(mG) + (static_cast<int>(argb.mG))<255)?(static_cast<int>(mG) + static_cast<int>(argb.mG)):255);
        r.mB = static_cast<unsigned char>( (static_cast<int>(mB) + (static_cast<int>(argb.mB))<255)?(static_cast<int>(mB) + static_cast<int>(argb.mB)):255);
        return r;
    }

    Texel GetTexel( const TexelType texelType, const unsigned char* clut = 0, int numClutColours = 0, const TexelType clutType = TEX_INVALID ) const;

    Texel GetTexelIndex( const unsigned char* clut, int numClutColours, const TexelType clutType ) const;
    Texel GetTexelIndex( const ARGB* clut, int numClutColours ) const;
    Texel GetTexelIndex( unsigned char* inverseTable ) const;

#if defined( TEX_ARGB_6666 )
    inline Texel GetTexel6666() const; //!< Return an ARGB converted to a Texel in argb 6666 format
#endif

#if defined( TEX_ARGB_1555 )
    inline Texel GetTexel1555() const; //!< Return an ARGB converted to a Texel in argb 1555 format
#endif

#if defined( TEX_RGB_565 )
    inline Texel GetTexel565() const;     //!< Return an ARGB converted to a Texel in rgb 565 format
#endif

#if defined( TEX_ARGB_4444 )
    inline Texel GetTexel4444() const;    //!< Return an ARGB converted to a Texel in argb 4444 format
#endif

#if defined( TEX_AL_44 )
    inline Texel GetTexelAL44() const;    //!< Return an ARGB converted to a Texel in al 44 format
#endif

#if defined( TEX_AL_88 )
    inline Texel GetTexelAL88() const;    //!< Return an ARGB converted to a Texel in al 88 format
#endif

#if defined( TEX_A_8 )
    inline Texel GetTexelA8() const;      //!< Return an ARGB converted to a Texel in a 8 format
#endif

#if defined( TEX_L_4 )
    inline Texel GetTexelL4() const;      //!< Return an ARGB converted to a Texel in l 4 format
#endif

#if defined( TEX_L_8 )
    inline Texel GetTexelL8() const;      //!< Return an ARGB converted to a Texel in l 8 format
#endif

#if defined( TEX_ARGB_2101010 )
    inline Texel GetTexel2101010() const; //!< Return an ARGB converted to a Texel in argb 2101010 format
#endif

#if defined( TEX_YUY2 )
    inline Texel GetTexelYUY2() const;    //!< Return an ARGB converted to the first half of a YUY2 texel
#endif

#if defined( TEX_RGB_888 )
    inline Texel GetTexel888() const;     //!< Return an ARGB converted to a texel in rgb 888 format
#endif

#if defined( TEX_ARGB_8888 )
    inline Texel GetTexel8888() const;    //!< Return an ARGB converted to a texel in argb 8888 format
#endif

#if defined( TEX_5A3 )
    inline Texel GetTexel5A3() const;     //!< Return an ARGB converted to a texel in 5A3 format
#endif

#if defined(PS2) && !defined(__NOINLINE__)

// note: removed this define and so that TEXEL_gettexel15 will be overloaded for PS2 if
// inlining is enabled (ARGB version + faster unsigned int version)

// #define TEXEL_gettexel15 TEXEL_INLINE_gettexel15
// static __inline int TEXEL_INLINE_gettexel15(unsigned int argbcol)
    static __inline int GetTexel1555(unsigned int argbcol)
    {
        register int result;
        __asm__ volatile (
            "ppac5 %0,%1;"
            "pextlw  %0,$0,%0;"
            : "=r" (result)
            : "r" (argbcol)
            );
        return(result);
    }
#endif /* PS2 */

    static ARGB FromTexel( const Texel texel, const TexelType type, const void *clut = NULL, const TexelType clutType = TEX_INVALID );
    static inline ARGB FromIndexedTexel( const Texel texel, const void *clut, const TexelType clutType );

#if defined( TEX_ARGB_1555 )
    static inline ARGB FromTexel1555( const Texel texel ); //!< assumes the Texel is in TEX_ARGB_1555 format
#endif

#if defined( TEX_RGB_565 )
    static inline ARGB FromTexel565( const Texel texel ); //!< assumes the Texel is in TEX_RGB_565 format
#endif

#if defined( TEX_ARGB_4444 )
    static inline ARGB FromTexel4444( const Texel texel ); //!< assumes the Texel is in TEX_ARGB_4444 format
#endif

#if defined( TEX_AL_44 )
    static inline ARGB FromTexelAL44( const Texel texel ); //!< assumes the Texel is in TEX_AL_44 format
#endif

#if defined( TEX_AL_88 )
    static inline ARGB FromTexelAL88( const Texel texel ); //!< assumes the Texel is in TEX_AL_88 format
#endif

#if defined( TEX_A_8 )
    static inline ARGB FromTexelA8( const Texel texel ); //!< assumes the Texel is in TEX_A_8 format
#endif

#if defined( TEX_L_4 )
    static inline ARGB FromTexelL4( const Texel texel ); //!< assumes the Texel is in TEX_L_4 format
#endif

#if defined( TEX_L_8 )
    static inline ARGB FromTexelL8( const Texel texel ); //!< assumes the Texel is in TEX_L_8 format
#endif

#if defined ( TEX_ARGB_6666 )
    static inline ARGB FromTexel6666( const Texel texel ); //!< assumes the Texel is in TEX_ARGB_6666 format
#endif

#if defined ( TEX_ARGB_2101010 )
    static inline ARGB FromTexel2101010( const Texel texel ); //!< assumes the Texel is in TEX_ARGB_2101010 format
#endif

#if defined ( TEX_RGB_888 )
    static inline ARGB FromTexel888( const Texel texel ); //!< assumes the Texel is in TEX_RGB_888 format
#endif

#if defined ( TEX_ARGB_8888 )
    static inline ARGB FromTexel8888( const Texel texel ); //!< assumes the Texel is in TEX_ARGB_8888 format
#endif

#if defined ( TEX_YUY2 )
    static inline ARGB FromTexelYUY2( const Texel texel ); //!< assumes the Texel is in TEX_YUY2 format.  The argb value of the first texel in the YUV pair is returned.
#endif

#if defined(TEX_5A3)
    static inline ARGB FromTexel5A3( const Texel texel ); //!< assumes the Texel is in TEX_5A3 format
#endif

private:

// member variables:
// these are colour channels defined in platform-dependent order for casting to int
#if defined(_MSC_VER) || defined(__i386__) // PC, XBOX, LINUX
    unsigned char mB,mG,mR,mA;
#elif defined(PS2)                     // PS2
    unsigned char mR,mG,mB,mA;
#elif defined(GC)                          // GameCube
    //unsigned char mA,mR,mG,mB;
    unsigned char mR,mG,mB,mA;
#else                                      // OSX, Solaris
    unsigned char mA,mR,mG,mB;
#endif
};

/*!
 * \ingroup g_shape_api
 *
 * \brief   The cross-platform ARGBObject class.
 *
 * The ARGBObject class is a singly-instanced class that manages a global palette
 * and helps perform colour reduction.
 *
 * This class contains only static member functions.
 * The user should not instance this class;
 * instead, member functions are called by referencing the class name, e.g. ARGBObject::FunctionName();
 *
 * \sa      class GraphObject
 * \sa      class Shape
 * \sa      class ARGB
 */
class REALAPI ARGBObject
{
public:
};

/*!
* \ingroup g_shape_api
*
* \name Common ARGB colours
*
* \brief Constant RealShape::ARGB representations of commonly used colours.
* These are provided as a convenience to the user.
*/
//@{
static const ARGB CA_BLACK        = ARGB::CreateARGB(0xff,0x00,0x00,0x00);
static const ARGB CA_BLUE         = ARGB::CreateARGB(0xff,0x00,0x00,0xaa);
static const ARGB CA_GREEN        = ARGB::CreateARGB(0xff,0x00,0xaa,0x00);
static const ARGB CA_CYAN         = ARGB::CreateARGB(0xff,0x00,0xaa,0xaa);
static const ARGB CA_RED          = ARGB::CreateARGB(0xff,0xaa,0x00,0x00);
static const ARGB CA_MAGENTA      = ARGB::CreateARGB(0xff,0xaa,0x00,0xaa);
static const ARGB CA_BROWN        = ARGB::CreateARGB(0xff,0xaa,0x50,0x00);
static const ARGB CA_LIGHTGREY    = ARGB::CreateARGB(0xff,0xaa,0xaa,0xaa);
static const ARGB CA_DARKGREY     = ARGB::CreateARGB(0xff,0x50,0x50,0x50);
static const ARGB CA_LIGHTBLUE    = ARGB::CreateARGB(0xff,0x50,0x50,0xff);
static const ARGB CA_LIGHTGREEN   = ARGB::CreateARGB(0xff,0x50,0xff,0x50);
static const ARGB CA_LIGHTCYAN    = ARGB::CreateARGB(0xff,0x50,0xff,0xff);
static const ARGB CA_LIGHTRED     = ARGB::CreateARGB(0xff,0xff,0x50,0x50);
static const ARGB CA_LIGHTMAGENTA = ARGB::CreateARGB(0xff,0xff,0x50,0xff);
static const ARGB CA_YELLOW       = ARGB::CreateARGB(0xff,0xff,0xff,0x50);
static const ARGB CA_WHITE        = ARGB::CreateARGB(0xff,0xff,0xff,0xff);
static const ARGB CA_TRANSPARENT  = ARGB::CreateARGB(0x00,0x00,0x00,0x00);
//@}

/*!
 * \ingroup g_shape_api
 *
 * \brief   The cross-platform ARGB helper class.
 *
 * The ARGBHelper class provides functions that work with a platform-independent representation of a colour.
 * ARGBHelper also provides convenience functions to create texels in a number of common colours.
 *
 * \sa      class ARGB
 * \sa      class Shape
 * \sa      class TexelHelper
 * \sa      class TexelTypeHelper
 */
class REALAPI ARGBHelper
{
public:
    static Texel Black(const TexelType type)        { return( CA_BLACK.GetTexel( type ) ); }        //!< return a new Texel from ARGB(0xff,0x00,0x00,0x00) packed in the specified TexelType format.
    static Texel Blue(const TexelType type)         { return( CA_BLUE.GetTexel( type ) ); }         //!< return a new Texel from ARGB(0xff,0x00,0x00,0xaa) packed in the specified TexelType format.
    static Texel Green(const TexelType type)        { return( CA_GREEN.GetTexel( type ) ); }        //!< return a new Texel from ARGB(0xff,0x00,0xaa,0x00) packed in the specified TexelType format.
    static Texel Cyan(const TexelType type)         { return( CA_CYAN.GetTexel( type ) ); }         //!< return a new Texel from ARGB(0xff,0x00,0xaa,0xaa) packed in the specified TexelType format.
    static Texel Red(const TexelType type)          { return( CA_RED.GetTexel( type ) ); }          //!< return a new Texel from ARGB(0xff,0xaa,0x00,0x00) packed in the specified TexelType format.
    static Texel Magenta(const TexelType type)      { return( CA_MAGENTA.GetTexel( type ) ); }      //!< return a new Texel from ARGB(0xff,0xaa,0x00,0xaa) packed in the specified TexelType format.
    static Texel Brown(const TexelType type)        { return( CA_BROWN.GetTexel( type ) ); }        //!< return a new Texel from ARGB(0xff,0xaa,0x50,0x00) packed in the specified TexelType format.
    static Texel LightGrey(const TexelType type)    { return( CA_LIGHTGREY.GetTexel( type ) ); }    //!< return a new Texel from ARGB(0xff,0xaa,0xaa,0xaa) packed in the specified TexelType format.
    static Texel DarkGrey(const TexelType type)     { return( CA_DARKGREY.GetTexel( type ) ); }     //!< return a new Texel from ARGB(0xff,0x50,0x50,0x50) packed in the specified TexelType format.
    static Texel LightBlue(const TexelType type)    { return( CA_LIGHTBLUE.GetTexel( type ) ); }    //!< return a new Texel from ARGB(0xff,0x50,0x50,0xff) packed in the specified TexelType format.
    static Texel LightGreen(const TexelType type)   { return( CA_LIGHTGREEN.GetTexel( type ) ); }   //!< return a new Texel from ARGB(0xff,0x50,0xff,0x50) packed in the specified TexelType format.
    static Texel LightCyan(const TexelType type)    { return( CA_LIGHTCYAN.GetTexel( type ) ); }    //!< return a new Texel from ARGB(0xff,0x50,0xff,0xff) packed in the specified TexelType format.
    static Texel LightRed(const TexelType type)     { return( CA_LIGHTRED.GetTexel( type ) ); }     //!< return a new Texel from ARGB(0xff,0xff,0x50,0x50) packed in the specified TexelType format.
    static Texel LightMagenta(const TexelType type) { return( CA_LIGHTMAGENTA.GetTexel( type ) ); } //!< return a new Texel from ARGB(0xff,0xff,0x50,0xff) packed in the specified TexelType format.
    static Texel Yellow(const TexelType type)       { return( CA_YELLOW.GetTexel( type ) ); }       //!< return a new Texel from ARGB(0xff,0xff,0xff,0x50) packed in the specified TexelType format.
    static Texel White(const TexelType type)        { return( CA_WHITE.GetTexel( type ) ); }        //!< return a new Texel from ARGB(0xff,0xff,0xff,0xff) packed in the specified TexelType format.
    static Texel Transparent( const TexelType type ){ return( CA_TRANSPARENT.GetTexel( type ) ); }//!< return a new Texel from ARGB(0x00,0x00,0x00,0x00) packed in the specified TexelType format.

    static void SetARGBClut( const unsigned char *srcClut, const TexelType clutType, int numClutColours );
    static void SetTexelClut( const ARGB* const argbClut, const TexelType clutType, int numClutColours );

    static inline void AlphaBlend( ARGB *dstArgb, const ARGB srcArgb );
    static inline void Shade( ARGB *argb, const ARGB shadeArgb );
    static inline void ShadeAlphaBlend( ARGB *dstArgb, const ARGB srcArgb, const ARGB shadeArgb );
};

// inlined function implementations:
#include "argb.inl"

} // end namespace RealShape

#endif /* REALTEXEL_H */


