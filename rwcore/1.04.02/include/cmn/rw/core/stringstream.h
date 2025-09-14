// (c) Electronic Arts. All Rights Reserved.

/// \file
/// Simple std::stringstream workalike for formatted input/output to and from an RwString.

#ifndef PUBLIC_RW_CORE_STRINGSTREAM_H
#define PUBLIC_RW_CORE_STRINGSTREAM_H

#include "rw/core/base/ostypes.h"
#include "rw/core/stltypes.h"
#include "rw/core/memory/memoryfunctions.h"

// ***********************************************************************************************************
// Forward Declarations

// ***********************************************************************************************************
// Typedefs

// ***********************************************************************************************************
// Defines + Enums + Consts

namespace RwIOS
{
    /**
    Flags to control the streams
    
    \enum left          Left adjust.
    \enum right         Right adjust.
    \enum internal      Internal.
    \enum dec           Decimal mode.
    \enum hex           Hexadecimal mode.
    \enum oct           Octal mode.
    \enum fixed         Fixed precision floating point output.
    \enum scientific    Scientific format floating point output.
    \enum boolalpha     to do
    \enum showbase      to do
    \enum showpoint     to do
    \enum showpos       to do
    \enum skipws        Set stream to skip white space.
    \enum unitbuf       to do
    \enum uppercase     Set output to upper case.

    \enum adjustfield   Mask of stream left right adjustment flags.
    \enum basefield     Mask of stream numeric base flags.
    \enum floatfield    Mask of stream floating point output format flags.
    \enum fmtmask       Mask of all stream format flags.

    \enum goodbit   Stream is in good condition.
    \enum badbit    Stream is in bad condition.
    \enum eofbit    Stream is at end.
    \enum failbit   Stream input/output has failed.

    \enum app       File open mode.
    \enum ate       File open mode.
    \enum binary    File open mode.
    \enum in        File open mode.
    \enum out       File open mode.
    \enum trunc     File open mode.

    \enum beg   Seek offset from beginning of stream.
    \enum cur   Seek offset from current position in stream.
    \enum end   Seek offset from end of stream.
    */
    enum flags
    {
    // Format flags
    left       = 0x0001,
    right      = 0x0002,
    internal   = 0x0004,
    dec        = 0x0008,
    hex        = 0x0010,
    oct        = 0x0020,
    fixed      = 0x0040,
    scientific = 0x0080,
    boolalpha  = 0x0100,
    showbase   = 0x0200,
    showpoint  = 0x0400,
    showpos    = 0x0800,
    skipws     = 0x1000,
    unitbuf    = 0x2000,
    uppercase  = 0x4000,
    adjustfield = left | right | internal,
    basefield   = dec | hex | oct,
    floatfield  = scientific | fixed,
    fmtmask     = 0x7fff,

    // State flags.
    goodbit = 0x00,
    badbit  = 0x01,
    eofbit  = 0x02,
    failbit = 0x04,

    // Openmode flags.
    __default_mode = 0x0, /* implementation detail */
    app    = 0x01,
    ate    = 0x02,
    binary = 0x04,
    in     = 0x08,
    out    = 0x10,
    trunc  = 0x20,

    // Seekdir flags

    beg = 0x01,
    cur = 0x02,
    end = 0x04

    };

}

// ***********************************************************************************************************
// Global Variables


// ***********************************************************************************************************
// External Functions Prototypes


// ***********************************************************************************************************
// Structs + Unions + Classes

namespace RwIOS
{
// ***********************************************************************************************************
//                                           RwIOS::width CLASS
// ***********************************************************************************************************

    /**
    The width of output od a stream.
    \member mWidth  Output width of stream.
    */
    struct width
    {
        /**
        Constructs a width stream modifier.
        \param w Stream output width.
        */
        width(int32_t w) : mWidth(w)
        {
        }

        /**
        Gets the current stream output width.
        \return Stream output width.
        */
        int32_t
        getw() const
        {
            return mWidth;
        }

        int32_t mWidth;
    };
}

// ***********************************************************************************************************
//                                           RwStringStream CLASS
// ***********************************************************************************************************

/**
RwStringStream allows formatted input and output via a C++ IOStream like mechanism using << and >> operators
to insert and extract data from an RwString stream representation. It is intended to behave like a C++
std::stringstream.

\member mStream The stream .
\member mpPos  The position in the stream where the next put will happen.
\member mgPos  The position in the stream where the next get will happen.
\member mWidth The output width of the stream.
\member mPrec  The output precision of stream.
\member mFmtFlags The IO Stream format flags.
\member mStateFlags The flags for the current state of the stream.
*/
class RwStringStream
{
public:

