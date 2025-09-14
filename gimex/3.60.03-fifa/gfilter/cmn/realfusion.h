// (c) 2003-2007 Electronic Arts Inc.
/*!
 * \file        realfusion.h
 * \brief       The REALFusion library public header.
 *
 * \internal
 * Version    : $Version$
 * File      : $Header$
 * Platform  : Common
 * Maintained by: $Author$
 * Last Modified: $Author$ on $DateTime$
 */
#ifndef REALFUSION_H
#define REALFUSION_H  1

#include "realargb.h"

/*!
 *  If the user doesn't want to use RealFusion cache management,
 *  NOT RECOMMENED, they can comment out the following line and
 *  rebuild the libs.
 */
//#define    USING_SHAPES
#ifdef USING_SHAPES
    #include "realshape.h"
#endif

#include <wchar.h>

/*
 *  This is done because MAX is defined both in realbase.h
 *  as well as bitpfrread.cpp.  This change avoids compiler conflicts.
 */
#ifdef MAX
    #undef MAX
#endif

/*!
* \ingroup g_fusion_api
*/
namespace RealFont
{

    /*!
     * \ingroup g_fusion_api
     * \name Public #defines for RealFusion library.
     * \brief These defines are publically available defines to be used with the RealFusion library.
     */
    //@{
    #define FF_ERR                      1    //!< RealFusion failure return code
    #define FF_SUCCESS                  0    //!< RealFusion success return code
    #define FONTFAMILY_BUF_SIZE         256  //!< Font family string buffer size
    #define CACHE_PAGE_SIZE             5120 //!< The number of cache pages
    #define MAX_FONTFAMILY_LIST_SIZE    16   //!< Maximum number of font families allowed in a given font file
    //@}

    class FusionEngine;
    class FusionFontFile;
    class FusionFont;
    struct ThirdPartyMemObject;
    struct ThirdPartyCore;
    struct ThirdPartyUtil;

    /*!
     *  \ingroup g_fusion_api
     *  \brief The PC Platform which fonts are generated for.
     *
     *  The platform ID and platform-specific encoding ID are used to
     *  specify the subtable; this means that each platform ID/platform-specific
     *  encoding ID pair may only appear once in the cmap table. Each
     *  subtable can specify a different character encoding.
     *  \sa CharacterMapLanguageGroupID
     *  \sa CharacterMapEncodingID
     */
    enum CharacterMapPlatformID
    {
        PLATFORM_APPLE_UNICODE = 0,     //!< none
        PLATFORM_MAC,                   //!< Mac computers
        PLATFORM_ISO,                   //!< ISO encoding   (Bitstream doesn't support)
        PLATFORM_WIN                    //!< Microsoft encoding (default and strongly recommended).
    };

    /*!
    *   \ingroup g_fusion_api
    *   \brief The platform ID and platform-specific encoding ID are used to
    *   specify the font file subtable;

    *   This means that each platform ID/platform-specific encoding ID pair
    *   may only appear once in the cmap table of a font. Each subtable can
    *   specify a different character encoding. The entries in these tables
    *   must be sorted first by platform ID and then by platform-specific
    *   encoding ID.
    *
    *   Based on the following TrueType and OpenType documentation:
    *   http://www.microsoft.com/typography/otspec/cmap.htm
    *   \sa CharacterMapPlatformID
    *   \sa CharacterMapLanguageGroupID
    */
    enum CharacterMapEncodingID
    {
        ENCODING_SYMBOL = 0,        //!< Used for Mac spefic fonts.
        ENCODING_UNICODE,           //!< Default encoding and STRONGLY recommended type to use.
        ENCODING_SHIFT_JIS,         //!< Shift_jis Japanese encoding
        ENCODING_BIG5,              //!< Big5 Chinese encoding
        ENCODING_PRC,               //!< PRC encoding
        ENCODING_WANSUNG,           //!< Wangsung Chinese encoding
        ENCODING_JOHAB,             //!< Johab Korean encoding
        ENCODING_UCS4 = 10          //!< Universal Character Set (UCS) 32bit/4byte encoding
    };


