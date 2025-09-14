// (c) Electronic Arts. All Rights Reserved.

#include <EABase/eabase.h>

EA_DISABLE_VC_WARNING(4574)
#include <new>
EA_RESTORE_VC_WARNING()

#include "coreallocator/icoreallocator_interface.h"
#include "EAAssert/eaassert.h"

#include "rw/core/codec/chunkcodec.h"
#include "detail/decompressors.h"

using namespace EA::Jobs;

namespace rw {
namespace core {
namespace codec {

ChunkDecompressor::ChunkDecompressor(EA::Allocator::ICoreAllocator *allocator, 
        EA::Jobs::JobScheduler *jobScheduler, 
        uint32_t, 
        EA::Jobs::JobAffinity jobAffinity, 
        EA::Jobs::JobPriority jobPriority)
    : mAllocator(*allocator),
      mJobScheduler(jobScheduler),
      mAffinity(jobAffinity),
      mPriority(jobPriority)
{
}

ChunkDecompressor::~ChunkDecompressor()
{
}

void ChunkDecompressor::SetNumberOfJobs(uint32_t)
{
    EA_FAIL_MSG("Deprecated, do not use.");
}

void ChunkDecompressor::SetJobAffinities(EA::Jobs::JobAffinity jobAffinity)
{
    mAffinity = jobAffinity;
}

void ChunkDecompressor::SetJobPriorities(EA::Jobs::JobPriority jobPriority)
{
    mPriority = jobPriority;
}


}}}
