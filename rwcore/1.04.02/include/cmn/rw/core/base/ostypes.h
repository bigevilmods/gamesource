// (c) Electronic Arts. All Rights Reserved.
#ifndef RW_OSTYPES_H
#define RW_OSTYPES_H

/**
\file
RenderWare base types and macros.  Some of these are legacy and hence are
not documented.  Where EABase and these types and macros overlap, prefer
to use the EABase versions.
*/
#include <EABase/eabase.h>

// backward-compatibility.  Prefer the EABase versions in new
// code; refactor legacy code to use the EABase defines.
#if !defined(rwCOMPILER_IS_ANSIC) && defined(EA_COMPILER_IS_ANSIC)
#define rwCOMPILER_IS_ANSIC EA_COMPILER_IS_ANSIC
#endif

#if !defined(rwCOMPILER_IS_C99) && defined(EA_COMPILER_IS_C99)
#define rwCOMPILER_IS_C99 EA_COMPILER_IS_C99
#endif

#if !defined(rwCOMPILER_HAS_C99_TYPES) && defined(EA_COMPILER_HAS_C99_TYPES)
#define rwCOMPILER_HAS_C99_TYPES EA_COMPILER_HAS_C99_TYPES
#endif

#if !defined(rwWCHAR_SIZE) && defined(EA_WCHAR_SIZE)
#define rwWCHAR_SIZE EA_WCHAR_SIZE
#endif

#if !defined(rwCOMPILER_HAS_INTTYPES) && defined(EA_COMPILER_HAS_INTTYPES)
#define rwCOMPILER_HAS_INTTYPES EA_COMPILER_HAS_INTTYPES
#endif

#ifndef EA_DEBUG
/* Use RWSUPPRESSINLINE to prevent RWFORCEINLINE from being defined */
#ifndef RWSUPPRESSINLINE
/* RWFORCEINLINE allow functions to be replaced with macros */
#ifndef RWFORCEINLINE
#define RWFORCEINLINE
#endif /* !RWFORCEINLINE */
#endif /* !RWSUPPRESSINLINE */
#endif /* !EA_DEBUG */

/* RWNORETURN should be added to functions which do not return (generally those which always throw
   an exception or call exit(), abort() etc. */
#ifndef RWNORETURN
#if defined(__GNUC__)
#define RWNORETURN __attribute__((noreturn))
#else
#define RWNORETURN
#endif /* !__GNUC__ */
#endif /* !RWNORETURN */

#define rwNAENDIAN (0)
#define rwLIENDIAN (1)
#define rwBIENDIAN (2)

#ifndef RWUNUSEDATTRIBUTE
#ifdef __GNUC__
#define RWUNUSEDATTRIBUTE __attribute__ ((__unused__))
#else /* __GNUC__ */
#define RWUNUSEDATTRIBUTE
#endif /* __GNUC__ */
#endif

#include "rw/core/base/targtype.h"

// These macros need to be defined here as the hosttype detail header file requires them as well.
#define rwFUNCTION()
#define rwRETURN(x) return x
#define rwRETURNVOID() return

#include "rw/core/base/detail/hosttype.h"

/* Figure out what the type of RwChar should be */

#if (rwCHAR_IS_SIGNED)

#if (rwSIZEOF_CHAR == 8)
typedef char RwChar;
#elif (rwSIZEOF_CHAR == 16)
typedef int16_t RwChar;
#elif (rwSIZEOF_CHAR == 32)
typedef int32_t RwChar;
#else
#error Illegal size for RwChar
#endif

#else /* rwCHAR_IS_SIGNED */

#if (rwSIZEOF_CHAR == 8)
typedef char RwChar;
#elif (rwSIZEOF_CHAR == 16)
typedef uint16_t RwChar;
#elif (rwSIZEOF_CHAR == 32)
typedef uint32_t RwChar;
#else
#error Illegal size for RwChar
#endif

#endif /* rwCHAR_IS_SIGNED */

typedef float   float32_t;
typedef double  float64_t;
typedef int32_t bool32_t;

