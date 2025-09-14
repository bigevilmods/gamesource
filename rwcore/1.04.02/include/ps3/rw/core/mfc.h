// (c) Electronic Arts. All Rights Reserved.

/// file
/// MFC management

#ifndef RW_MFC_H
#define RW_MFC_H

// ***********************************************************************************************************
// Includes

#include <sdk_version.h>
#include <spu_mfcio.h>

// ***********************************************************************************************************
// Defines

namespace rw
{

// ***********************************************************************************************************
// Forward Declarations


// ***********************************************************************************************************
// Typedefs


// ***********************************************************************************************************
// Enums + Consts


// ***********************************************************************************************************
// Global Variables


// ***********************************************************************************************************
// External Functions Prototypes


// ***********************************************************************************************************
// Structs + Unions + Classes

// ***********************************************************************************************************
//                                                MFC Definition
// ***********************************************************************************************************

class MFC
{
public:

    static uint32_t LocalToEffectiveAddress(uint32_t spu, uint32_t localAddress);

    // Synchronization
    static void ResetTagQuery();
    static void WaitForAllTags(uint32_t tagMask);
    static void WaitForAnyTag(uint32_t tagMask);
    static void WaitForStores();

    // Signals
    static uint32_t GetSignal1();
    static uint32_t GetSignal2();
    static uint32_t GetMail();
    static void SendSignal1(uint32_t *value, uint32_t spu, uint32_t tag);
    static void SendSignal2(uint32_t *value, uint32_t spu, uint32_t tag);
    static void SendMail(uint32_t value);
    static void SendInterruptMail(uint32_t value);

    // DMA transfers
    static void Get(void *localAddress, GlobalAddress effectiveAddress, uint32_t size, uint32_t tag);
    template<typename T> static void Get(T *localAddress, GlobalAddress effectiveAddress, uint32_t tag);
    static void Getf(void *localAddress, GlobalAddress effectiveAddress, uint32_t size, uint32_t tag);
    template<typename T> static void Getf(T *localAddress, GlobalAddress effectiveAddress, uint32_t tag);
    static void Put(const void *localAddress, GlobalAddress effectiveAddress, uint32_t size, uint32_t tag);
    template<typename T> static void Put(const T *localAddress, GlobalAddress effectiveAddress, uint32_t tag);
    static void Putf(const void *localAddress, GlobalAddress effectiveAddress, uint32_t size, uint32_t tag);
    template<typename T> static void Putf(const T *localAddress, GlobalAddress effectiveAddress,
        uint32_t tag);

    // Locking/unlocking
    template<typename T> static void GetLockLineNoWait(T *localAddress, GlobalAddress effectiveAddress);
    template<typename T> static void GetLockLine(T *localAddress, GlobalAddress effectiveAddress);
    template<typename T> static void PutLockLineNoWait(const T *localAddress, GlobalAddress effectiveAddress);
    static bool PutLockLineSucceeded();
    template<typename T> static bool PutLockLine(const T *localAddress, GlobalAddress effectiveAddress);
    template<typename T> static void Lock(T *localAddress, GlobalAddress effectiveAddress);
    template<typename T> static void Unlock(T *localAddress, GlobalAddress effectiveAddress, uint32_t tag);

    // Debugging
#ifdef EA_DEBUG
    template<typename T> static void DebugLock(T *localAddress, GlobalAddress effectiveAddress,
        uint32_t flags);
    template<typename T> static void DebugUnlock(T *localAddress, GlobalAddress effectiveAddress,
        uint32_t flags);
#endif

private:

    static const uint32_t BaseAddress    = 0xe0000000;
    static const uint32_t Signal1Address = 0xe005400c;
    static const uint32_t Signal2Address = 0xe005c00c;
    static const uint32_t Offset         = 0x00100000;
    static const uint32_t MemorySize     = 0x00040000;
    static const uint32_t LineSize       = 0x00000080;