    RWMEMORYFUNCTIONS_NEWANDDELETE();

    /**
    \enum beg Seek from beginning of stream.
    \enum cur Seek from current position in the stream.
    \enum end Seek from end of stream.
    */
    enum pos { beg, cur, end };

    /**
    Constructs stream with default settings.
    \see RwIOS::flags
    */
    RwStringStream() : mStream(), mpPos(0), mgPos(0), mWidth(0), mPrec(6),
                       mFmtFlags(RwIOS::dec|RwIOS::left), mStateFlags(RwIOS::goodbit)
    {
        rwFUNCTION();
        rwRETURNVOID();
    }

    /**
    Constructs stream by copying.
    \param sStream stream to copy to this.
    \see RwIOS::flags
    */
    RwStringStream(const RwString &sStream)
        : mStream(sStream), mpPos(0), mgPos(0), mWidth(0), mPrec(6),
        mFmtFlags(RwIOS::dec|RwIOS::left), mStateFlags(RwIOS::goodbit)
    {
        rwFUNCTION();
        rwRETURNVOID();
    }

    /**
    Destroys stream.
    */
    ~RwStringStream()
    {
        rwFUNCTION();
        rwRETURNVOID();
    }

    /**
    Retrieves the streams string contents.
    \return Contents of stream.
    */
    RwString &
    str()
    {
        rwFUNCTION();
        rwRETURN(mStream);
    }

    /**
    Sets the streams string contents.
    \param stream The string to set.
    */
    void
    str(RwString &stream)
    {
        rwFUNCTION();

        mStream = stream;
        mStateFlags = RwIOS::goodbit;
        mpPos = 0;
        mgPos = 0;

        rwRETURNVOID();
    }

    /**
    Returns the position at which data will be insterted to the stream.
    \return Stream put position.
    */
    int32_t
    tellp() const
    {
        rwFUNCTION();
        rwRETURN(mpPos);
    }

    /**
    Retrieves the position from which data will be read from the stream.
    \return Stream get position.
    */
    int32_t
    tellg() const
    {
        rwFUNCTION();
        rwRETURN(mgPos);
    }

    /**
    Seek to a position within the stream. Subsequent inserts will begin at the new position.
    \param p Offset to seek to.
    \param off Offset base specifier (begining, current or end of stream).
    \return Current stream.
    \see RwIOS::pos
    */
    RwStringStream &
    seekp(int32_t p, pos off = beg)
    {
        rwFUNCTION();

        if (off == beg)
        {
            mpPos = p;
        }
        else if (off == cur)
        {
            mpPos += p;
        }
        else if (off == end)
        {
            mpPos = (int32_t)(mStream.length()-p);
        }

        rwRETURN(*this);
    }

    /**
    Seek to a position within the stream. Subsequent reads will be from the new position.
    \param p Offset to seek to.
    \param off Offset base specifier (begining, current or end of stream).
    \return Current stream.
    \see RwIOS::pos
    */
    RwStringStream &
    seekg(int32_t p, pos off = beg)
    {
        rwFUNCTION();
        if (off == beg)
        {
            mgPos = p;
        }
        else if (off == cur)
        {
            mgPos += p;
        }
        else if (off == end)
        {
            mgPos = (int32_t)(mStream.length()-p);
        }

        if (mgPos >= int32_t(mStream.length()))
        {
            mStateFlags |= RwIOS::eofbit;
        }
        else if (mgPos < 0)
        {
            mStateFlags |= RwIOS::badbit;
        }

        rwRETURN(*this);
    }

    /**
    Returns the stream's current format flags.
    \return Stream format flags.
    */
    int32_t
    flags() const
    {
        rwFUNCTION();
        rwRETURN(mFmtFlags);
    }

    /**
    Sets the stream's format flags to those supplied.
    \param fl New flags to set.
    \return Old stream flags.
    */
    int32_t
    flags(int32_t fl)
    {
        rwFUNCTION();
        int32_t oflags = int32_t(mFmtFlags);
        mFmtFlags = fl & RwIOS::fmtmask;
        rwRETURN(oflags);
    }

    /**
    Merges flags into the stream's current format flags.  Only bit flags which are present in the mask are affected.
    \param fl New flag bits to set.
    \param mask Bit mask applied to flag fields.
    \return Old stream flags.
    */
    int32_t
    setf(int32_t fl, int32_t mask)
    {
        int32_t oflags = int32_t(mFmtFlags);
        mFmtFlags = (mFmtFlags & ~mask) | (fl & mask & RwIOS::fmtmask);
        rwRETURN(oflags);
    }

