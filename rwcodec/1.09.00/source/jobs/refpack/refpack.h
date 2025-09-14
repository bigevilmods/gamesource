// (c) Electronic Arts. All Rights Reserved.

/**
\file
Contains file signature and flags.
*/

#ifndef RW_CORE_CODEC_REFPACK_INTERNAL_H
#define RW_CORE_CODEC_REFPACK_INTERNAL_H

#ifdef _MSC_VER
#pragma once
#endif

// Refpack file signature
const unsigned short REF_SIG = 0x10fb;
const unsigned short REF_SIG_MASK = 0x3eff;
/**
 Long references were used when compressing this data;
 a 128K back reference buffer is required for streamed
 decompression.
*/
const unsigned short REF_HEADER_FLAG_SHORT_REFS = 0x4000;
/**
 input file size is larger than 2^24; sizeof(size) field is 32 bits.
*/
const unsigned short REF_HEADER_FLAG_SIZE32 = 0x8000;

const int MAX_LOOKBACK_SHORT = (16*1024)-1;
const int MAX_LOOKBACK_LONG = (128*1024)-1;

namespace rw {
namespace core {
namespace codec {
namespace Refpack {

unsigned int Inflate(void *outputBuffer, void const *inputBuffer);
unsigned int GetDecompressedSize(void const *inputBuffer);

}}}}

#endif