    static void Dma(const void *localAddress, GlobalAddress effectiveAddress, uint32_t size, uint32_t tag,
        uint32_t command);
    template<typename T> static void Dma(const T *localAddress, GlobalAddress effectiveAddress, uint32_t tag,
        uint32_t command);

};

inline uint32_t MFC::LocalToEffectiveAddress(uint32_t spu, uint32_t localAddress)
{
    rwFUNCTION();
    rwASSERT(localAddress < MemorySize);
    rwRETURN(BaseAddress + spu * Offset + localAddress);
}

inline void MFC::Dma(const void *localAddress, GlobalAddress effectiveAddress, uint32_t size, uint32_t tag,
         uint32_t command)
{
    rwFUNCTION();
    // Debug checks
    rwMESSAGE(("Dma CMD=%02x TAG=%02x LA=%08x EA=%08x SZ=%04x",
              command, tag, localAddress, effectiveAddress, size));
    rwASSERT(size <= MFC_MAX_DMA_SIZE);
    if (MFC_GETLLAR_CMD == command || MFC_PUTLLC_CMD == command || MFC_PUTLLUC_CMD == command)
    {
        rwASSERT(128 == size);
    }
    if (size >= 128)
    {
        rwASSERT(0 == (127 & size));;
        rwASSERT(IsMemAligned(localAddress, 128));
        rwASSERT(0 == (127 & effectiveAddress));
    }
    if (size >= 16)
    {
        rwASSERT(0 == (15 & size));;
        rwASSERT(IsMemAligned(localAddress, 16));
        rwASSERT(0 == (15 & effectiveAddress));
    }
    else
    {
        rwASSERT(0 == (size & (size - 1)));
        rwASSERT(IsMemAligned(localAddress, size));
        rwASSERT((15 & reinterpret_cast<rw::HostPtr>(localAddress)) == (15 & effectiveAddress));
    }
    rwASSERT(tag < 32);

    spu_mfcdma32(const_cast<void *>(localAddress), effectiveAddress, size, tag, command);

    rwRETURNVOID();
}

template<typename T>
inline void MFC::Dma(const T *localAddress, GlobalAddress effectiveAddress, uint32_t tag, uint32_t command)
{
    rwFUNCTION();
    Dma(localAddress, effectiveAddress, sizeof(T), tag, command);
    rwRETURNVOID();
}

inline void MFC::ResetTagQuery()
{
    rwFUNCTION();
    spu_writech(MFC_WrTagUpdate, 0);
    spu_readch(MFC_RdTagStat);
    do {} while (1 != spu_readchcnt(MFC_WrTagUpdate));
    rwRETURNVOID();
}

inline void MFC::WaitForAllTags(uint32_t tagMask)
{
    rwFUNCTION();
    spu_writech(MFC_WrTagMask, tagMask);
    spu_writech(MFC_WrTagUpdate, 2);
    spu_readch(MFC_RdTagStat);
    rwRETURNVOID();
}

inline void MFC::WaitForAnyTag(uint32_t tagMask)
{
    rwFUNCTION();
    spu_writech(MFC_WrTagMask, tagMask);
    spu_writech(MFC_WrTagUpdate, 1);
    spu_readch(MFC_RdTagStat);
    rwRETURNVOID();
}

inline void MFC::WaitForStores()
{
    rwFUNCTION();
    asm volatile ("dsync");
    rwRETURNVOID();
}

inline uint32_t MFC::GetSignal1()
{
    rwFUNCTION();
    rwRETURN(spu_readch(SPU_RdSigNotify1));
}

inline uint32_t MFC::GetSignal2()
{
    rwFUNCTION();
    rwRETURN(spu_readch(SPU_RdSigNotify2));
}

inline uint32_t MFC::GetMail()
{
    rwFUNCTION();
    rwRETURN(spu_readch(SPU_RdInMbox));
}

inline void MFC::SendSignal1(uint32_t *value, uint32_t spu, uint32_t tag)
{
    rwFUNCTION();
    Dma(value, Signal1Address + spu * Offset, 4, tag, MFC_SNDSIG_CMD);
    rwRETURNVOID();
}

inline void MFC::SendSignal2(uint32_t *value, uint32_t spu, uint32_t tag)
{
    rwFUNCTION();
    Dma(value, Signal2Address + spu * Offset, 4, tag, MFC_SNDSIG_CMD);
    rwRETURNVOID();
}

inline void MFC::SendMail(uint32_t value)
{
    rwFUNCTION();
    spu_writech(SPU_WrOutMbox, value);
    rwRETURNVOID();
}

inline void MFC::SendInterruptMail(uint32_t value)
{
    rwFUNCTION();
    spu_writech(SPU_WrOutIntrMbox, value);
    rwRETURNVOID();
}

inline void MFC::Get(void *localAddress, GlobalAddress effectiveAddress, uint32_t size, uint32_t tag)
{
    rwFUNCTION();
    Dma(localAddress, effectiveAddress, size, tag, MFC_GET_CMD);
    rwRETURNVOID();
}

template<class T>
inline void MFC::Get(T *localAddress, GlobalAddress effectiveAddress, uint32_t tag)
{
    rwFUNCTION();
    Dma(localAddress, effectiveAddress, tag, MFC_GET_CMD);
    rwRETURNVOID();
}

inline void MFC::Getf(void *localAddress, GlobalAddress effectiveAddress, uint32_t size, uint32_t tag)
{
    rwFUNCTION();
    Dma(localAddress, effectiveAddress, size, tag, MFC_GETF_CMD);
    rwRETURNVOID();
}

template<class T>
inline void MFC::Getf(T *localAddress, GlobalAddress effectiveAddress, uint32_t tag)
{
    rwFUNCTION();
    Dma(localAddress, effectiveAddress, tag, MFC_GETF_CMD);
    rwRETURNVOID();
}

inline void MFC::Put(const void *localAddress, GlobalAddress effectiveAddress, uint32_t size, uint32_t tag)
{
    rwFUNCTION();
    Dma(localAddress, effectiveAddress, size, tag, MFC_PUT_CMD);
    rwRETURNVOID();
}

template<class T>
inline void MFC::Put(const T *localAddress, GlobalAddress effectiveAddress, uint32_t tag)
{
    rwFUNCTION();
    Dma(localAddress, effectiveAddress, tag, MFC_PUT_CMD);
    rwRETURNVOID();
}

inline void MFC::Putf(const void *localAddress, GlobalAddress effectiveAddress, uint32_t size, uint32_t tag)
{
    rwFUNCTION();
    Dma(localAddress, effectiveAddress, size, tag, MFC_PUTF_CMD);
    rwRETURNVOID();
}

template<class T>
inline void MFC::Putf(const T *localAddress, GlobalAddress effectiveAddress, uint32_t tag)
{
    rwFUNCTION();
    Dma(localAddress, effectiveAddress, tag, MFC_PUTF_CMD);
    rwRETURNVOID();
}

template<class T>
inline void MFC::GetLockLineNoWait(T *localAddress, GlobalAddress effectiveAddress)
{
    rwFUNCTION();
    Dma(localAddress, effectiveAddress, 0, MFC_GETLLAR_CMD);
    rwRETURNVOID();
}

template<class T>
inline void MFC::GetLockLine(T *localAddress, GlobalAddress effectiveAddress)
{
    rwFUNCTION();
    GetLockLineNoWait(localAddress, effectiveAddress);
    spu_readch(MFC_RdAtomicStat);
    rwRETURNVOID();
}

template<class T>
inline void MFC::PutLockLineNoWait(const T *localAddress, GlobalAddress effectiveAddress)
{
    rwFUNCTION();
    Dma(localAddress, effectiveAddress, 0, MFC_PUTLLC_CMD);
    rwRETURNVOID();
}

inline bool MFC::PutLockLineSucceeded()
{
    rwFUNCTION();
    rwRETURN(0 == (MFC_PUTLLC_STATUS & spu_readch(MFC_RdAtomicStat)));
}

template<class T>
inline bool MFC::PutLockLine(const T *localAddress, GlobalAddress effectiveAddress)
{
    rwFUNCTION();
    PutLockLineNoWait(localAddress, effectiveAddress);
    rwRETURN(PutLockLineSucceeded());
}

template<class T>
inline void MFC::Lock(T *localAddress, GlobalAddress effectiveAddress)
{
    rwFUNCTION();
    do
    {
        do
        {
            GetLockLine(localAddress, effectiveAddress);
        } while (__builtin_expect(localAddress->IsLocked(), 0));
        localAddress->Lock();
        WaitForStores();
    } while (__builtin_expect(!PutLockLine(localAddress, effectiveAddress), 0));
    rwRETURNVOID();
}

template<class T>
inline void MFC::Unlock(T *localAddress, GlobalAddress effectiveAddress, uint32_t tag)
{
    rwFUNCTION();
    localAddress->Unlock();
    WaitForStores();
    Putf(localAddress, effectiveAddress, tag);
    rwRETURNVOID();
}

// Debugging
#ifdef EA_DEBUG

template<typename T>
inline void MFC::DebugLock(T *localAddress, GlobalAddress effectiveAddress, uint32_t flags)
{
    rwFUNCTION();
    do
    {
        GetLockLine(localAddress, effectiveAddress);
        localAddress->DebugLock(flags);
        WaitForStores();
    } while (!PutLockLine(localAddress, effectiveAddress));
    rwRETURNVOID();
}

template<typename T>
inline void MFC::DebugUnlock(T *localAddress, GlobalAddress effectiveAddress, uint32_t flags)
{
    rwFUNCTION();
    do
    {
        GetLockLine(localAddress, effectiveAddress);
        localAddress->DebugUnlock(flags);
        WaitForStores();
    } while (!PutLockLine(localAddress, effectiveAddress));
    rwRETURNVOID();
}

#endif

} //namespace rw

#endif // RW_MFC_H