/**
\namespace rw
Contains core macros, types, templates and classes that are used in both
conditioning and runtime code.  There are many undocumented types in this
namespace; these are generally only used by packages that are derived from
the former RenderWare Platform SDK.
*/
namespace rw
{

/* RW_W64 disables the 64 bit safety warning checking when used along a variable declaration*/
#ifndef RW_W64
#define RW_W64
#endif

#if (rwTARGET_POINTERSIZE == 4)

#ifdef rwHOST_WIN
    typedef uint32_t RW_W64 TargetPtr; //RW_W64 is to enable 64 bit type safety checking at compile time
    typedef int32_t RW_W64 TargetOffset;
#else
    typedef uint32_t TargetPtr;
    typedef int32_t  TargetOffset;
#endif

#elif (rwTARGET_POINTERSIZE == 8)
    typedef uint64_t TargetPtr;
    typedef int64_t TargetOffset;
#else
#error Illegal size for rw::TargetPtr
#endif
}
// TODO: remove this once all usages of RwTargetPtr are
//       replace with rw::TargetPtr
typedef rw::TargetPtr RwTargetPtr;
typedef rw::TargetOffset RwTargetOffset;

// printf format specifier is different between MSVC and other compilers
#if defined(_MSC_VER)
#define RWSIZETFORMATSPECIFIER "%Iu"
#else // !defined(_MSC_VER)
#define RWSIZETFORMATSPECIFIER "%zu"
#endif // !defined(_MSC_VER)

namespace rw
{
#if (rwTARGET_GLOBALPOINTERSIZE == 4)
typedef uint32_t GlobalAddress;
#elif (rwTARGET_GLOBALPOINTERSIZE == 8)
typedef uint64_t GlobalAddress;
#endif
}

// printf format specifier is different between MSVC and other compilers
#if defined(_MSC_VER)
#define RWSIZETFORMATSPECIFIER "%Iu"
#else // !defined(_MSC_VER)
#define RWSIZETFORMATSPECIFIER "%zu"
#endif // !defined(_MSC_VER)


/* RWCHECKIGNORE */
#ifndef FALSE
#define FALSE   0
#endif

#ifndef TRUE
#define TRUE    (!FALSE)
#endif

#ifndef NULL
#define NULL    0
#endif
/* RWCHECKIGNOREEND */

#define RWUNUSED(PARAM) /* PARAM */

#ifndef RWUNUSED_ARRAY
#ifdef __GNUC__
#define RWUNUSED_ARRAY(PARAM,BRACKETS) PARAM BRACKETS RWUNUSEDATTRIBUTE
#else /* __GNUC__ */
#define RWUNUSED_ARRAY(PARAM,BRACKETS) PARAM BRACKETS
#endif /* __GNUC__ */
#endif /* !RWUNUSED */

#ifndef RWUNUSED_RELEASE
#if !defined(EA_DEBUG)
#define RWUNUSED_RELEASE(PARAM) RWUNUSED(PARAM)
#else /* !defined(EA_DEBUG) */
#define RWUNUSED_RELEASE(PARAM) PARAM
#endif /* !defined(EA_DEBUG) */
#endif /* !RWUNUSED_RELEASE */

#ifndef RWUNUSED_NATIVE
#if !defined(RWCROSS)
#define RWUNUSED_NATIVE(PARAM) RWUNUSED(PARAM)
#else /* !defined(RWCROSS) */
#define RWUNUSED_NATIVE(PARAM) PARAM
#endif /* !defined(RWCROSS) */
#endif /* !RWUNUSED_NATIVE */

#ifndef RWUNUSED_NATIVE_RELEASE
#if !defined(RWCROSS) && !defined(EA_DEBUG)
#define RWUNUSED_NATIVE_RELEASE(PARAM) RWUNUSED(PARAM)
#else /* !defined(RWCROSS) && !defined(EA_DEBUG) */
#define RWUNUSED_NATIVE_RELEASE(PARAM) PARAM
#endif /* !defined(RWCROSS) && !defined(EA_DEBUG) */
#endif /* !RWUNUSED_CROSS */

#ifndef RWUNUSED_CROSS
#if defined(RWCROSS)
#define RWUNUSED_CROSS(PARAM) RWUNUSED(PARAM)
#else /* defined(RWCROSS) */
#define RWUNUSED_CROSS(PARAM) PARAM
#endif /* defined(RWCROSS) */
#endif /* !RWUNUSED_CROSS */

