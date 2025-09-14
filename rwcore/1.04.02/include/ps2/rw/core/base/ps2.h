// (c) Electronic Arts. All Rights Reserved.
/* RWPUBLIC */
/*************************************************************************************************************

    File: rwps2.h

    Purpose: A short description of the file.

 */

#ifndef RW_PS2_H
#define RW_PS2_H

/* RWOLDPUBLIC */
/*************************************************************************************************************
 Includes
 */

/*************************************************************************************************************
 Defines
 */
/*
DMA tags
\todoc
*/
#define rwDMA_ID_REFE (0)
#define rwDMA_ID_CNT  (1)
#define rwDMA_ID_NEXT (2)
#define rwDMA_ID_REF  (3)
#define rwDMA_ID_REFS (4)
#define rwDMA_ID_CALL (5)
#define rwDMA_ID_RET  (6)
#define rwDMA_ID_END  (7)

#define rwDMA_TAG(qwc, id, addr) ((uint64_t)(qwc) | ((uint64_t)(id) << 28) | ((uint64_t)(addr) << 32))

#define rwDMA_REFE(qwc, addr) rwDMA_TAG(qwc, rwDMA_ID_REFE, addr)
#define rwDMA_CNT(qwc)        rwDMA_TAG(qwc, rwDMA_ID_CNT, 0)
#define rwDMA_NEXT(qwc, addr) rwDMA_TAG(qwc, rwDMA_ID_NEXT, addr)
#define rwDMA_REF(qwc, addr)  rwDMA_TAG(qwc, rwDMA_ID_REF, addr)
#define rwDMA_REFS(qwc, addr) rwDMA_TAG(qwc, rwDMA_ID_REFS, addr)
#define rwDMA_CALL(qwc, addr) rwDMA_TAG(qwc, rwDMA_ID_CALL, addr)
#define rwDMA_RET(qwc)        rwDMA_TAG(qwc, rwDMA_ID_RET, 0)
#define rwDMA_END(qwc)        rwDMA_TAG(qwc, rwDMA_ID_END, 0)

/*
Control registers
*/
#define rwVPUSTAT_VBS0_M (0x1)
#define rwVPUSTAT_VDS0_M (0x2)
#define rwVPUSTAT_VTS0_M (0x4)
#define rwVPUSTAT_VFS0_M (0x8)
#define rwVPUSTAT_DIV0_M (0x20)
#define rwVPUSTAT_IBS0_M (0x80)
#define rwVPUSTAT_VBS1_M (0x100)
#define rwVPUSTAT_VDS1_M (0x200)
#define rwVPUSTAT_VTS1_M (0x400)
#define rwVPUSTAT_VFS1_M (0x800)
#define rwVPUSTAT_VGW1_M (0x1000)
#define rwVPUSTAT_DIV1_M (0x2000)
#define rwVPUSTAT_EFU1_M (0x4000)

/*
Memory sizes.
\todoc
*/
#define rwSPR_SIZE (1024)
#define rwVU0MEM_SIZE (256)
#define rwVU1MEM_SIZE (1024)
#define rwGSMEM_SIZE (512)

/* Addresses.
\todoc
*/
#define rwUNCACHED(x)      (0x20000000 + (uint32_t)(x))
#define rwUNCACHEDACCEL(x) (0x30000000 + (uint32_t)(x))
#define rwSCRATCHPAD(x)    (0x70000000 + (uint32_t)(x))

/*************************************************************************************************************
 Enums
 */
/*************************************************************************************************************
 Typedef Enums
 */

 /*************************************************************************************************************
 Typedef Structs + Unions
 */

/*************************************************************************************************************
 Typedef Function Pointers
 */

/*************************************************************************************************************
 Structs + Unions
 */

/*************************************************************************************************************
 Global Variables
 */

/*************************************************************************************************************
 Function prototypes
 */

/* RWOLDPUBLICEND */

#endif /* RW_PS2_H */
/* RWPUBLICEND */