    /**
    Sets the stream's format flags.

    \param fl Flag bits to set.
    \return Old stream flags.

    The specified flag bits are set to 1.
    */
    int32_t
    setf(int32_t fl)
    {
        int32_t oflags = int32_t(mFmtFlags);
        mFmtFlags = mFmtFlags | (fl & RwIOS::fmtmask);
        rwRETURN(oflags);
    }

    /**
    Clears the stream's format flags.
    \param fl Flag bits to clear.

    The specified flag bits are set to 0.
    */
    void
    unsetf(RwIOS::flags fl)
    {
        rwFUNCTION();
        mFmtFlags &= ~fl;
        rwRETURNVOID();
    }

    /**
    Returns the current stream output width.
    \return Output width of stream.
    */
    int32_t
    width() const
    {
        rwFUNCTION();
        rwRETURN(mWidth);
    }

    /**
    Returns the current stream floating point output precsision.
    \return Floating point precision.
    */
    int32_t
    precision() const
    {
        rwFUNCTION();
        rwRETURN(mPrec);
    }

    /**
    Sets the stream's output width.
    \param w Output width of stream.
    \return Old output width.
    */
    int32_t
    setw(int32_t w)
    {
        rwFUNCTION();
        int32_t oldw = mWidth;
        mWidth = w;
        rwRETURN(oldw);
    }

    /**
    Sets the stream's output width.
    \param w Output width of stream.
    \return Old output width.
    */
    int32_t
    width(int32_t w)
    {
        rwFUNCTION();
        rwRETURN(setw(w));
    }

    /**
    Sets the stream's floating point output precision.
    \param p Output precision of stream.
    \return Old output precision.
    */
    int32_t
    setprecision(int32_t p)
    {
        rwFUNCTION();
        int32_t oldp = mPrec;
        mPrec = p;
        rwRETURN(oldp);
    }

    /**
    Sets the stream's floating point output precision.
    \param p Output precision of stream.
    \return Old output precision.
    */
    int32_t
    precision(int32_t p)
    {
        rwFUNCTION();
        rwRETURN(setprecision(p));
    }

    /**
    Skips white space in the stream. The stream's get position is updated.
    */
    void
    skipws()
    {
        rwFUNCTION();

        RwString::size_type newpos = mStream.find_first_not_of(" \t\r\n", (uint32_t)mgPos);

        if (newpos != mStream.npos)
        {
            seekg((int32_t)newpos, beg);
        }
        else
        {
            seekg(0, end);
        }

        rwRETURNVOID();
    }

    /**
    Checks if the stream has reached it's end when reading from it.
    \return true if stream is at end, false otherwise.
    */
    bool
    eof() const
    {
        rwFUNCTION();
        rwRETURN(((mStateFlags & RwIOS::eofbit) != 0));
    }

    /**
    Checks if the stream is in a readable/writable state.
    \return true if stream is readable/writeable, false otherwise.
    */
    bool
    good() const
    {
        rwFUNCTION();
        rwRETURN(mStateFlags == 0);
    }

    /**
    Checks whether a stream read/write operation has failed.
    \return true if stream operation failed, false otherwise.
    */
    bool
    fail() const
    {
        rwFUNCTION();
        rwRETURN((mStateFlags & (RwIOS::badbit | RwIOS::failbit)) != 0);
    }

    /**
    Checks if the stream is in a readable/writable state.
    \return false if stream is readable/writeable, true otherwise.
    */
    bool
    bad() const
    {
        rwFUNCTION();
        rwRETURN((mStateFlags & RwIOS::badbit) != 0);
    }

    /**
    Sets the stream's state flags (end of file, good, bad etc.).
    \param state State flags to set (defaults to RwIOS::goodbit)
    */
    void
    clear(int32_t state = RwIOS::goodbit)
    {
        rwFUNCTION();
        mStateFlags = state;
        rwRETURNVOID();
    }

    /**
    Gets the current stream state flags.
    \return Stream state flags.
    */
    int32_t
    rdstate(void)
    {
        rwFUNCTION();
        rwRETURN(mStateFlags);
    }

    /**
    Sets the current stream state flags.
    The state is combined with the current state flags.
    */
    void
    setstate(int32_t state)
    {
        rwFUNCTION();
        clear(rdstate() | state);
        rwRETURNVOID();
    }

    /**
    Un-reads a single character from the stream. A subsequent read will read the same character again.
    */
    void
    unget()
    {
        rwFUNCTION();
        clear(); // libstdc++ does not clear, MSVC C++ does clear!!!
        seekg(-1, cur);
        rwRETURNVOID();
    }