#ifndef RWUNUSED_CROSS_RELEASE
#if defined(RWCROSS) && !defined(EA_DEBUG)
#define RWUNUSED_CROSS_RELEASE(PARAM) RWUNUSED(PARAM)
#else /* defined(RWCROSS) && !defined(EA_DEBUG) */
#define RWUNUSED_CROSS_RELEASE(PARAM) PARAM
#endif /* defined(RWCROSS) && !defined(EA_DEBUG) */
#endif /* !RWUNUSED_CROSS_RELEASE */

#ifndef RWFORMAT
#ifdef __GNUC__
#define RWFORMAT(T, S, F) __attribute__ ((__format__ (T, S, F)))
#else /* __GNUC__ */
#define RWFORMAT(T, S, F)
#endif /* __GNUC__ */
#endif /* !RWFORMAT */

#ifndef RWALIGN
#ifdef __GNUC__
#define RWALIGN(O, A) O __attribute__ ((__aligned__ (A)))
#elif defined _MSC_VER
#define RWALIGN(O, A) __declspec(align(A)) O
#else
#define RWALIGN(O, A) O
#endif /* __GNUC__ */
#endif /* !RWALIGN */

#ifndef RWCLASSPREALIGN
#ifdef __GNUC__
#define RWCLASSPREALIGN(A)
#elif defined _MSC_VER
#define RWCLASSPREALIGN(A) __declspec(align(A))
#else /* __GNUC__ */
#define RWCLASSPREALIGN(A)
#endif /* __GNUC__ */
#endif /* !RWCLASSPREALIGN */

#ifndef RWCLASSPOSTALIGN
#ifdef __GNUC__
#define RWCLASSPOSTALIGN(A) __attribute__ ((__aligned__ (A)))
#elif defined _MSC_VER
#define RWCLASSPOSTALIGN(A)
#else /* __GNUC__ */
#define RWCLASSPOSTALIGN(A)
#endif /* __GNUC__ */
#endif /* !RWCLASSPOSTALIGN */

#ifndef RWFASTCALL
#define RWFASTCALL
#endif /* !RWFASTCALL */

/* Define some useful pad initialisation macros */
#ifndef RWPAD1
#define RWPAD1 {0}
#endif /* RWPAD1 */
#ifndef RWPAD2
#define RWPAD2 {0,0}
#endif /* RWPAD2 */
#ifndef RWPAD3
#define RWPAD3 {0,0,0}
#endif /* RWPAD3 */
#ifndef RWPAD4
#define RWPAD4 {0,0,0,0}
#endif /* RWPAD4 */
#ifndef RWPAD5
#define RWPAD5 {0,0,0,0,0}
#endif /* RWPAD5 */
#ifndef RWPAD6
#define RWPAD6 {0,0,0,0,0,0}
#endif /* RWPAD6 */
#ifndef RWPAD7
#define RWPAD7 {0,0,0,0,0,0,0}
#endif /* RWPAD7 */

#ifndef RWFORCEENUMSIZEINT
#define RWFORCEENUMSIZEINT ((int32_t)((~((uint32_t)0))>>1))
#endif /* !RWFORCEENUMSIZEINT */

/* To include a single copy of a global variable only if ever used. (needed for .Net) */
#if defined(_MSC_VER) && _MSC_VER >= 1200
#define rwSELECTIFANY __declspec(selectany)
#else
#define rwSELECTIFANY
#endif

#if (defined(RWDLL))
#if (!defined(RWAPIEXPORT) || (!defined(RWAPIIMPORT)))
#error API export/import not defined for this host/compiler configuration
#endif /* (!defined(RWAPI)) */
#endif /* (defined(RWDLL)) */

#if (defined(RWDLL))
#undef RWAPI
#define RWAPI RWAPIEXPORT
#else
#define RWAPI
#endif

/* To indicate that a symbol is not aliased in the current scope */
#ifndef RWRESTRICT
#define RWRESTRICT
#endif


