// (c) Electronic Arts. All Rights Reserved.
/* RWPUBLIC */
/*************************************************************************************************************

 File: rwdmac.h

 Purpose: PS2 DMAC interface

 */

#ifndef RW_DMAC_H
#define RW_DMAC_H

/* RWOLDPUBLIC */

#include <eeregs.h>
#include <rw/core/base/ostypes.h>
#include <rw/core/debug/debug.h>
#include <rw/core/debug/exception.h>


namespace rw
{


/*
Interface to DMAC registers.
*/
class Dmac
{
public:

    static const uint32_t CHANNEL_VIF0    = 0x001;
    static const uint32_t CHANNEL_VIF1    = 0x002;
    static const uint32_t CHANNEL_GIF     = 0x004;
    static const uint32_t CHANNEL_FROMIPU = 0x008;
    static const uint32_t CHANNEL_TOIPU   = 0x010;
    static const uint32_t CHANNEL_SIF0    = 0x020;
    static const uint32_t CHANNEL_SIF1    = 0x040;
    static const uint32_t CHANNEL_SIF2    = 0x080;
    static const uint32_t CHANNEL_FROMSPR = 0x100;
    static const uint32_t CHANNEL_TOSPR   = 0x200;

    static const uint32_t ENABLE_OFF = 0x00010000;
    static const uint32_t ENABLE_ON  = 0x00000000;

    static void  ChainToVif1(const void *source);
    static void  ChainToGif(const void *source);
    static bool  HasCompleted(uint32_t channels);
    static void  Lock(uint32_t channels);
    static void  NormalFromSpr(void *destination, const void *source, uint32_t size);
    static void  NormalFromVif1(void *destination, uint32_t size);
    static void  NormalToFromSpr(void *toDestination, const void *toSource, uint32_t toSize,
        void *fromDestination, const void *fromSource, uint32_t fromSize);
    static void  NormalToSpr(void *destination, const void *source, uint32_t size);
    static void  NormalToVif1(const void *source, uint32_t size);
    static void  SetEnable(uint32_t enable);
    static void  Unlock(uint32_t channels);
    static void  WaitForCompletion(uint32_t channels);

private:

    static Dmac *Get();
#ifdef EA_DEBUG
    static uint32_t mLocked;
#endif

    volatile uint32_t mD0_CHCR;   uint32_t pad74[3];
    volatile uint32_t mD0_MADR;   uint32_t pad75[3];
    volatile uint32_t mD0_QWC;    uint32_t pad76[3];
    volatile uint32_t mD0_TADR;   uint32_t pad77[3];
    volatile uint32_t mD0_ASR0;   uint32_t pad78[3];
    volatile uint32_t mD0_ASR1;   uint32_t pad79[1003];

    volatile uint32_t mD1_CHCR;   uint32_t pad80[3];
    volatile uint32_t mD1_MADR;   uint32_t pad81[3];
    volatile uint32_t mD1_QWC;    uint32_t pad82[3];
    volatile uint32_t mD1_TADR;   uint32_t pad83[3];
    volatile uint32_t mD1_ASR0;   uint32_t pad84[3];
    volatile uint32_t mD1_ASR1;   uint32_t pad85[1003];

    volatile uint32_t mD2_CHCR;   uint32_t pad86[3];
    volatile uint32_t mD2_MADR;   uint32_t pad87[3];
    volatile uint32_t mD2_QWC;    uint32_t pad88[3];
    volatile uint32_t mD2_TADR;   uint32_t pad89[3];
    volatile uint32_t mD2_ASR0;   uint32_t pad90[3];
    volatile uint32_t mD2_ASR1;   uint32_t pad91[1003];

    volatile uint32_t mD3_CHCR;   uint32_t pad92[3];
    volatile uint32_t mD3_MADR;   uint32_t pad93[3];
    volatile uint32_t mD3_QWC;    uint32_t pad94[247];

    volatile uint32_t mD4_CHCR;   uint32_t pad95[3];
    volatile uint32_t mD4_MADR;   uint32_t pad96[3];
    volatile uint32_t mD4_QWC;    uint32_t pad97[3];
    volatile uint32_t mD4_TADR;   uint32_t pad98[755];

    volatile uint32_t mD5_CHCR;   uint32_t pad99[3];
    volatile uint32_t mD5_MADR;   uint32_t pad100[3];
    volatile uint32_t mD5_QWC;    uint32_t pad101[247];