    /**
    Reads a single character from the stream and advances to the next character.
    \return Character read from stream.
    */
    int32_t
    get()
    {
        rwFUNCTION();
        if (eof())
        {
            rwRETURN(static_cast<int32_t>(RwString::npos));
        }
        int32_t val = mStream[(uint32_t)mgPos];
        seekg(1, cur);
        rwRETURN(val);
    }

    /**
    Reads a single character from the stream into the supplied output, advancing to the next character.
    \param v Character read from stream.
    */
    void
    get(RwChar &v)
    {
        rwFUNCTION();
        v = (RwChar)get();
        rwRETURNVOID();
    }

    /**
    Returns the next character which would be read from the stream. Does not adjust the stream position.
    If the stream is at it's end, the value RwString::npos is returned.
    \return Next character in stream or RwString::npos if at end of stream.
    */
    int32_t
    peek()
    {
        rwFUNCTION();
        if (eof())
        {
            rwRETURN(static_cast<int32_t>(RwString::npos));
        }
        int32_t val = mStream[(uint32_t)mgPos];
        rwRETURN(val);
    }

    /**
    Skip over a number of characters in the stream.
    \param n Number of character to skip, 1 by default.
    */
    void
    ignore(const int32_t n = 1)
    {
        rwFUNCTION();
        seekg(n,cur);
        rwRETURNVOID();
    }

    friend RwStringStream& operator<< (RwStringStream &stream, const RwIOS::flags flag);
    friend RwStringStream& operator<< (RwStringStream &stream, const RwIOS::width &w);

    friend RwStringStream& operator<< (RwStringStream &stream, const int64_t val);
    friend RwStringStream& operator<< (RwStringStream &stream, const uint64_t val);
    friend RwStringStream& operator<< (RwStringStream &stream, const int32_t val);
    friend RwStringStream& operator<< (RwStringStream &stream, const uint32_t val);
    friend RwStringStream& operator<< (RwStringStream &stream, const RwChar val);
    friend RwStringStream& operator<< (RwStringStream &stream, const uint8_t val);
    friend RwStringStream& operator<< (RwStringStream &stream, const int8_t val);
    friend RwStringStream& operator<< (RwStringStream &stream, const float32_t val);
    friend RwStringStream& operator<< (RwStringStream &stream, const float64_t val);
    friend RwStringStream& operator<< (RwStringStream &stream, const RwChar *val);
    friend RwStringStream& operator<< (RwStringStream &stream, const RwString &val);
#if rwHOST_SPECIALIZE_SIZET_TRAITS
    friend RwStringStream& operator<< (RwStringStream &stream, const size_t val);
#endif

    friend RwStringStream& operator>>(RwStringStream &stream, rw::int128_t &val);
    friend RwStringStream& operator>>(RwStringStream &stream, rw::uint128_t &val);
    friend RwStringStream& operator>>(RwStringStream &stream, int64_t &val);
    friend RwStringStream& operator>>(RwStringStream &stream, uint64_t &val);
    friend RwStringStream& operator>>(RwStringStream &stream, int32_t &val);
    friend RwStringStream& operator>>(RwStringStream &stream, uint32_t &val);
    friend RwStringStream& operator>>(RwStringStream &stream, int16_t &val);
    friend RwStringStream& operator>>(RwStringStream &stream, uint16_t &val);
    friend RwStringStream& operator>>(RwStringStream &stream, RwChar &val);
    friend RwStringStream& operator>>(RwStringStream &stream, uint8_t &val);
    friend RwStringStream& operator>>(RwStringStream &stream, int8_t &val);
    friend RwStringStream& operator>>(RwStringStream &stream, float64_t &val);
    friend RwStringStream& operator>>(RwStringStream &stream, float32_t &val);
    friend RwStringStream& operator>>(RwStringStream &stream, RwChar *val);
    friend RwStringStream& operator>>(RwStringStream &stream, RwString &val);
#if rwHOST_SPECIALIZE_SIZET_TRAITS
    friend RwStringStream& operator>>(RwStringStream &stream, size_t &val);
#endif

private:

    static RwChar   *fFormat(RwChar *fBuf, int32_t fmtFlags);
    static RwChar   *iFormat(RwChar *fBuf, RwChar form, const RwChar* sizeSpec, int32_t fmtFlags);

    template<class T> void 
    ReadLargeType(T& object);

    void
    ReadHexBuffer(uint8_t* buf, uint32_t size);

    RwString        mStream;
    int32_t         mpPos;
    int32_t         mgPos;
    int32_t         mWidth;
    int32_t         mPrec;
    int32_t         mFmtFlags;
    int32_t         mStateFlags;
};

#endif // PUBLIC_RW_CORE_STRINGSTREAM_H