/**
Calculates the number of entries in an array. The array must not have been
dynamically allocated.
\param ARRAY The array to find the number of entries.
*/
#ifndef RwArraySize
#define RwArraySize(ARRAY) (sizeof(ARRAY) / sizeof((ARRAY)[0]))
#endif /* RwArraySize */

/**
Rounds an integer number up to be a multiple of another integer number.
The round value must be a power of two.

\param N The number to round up.
\param R The value to round N to.
*/
#ifndef RwRoundUp
#define RwRoundUp(N, R) (((N) + ((R) - 1)) & ~((R) - 1))
#endif /* RwRoundUp */

/**
Rounds an integer number down to be a multiple of another integer number.
The round value must be a power of two.

\param N The number to round down.
\param R The value to round N to.
*/
#ifndef RwRoundDown
#define RwRoundDown(N, R) ((N) & ~((R) - 1))
#endif /* RwRoundDown */

/**
Aligns an address to a given alignment. The alignment must be a power of two.

\param M The address to align.
\param align The alignment. This must be a non-negative power of 2. (This is not enforced).
*/
#ifndef RwMemAlign
#define RwMemAlign(M, A)    \
    ((void *)(((rw::HostPtr)(M) + ((rw::HostPtr)(A) - 1)) & ~((rw::HostPtr)(A) - 1)))
#endif

namespace rw
{
/**
Checks that a pointer is aligned a given alignment.
\param ptr The address to check.
\param align The alignment.  This must be a non-negative power of 2. (This is not enforced).
*/
inline RwBool IsMemAligned(const void* ptr, uint32_t align)
{
    if (align <= 1)
    {
        return TRUE;
    }

    const rw::HostPtr alignMask = align - 1;
    return (RwBool)((reinterpret_cast<rw::HostPtr>(ptr) & alignMask) == 0);
}

/**
Aligns a size to a given alignment.

\param size The size to align.
\param align The alignment.  This must be a non-negative power of 2. (This is not enforced).
*/
inline uint32_t SizeAlign(uint32_t RW_W64 size, uint32_t align)
{
    if (align <= 1)
    {
        return static_cast<uint32_t>(size);
    }

    const uint32_t alignMask = align - 1;
    return static_cast<uint32_t>( (size + alignMask) & ~alignMask);
}

/**
Aligns a size to a given alignment by rounding down.

\param ptr The address to align.
\param align The alignment. This must be a non-negative power of 2 - not enforced.
*/
inline uint32_t SizeAlignDown(uint32_t RW_W64  size, uint32_t align)
{
    // Need to enforce: 0 != align && 0 == (align & (align - 1))
    return  static_cast<uint32_t>((0 - align) & size);
}

/**
Aligns an address to a given alignment.

\param ptr The address to align.
\param align The alignment.  This must be a non-negative power of 2. (This is not enforced).
*/
template <typename T>
inline T* MemAlign(T* ptr, uint32_t align)
{
    if (align <= 1)
    {
        return ptr;
    }

    const rw::HostPtr alignMask = align - 1;
    return reinterpret_cast<T *>((reinterpret_cast<rw::HostPtr>(ptr) + alignMask) & ~alignMask);
}

/**
Aligns an address to a given alignment by rounding down.

\param ptr The address to align.
\param align The alignment. This must be a non-negative power of 2 - not enforced.
*/
template <typename T>
inline T* MemAlignDown(T* ptr, uint32_t align)
{
    // Need to enforce: 0 != align && 0 == (align & (align - 1))
    return reinterpret_cast<T *>((0 - align) & reinterpret_cast<rw::HostPtr>(ptr));
}

/**
Add an offset (in bytes) to a pointer.
\param ptr the pointer to which the offset is added.
\param offset the offset in bytes.
\returns a pointer whose address is ptr + offset
*/
template <typename ValueType>
inline ValueType* MemAddOffset(ValueType* ptr, rw::HostPtr offset)
{
    return reinterpret_cast<ValueType*>(reinterpret_cast<rw::HostPtr>(ptr) + offset);
}

/**
Subtract an offset (in bytes from a pointer).
\param ptr the pointer.
\param offset the offset in bytes.
\return a pointer whose address is ptr - offset
*/
template <typename ValueType>
inline ValueType* MemSubtractOffset(ValueType* ptr, rw::HostPtr offset)
{
    return reinterpret_cast<ValueType*>(reinterpret_cast<rw::HostPtr>(ptr) - offset);
}

/**
Calculate the offset (in bytes) between two pointers.
\param first the pointer. Must be greater than second (not enforced).
\param second the pointer.
\return offset the offset (first - second) in bytes.
*/
inline rw::HostPtr MemCalculateOffset(const void* first, const void* second)
{
    return reinterpret_cast<rw::HostPtr>(first) - reinterpret_cast<rw::HostPtr>(second);
}

/**
Add an offset pointer to a base pointer.
\param ptr the pointer to which the offset is added.
\param offset the offset in bytes.
\returns a pointer whose address is ptr + offset
*/
template <typename ValueType, typename OtherValueType>
inline ValueType* MemAddBasePtr(ValueType* ptr, OtherValueType* base)
{
    return reinterpret_cast<ValueType*>(reinterpret_cast<rw::HostPtr>(ptr) + reinterpret_cast<rw::HostPtr>(base));
}

/**
Give the offset pointer of a pointer from a base pointer.
\param ptr the pointer. Must be > ptr (not enforced).
\param base the base pointer to subtract from ptr.
\return ptr - base (in bytes)
*/
template <typename ValueType, typename OtherValueType>
inline ValueType* MemSubtractBasePtr(ValueType* ptr, OtherValueType* base)
{
    return reinterpret_cast<ValueType*>(reinterpret_cast<rw::HostPtr>(ptr) - reinterpret_cast<rw::HostPtr>(base));
}

}