    volatile uint32_t mD6_CHCR;   uint32_t pad102[3];
    volatile uint32_t mD6_MADR;   uint32_t pad103[3];
    volatile uint32_t mD6_QWC;    uint32_t pad104[3];
    volatile uint32_t mD6_TADR;   uint32_t pad105[243];

    volatile uint32_t mD7_CHCR;   uint32_t pad106[3];
    volatile uint32_t mD7_MADR;   uint32_t pad107[3];
    volatile uint32_t mD7_QWC;    uint32_t pad108[503];

    volatile uint32_t mD8_CHCR;   uint32_t pad109[3];
    volatile uint32_t mD8_MADR;   uint32_t pad110[3];
    volatile uint32_t mD8_QWC;    uint32_t pad111[23];
    volatile uint32_t mD8_SADR;   uint32_t pad112[223];

    volatile uint32_t mD9_CHCR;   uint32_t pad113[3];
    volatile uint32_t mD9_MADR;   uint32_t pad114[3];
    volatile uint32_t mD9_QWC;    uint32_t pad115[3];
    volatile uint32_t mD9_TADR;   uint32_t pad116[19];
    volatile uint32_t mD9_SADR;   uint32_t pad117[735];

    volatile uint32_t mD_CTRL;    uint32_t pad118[3];
    volatile uint32_t mD_STAT;    uint32_t pad119[3];
    volatile uint32_t mD_PCR;     uint32_t pad120[3];
    volatile uint32_t mD_SQWC;    uint32_t pad121[3];
    volatile uint32_t mD_RBSR;    uint32_t pad122[3];
    volatile uint32_t mD_RBOR;    uint32_t pad123[3];
    volatile uint32_t mD_STADR;   uint32_t pad124[1327];
    volatile uint32_t mD_ENABLER; uint32_t pad125[27];
    volatile uint32_t mD_ENABLEW; uint32_t pad126[667];
};


/*
Gets the address of the DMAC registers. This function allows GCC to share the instructions used here between
accesses to the registers.
*/
inline Dmac *Dmac::Get()
{
    rwFUNCTION();
    Dmac *base;
    asm ("lui %0, 0x1001" : "=r" (base));
    rwRETURN(base - 1);
}


/*
*/
inline void Dmac::SetEnable(uint32_t enable)
{
    rwFUNCTION();
    Get()->mD_ENABLEW = enable;
    rwRETURNVOID();
}


/*
*/
inline void Dmac::ChainToVif1(const void *source)
{
    rwFUNCTION();
    rwENFORCE(IsMemAligned(source, 16), ("Invalid address for DMA transfer."));
    Get()->mD_STAT  = CHANNEL_VIF1;
    Get()->mD1_TADR = reinterpret_cast<uint32_t>(source);
    Get()->mD1_QWC  = 0;
    Get()->mD1_CHCR = 0x145;
    asm volatile ("sync.l");
    rwRETURNVOID();
}


/*
*/
inline void Dmac::ChainToGif(const void *source)
{
    rwFUNCTION();
    rwENFORCE(IsMemAligned(source, 16), ("Invalid address for DMA transfer."));
    Get()->mD_STAT  = CHANNEL_GIF;
    Get()->mD2_TADR = reinterpret_cast<uint32_t>(source);
    Get()->mD2_QWC  = 0;
    Get()->mD2_CHCR = 0x104;
    asm volatile ("sync.l");
    rwRETURNVOID();
}


/*
*/
inline bool Dmac::HasCompleted(uint32_t channels)
{
    rwFUNCTION();
    rwRETURN(channels == (channels & Get()->mD_STAT));
}


/*
*/
inline void Dmac::Lock(uint32_t RWUNUSED_RELEASE(channels))
{
    rwFUNCTION();
#ifdef EA_DEBUG
    rwENFORCE(0 == (mLocked & channels),
        ("Cannot lock DMA channels %03x as channels %03x are already locked.", channels, mLocked & channels));
    mLocked |= channels;
#endif
    rwRETURNVOID();
}


/*
*/
inline void Dmac::NormalFromSpr(void *destination, const void *source, uint32_t size)
{
    rwFUNCTION();
    Get()->mD_STAT  = CHANNEL_FROMSPR;
    Get()->mD8_SADR = reinterpret_cast<uint32_t>(source);
    Get()->mD8_MADR = reinterpret_cast<uint32_t>(destination);
    Get()->mD8_QWC  = size;
    Get()->mD8_CHCR = 0x100;
    asm volatile ("sync.l");
    rwRETURNVOID();
}


/*
*/
inline void Dmac::NormalFromVif1(void *destination, uint32_t size)
{
    rwFUNCTION();
    Get()->mD_STAT  = CHANNEL_VIF1;
    Get()->mD1_MADR = reinterpret_cast<uint32_t>(destination);
    Get()->mD1_QWC  = size;
    Get()->mD1_CHCR = 0x100;
    asm volatile ("sync.l");
    rwRETURNVOID();
}


/*
Starts a transfer to and a transfer from scratchpad simultaneously. If the two transfers are started
independently the first transfer can start before the registers for the second transfer have been written to
the DMAC. In this case the EE-Core will stall until the first transfer has completed.
*/
inline void Dmac::NormalToFromSpr(void *toDestination, const void *toSource, uint32_t toSize,
                                  void *fromDestination, const void *fromSource, uint32_t fromSize)
{
    rwFUNCTION();
    Get()->mD_STAT = CHANNEL_FROMSPR | CHANNEL_TOSPR;
    SetEnable(ENABLE_OFF);
    Get()->mD8_SADR = reinterpret_cast<uint32_t>(fromSource);
    Get()->mD8_MADR = reinterpret_cast<uint32_t>(fromDestination);
    Get()->mD8_QWC  = fromSize;
    Get()->mD8_CHCR = 0x100;
    Get()->mD9_SADR = reinterpret_cast<uint32_t>(toDestination);
    Get()->mD9_MADR = reinterpret_cast<uint32_t>(toSource);
    Get()->mD9_QWC  = toSize;
    Get()->mD9_CHCR = 0x100;
    SetEnable(ENABLE_ON);
    asm volatile ("sync.l");
    rwRETURNVOID();
}


/*
*/
inline void Dmac::NormalToSpr(void *destination, const void *source, uint32_t size)
{
    rwFUNCTION();
    Get()->mD_STAT  = CHANNEL_TOSPR;
    Get()->mD9_SADR = reinterpret_cast<uint32_t>(destination);
    Get()->mD9_MADR = reinterpret_cast<uint32_t>(source);
    Get()->mD9_QWC  = size;
    Get()->mD9_CHCR = 0x100;
    asm volatile ("sync.l");
    rwRETURNVOID();
}


/*
*/
inline void Dmac::NormalToVif1(const void *source, uint32_t size)
{
    rwFUNCTION();
    Get()->mD_STAT  = CHANNEL_VIF1;
    Get()->mD1_MADR = reinterpret_cast<uint32_t>(source);
    Get()->mD1_QWC  = size;
    Get()->mD1_CHCR = 0x101;
    asm volatile ("sync.l");
    rwRETURNVOID();
}


/*
*/
inline void Dmac::Unlock(uint32_t RWUNUSED_RELEASE(channels))
{
    rwFUNCTION();
#ifdef EA_DEBUG
    rwENFORCE(channels == (mLocked & channels),
        ("Cannot unlock DMA channels %03x as channels %03x aren't locked.", channels, channels & ~mLocked));
    mLocked |= channels;
#endif
    rwRETURNVOID();
}


/*
*/
inline void Dmac::WaitForCompletion(uint32_t channels)
{
    rwFUNCTION();
    Get()->mD_PCR = channels;
    asm volatile ("sync.l");
    asm volatile ("sync.p");
    asm volatile ("1:");
    asm volatile ("bc0tl 2f");
    asm volatile ("bc0fl 1b");
    asm volatile ("2: ");

    // This shouldn't be necessary
    do {} while (!HasCompleted(channels));
    rwRETURNVOID();
}

} //namespace rw

