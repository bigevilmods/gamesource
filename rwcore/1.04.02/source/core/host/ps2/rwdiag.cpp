// (c) Electronic Arts. All Rights Reserved.
#include <eeregs.h>
#include "rw/core/base/ostypes.h"
#include "rw/core/base/ps2.h"
#include "rw/core/debug/debug.h"
#include "rw/core/diag.h"

static int32_t rwDiagHid = -1;

/* !!! For now we check ignore the interupt handlers. */
/* RWCHECKIGNORE */
/*
\todoc
*/
static int
rw_DiagGIFPath(int RWUNUSED(ca))
{
    rwFUNCTION();
    /* Colors for GIF paths. 1:Red/Yellow 2:Green 3:Blue */
    static uint32_t colors[4] = {0x000000, 0x0000ff, 0x00ff00, 0xff0000};
    uint32_t gifStat;
    uint8_t path;

    gifStat = DGET_GIF_STAT();

    path = (gifStat & GIF_STAT_APATH_M) >> GIF_STAT_APATH_O;
    if (1 == path && (gifStat & GIF_STAT_IP3_M))
        DPUT_GS_BGCOLOR(0x00ffff);
    else
        DPUT_GS_BGCOLOR(colors[path]);

    DPUT_T1_MODE(0x5c3);
    ExitHandler();
    rwRETURN(0);
}

static int
rw_DiagDMA(int RWUNUSED(ca))
{
    rwFUNCTION();
    uint32_t dmaStat;
    uint32_t color;

    dmaStat = DGET_D_STAT();
    color = 0;
    if (0 == (D_STAT_CIS0_M & dmaStat))
    {
        color |= 0xff;
    }
    if (0 == (D_STAT_CIS1_M & dmaStat))
    {
        color |= 0xff00;
    }
    if (0 == (D_STAT_CIS2_M & dmaStat))
    {
        color |= 0xff0000;
    }
    DPUT_GS_BGCOLOR(color);

    DPUT_T1_MODE(0x5c3);
    ExitHandler();
    rwRETURN(0);
}

static int
rw_DiagDMAWait(int RWUNUSED(ca))
{
    rwFUNCTION();
    uint32_t dmaPcr;
    uint32_t color;

    dmaPcr = DGET_D_PCR();
    color = 0;
    if (0 != (D_PCR_CPC0_M & dmaPcr))
    {
        color |= 0xff;
    }
    if (0 != (D_PCR_CPC1_M & dmaPcr))
    {
        color |= 0xff00;
    }
    if (0 != (D_PCR_CPC2_M & dmaPcr))
    {
        color |= 0xff0000;
    }
    DPUT_GS_BGCOLOR(color);

    DPUT_T1_MODE(0x5c3);
    ExitHandler();
    rwRETURN(0);
}

static int
rw_DiagVU1Status(int RWUNUSED(ca))
{
    rwFUNCTION();
    uint32_t vpuStat;
    uint32_t color;

    asm volatile ("cfc2.ni %0, $vi29" : "=r" (vpuStat));
    if (0 != (rwVPUSTAT_VGW1_M & vpuStat))
    {
        color = 0xff;
    }
    else if (0 != (rwVPUSTAT_VBS1_M & vpuStat))
    {
        color = 0xff00;
        if (0 != (rwVPUSTAT_DIV1_M & vpuStat))
        {
            color |= 0xff;
        }
        if (0 != (rwVPUSTAT_EFU1_M & vpuStat))
        {
            color |= 0xff0000;
        }
    }
    else
    {
        color = 0;
    }
    DPUT_GS_BGCOLOR(color);

    DPUT_T1_MODE(0x5c3);
    ExitHandler();
    rwRETURN(0);
}

static int
rw_DiagVIF1Status(int RWUNUSED(ca))
{
       rwFUNCTION();
    uint32_t vif1Stat;
    uint32_t color;

    vif1Stat = DGET_VIF1_STAT();
    if (0 != (VIF1_STAT_VGW_M & vif1Stat))
    {
        color = 0xff;
    }
    else if (0 != (VIF1_STAT_VEW_M & vif1Stat))
    {
        color = 0xff00;
    }
    else if (0 != (VIF1_STAT_VPS_M & vif1Stat))
    {
        color = 0xff0000;
    }
    else
    {
        color = 0;
    }
    DPUT_GS_BGCOLOR(color);

    DPUT_T1_MODE(0x5c3);
    ExitHandler();
    rwRETURN(0);
}

static int
rw_DiagVIF1FIFO(int RWUNUSED(ca))
{
    rwFUNCTION();
    uint32_t vif1FQC;

    vif1FQC = DGET_VIF1_STAT() >> VIF1_STAT_FQC_O;

    if (0 == vif1FQC)
        DPUT_GS_BGCOLOR(0x0000ff);
    else
        DPUT_GS_BGCOLOR((vif1FQC - 1) * 0x111111);

    DPUT_T1_MODE(0x5c3);
    ExitHandler();
    rwRETURN(0);
}
/* RWCHECKIGNOREEND */

static int (*rw_DiagTimerHandler[])(int) =
{
    rw_DiagGIFPath,
    rw_DiagDMA,
    rw_DiagDMAWait,
    rw_DiagVU1Status,
    rw_DiagVIF1Status,
    rw_DiagVIF1FIFO,
};

/*
\todoc
*/
void
RwDiagOpen(RwDiagFunction function)
{
    rwFUNCTION();

    rwASSERT(-1 == rwDiagHid);
    rwDiagHid = AddIntcHandler(INTC_TIM1, rw_DiagTimerHandler[function], 0);
    rwASSERT(-1 != rwDiagHid);
    DPUT_T1_COMP(1);
    DPUT_T1_MODE(0x5c3);
    EnableIntc(INTC_TIM1);

    rwRETURNVOID();
}

/*
\todoc
*/
void
RwDiagClose(void)
{
    rwFUNCTION();

    DisableIntc(INTC_TIM1);
    RemoveIntcHandler(INTC_TIM1, rwDiagHid);
#ifdef EA_DEBUG
    rwDiagHid = -1;
#endif

    rwRETURNVOID();
}


/*
 *****************************************************************************
 */