// include endian swap macros and functions
#include "rw/core/base/endianswap.h"


// include target pointer objects and functions
#if (rwHOST_POINTERSIZE == rwTARGET_POINTERSIZE)
  #define rwUSERWPTRNATIVE 1
#elif (rwHOST_POINTERSIZE == 4) && (rwTARGET_POINTERSIZE == 8)
  #define rwUSERWPTRCROSS32TO64 1
#elif (rwHOST_POINTERSIZE == 8) && (rwTARGET_POINTERSIZE == 4)
  #define rwUSERWPTRCROSS64TO32 1
#else
  #error Unhandled RwPtr type.
#endif
#include "rw/core/base/rwptr/ptrnative.h"
#include "rw/core/base/rwptr/ptrcross32to64.h"
#include "rw/core/base/rwptr/ptrcross64to32.h"


namespace rw
{

#if !defined(rwHOST_PS3_SPU)
// Serializes types in endian-correct byte order
template <typename T>
class LargeTypeSerializer
{
public:
    LargeTypeSerializer(T& t)
       : m_t(&t)
    {
    }

    // Gets the i'th byte in the type, least significant bytes first
    // \param i The index of the byte to get
    uint8_t &
    operator[](size_t i)
    {
        rwFUNCTION();
#if rwHOST_ENDIAN == rwBIENDIAN
        rwRETURN(((uint8_t *)(m_t))[Sizeof()-i-1]);
#else
        rwRETURN(((uint8_t *)(m_t))[i]);
#endif
    }

    // Gets the i'th byte in the type, least significant bytes first
    // \param i The index of the byte to get
    const uint8_t&
    operator[](size_t i) const
    {
        rwFUNCTION();
#if rwHOST_ENDIAN == rwBIENDIAN
        rwRETURN(((const uint8_t *)(m_t))[Sizeof()-i-1]);
#else
        rwRETURN(((const uint8_t *)(m_t))[i]);
#endif
    }

    // Gets the size of the serializable bytes in the type
    static size_t Sizeof()
    {
        rwFUNCTION();
        rwRETURN(sizeof(T));
    }

private:
    T *m_t;
};

// Helper function to create a LargeTypeSerializer for a given object
// For example, LargeTypeSerialize<uint128_t>(myBigInt)[3] will return
// the third-least-significant byte of myBigInt.
// \param largeType The object to serialize
template <typename T>
LargeTypeSerializer<T> SerializeLargeType(T& largeType)
{
    rwFUNCTION();
    rwRETURN(LargeTypeSerializer<T>(largeType));
}

#endif // #if !defined(rwHOST_PS3_SPU)
} // end namespace rw