    /*!
     * \ingroup g_fusion_api
     * \brief language ID enumaration
     *
     * For a complete list of the langages associated with the language ID's
     * see the following webpages.
     * The platformID, encodingID, and languageID, help to uniquely identify the
     * character map tables contained with a font.  For most fonts this requires a
     * PLATFORM_WIN platform ID, ENCODING_UNICODE encoding ID, and a
     * GROUP_WESTERN_EUROPE_UNITED_STATES language group ID.  With these values most
     * of the characters within the font can be rasterized without issue.
     *
     * http://www.microsoft.com/globaldev/reference/win2k/setup/localsupport.mspx
     * http://www.science.co.il/Language-Locale-Codes.asp
     *
     *  \sa RealFont::FusionFont::FusionFont
     *  \sa CharacterMapPlatformID
     *  \sa CharacterMapEncodingID
     */
    enum CharacterMapLanguageGroupID
    {
        GROUP_CENTRAL_EUROPEAN              = 1029, //!< Central European languages (e.g.Croatian,Czech)
        GROUP_GREEK                         = 1032, //!< Greek language
        GROUP_WESTERN_EUROPE_UNITED_STATES  = 1033, //!< Western European and USA languages (e.g. Spanish, German, English)
        GROUP_HEBREW                        = 1037, //!< Hebrew language
        GROUP_HIDIC                         = 1039, //!< East Indian languages (e.g. Hindi, Tamil)
        GROUP_JAPANESE                      = 1041, //!< Japanese language
        GROUP_KOREAN                        = 1042, //!< Korean language
        GROUP_CYRILLIC                      = 1049, //!< Cyrillic languges (e.g. Russian,Bulgarian)
        GROUP_THAI                          = 1054, //!< Thai language
        GROUP_TURKISH                       = 1055, //!< Turkic language
        GROUP_BALTIC                        = 1061, //!< Baltic langugaes (e.g. Estonian )
        GROUP_ARABIC                        = 1065, //!< Arabic language
        GROUP_VIETNAMESE                    = 1066, //!< Vietnamese language
        GROUP_ARMENIAN                      = 1067, //!< Armenian language
        GROUP_GEORGIAN                      = 1079, //!< Gerorgain language
        GROUP_SIMPLIFIED_CHINESE            = 2052, //!< Simplified Chinese languages (e.g. Chinese_PRC, Chinese_Singapore)
        GROUP_TRADITIONAL_CHINESE           = 3076  //!< Traditional Chinese languages (e.g. Chinese_Taiwan, Chinese_Hong_Kong, Chinese_Macau)
    };

    /*!
     * \ingroup g_fusion_api
     * \brief Character Map Name Type
     * \sa RealFont::FusionFont::GetName
     */
    enum CharacterMapNameType
    {
        NAME_FONT_FAMILY = 1,       //!< Font Family name with weight appended (e.g. Arial Narrow).  Mulitple fonts can share the same name.
        NAME_FONT_SUBFAMILY,        //!< The Font Subfamily name distiguishes the font in a group with the same Font Family namejust style (e.g. Regular, Italic ).
        NAME_UNIQUE_ID,             //!< Unique font identifier (e.g. Monotype: Times New Roman Bold:1990)
        NAME_FULL_FONT_NAME         //!< Full font name is the combination of Font Family and Font Subfamily names. Typeface plus weight and style (e.g Arial Narrow Bold Italic )
    };

    /*!
     * \ingroup g_fusion_api
     * \brief Specifies the gray scale level to be used by FontFusion engine.
     *
     * \sa RealFont::FontOptions
     */
    enum BitmapQualityType
    {
        BITMAP_MONOCHROME   = 0,    //!< monochrome
        BITMAP_LOW_QUALITY,         //!< There is very little noticeable difference in quality and speed.
        BITMAP_MEDIUM_QUALITY,      //!< There is very little noticeable difference in quality and speed.
        BITMAP_HIGH_QUALITY,        //!< Recommended render quality value.
        BITMAP_HIGHER_QUALITY,      //!< There is very little noticeable difference in quality and speed.
        BITMAP_EXTREME_QUALITY      //!< Takes the engine the longest to render
    };