#define RwDMAWaitForCompletion(channels) rw::Dmac::WaitForCompletion(channels)

inline uint32_t RwPS2AddressScratchpadToCoreInternal(uint32_t scratchpadAddress, uint32_t offset = 0)
{
    rwFUNCTION();
    rwASSERT(offset + (scratchpadAddress << 4) < 16384);
    rwRETURN(0x70000000 + offset + (scratchpadAddress << 4));
}

inline void *RwPS2AddressScratchpadToCore(uint32_t scratchpadAddress, void *offset)
{
    rwFUNCTION();
    rwRETURN(reinterpret_cast<void *>(RwPS2AddressScratchpadToCoreInternal(
        scratchpadAddress, 15 & reinterpret_cast<rw::HostPtr>(offset))));
}
inline uint32_t RwPS2AddressCoreToDmaCount(void *startAddress, void *endAddress)
{
    rwFUNCTION();
    rwRETURN(((15 + reinterpret_cast<rw::HostPtr>(endAddress)) >> 4)
        - (reinterpret_cast<rw::HostPtr>(startAddress) >> 4));
}

inline uint32_t RwPS2AddressCoreToDmaStart(void *startAddress)
{
    rwFUNCTION();
    rwRETURN(~15 & reinterpret_cast<rw::HostPtr>(startAddress));
}