// A collection of base types for now. These are just here so that some
// imported code will compile. They really need to be moved
// As a device hack, I look for <TypeTag>Macro to allow overloading.
#ifndef RwRect32TagMacro
struct RwRect32Tag
{
    float32_t x;
    float32_t y;
    float32_t w;
    float32_t h;
};
#define RwRect32TagMacro struct RwRect32Tag
#endif /* !RwRect32TagMacro */
typedef RwRect32TagMacro RwRect32;

#ifndef RwRGBTagMacro
struct RwRGBTag
{
    uint8_t red;
    uint8_t green;
    uint8_t blue;
};
#define RwRGBTagMacro struct RwRGBTag
#endif /* !RwRGBTagMacro */
typedef RwRGBTagMacro RwRGB;

#ifndef RwRGBATagMacro
struct RwRGBATag
{
    uint8_t red;
    uint8_t green;
    uint8_t blue;
    uint8_t alpha;
};
#define RwRGBATagMacro struct RwRGBATag
#endif /* !RwRGBATagMacro */
typedef RwRGBATagMacro RwRGBA;

#ifndef RwRGBAClassMacro
namespace rw
{
class RGBA
{
 public:

    // allow uninitialized construction
    inline
    RGBA()
    {
        // empty
    }

    inline explicit
    RGBA(RwRGBA rgba)
    {
#if (rwHOST_ENDIAN == rwBIENDIAN)
        m_rgba  = (uint32_t)rgba.red   << 0;
        m_rgba |= (uint32_t)rgba.green << 8;
        m_rgba |= (uint32_t)rgba.blue  << 16;
        m_rgba |= (uint32_t)rgba.alpha << 24;
#else
        *(reinterpret_cast<RwRGBA *>(&m_rgba)) = rgba;
#endif
    }

    inline
    RGBA(uint8_t r, uint8_t g, uint8_t b, uint8_t a=0xff)
    {
        m_rgba  = (uint32_t)r << 0;
        m_rgba |= (uint32_t)g << 8;
        m_rgba |= (uint32_t)b << 16;
        m_rgba |= (uint32_t)a << 24;
    }

    inline
    RGBA(RwRGB rgb, uint8_t a=0xff)
    {
        m_rgba  = (uint32_t)rgb.red   << 0;
        m_rgba |= (uint32_t)rgb.green << 8;
        m_rgba |= (uint32_t)rgb.blue  << 16;
        m_rgba |= (uint32_t)a         << 24;
    }

    inline RwRGBA
    GetRwRGBA() const
    {
        RwRGBA rgba;

#if (rwHOST_ENDIAN == rwBIENDIAN)
        rgba.red   = (uint8_t)(m_rgba >>  0);
        rgba.green = (uint8_t)(m_rgba >>  8);
        rgba.blue  = (uint8_t)(m_rgba >> 16);
        rgba.alpha = (uint8_t)(m_rgba >> 24);
#else
        rgba = *(reinterpret_cast<const RwRGBA *>(&m_rgba));
#endif

        return rgba;
    }

    inline void
    Set(RwRGBA rgba)
    {
#if (rwHOST_ENDIAN == rwBIENDIAN)
        m_rgba  = (uint32_t)rgba.red   << 0;
        m_rgba |= (uint32_t)rgba.green << 8;
        m_rgba |= (uint32_t)rgba.blue  << 16;
        m_rgba |= (uint32_t)rgba.alpha << 24;
#else
        *(reinterpret_cast<RwRGBA *>(&m_rgba)) = rgba;
#endif
    }


    inline uint8_t
    GetRed() const
    {
        return (uint8_t)(m_rgba >> 0);
    }

    inline void
    SetRed(uint8_t value)
    {
        m_rgba &= ~(0xff << 0);
        m_rgba |= ((uint32_t)value << 0);
    }

    inline uint8_t
    GetGreen() const
    {
        return (uint8_t)(m_rgba >> 8);
    }

    inline void
    SetGreen(uint8_t value)
    {
        m_rgba &= ~(0xff << 8);
        m_rgba |= ((uint32_t)value << 8);
    }

