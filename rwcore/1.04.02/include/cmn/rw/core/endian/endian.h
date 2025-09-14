// (c) Electronic Arts. All Rights Reserved.

/// \file
/// Deprecated endian-conversion functions

#ifndef RW_CORE_ENDIAN_H
#define RW_CORE_ENDIAN_H
#include <rw/core/base/ostypes.h>

namespace rw {
    namespace core {
        /**
            All functions in the rw::core::endian namespace are deprecated.
            Use the ones in the Endian package instead.
        */
        namespace endian {

            /**
            \brief Convert little endian uint32_t to uint32_t
            \deprecated Use the Endian package instead.
            \return uint32_t
            */
            inline uint32_t ConvertLittle(const uint32_t x);

            /**
            \brief Convert little endian uint16_t to uint16_t
            \deprecated Use the Endian package instead.
            \return uint16_t
            */
            inline uint16_t ConvertLittle(const uint16_t x);

            /**
            \brief Convert little endian int32_t to int32_t
            \deprecated Use the Endian package instead.
            \return int32_t
            */
            inline int32_t ConvertLittle(const int32_t x);

            /**
            \brief Convert little endian int16_t to int16_t
            \deprecated Use the Endian package instead.
            \return int16_t
            */
            inline int16_t ConvertLittle(const int16_t x);

            /**
            \brief Convert little endian uint64_t to uint64_t
            \deprecated Use the Endian package instead.
            \return uint64_t
            */
            inline uint64_t ConvertLittle(const uint64_t x);

            /**
            \brief Convert little endian int64_t to int64_t
            \deprecated Use the Endian package instead.
            \return int64_t
            */
            inline int64_t ConvertLittle(const int64_t x);

            /**
            \brief Convert big endian uint32_t to uint32_t
            \deprecated Use the Endian package instead.
            \return uint32_t
            */
            inline uint32_t ConvertBig(const uint32_t x);

            /**
            \brief Convert big endian uint16_t to uint16_t
            \deprecated Use the Endian package instead.
            \return uint16_t
            */
            inline uint16_t ConvertBig(const uint16_t x);

            /**
            \brief Convert big endian int32_t to int32_t
            \deprecated Use the Endian package instead.
            \return int32_t
            */
            inline int32_t ConvertBig(const int32_t x);

            /**
            \brief Convert big endian int16_t to int16_t
            \deprecated Use the Endian package instead.
            \return int16_t
            */
            inline int16_t ConvertBig(const int16_t x);

            /**
            \brief Convert big endian uint64_t to uint64_t
            \deprecated Use the Endian package instead.
            \return uint64_t
            */
            inline uint64_t ConvertBig(const uint64_t x);

            /**
            \brief Convert big endian int64_t to int64_t
            \deprecated Use the Endian package instead.
            \return int64_t
            */
            inline int64_t ConvertBig(const int64_t x);

            /**
            \brief Combine 4 8-bit values into one 32-bit value
            \deprecated Use the Endian package instead.
            \return uint32_t
            */
            inline uint32_t make_id32(uint8_t a, uint8_t b, uint8_t c, uint8_t d);

            /**
            \brief Put n=1..4 bytes of a big endian uint32_t to memory.
            \deprecated Use the Endian package instead.

            Generally, the destination address must be aligned to the
            number of bytes being written (i.e. if 4 bytes are being written
            dst must be aligned on a 4-byte boundary).  Some platforms
            will handle the misaligned address, others won't.
            */
            inline void PutBig(void* dst, uint32_t data, int32_t n);

            /**
            \brief Put n=1..4 bytes of a little endian uint32_t to memory
            \deprecated Use the Endian package instead.

            Generally, the destination address must be aligned to the
            number of bytes being written (i.e. if 4 bytes are being written
            dst must be aligned on a 4-byte boundary).  Some platforms
            will handle the misaligned address, others won't.
            */
            inline void PutLittle(void *dst, uint32_t data, int32_t n);

            /**
            \brief Get n=1..4 bytes of a big endian uint32_t from memory
            \deprecated Use the Endian package instead.

            Generally, the source address must be aligned to the
            number of bytes being read (i.e. if 4 bytes are being read
            src must be aligned on a 4-byte boundary).  Some platforms
            will handle the misaligned address, others won't.
            */
            inline uint32_t GetBig(const void* src, int32_t n);

            /**
            \brief Get n=1..4 bytes of a little endian uint32_t from memory
            \deprecated Use the Endian package instead.

            Generally, the source address must be aligned to the
            number of bytes being read (i.e. if 4 bytes are being read
            src must be aligned on a 4-byte boundary).  Some platforms
            will handle the misaligned address, others won't.
            */
            inline uint32_t GetLittle(const void* src, int32_t n);

            /**
            \brief Put n=1..8 bytes of a big endian uint64_t to memory
             dst should be aligned for maximum portability.
            \deprecated Use the Endian package instead.

            Although the implementation at the time of this writing will handle
            misaligned writes, don't count on that behaviour
            */
            inline void PutBig64(void* dst, uint64_t data, int32_t n);

            /**
            \brief Put n=1..8 bytes of a little endian uint64_t to memory
            dst should be aligned for maximum portability.
            \deprecated Use the Endian package instead.

            Although the implementation at the time of this writing will handle
            misaligned writes, don't count on that behaviour
            */
            inline void PutLittle64(void *dst, uint64_t data, int32_t n);

            /**
            \brief Get n=1..8 bytes of a big endian uint64_t from memory
            src should be aligned for maximum portability.
            \deprecated Use the Endian package instead.

            Although the implementation at the time of this writing will handle
            misaligned reads, don't count on that behaviour
            */
            inline uint64_t GetBig64(const void* src, int32_t n);

            /**
            \brief Get n=1..8 bytes of a little endian uint64_t from memory
             src should be aligned for maximum portability.
            \deprecated Use the Endian package instead.

            Although the implementation at the time of this writing will handle
            misaligned reads, don't count on that behaviour
            */
            inline uint64_t GetLittle64(const void* src, int32_t n);
        }
    }
}

#include "detail/endian_inlines.h"

#endif // RW_CORE_ENDIAN_H