    /*!
     * \ingroup g_fusion_api
     * \brief Speciefies the effect type set or retrieve.
     *
     * All _NATIVE_ effects are applied by Font Fusion.
     * All _EFFECT_ are post processing filters not native to Font Fusion
     * \sa RealFont::FontEffects
     */
    enum EffectType
    {
        FF_EFFECT_NONE = 0,         //!< Applies none effect to glyphs.
        FF_NATIVE_RAISED,           //!< Raised effect
        FF_NATIVE_DEPRESSED,        //!< Depressed effect
        FF_NATIVE_OUTLINE,          //!< Basic Outline effect
        FF_NATIVE_DROPSHADOWL,      //!< Dropshadow to the left
        FF_NATIVE_DROPSHADOWR,      //!< Dropshadow to the right
        FF_NATIVE_ROTATE,           //!< Rotation glpyhs (NOT SUPPORTED)
        FF_EFFECT_SHARPEN,          //!< Sharpen bitmap (NOT SUPPORTED)
        FF_EFFECT_OUTLINE,          //!< Advanced outline effect
        FF_EFFECT_DROPSHADOW        //!< Advanced dropshadow effect
    };

    /*!
     * \ingroup g_fusion_api
     * \brief Speciefies the weight values.
     *
     * \sa RealFont::FontOptions
     */
    enum WeightType
    {
        WEIGHT_FFS_FORMAT = 0,      //!< Adjust the weight of Bitstream stroke based format (FFS)
        WEIGHT_NONE = (8L << 13),   //!< DO NOT apply any weight to pixels = level 1.0
        WEIGHT_1 = (9L << 13),      //!< weight level = 1.1
        WEIGHT_2 = (10L << 13),     //!< weight level = 1.2
        WEIGHT_3 = (11L << 13),     //!< weight level = 1.3
        WEIGHT_4 = (12L << 13),     //!< weight level = 1.4
        WEIGHT_5 = (13L << 13),     //!< weight level = 1.5
        WEIGHT_6 = (14L << 13),     //!< weight level = 1.6
        WEIGHT_7 = (15L << 13),     //!< weight level = 1.7
        WEIGHT_8 = (16L << 13),     //!< weight level = 1.8
        WEIGHT_9 = (17L << 13)      //!< weight level = 1.9
    };

    /*!
     * \ingroup g_fusion_api
     * \brief Specifies valid values for raw texture buffer types.
     *
     * \sa RealFont::FusionFont::RenderText
     */
    enum BufferType
    {
        BUFTYPE_INVALID = 0,
        BUFTYPE_ARGB_8888,  //!< 32 bit format

#if defined( PS2 ) || defined( XBOX) || defined( PC)
        BUFTYPE_RGB_888,    //!< 24 bit opaque format
#endif

#if defined( GC ) || defined( XBOX) || defined( PC)
        BUFTYPE_RGB_565,    //!< 16 bit opaque format (6 bits for green)
#endif

#if defined( PS2 ) || defined( XBOX) || defined( PC)
        BUFTYPE_ARGB_1555,  //!< 16 bit format with 5 bits per channel and one bit for alpha
#endif

#if defined( GC )
        BUFTYPE_ARGB_5A3,   //!< special GC 16 bit format that possesses 3 bits of alpha, and a maximum of 5 bits for other channels
#endif
#if defined (XBOX) || defined(PC)
        BUFTYPE_A_8         //!< 8 bit texture format only contain alpha information
#endif
    };

    /*!
     * \ingroup g_fusion_api
     * \brief Raw file format type.
     */
    enum FontFileType
    {
        FONTFILE_TTF = 0,   //!< True Type Format, pc standard format
        FONTFILE_TTC,       //!< True Type Collection, contains multiple ttf's (i.e. Font Families)
        FONTFILE_PFR,       //!< Portable Font Resource, Bitstream's Proprietary format, RealGraph recommended font format
        FONTFILE_FFS,       //!< Font Fusion Stroke
        FONTFILE_PFB        //!< Postscript Font Binary
    };

    /*!
     * \ingroup g_fusion_api
     * \brief A structure containing information about one kern pair
     */
    struct KernPair
    {
        unsigned short  mPreviousCharCode;  //!< previous character code
        unsigned short  mCurrentCharCode;   //!< current character code
        int             mKernValueX;        //!< horizontal kerned value
        int             mKernValueY;        //!< vertical kerned value
    };

