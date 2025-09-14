// (c) Electronic Arts. All Rights Reserved.

// This file does not contain any user-callable functions,
// so it is omitted from the public documentation.

#ifndef PUBLIC__ENDIAN_H_
#define PUBLIC__ENDIAN_H_

namespace rwinternal
{

// Copy swap for array of 16 bit values... we must use this for data that is not
// necessary on a 2 byte boundary.
static inline void EndianCopy16(const void *src, void *dst, uint32_t length)
{
    rwFUNCTION();

    RwMemcpy(dst, src, length * sizeof(uint16_t));
    RWFORCEDENDIANSWAP16_ARRAY(dst, (int32_t)length, sizeof(uint16_t));

    rwRETURNVOID();
}


// Copy swap for array of 32 bit values... we must use this for data that is not
// necessary on a 4 byte boundary.
static inline void EndianCopy32(const void *src, void *dst, uint32_t length)
{
    rwFUNCTION();

    RwMemcpy(dst, src, length * sizeof(uint32_t));
    RWFORCEDENDIANSWAP32_ARRAY(dst, (int32_t)length, sizeof(uint32_t));

    rwRETURNVOID();
}


// Copy swap for array of 64 bit values... we must use this for data that is not
// necessary on an 8 byte boundary.
static inline void EndianCopy64(const void *src, void *dst, uint32_t length)
{
    rwFUNCTION();

    RwMemcpy(dst, src, length * sizeof(uint64_t));
    RWFORCEDENDIANSWAP64_ARRAY(dst, (int32_t)length, sizeof(uint64_t));

    rwRETURNVOID();
}


} // namespace rwinternal

#endif // PUBLIC__ENDIAN_H_