    inline uint8_t
    GetBlue() const
    {
        return (uint8_t)(m_rgba >> 16);
    }

    inline void
    SetBlue(uint8_t value)
    {
        m_rgba &= ~(0xff << 16);
        m_rgba |= ((uint32_t)value << 16);
    }

    inline uint8_t
    GetAlpha() const
    {
        return (uint8_t)(m_rgba >> 24);
    }

    inline void
    SetAlpha(uint8_t value)
    {
        m_rgba &= ~(0xff << 24);
        m_rgba |= ((uint32_t)value << 24);
    }

    bool operator==(const RGBA &other) const
    {
        return (m_rgba == other.m_rgba);
    }

 private:
    uint32_t  m_rgba;
};

} // namespace rw
#define RwRGBAClassMacro
#endif  // ifndef RwRGBAClassMacro

#ifndef RwRGBA16TagMacro
struct RwRGBA16Tag
{
    uint16_t red;
    uint16_t green;
    uint16_t blue;
    uint16_t alpha;
};
#define RwRGBA16TagMacro struct RwRGBA16Tag
#endif /* !RwRGBA16TagMacro */
typedef RwRGBA16TagMacro RwRGBA16;

#ifndef RwRGBRealTagMacro
struct RwRGBRealTag
{
    float32_t red;
    float32_t green;
    float32_t blue;
};
#define RwRGBRealTagMacro struct RwRGBRealTag
#endif /* !RwRGBRealTagMacro */
typedef RwRGBRealTagMacro RwRGBReal;

#ifndef RwRGBARealTagMacro
struct RwRGBARealTag
{
    float32_t red;
    float32_t green;
    float32_t blue;
    float32_t alpha;
    bool operator==(const RwRGBARealTag &other) const
    {
        return red == other.red && green == other.green && blue == other.blue && alpha == other.alpha;
    }
};
#define RwRGBARealTagMacro struct RwRGBARealTag
#endif /* !RwRGBARealTagMacro */
typedef RwRGBARealTagMacro RwRGBAReal;

#ifndef RwPoint2TemplateTagMacro
namespace rw
{
template <typename TYPE> class Point2Template
{
 public:
    inline
    Point2Template(void) {}

    inline
    Point2Template(TYPE x, TYPE y) : m_x(x), m_y(y) { }

    inline TYPE
    X() const
    {
        return m_x;
    }

    inline TYPE
    Y() const
    {
        return m_y;
    }

    inline TYPE
    GetX() const
    {
        return m_x;
    }

    inline TYPE
    GetY() const
    {
        return m_y;
    }

    inline void
    SetX(TYPE x)
    {
        m_x = x;
    }

    inline void
    SetY(TYPE y)
    {
        m_y = y;
    }

    inline void
    Set(TYPE x, TYPE y)
    {
        m_x = x;
        m_y = y;
    }

private:
    TYPE     m_x;
    TYPE     m_y;
};
#define RwPoint2TemplateTagMacro
#endif /* RwPoint2TemplateTagMacro */
typedef Point2Template<uint16_t> Point2_16;
} /* namespace rw */

namespace rw
{
typedef union SplitBits SplitBits;
union SplitBits
{
    float32_t nReal;
    volatile int32_t nInt;
    volatile uint32_t nUInt;
};
}
// TODO: Remove this once all references to SplitBits
//       are referencing rw::SplitBits
typedef rw::SplitBits RwSplitBits;

namespace rw
{
typedef union MutablePointer MutablePointer;
union MutablePointer
{
    const void *cptr;
    void *ptr;
};
}
// TODO: Remove this once all references to RwMutablePointer
//       are referencing rw::MutablePointer
typedef rw::MutablePointer RwMutablePointer;

namespace rw
{
/**
This enumeration is used as a function's result values. It encapsulates API level
error handling and status code.

\enum STATUS_FAIL                The operation failed
\enum STATUS_SUCCESS             The operation was successful. Equal to zero.
*/
enum Status
{
    STATUS_NA = -2,

    STATUS_FAIL = false,
    STATUS_SUCCESS = true,

    STATUS_FORCEENUMSIZEINT = RWFORCEENUMSIZEINT
};
}// namespace rw

#endif /* !RW_OSTYPES_H */