    /*!
     * \ingroup g_fusion_api
     * \brief A structure containing information about one glyph
     */
    struct GlyphInfo
    {
        KernPair*       mKernPairs;     //!< the list of kerning pairs associated with this glyph
        unsigned char   mWidth;         //!< width in pixels
        unsigned char   mHeight;        //!< height in pixels
        signed char     mOffsetX;       //!< X offset to draw character in pixels
        signed char     mOffsetY;       //!< Y offset to draw character in pixels
        signed short    mAdvanceX;      //!< X increment in pixels
        signed short    mAdvanceY;      //!< Y increment in pixels
        unsigned short  mCharCode;      //!< the character code for this glyph
        unsigned short  mNumKern;       //!< number of kerning entries
    };

    /*!
     * \ingroup g_fusion_api
     * \brief Miscellaneous RealFusion utility functions.
     */
    class FusionEngineUtil
    {
    public:

        /*!
         * \brief converts a float to a 16.16 integer
         * \param x a floating point number
         * \return a 16.16 integer
         */
        static inline int   FloatTo16Dot16( float x)
        {
            return static_cast<int>( x * 65536.0f);
        }

        /*!
         * \brief converts 16.16 integer into a float
         * \param x a 16.16 integer
         * \return the float equivalent to x.
         */
        static inline float F16Dot16ToFloat( int x )
        {
            return ((static_cast<float>(x))*(1.0f/65536.0f));
        }

        /*!
         * \brief converts an 26.6 int to a float
         * \param x a 26.6 int.
         * \return the float equivalent to x.
         */
        static inline float F26Dot6ToFloat( int x )
        {
            return ((static_cast<float>(x))*(1.0f/64.0f));
        }

        /*!
         * \brief converts a float into a 32 bit integer
         * \param x a float.
         * \return the 32 bit integer equivalent to x.
         */
        static inline int   FloatToInt(float x)
        {
            return (static_cast<int>((static_cast<float>(x))+(((static_cast<float>(x))>=0.0f)?0.5f:-0.5f)));
        }

        /*!
         * \brief returns the maximum of two integers
         * \param a the first value to compare
         * \param b the second value to compare
         * \return the maximum of a and b
         */
        static inline int   Max( int a, int b )
        {
            return((a)>(b)?(a):(b));
        }

        /*!
         * \brief returns the maximum of two floats
         * \param a the first value to compare
         * \param b the second value to compare
         * \return the maximum of a and b
         */
        static inline float Max( float a, float b )
        {
            return((a)>(b)?(a):(b));
        }

        /*!
         * \brief returns the minimum of two integers
         * \param a the first value to compare
         * \param b the second value to compare
         * \return the minimum of a and b
         */
        static inline int   Min( int a, int b )
        {
            return((a)<(b)?(a):(b));
        }

        /*!
         * \brief returns the minimum of two floats
         * \param a the first value to compare
         * \param b the second value to compare
         * \return the minimum of a and b
         */
        static inline float Min( float a, float b )
        {
            return((a)<(b)?(a):(b));
        }

        /*!
         * \brief returns the absolute value of an integer
         * \param x the integer
         * \return the absolute value of x
         */
        static inline int   Abs( int x )
        {
            return((x)>=0?(x):-(x));
        }

        /*!
         * \brief converts a string of ascii characters to unicode.
         * \param dst the destination buffer to place the unciode characters into
         * \param src the string of ascii characters
         * \return none
         */
        static inline void CharToWchar( wchar_t* dst, const char* src )
        {
            int pos = 0;
            while (src[pos] != '\0')
            {
                dst[pos] = static_cast<wchar_t>(src[pos]);
                ++pos;
            }
            dst[pos] = L'\0';
        }
    };

    /*!
     * \ingroup g_fusion_api
     * \brief Font effects structure
     *
     * This structure describes the effects to apply when rendering text using this library.
     */
    struct FontEffects
    {
    public:

        /*!
         * \brief Default constructor
         */
        explicit FontEffects():
            mTighten(1.0f),
            mTracking(0),
            mDrpShdX(0),
            mDrpShdY(0),
            mSmallCapsDiff(0),
            mEffectThickness(0),
            mEffect(FF_EFFECT_NONE),
            mEffectColour(RealShape::CA_BLACK)
        {
        }