inline uint32_t RwPS2AddressCoreToDmaCount(uint32_t startAddress, uint32_t endAddress)
{
    rwFUNCTION();
    rwRETURN(((15 + endAddress) >> 4) - (startAddress >> 4));
}
inline uint32_t RwPS2AddressScratchpadToCore(uint32_t scratchpadAddress, uint32_t offset)
{
    rwFUNCTION();
    rwRETURN(RwPS2AddressScratchpadToCoreInternal(scratchpadAddress, 15 & offset));
}
inline uint32_t RwPS2AddressCoreToDmaStart(uint32_t startAddress)
{
    rwFUNCTION();
    rwRETURN(~15 & startAddress);
}
inline uint32_t RwPS2AddressCoreToScratchpad(void *coreAddress)
{
    rwFUNCTION();
    rwASSERT(0x70000000 <= reinterpret_cast<rw::HostPtr>(coreAddress));
    rwASSERT(0x70004000 > reinterpret_cast<rw::HostPtr>(coreAddress));
    rwASSERT(0 == (15 & reinterpret_cast<rw::HostPtr>(coreAddress)));
    rwRETURN((reinterpret_cast<rw::HostPtr>(coreAddress) - 0x70000000) >> 4);
}
inline rw::HostPtr RwPS2AddressCachedToUncached(rw::HostPtr cachedAddress)
{
    rwFUNCTION();
    rwASSERT(0 == (0xf0000000 & cachedAddress));
    rwRETURN(cachedAddress + 0x20000000);
}
template <typename T> inline T *RwPS2AddressCachedToUncached(T *cachedAddress)
{
    rwFUNCTION();
    rwRETURN(reinterpret_cast<T *>(RwPS2AddressCachedToUncached(
        reinterpret_cast<rw::HostPtr>(cachedAddress))));
}
inline rw::HostPtr RwPS2AddressUncachedToCached(rw::HostPtr uncachedAddress)
{
    rwFUNCTION();
    rwASSERT(0x20000000 == (0xf0000000 & uncachedAddress));
    rwRETURN(uncachedAddress - 0x20000000);
}
template <typename T> inline T *RwPS2AddressUncachedToCached(T *uncachedAddress)
{
    rwFUNCTION();
    rwRETURN(reinterpret_cast<T *>(RwPS2AddressUncachedToCached(
        reinterpret_cast<rw::HostPtr>(uncachedAddress))));
}
inline rw::HostPtr RwPS2AddressCachedToUncachedAccelerated(rw::HostPtr cachedAddress)
{
    rwFUNCTION();
    rwASSERT(0 == (0xf0000000 & cachedAddress));
    rwRETURN(cachedAddress + 0x30000000);
}
template <typename T> inline T *RwPS2AddressCachedToUncachedAccelerated(T *cachedAddress)
{
    rwFUNCTION();
    rwRETURN(reinterpret_cast<T *>(RwPS2AddressCachedToUncachedAccelerated(
        reinterpret_cast<rw::HostPtr>(cachedAddress))));
}
inline rw::HostPtr RwPS2AddressUncachedAcceleratedToCached(rw::HostPtr uncachedAddress)
{
    rwFUNCTION();
    rwASSERT(0x30000000 == (0xf0000000 & uncachedAddress));
    rwRETURN(uncachedAddress - 0x30000000);
}
template <typename T> inline T *RwPS2AddressUncachedAcceleratedToCached(T *uncachedAddress)
{
    rwFUNCTION();
    rwRETURN(reinterpret_cast<T *>(RwPS2AddressUncachedAcceleratedToCached(
        reinterpret_cast<rw::HostPtr>(uncachedAddress))));
}

/* RWOLDPUBLICEND */

#endif /* RW_DMAC_H */
/* RWPUBLICEND */
