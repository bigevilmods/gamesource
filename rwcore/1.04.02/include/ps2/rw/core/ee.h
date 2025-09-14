// (c) Electronic Arts. All Rights Reserved.
/* RWPUBLIC */
/*************************************************************************************************************

 File: rwee.h

 Purpose: PS2 EE interface

 */

#ifndef RW_EE_H
#define RW_EE_H

/* RWOLDPUBLIC */

namespace rw
{


/*
*/
class Ee
{
public:

    static const uint32_t UNIT_VU0      = 0x01;
    static const uint32_t UNIT_VIF0FIFO = 0x02;
    static const uint32_t UNIT_VU1      = 0x04;
    static const uint32_t UNIT_VIF1FIFO = 0x08;
    static const uint32_t UNIT_GIFFIFO  = 0x10;

    static const uint32_t VPU_STAT_VBS0 = 0x0001;
    static const uint32_t VPU_STAT_VDS0 = 0x0002;
    static const uint32_t VPU_STAT_VTS0 = 0x0004;
    static const uint32_t VPU_STAT_VFS0 = 0x0008;
    static const uint32_t VPU_STAT_DIV0 = 0x0020;
    static const uint32_t VPU_STAT_IBS0 = 0x0080;
    static const uint32_t VPU_STAT_VBS1 = 0x0100;
    static const uint32_t VPU_STAT_VDS1 = 0x0200;
    static const uint32_t VPU_STAT_VTS1 = 0x0400;
    static const uint32_t VPU_STAT_VFS1 = 0x0800;
    static const uint32_t VPU_STAT_VGW1 = 0x1000;
    static const uint32_t VPU_STAT_DIV1 = 0x2000;
    static const uint32_t VPU_STAT_EFU1 = 0x4000;

    static const uint32_t VIF_STAT_FQC = 0x1f000000;

    static const uint32_t GIF_STAT_PATH3MASK = 0x00000002;
    static const uint32_t GIF_STAT_FQC       = 0x1f000000;

    static bool IsActive(uint32_t units);
    static bool IsGifPath3Masked();
    static void WaitForCompletion(uint32_t units);

private:
    static Ee *Get();

    // Timers
    volatile uint32_t  mT0_COUNT;      uint32_t pad0[3];
    volatile uint32_t  mT0_MODE;       uint32_t pad1[3];
    volatile uint32_t  mT0_COMP;       uint32_t pad2[3];
    volatile uint32_t  mT0_HOLD;       uint32_t pad3[499];
    volatile uint32_t  mT1_COUNT;      uint32_t pad4[3];
    volatile uint32_t  mT1_MODE;       uint32_t pad5[3];
    volatile uint32_t  mT1_COMP;       uint32_t pad6[3];
    volatile uint32_t  mT1_HOLD;       uint32_t pad7[499];
    volatile uint32_t  mT2_COUNT;      uint32_t pad8[3];
    volatile uint32_t  mT2_MODE;       uint32_t pad9[3];
    volatile uint32_t  mT2_COMP;       uint32_t pad10[503];
    volatile uint32_t  mT3_COUNT;      uint32_t pad11[3];
    volatile uint32_t  mT3_MODE;       uint32_t pad12[3];
    volatile uint32_t  mT3_COMP;       uint32_t pad13[503];

    // IPU
    volatile uint32_t  mIPU_CMD;       uint32_t pad14[3];
    volatile uint32_t  mIPU_CTRL;      uint32_t pad15[3];
    volatile uint32_t  mIPU_BP;        uint32_t pad16[3];
    volatile uint32_t  mIPU_TOP;       uint32_t pad17[1011];

    // GIF
    volatile uint32_t  mGIF_CTRL;      uint32_t pad18[3];
    volatile uint32_t  mGIF_MODE;      uint32_t pad19[3];
    volatile uint32_t  mGIF_STAT;      uint32_t pad20[3];
    volatile uint32_t  mGIF_TAG0;      uint32_t pad21[3];
    volatile uint32_t  mGIF_TAG1;      uint32_t pad22[3];
    volatile uint32_t  mGIF_TAG2;      uint32_t pad23[3];
    volatile uint32_t  mGIF_TAG3;      uint32_t pad24[3];
    volatile uint32_t  mGIF_CNT;       uint32_t pad25[3];
    volatile uint32_t  mGIF_P3CNT;     uint32_t pad26[3];
    volatile uint32_t  mGIF_P3TAG;     uint32_t pad27[475];