        /*!
         * \brief Constructor for constant structures below.
         * \sa FES_NO_EFFECT
         * \sa FES_OUTLINE
         * \sa FES_DROPSHADOW
         */
        FontEffects(EffectType effect, short effectThickness = 0, RealShape::ARGB effectColour = RealShape::CA_BLACK, short drpShX = 0, short drpShY = 0, int tracking = 0, float tighten = 1.0f)
            : mTighten(tighten),
            mTracking(tracking),
            mDrpShdX(drpShX),
            mDrpShdY(drpShY),
            mSmallCapsDiff(0),
            mEffectThickness( effectThickness),
            mEffect(effect),
            mEffectColour(effectColour)
        {
        }

        float           mTighten;           //!< multiplies the advance value of each glyph by a coefficient
        int             mTracking;          //!< translates the advance value of each glyph
        short           mDrpShdX;           //!< drop shadow x offset
        short           mDrpShdY;           //!< drop shadow y offset, up is (-) and down is (+)
        short           mSmallCapsDiff;     //!< describes size ratio of small capitals letters to regular capital letters (percent value between 0 <= value <= 400)
        short           mEffectThickness;   //!< thickness of either the drop shadow or outline
        EffectType      mEffect;            //!< effect modifier: 0 = NONE, 1 = dropShadow, 2 = outline, 3 = sharpen
        RealShape::ARGB mEffectColour;      //!< effect colour
    };

    /*!
     * \ingroup g_shape_api
     * \name Effect Constants
     * \brief FontEffects constants that can be used to create simple outline and drop shadow effects.
     */
    //@{
    const FontEffects   FES_NO_EFFECT;                                                      //!< no effect
    const FontEffects   FES_OUTLINE( FF_EFFECT_OUTLINE, 0, RealShape::CA_BLACK );           //!< outline effect
    const FontEffects   FES_DROPSHADOW( FF_EFFECT_DROPSHADOW, 0, RealShape::CA_BLACK,1,1 ); //!< drop shadow effect
    //@}

    /*!
     * \ingroup g_fusion_api
     * \brief Font options structure
     *
     * This structure describes the options for creating a fusion font from a font file.
     * \sa FusionFont
     * \sa FusionFile
     */
    struct FontOptions
    {
    public:
        explicit FontOptions()
            : mScaleX(12.0f),
            mScaleY(12.0f),
            mFFSWeight(50),
            mWeight(WEIGHT_NONE),
            mEffect( FF_EFFECT_NONE ),
            mPlatformID(PLATFORM_WIN ),
            mEncodingID(ENCODING_UNICODE ),
            mLanguageGroupID(GROUP_WESTERN_EUROPE_UNITED_STATES),
            mBitmapQuality(BITMAP_HIGH_QUALITY),
            mRotAngle(0),
            mBitmapFilter(NULL),
            mBitmapFilterParams(NULL),
            mBitmapFilterTag(0)
            {}

            float                       mScaleX;                                                //!< the horizontal scale value for fonts.  Equivalent to pointsize.
            float                       mScaleY;                                                //!< the vertical scale value for fonts.  Equivalent to pontsize.
            int                         mFFSWeight;                                             //!< weight to applied ONLY to Bitstream's stroke format (FFS)
            WeightType                  mWeight;                                                //!< weight or bolding applied to glyphs.
            EffectType                  mEffect;                                                //!< various post rasterization effects applied to glpyhs.
            CharacterMapPlatformID      mPlatformID;                                            //!< the character map platform ID
            CharacterMapEncodingID      mEncodingID;                                            //!< the character map encoding ID
            CharacterMapLanguageGroupID mLanguageGroupID;                                       //!< the character map language ID
            BitmapQualityType           mBitmapQuality;                                         //!< bitmap quality level
            int                         mRotAngle;                                              //!< rotation angle used for FF_NATIVE_ROTATE effect
            void*                       (*mBitmapFilter)( void *t2k, void *filterParamsPtr);    //!< call back filter function (NOT YET SUPPORTED)
            void*                       mBitmapFilterParams;                                    //!< NOT YET SUPPORTED
            int                         mBitmapFilterTag;                                       //!< NOT YET SUPPORTED
    };


    /*!
     * \ingroup g_shape_api
     * \name font file types
     * \brief describesd whether the font file is loaded from a memory or the disk.
     * \sa FusionFontFile
     */
    enum OwnedFileData
    {
        FILE_DATA_NOT_OWNED = 0,    //!< file data is not owned and should not be freed by FusionFontFile when it is destroyed
        FILE_DATA_OWNED             //!< file data is owned and should be freed by FusionFontFile when it is detroyed
    };

    /*!
     * \ingroup g_shape_api
     * \name Blend flags
     * \brief describes whether or not to perform alpha blending.
     */
    enum FusionBlendFlags
    {
        FUSIONBLEND_OFF = 0,    //!< no blending
        FUSIONBLEND_ALPHA       //!< alpha blending
    };

    /*!
     * \ingroup g_fusion_api
     * \brief Other useful RealFusion constants.
     *  Options used to set up Bitstream's rasterization engine.  If users
     *  are familiar with the system they can modify these values.
     */
    //@{
    const unsigned short    DEFAULT_CMD  = 0x0040 | 0x0200  | 0x0002 | 0x0010; //!<(T2K_TV_MODE_2 | T2K_SCAN_CONVERT |T2K_USE_FRAC_PEN) Bitstream specific engine initialization flags.
    const char              X_FRACTIONAL = 63; //!< Fractional X
    const char              Y_FRACTIONAL = 63; //!< Fractional Y
    //@}

    /*!
     * \ingroup g_fusion_api
     * \brief  The FontFusion class is used to represent the rasterizing engine.
     *
     * This class is essentially a collection of static functions that provide entry points into the internals of Font Fusion.
     *
     * Prior to using this library, the SetMemCallback function is to be called, followed by the Init Function.
     *
     * When finished using RealFusion the Release function should be called.
     *
     * The SetRenderCallbacks function can be used to supply custom user font rendering functions.
     */
    class FusionEngine
    {
    public:
        explicit FusionEngine() {};
        ~FusionEngine() {};

        static void Init();
        static void FlushCache();
        static void Release();
        static void SetMemCallbacks( void * (*MEMallocalign)(const char *name, int size, int alignment, int alignheadersize, int type),
                            void (*MEMfree)(void *ptr) );
        static void GetMemCallbacks( void * (**MEMallocalign)(const char *name, int size, int alignment, int alignheadersize, int type),
                            void (**MEMfree)(void *ptr) );

        static void SetRenderCallbacks(void* (*CopyRect) ( void *dst, int dstWidth, int dstHeight, int dstRowBytes, int dstX, int dstY, BufferType dstType, void* userData, RealShape::ARGB textColour, const void *src, int srcWidth, int srcHeight, int srcRowbytes, BitmapQualityType srcBitmapType ));
        static void GetRenderCallbacks(void* (**CopyRect) ( void *dst, int dstWidth, int dstHeight, int dstRowBytes, int dstX, int dstY, BufferType dstType, void* userData, RealShape::ARGB textColour, const void *src, int srcWidth, int srcHeight, int srcRowbytes, BitmapQualityType srcBitmapType ));
    };

    /*!
     * \ingroup g_fusion_api
     * \brief  This class represents RealFusion-Initialized font file.
     *
     * Before creating a fusion font instance, this class must be instantiated.
     */
    class FusionFontFile
    {
    public:
        explicit FusionFontFile( const char *path );
        FusionFontFile( const void *fontData, unsigned int size, FontFileType fileType);
        ~FusionFontFile(void);

        wchar_t**   GetFamilyNames(int* numFamilies) const;

        void*                   mFontData;                          //!< pointer to a font file in memory(e.g. TTF, PFR)
        unsigned int            mLength;                            //!< length of font file in memory
        ThirdPartyMemObject*    mThirdPartyMem;                     //!< memory object associated with ThirdParty rasterization system.
        OwnedFileData           mOwnedFileData;                     //!< Specifies whether or not to free the file data when this object gets destroyed
    };


    /*!
     * \ingroup g_fusion_api
     * \brief  This class represents RealFusion-Initialized font.
     *
     * An instance of this class is required to render text or to obtain information about the layout of glyphs.
     */
    class FusionFont
    {
    public:
        explicit FusionFont( wchar_t* fontFamilyName, const FontOptions* fontOptions );
        FusionFont( const FusionFont& srcFont );
        ~FusionFont();
        FusionFont* Clone() const;