    // VIF0
    volatile uint32_t  mVIF0_STAT;     uint32_t pad28[3];
    volatile uint32_t  mVIF0_FBRST;    uint32_t pad29[3];
    volatile uint32_t  mVIF0_ERR;      uint32_t pad30[3];
    volatile uint32_t  mVIF0_MARK;     uint32_t pad31[3];
    volatile uint32_t  mVIF0_CYCLE;    uint32_t pad32[3];
    volatile uint32_t  mVIF0_MODE;     uint32_t pad33[3];
    volatile uint32_t  mVIF0_NUM;      uint32_t pad34[3];
    volatile uint32_t  mVIF0_MASK;     uint32_t pad35[3];
    volatile uint32_t  mVIF0_CODE;     uint32_t pad36[3];
    volatile uint32_t  mVIF0_ITOPS;    uint32_t pad37[15];
    volatile uint32_t  mVIF0_ITOP;     uint32_t pad38[11];
    volatile uint32_t  mVIF0_R0;       uint32_t pad39[3];
    volatile uint32_t  mVIF0_R1;       uint32_t pad40[3];
    volatile uint32_t  mVIF0_R2;       uint32_t pad41[3];
    volatile uint32_t  mVIF0_R3;       uint32_t pad42[3];
    volatile uint32_t  mVIF0_C0;       uint32_t pad43[3];
    volatile uint32_t  mVIF0_C1;       uint32_t pad44[3];
    volatile uint32_t  mVIF0_C2;       uint32_t pad45[3];
    volatile uint32_t  mVIF0_C3;       uint32_t pad46[163];

    // VIF1
    volatile uint32_t  mVIF1_STAT;     uint32_t pad47[3];
    volatile uint32_t  mVIF1_FBRST;    uint32_t pad48[3];
    volatile uint32_t  mVIF1_ERR;      uint32_t pad49[3];
    volatile uint32_t  mVIF1_MARK;     uint32_t pad50[3];
    volatile uint32_t  mVIF1_CYCLE;    uint32_t pad51[3];
    volatile uint32_t  mVIF1_MODE;     uint32_t pad52[3];
    volatile uint32_t  mVIF1_NUM;      uint32_t pad53[3];
    volatile uint32_t  mVIF1_MASK;     uint32_t pad54[3];
    volatile uint32_t  mVIF1_CODE;     uint32_t pad55[3];
    volatile uint32_t  mVIF1_ITOPS;    uint32_t pad56[3];
    volatile uint32_t  mVIF1_BASE;     uint32_t pad57[3];
    volatile uint32_t  mVIF1_OFST;     uint32_t pad58[3];
    volatile uint32_t  mVIF1_TOPS;     uint32_t pad59[3];
    volatile uint32_t  mVIF1_ITOP;     uint32_t pad60[3];
    volatile uint32_t  mVIF1_TOP;      uint32_t pad61[7];
    volatile uint32_t  mVIF1_R0;       uint32_t pad62[3];
    volatile uint32_t  mVIF1_R1;       uint32_t pad63[3];
    volatile uint32_t  mVIF1_R2;       uint32_t pad64[3];
    volatile uint32_t  mVIF1_R3;       uint32_t pad65[3];
    volatile uint32_t  mVIF1_C0;       uint32_t pad66[3];
    volatile uint32_t  mVIF1_C1;       uint32_t pad67[3];
    volatile uint32_t  mVIF1_C2;       uint32_t pad68[3];
    volatile uint32_t  mVIF1_C3;       uint32_t pad69[163];

    // FIFO
    volatile uint128_t mVIF0_FIFO;     uint128_t pad70[255];
    volatile uint128_t mVIF1_FIFO;     uint128_t pad71[255];
    volatile uint128_t mGIF_FIFO;      uint128_t pad72[255];
    volatile uint128_t mIPU_out_FIFO;
    volatile uint128_t mIPU_in_FIFO;   uint128_t pad73[254];
};


/*
*/
inline Ee *Ee::Get()
{
    rwFUNCTION();
    Ee *base;
    asm ("lui %0, 0x1000" : "=r" (base));
    rwRETURN(base);
}


/*
*/
inline bool Ee::IsActive(uint32_t units)
{
    rwFUNCTION();
    uint32_t vpuStat;

    if (0 != ((UNIT_VU0 | UNIT_VU1) & units))
    {
        asm volatile ("cfc2 %0, $vi29" : "=r" (vpuStat));
    }

    if (0 != (UNIT_VIF0FIFO & units) && 0 != (VIF_STAT_FQC  & Get()->mVIF0_STAT)) rwRETURN(true);
    if (0 != (UNIT_VU0      & units) && 0 != (VPU_STAT_VBS0 & vpuStat)          ) rwRETURN(true);
    if (0 != (UNIT_VIF1FIFO & units) && 0 != (VIF_STAT_FQC  & Get()->mVIF1_STAT)) rwRETURN(true);
    if (0 != (UNIT_VU1      & units) && 0 != (VPU_STAT_VBS1 & vpuStat)          ) rwRETURN(true);
    if (0 != (UNIT_GIFFIFO  & units) && 0 != (GIF_STAT_FQC  & Get()->mGIF_STAT) ) rwRETURN(true);
    rwRETURN(false);
}


/*
*/
inline bool Ee::IsGifPath3Masked()
{
    rwFUNCTION();
    rwRETURN(0 != (GIF_STAT_PATH3MASK & Get()->mGIF_STAT));
}


/*
*/
inline void Ee::WaitForCompletion(uint32_t units)
{
    rwFUNCTION();
    do {} while (IsActive(units));
    rwRETURNVOID();
}

} // namespace rw

/* RWOLDPUBLICEND */

#endif /* RW_EE_H */
/* RWPUBLICEND */