        /*!
         * \brief obtain the ascent value for this font.
         * \return the ascent value
         */
        inline int  GetAscent()const {return mAscent;}

        /*!
         * \brief obtain the descent value for this font.
         * \return the descent value
         */
        inline int  GetDescent()const {return mDescent;}

        /*!
         * \brief set the ascent value for this font.
         * \param the ascent value
         */
        inline void SetAscent(int ascent){mAscent = ascent;}

        /*!
         * \brief set the descent value for this font.
         * \param the descent value
         */
        inline void SetDescent( int descent){mDescent = descent;}

        /*!
         * \brief get the horizontal scale value for this font.
         * The scale value is in point size.
         * \return the horizontal scale value
         */
        inline float GetScaleX() const {return mScaleX;}

        /*!
         * \brief get the vertical scale value for this font.
         * The scale value is in point size.
         * \return the vertical scale value
         */
        inline float GetScaleY() const {return mScaleY;}

        void        SetScale( float scaleX, float scaleY );

        int         GetName( CharacterMapNameType charMapName, wchar_t* name) const;

        int         SetFamilyName( const wchar_t* familyName);

        void        GetGlyphInfo( unsigned short glyphCode, GlyphInfo *glyphInfo ) const;
        KernPair*   GetKernPairs( const unsigned short* srcCodePoints, unsigned short numCodePoints, unsigned short glyphCode, unsigned short* numKernPairs) const;
        void        FillKernPairValue( KernPair* kernPair ) const;

        bool        IsLanguageGroupSupported( CharacterMapLanguageGroupID langGroup) const;
        bool        IsEncodingSupported( CharacterMapEncodingID encoding );

#ifdef USING_SHAPES
        RealShape::Shape* GetTextShape( RealShape::CreateStruct& createStruct, float* x, float *y, const FontEffects& fontEffects, const wchar_t* text );
        RealShape::Shape* GetTextShape( RealShape::CreateStruct& createStruct, float* x, float *y, const FontEffects& fontEffects, const char* text);

        RealShape::Shape* GetTextShapeF( RealShape::CreateStruct& createStruct, float* x, float *y,const FontEffects& fontEffects, const char *text, ...);
        RealShape::Shape* GetTextShapeF( RealShape::CreateStruct& createStruct, float* x, float *y,const FontEffects& fontEffects, const wchar_t *text, ...);

        RealShape::Shape* VGetTextShape( RealShape::CreateStruct& createStruct, float* x, float *y,const FontEffects& fontEffects, const char *text, va_list arglist);
        RealShape::Shape* VGetTextShape( RealShape::CreateStruct& createStruct, float* x, float *y,const FontEffects& fontEffects, const wchar_t *text, va_list arglist);
#endif

        int GetTextRect(float* x, float *y, int* width, int* height, const FontEffects& fontEffects, const wchar_t *text);
        int GetTextRect(float* x, float *y, int* width, int* height, const FontEffects& fontEffects, const char *text);

        int GetTextRectF(float* x, float *y, int* width, int* height, const FontEffects& fontEffects, const wchar_t *text, ...);
        int GetTextRectF(float* x, float *y, int* width, int* height, const FontEffects& fontEffects, const char *text, ...);

        int VGetTextRect(float* x, float *y, int* width, int* height, const FontEffects& fontEffects, const wchar_t *text, va_list arglist);
        int VGetTextRect(float* x, float *y, int* width, int* height, const FontEffects& fontEffects, const char *text, va_list arglist);

#ifdef USING_SHAPES
        int  RenderTextToShape( RealShape::Shape* dstBuf, float x, float y, RealShape::ARGB textColour, const FontEffects& fontEffects, const wchar_t *text);
        int  RenderTextToShape( RealShape::Shape* dstBuf, float x, float y, RealShape::ARGB textColour, const FontEffects& fontEffects, const char *text);

        int  RenderTextToShapeF( RealShape::Shape* dstBuf, float x, float y, RealShape::ARGB textColour, const FontEffects& fontEffects, const wchar_t* text, ...);
        int  RenderTextToShapeF( RealShape::Shape* dstBuf, float x, float y, RealShape::ARGB textColour, const FontEffects& fontEffects, const char* text, ...);

        int  VRenderTextToShape( RealShape::Shape* dstBuf, float x, float y, RealShape::ARGB textColour, const FontEffects& fontEffects, const wchar_t* text, va_list arglist);
        int  VRenderTextToShape( RealShape::Shape* dstBuf, float x, float y, RealShape::ARGB textColour, const FontEffects& fontEffects, const char* text, va_list arglist);
#endif

        int  RenderText( void* buf, BufferType type, int width, int height, int rowbytes, void* userData, float x, float y, RealShape::ARGB textColour, const FontEffects& fontEffects, const char *text);
        int  RenderText( void* buf, BufferType type, int width, int height, int rowbytes, void* userData, float x, float y, RealShape::ARGB textColour, const FontEffects& fontEffects, const wchar_t *text);

        int  RenderTextF( void* buf, BufferType type, int width, int height, int rowbytes, void* userData, float x, float y, RealShape::ARGB textColour, const FontEffects& fontEffects, const char *text, ...);
        int  RenderTextF( void* buf, BufferType type, int width, int height, int rowbytes, void* userData, float x, float y, RealShape::ARGB textColour, const FontEffects& fontEffects, const wchar_t *text, ...);

        int  VRenderText( void* buf, BufferType type, int width, int height, int rowbytes, void* userData, float x, float y, RealShape::ARGB textColour, const FontEffects& fontEffects, const char *text, va_list arglist);
        int  VRenderText( void* buf, BufferType type, int width, int height, int rowbytes, void* userData, float x, float y, RealShape::ARGB textColour, const FontEffects& fontEffects, const wchar_t *text, va_list arglist);

    protected:
        void ApplyEffect( const unsigned char* srcBuf, int srcWidth, int srcHeight, int srcRowbytes, RealShape::ARGB srcColour, unsigned char* dstBuf, int dstWidth, int dstHeight, int dstRowbytes, int dstX, int dstY, const FontEffects& effects );

#ifdef USING_SHAPES
        void ApplyEffect( RealShape::TextureElement* srcBuf, RealShape::ARGB srcColour, RealShape::Converter* converterList, int convertListLength, RealShape::TextureElement* dstBuf, int dstX, int dstY, const FontEffects& effects );
        template<class CharType> int RenderToShapeX( RealShape::Shape* dstBuf, float x, float y, RealShape::ARGB textColour, const FontEffects& fontEffects, const CharType *text);
#endif
        void CopyData( char *dstBuff, int dstWidth, int dstHeight, int dstRowBytes,int dstX, int dstY, BufferType dstType, FusionBlendFlags blendFlag, const RealShape::ARGB& textColour, const FontEffects& effects );
        void ApplyFontSettings( ThirdPartyUtil* metrics, WeightType weighting, float scaleX, float scaleY, EffectType rotate , int rotateAngle);
        int GetLine( const wchar_t* paragraph, const FontEffects& effects, float* x, float* y, int *width,int* numChars );
        void ResetCharMap();
        void ReloadFont();

        template<class CharType> int GetTextRectX(float* x, float* y, int* width, int* height, const FontEffects& fontEffects, const CharType *text);
        template<class CharType> int RenderTextX(void* buff, BufferType type, int width, int height, int rowbytes, void* userData, float x, float y, RealShape::ARGB textColour, const FontEffects& fontEffects, const CharType *text);

        unsigned short  mFamilyIndex;
        wchar_t*        mTypeface;
        unsigned short  mFontCode;
        int             mFFSWeight;         // weight of the FFS format only.
        int             mAscent;
        int             mDescent;
        int             mLineGap;
        float           mScaleX;
        float           mScaleY;
        int             mRotAngle;

        ThirdPartyCore* mCore;
        EffectType      mEffect;
        WeightType      mWeight;
        BitmapQualityType           mBitmapQuality;
        CharacterMapPlatformID      mPlatformID;
        CharacterMapEncodingID      mEncodingID;
        CharacterMapLanguageGroupID mLanguageGroupID;
    }; // end FusionFont class

} // end RealFont namespace
#endif // end IFUSION_H
