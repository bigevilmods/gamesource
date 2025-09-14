// (c) 2000-2007 Electronic Arts Inc.
//
//
//		PSX2/PC IDEC Encoder/Decoder
//
//		Martin Griffiths, Bullfrog Productions Ltd.
//
//
//		Last Updated : December 6th 1999
//
//		V1.01
//

#include <stdio.h>
#include <stdlib.h>
#include "idec_decode.h"
#include "idec_encode.h"

#ifdef R5900

// PSX2 Decoder

#include <eeregs.h>
#include <eekernel.h>
#include <sifdev.h>
#include <libgraph.h>
#include <libipu.h>
#include "dma.h"


#define DMAFROMIPU_QWCHUNKSIZE (32768)				// QW chunksize From IPU DMA (decompress upto half a meg at a time)
#define MAX_BLOCKS (64)								// Enough blocks for a bitmap that is 1024 pixels

static int  BlockGifTags[(MAX_BLOCKS*12)+32]  __attribute__((aligned(64)));

static char *dma_source_ptr;
static int qword_totalsize;

//
//	IpuWait
//
//	Wait for IPU to be idle.
//

#define IpuWait()    while ((volatile int)DGET_IPU_CTRL() < 0)

//
//		IpuReset
//
//		Resets IPU, clearing fifo.
//

#define IpuReset()		DPUT_IPU_CTRL(IPU_CTRL_RST_M)


//
//		IpuBCLR - fifo initialisation
//
//		bp	bit position (initial bit position for fifo)
//

#define IpuBCLR(_bp)		DPUT_IPU_CMD(SCE_IPU_BCLR | _bp)


//
//		IpuIDEC - IDEC block decode
//
//      void IpuIDEC(
//           int ofm,
//           int dte,
//           int sgn,
//           int dtd,
//           int qsc,
//           int fb
//      )
//
//      ofm  Output Format
//           (0)		: RGB32
//           (1)		: RGB16
//      dte  Dither Enable
//           (0)		: No Dither	 
//           (1)		: Dither (only use when ofm = RGB16)
//      sgn  Pseudo Sign Offset
//           (0)		: offset 0	
//           (1)		: offset -128
//      dtd  DT Decode
//          (0)			: Dct Type (don't decode)
//          (1)			: Dct Type (decode)
//      qsc  Quantizer Step Code (5bit)
//      fb   Forward Bit (bits to skip)
//

static inline void IpuIDEC(int ofm, int dte, int sgn, int dtd, int qsc, int fb)
{	DPUT_IPU_CMD(SCE_IPU_IDEC
        | (ofm << 27)
        | (dte << 26)
        | (sgn << 25)
        | (dtd << 24)
        | (qsc << 16)
        | fb
    );
}

//
//		IpuBDEC	- BDEC block decode
//
//      void IpuBDEC(
//           int mbi,
//           int dcr,
//           int dt,
//           int qsc,
//           int fb
//      )
//
//      mbi  Macroblock Intra
//           (0)		: non-intra
//           (1)		: intra
//      dcr  DC Reset
//           (0)		: don't reset DC diff's
//           (1)		: reset DC diff's
//      dt   DCT Type
//           (0)		: frame DCT
//           (1)		: field DCT
//      qsc  Quantiser Step Code
//      fb   Forward Bit
//

static inline void IpuBDEC(int mbi, int dcr, int dt, int qsc, int fb)
{   DPUT_IPU_CMD(SCE_IPU_BDEC
        | (mbi << 27)
        | (dcr << 26)
        | (dt  << 25)
        | (qsc << 16)
        | fb
    );
}



// (simple) IDEC_decode (PS2)  (alpha channel is not decoded)
// image is output as a rectangular image, not as blocks.
// (this is done using the SQWC feature when DMA'ing from the scratchpad)
// returns 1 for success, 0 for failure.


int CDECL IDEC_decode( void *destbuffer, int destrowbytes,const void *sourcebuffer,int width, int height)
{	
	if ( (sourcebuffer) && (((IDEC_HEADER *)sourcebuffer)->id == IDEC_HEADER_ID))
	{	int i,j;

		IpuBCLR(64);	
		IpuWait();	

		if (((u_int)sourcebuffer>>28) == 0x7)							// DMA data to scratch pad??
		{	DPUT_D4_MADR(((u_int)sourcebuffer)|0x80000000);				// yes then set the appropriate flag
		} else
		{	DPUT_D4_MADR((u_int) sourcebuffer);	
		}
		DPUT_D4_QWC(((IDEC_HEADER *)sourcebuffer)->size>>4);	
		DPUT_D4_CHCR(D_CHCR_STR_M);		

		DPUT_IPU_CTRL(SCE_IPU_CTRL_PCT_IPIC|SCE_IPU_CTRL_MP1_COMPATIBLE);
				
		// Setup the skip quadword and transfer quadword register, ready for an interleaved transfer

		DPUT_D_SQWC(MAKE_SQWC((destrowbytes >> 4)-4,4));

		// setup IPU with decode format etc.

		IpuIDEC(
				0,			// ofm: RGB32
				0,			// dte: no dither
				0,			// sgn: no offset
				0,			// dtd: no dt decode
				2,			// qsc: dummy
				0			// fb
				);	

		u_long s = ADDR_SCRATCHPAD;

		for( i = 0 ; i < height ; i += 16 )
		{   
			for( j = 0 ; j < width ; j += 16 )
			{	ARGB* dst;	
		
				DPUT_D3_MADR(((u_int)s)|0x80000000);			
				DPUT_D3_QWC(sizeof(sceIpuRGB32)/16);						
				DPUT_D3_CHCR(D_CHCR_STR_M);		
				dst =  ((ARGB *)destbuffer+(i*(destrowbytes>>2))+j);	

				DPUT_D8_MADR((unsigned int )dst);						
				DPUT_D8_SADR((s));					
				DPUT_D8_QWC(sizeof(sceIpuRGB32)/16);
				DMA_Wait(D3_CHCR);					

				DPUT_D8_CHCR(D_CHCR_STR_M| D_CHCR_INTERL_MODE_M);		
				DMA_Wait(D8_CHCR);

			}
	
		}	

		return 1;
	}
}

// IDEC Init Decoder.

static int my_fromipu_dma_handler(int ch);


int CDECL IDEC_Init_Decoder(void)
{		DisableDmac(DMAC_FROM_IPU);
        AddDmacHandler(DMAC_FROM_IPU,(int (*)(int ch)) my_fromipu_dma_handler,0);
		EnableDmac(DMAC_FROM_IPU);
}




int CDECL IDEC_Load_Image(const unsigned char *sourcebuffer,const unsigned char *Image_Buffer,int dest_addr, int width, int height,int dest_width,int PSM,int dxoffset,int dyoffset, unsigned int flags)
{
	if ( (sourcebuffer) && (((IDEC_HEADER *)sourcebuffer)->id == IDEC_HEADER_ID))
	{	int ofm,j,quadwperblocks,area;
		unsigned char *alpha_channel;
		unsigned int alpha_channel_size;
		unsigned int sourcedma_size;
		unsigned int alpha_PSM;
		unsigned int bp;
		u_long *p;
		unsigned char * this_block;
	
		if (PSM == SCE_GS_PSMCT32)
		{	quadwperblocks = 4*16;
			ofm = 0;
		} else if (PSM == SCE_GS_PSMCT16)
		{	quadwperblocks = 2*16;
			ofm = 1;
		} else
			return 0;		// unknown format.

		area = width * height;
		sourcedma_size = ((IDEC_HEADER *)sourcebuffer)->size;

		if (flags & IDECLOADIMAGE_DMA_WAITBEFORE)
			DMA_Wait(D2_CHCR);

		
		bp = 64 + ((((unsigned int) sourcebuffer) & 15)*8); 
		if (bp >= 128)
		{	sourcebuffer +=16;
			bp -= 128;
		}
		
		IpuBCLR(bp);				// set bp to skip past our 8 byte header

		// Create the Source Chain GIF DMA Image Upload list.

		p= (u_long *)UNCACHEDADDRESSACCEL(BlockGifTags); 
		p[0] = MAKE_DMATAG(3,0,TAGID_CNT,0, 0x0);
		p[1] = 0;
		p[2] = SCE_GIF_SET_TAG(2,0,0,0,SCE_GIF_PACKED,1);				//(nloop, eop, pre, prim, flg, nreg)
		p[3] = 0xe;	
		p[4] = SCE_GS_SET_BITBLTBUF(0,0,0,dest_addr,dest_width>>6,PSM); // SCE_GS_SET_BITBLTBUF(sbp, sbw, spsm, dbp, dbw, dpsm)
		p[5] = SCE_GS_BITBLTBUF;
		p[6] = SCE_GS_SET_TRXREG(16,height);
		p[7] = SCE_GS_TRXREG;			
		p+=8;
		this_block = (unsigned char *) Image_Buffer;


		for( j = 0 ; j < width ; j += 16 )
		{	int size =  (quadwperblocks*height);

			p[0] = MAKE_DMATAG(4,0,TAGID_CNT,0, 0x0);
			p[1] = 0;
			p[2] = SCE_GIF_SET_TAG(2,0,0,0,SCE_GIF_PACKED,1);						//(nloop, eop, pre, prim, flg, nreg)
			p[3] = 0xe;
			p[4] = SCE_GS_SET_TRXPOS(0,0, (dxoffset + j), (dyoffset + 0), 0);
			p[5] = SCE_GS_TRXPOS;	
			p[6] = 0;																// host to local transfer 
			p[7] = SCE_GS_TRXDIR;													// writing initiates transfer!
			p[8] = SCE_GIF_SET_TAG(size>>4,0,0,0,SCE_GIF_IMAGE,0);					//(nloop, eop, pre, prim, flg, nreg)
			p[9] = 0;
			p[10] = MAKE_DMATAG(size>>4,0,TAGID_REFS  ,0, (u_int)(this_block));
			p[11] = 0;
			p+=12;
	
			this_block += (size);
		}
		
		
		if ((((IDEC_HEADER *)sourcebuffer)->flags & IDECFLAG_HASALPHA ) )
		{	alpha_channel_size = area;
			alpha_PSM = SCE_GS_PSMT8H;

			if ((((IDEC_HEADER *)sourcebuffer)->alpha_type == IDEC_ALPHA4BITRAW) )
			{	alpha_channel_size>>=1;
				alpha_PSM = SCE_GS_PSMT4HH;
			}

			sourcedma_size -= alpha_channel_size;
			alpha_channel = (unsigned char *)sourcebuffer+sourcedma_size;
			
			
			p[0] = MAKE_DMATAG(6,0,TAGID_CNT,0, 0x0);
			p[1] = 0;
			p[2] = SCE_GIF_SET_TAG(4,0,0,0,SCE_GIF_PACKED,1);				//(nloop, eop, pre, prim, flg, nreg)
			p[3] = 0xe;	
			p[4] = SCE_GS_SET_BITBLTBUF(0,0,0,dest_addr,dest_width>>6,alpha_PSM); // SCE_GS_SET_BITBLTBUF(sbp, sbw, spsm, dbp, dbw, dpsm)
			p[5] = SCE_GS_BITBLTBUF;
			p[6] = SCE_GS_SET_TRXREG(width,height);
			p[7] = SCE_GS_TRXREG;			
			p[8] = SCE_GS_SET_TRXPOS(0,0, (dxoffset ), (dyoffset ), 0);
			p[9] = SCE_GS_TRXPOS;	
			p[10] = 0;																// host to local transfer 
			p[11] = SCE_GS_TRXDIR;													// writing initiates transfer!
			p[12] = SCE_GIF_SET_TAG(alpha_channel_size>>4,1,0,0,SCE_GIF_IMAGE,0);//(nloop, eop, pre, prim, flg, nreg)
			p[13] = 0;
			p[14] = MAKE_DMATAG(alpha_channel_size>>4,0,TAGID_REFE  ,0, (u_int)(alpha_channel));
			p[15] = 0;
			
		} else
		{	p[0] = MAKE_DMATAG(0,0,TAGID_END  ,0, 0);
			p[1] = 0;
		}

	
		IpuWait();	

		DPUT_D4_MADR(((u_int) sourcebuffer) & 0xfffffff0);			
		DPUT_D4_QWC(sourcedma_size>>4);	
		DPUT_D4_CHCR(D_CHCR_STR_M);										// Kick off DMA to IPU						
		DPUT_IPU_CTRL(SCE_IPU_CTRL_PCT_IPIC|SCE_IPU_CTRL_MP1_COMPATIBLE);															
		IpuIDEC(ofm,0,	0,	0,	2,	0 );								//  IDEC IPU command
		DMAC_SET_STALL_CTRL(D_CTRL_STALLSRC_fromIPU,D_CTRL_STALLDST_GIF);// Set Stall Control.
		DPUT_D_STADR((unsigned int) (Image_Buffer));					// Set Stall Address
		DPUT_D3_MADR((((u_int)Image_Buffer)));							// DMA from to THIS buffer

		dma_source_ptr = (char *) Image_Buffer;
		qword_totalsize = quadwperblocks*(area>>8);
		
		if (qword_totalsize > DMAFROMIPU_QWCHUNKSIZE)
		{	DPUT_D3_QWC(DMAFROMIPU_QWCHUNKSIZE );	
			qword_totalsize -= DMAFROMIPU_QWCHUNKSIZE;
			dma_source_ptr += (16*DMAFROMIPU_QWCHUNKSIZE);
		}
		else
		{	DPUT_D3_QWC(qword_totalsize );		
			qword_totalsize = 0;
		}

	
		DPUT_D3_CHCR(D_CHCR_STR_M);										// Kick off DMA from IPU


		DPUT_D2_TADR((u_int)BlockGifTags);								// GIF Dmatags
		DPUT_D2_CHCR(D_CHCR_STR_M|D_CHCR_SCHAIN_MODE_M);				// trigger source-chain DMA
		

		if (flags & IDECLOADIMAGE_DMA_WAITCOMPLETED)
		{	DMA_Wait(D2_CHCR);					

		}
		

		return 1;
	}

}

// The interrupt handler for the END of DMA from the IPU

static int my_fromipu_dma_handler(int ch)
{
		if (qword_totalsize)								// any left to transfer
		{
			DPUT_D3_MADR((((u_int)dma_source_ptr)));			
			
			if (qword_totalsize > DMAFROMIPU_QWCHUNKSIZE)
			{	DPUT_D3_QWC(DMAFROMIPU_QWCHUNKSIZE );	
				qword_totalsize -= DMAFROMIPU_QWCHUNKSIZE;
				dma_source_ptr += (16*DMAFROMIPU_QWCHUNKSIZE);
			}
			else
			{	DPUT_D3_QWC(qword_totalsize );		
				qword_totalsize=0;
			}
			DPUT_D3_CHCR(D_CHCR_STR_M);						// Kick off DMA from IPU
		} else
		{	
			DMAC_SET_STALL_CTRL(D_CTRL_STALLSRC_NONE,D_CTRL_STALLSRC_NONE);// Set Stall Control.

		}

        return 0;

}


/*

static unsigned char test_buffer[4096] __attribute__((aligned(64)));

// IDEC_Load_Image (PS2)
// returns 1 for success, 0 for failure.

int CDECL IDEC_Load_Image( const void *sourcebuffer,int dest_addr, int width, int height,int dest_width,int PSM,int dxoffset,int dyoffset,unsigned int  spr_addr, unsigned int flags )
{	
	IDEC_HEADER *header = (IDEC_HEADER *) sourcebuffer;

	if ((header) && (header->id == IDEC_HEADER_ID))
	{	
		int ofm,i,j,bytesperblock,mblocks;
		
		if (PSM == SCE_GS_PSMCT32)
		{	bytesperblock = 4*16;
			ofm = 0;
		}
		else if (PSM == SCE_GS_PSMCT16)
		{	bytesperblock = 2*16;
			ofm = 1;
		}
		else
			return 0;

		IpuReset();			// reset IPU
		IpuWait();	
		IpuBCLR(64);	
		IpuWait();	

		u_long *p= (u_long *)(spr_addr+2048); 
		
		DPUT_D4_MADR((u_int) sourcebuffer);			
		DPUT_D4_QWC(header->size>>4);	
		DPUT_D4_CHCR(D_CHCR_STR_M);										// Kick off DMA to IPU						
		DPUT_IPU_CTRL(SCE_IPU_CTRL_PCT_IPIC|SCE_IPU_CTRL_MP1_COMPATIBLE);															
		IpuIDEC(ofm,0,	0,	0,	2,	0 );								// kickof IDEC IPU command


		p[0] = SCE_GIF_SET_TAG(2,0,0,0,SCE_GIF_PACKED,1);				//(nloop, eop, pre, prim, flg, nreg)
		p[1] = 0xe;	
		p[2] = SCE_GS_SET_BITBLTBUF(0,0,0,dest_addr,dest_width>>6,PSM); // SCE_GS_SET_BITBLTBUF(sbp, sbw, spsm, dbp, dbw, dpsm)
		p[3] = SCE_GS_BITBLTBUF;
		p[4] = SCE_GS_SET_TRXREG(16,16);
		p[5] = SCE_GS_TRXREG;	

		DPUT_D2_MADR(((u_int)p|0x80000000));			
		DPUT_D2_QWC(3);			
		DPUT_D2_CHCR(D_CHCR_STR_M);		
		DMA_Wait(D2_CHCR);


		p[0] = MAKE_DMATAG(4,0,TAGID_CNT,0, 0x0);
		p[1] = 0;
		p[2] = SCE_GIF_SET_TAG(2,0,0,0,SCE_GIF_PACKED,1);					//(nloop, eop, pre, prim, flg, nreg)
		p[3] = 0xe;
		p[4] = SCE_GS_SET_TRXPOS(0,0, 0, 0, 0);
		p[5] = SCE_GS_TRXPOS;	
		p[6] = 0;															// host to local transfer 
		p[7] = SCE_GS_TRXDIR;												// writing initiates transfer!
		p[8] = SCE_GIF_SET_TAG(bytesperblock,1,0,0,SCE_GIF_IMAGE,0);	//(nloop, eop, pre, prim, flg, nreg)
		p[9] = 0;
		p[10] = MAKE_DMATAG(bytesperblock,0,TAGID_REFS  ,0, ((u_int)(spr_addr)|0x80000000));
		p[11] = 0;
		p[12] = MAKE_DMATAG(0,0,TAGID_END  ,0, 0);
		p[13] = 0;

		DMAC_SET_STALL_CTRL(D_CTRL_STALLSRC_fromIPU,D_CTRL_STALLDST_GIF);
		for( j = 0 ; j < width ; j += 16 )
		{	for( i = 0 ; i < height ; i += 16 )
			{
				p[4] = SCE_GS_SET_TRXPOS(0,0, (dxoffset + j), (dyoffset + i), 0);
				DPUT_D_STADR((unsigned int) (spr_addr));
				DPUT_D2_TADR(((u_int)p|0x80000000));			
				DPUT_D3_MADR((((u_int)spr_addr)|0x80000000));			
				DPUT_D3_QWC(bytesperblock);						

				DPUT_D2_CHCR(D_CHCR_STR_M|D_CHCR_SCHAIN_MODE_M);		
				DPUT_D3_CHCR(D_CHCR_STR_M);		
				DMA_Wait(D2_CHCR);

			}
		}

	
		if ((header->flags & IDECFLAG_HASALPHA ) && (bytesperblock == 4))
		{	int firstblock =1;
			IpuWait();	
			sceIpuFDEC(32);

			p[4] = SCE_GS_SET_BITBLTBUF(0,0,0,dest_addr,dest_width>>6,SCE_GS_PSMT8H); // SCE_GS_SET_BITBLTBUF(sbp, sbw, spsm, dbp, dbw, dpsm)											
			p[16] = SCE_GIF_SET_TAG(16,1,0,0,SCE_GIF_IMAGE,0);							//(nloop, eop, pre, prim, flg, nreg)
			p[18] = MAKE_DMATAG(16,0,TAGID_REFE  ,0, ((u_int)(spr_addr+sizeof(sceIpuRAW16)))|0x80000000);
			
			for( i = 0 ; i < height ; i += 16 )
			{	for( j = 0 ; j < width ; j += 16 )
				{		
			
					IpuBDEC(
	                 1,					// non-intra=0,intra = 1
				     firstblock,		// dc reset (1 = reset)
			         0,					// dct type (0 = frame,1=field)
		             header->aquant,	// quantscale
	                 0					// forward bit
					);
				
				
					// Trigger DMA From the IPU
					DPUT_D3_MADR(((u_int)spr_addr)|0x80000000);			
					DPUT_D3_QWC(sizeof(sceIpuRAW16)/16);						
					DPUT_D3_CHCR(D_CHCR_STR_M);		
					p[12] = SCE_GS_SET_TRXPOS(0,0, (dxoffset + j), (dyoffset + i), 0);
					firstblock = 0;
					DMA_Wait(D3_CHCR);
					if (flags & IDECLOADIMAGE_8BITALPHA)
					{	
						__asm__ volatile (	"       .set    noreorder\n"
											"		li		$12,0\n"
											"		li		$13,255\n"
											"		li		$15,128\n"
											"		pextlh	$12,$12,$12\n"
											"		pextlh	$13,$13,$13\n"
											"		pextlh	$15,$15,$15\n"
											"		pextlw	$12,$12,$12\n"
											"		pextlw	$13,$13,$13\n"
											"		pextlw	$15,$15,$15\n"
											"		pcpyld	$12,$12,$12\n"
											"		pcpyld	$13,$13,$13\n"										
											"		pcpyld	$15,$15,$15\n"	
											"		li		$14,8\n"
											"_lp1:	lq		$8,((0*32)+0)(%0)\n" 
											"       addi    %1, %1, 32\n"
											"		lq		$9,((0*32)+16)(%0)\n" 
											"       addi    $14, $14, -1\n"
											"		lq		$10,((1*32)+0)(%0)\n" 
											"		psubh	$8,$8,$15\n"
											"		lq		$11,((1*32)+16)(%0)\n" 										
											"		psubh	$9,$9,$15\n"
											"		psubh	$10,$10,$15\n"
											"		psubh	$11,$11,$15\n"
										
											"		pminh	$8,$8,$13\n"
											"		pminh	$9,$9,$13\n"
											"		pmaxh	$8,$8,$12\n"
											"		pmaxh	$9,$9,$12\n"
											"		pminh	$10,$10,$13\n"
											"		pminh	$11,$11,$13\n"
											"		pmaxh	$10,$10,$12\n"
											"		pmaxh	$11,$11,$12\n"
											"		ppacb	$8,$9,$8\n"
											"		ppacb	$10,$11,$10\n"
											"       addi    %0, %0, 64\n"
											"		sq		$8,((0*16)-32)(%1)\n"	
											"       bgtz    $14, _lp1\n"
											"		sq		$10,((1*16)-32)(%1)\n"	
											"		addi	%0,%0,-(64*8)\n"
											"		addi	%1,%1,-(32*8)\n"
											"       .set    reorder\n"



					
						:
						: "r" (spr_addr),"r" (spr_addr+sizeof(sceIpuRAW16))
						:  "$8","$9","$10","$11","$12","$13","$14","$15"
						);  
					} else
					{
					


						__asm__ volatile (	"       .set    noreorder\n"
											"		li		$12,0\n"
											"		li		$13,127\n"
											"		li		$15,128\n"
											"		pextlh	$12,$12,$12\n"
											"		pextlh	$13,$13,$13\n"
											"		pextlh	$15,$15,$15\n"
											"		pextlw	$12,$12,$12\n"
											"		pextlw	$13,$13,$13\n"
											"		pextlw	$15,$15,$15\n"
											"		pcpyld	$12,$12,$12\n"
											"		pcpyld	$13,$13,$13\n"										
											"		pcpyld	$15,$15,$15\n"	
											"		li		$14,8\n"
											"_lp2:	lq		$8,((0*32)+0)(%0)\n" 
											"       addi    %1, %1, 32\n"
											"		lq		$9,((0*32)+16)(%0)\n" 
											"       addi    $14, $14, -1\n"
											"		lq		$10,((1*32)+0)(%0)\n" 
											"		psubh	$8,$8,$15\n"
											"		lq		$11,((1*32)+16)(%0)\n" 										
											"		psubh	$9,$9,$15\n"
											"		psubh	$10,$10,$15\n"
											"		psubh	$11,$11,$15\n"
											"		psrah	$8,$8,1\n"										
											"		psrah	$9,$9,1\n"
											"		psrah	$10,$10,1\n"
											"		psrah	$11,$11,1\n"
											"		pminh	$8,$8,$13\n"
											"		pminh	$9,$9,$13\n"
											"		pmaxh	$8,$8,$12\n"
											"		pmaxh	$9,$9,$12\n"
											"		pminh	$10,$10,$13\n"
											"		pminh	$11,$11,$13\n"
											"		pmaxh	$10,$10,$12\n"
											"		pmaxh	$11,$11,$12\n"
											"		ppacb	$8,$9,$8\n"
											"		ppacb	$10,$11,$10\n"
											"       addi    %0, %0, 64\n"
											"		sq		$8,((0*16)-32)(%1)\n"	
											"       bgtz    $14, _lp2\n"
											"		sq		$10,((1*16)-32)(%1)\n"	
											"		addi	%0,%0,-(64*8)\n"
											"		addi	%1,%1,-(32*8)\n"
											"       .set    reorder\n"
					
						:
						: "r" (spr_addr),"r" (spr_addr+sizeof(sceIpuRAW16))
						:  "$8","$9","$10","$11","$12","$13","$14","$15"
						);  

					}
					//IpuWait();		

					// Trigger Source Chain Gif DMA
					//printf("here...\n");							
					DPUT_D2_TADR((((u_int)p)|0x80000000));	
					DPUT_D2_CHCR( D_CHCR_STR_M|D_CHCR_SCHAIN_MODE_M);	
					DMA_Wait(D2_CHCR);
					
				}
		
			}	
	

		}

		return 1;
	}
}
*/


#else

// Here we have ...
// the non PS2 IDEC decoder - should be reasonably portable

#define ERROR (-1)
#define MACROBLOCK_INTRA                        1
#define MACROBLOCK_QUANT                        16

typedef struct {
  char run, level, len;
} IDEC_DCTtab;

typedef struct {
  char val, len;
} VLCtab;


static int quantizer_scale=6;
static short block[6][64];
static int dc_dct_pred[3];
static unsigned char Clip[1024];
static unsigned char * MBufPtr;
static unsigned int	mBufPos;
static short iclip[1024]; /* clipping table */
static const unsigned int bitmasks[24]={0x0,0x1,0x3,0x7,0xf,0x1f,0x3f,0x7f,0xff,0x1ff,0x3ff,0x7ff,0xfff,0x1fff,0x3fff,0x7fff,0xffff,0x1ffff,0x3ffff,0x7ffff,0xfffff,0x1fffff,0x3fffff,0x7fffff};

#ifdef _WIN32
#pragma warning( disable : 4035 )
unsigned __inline int bswap(unsigned int a)
{
	_asm
	{
		mov eax,a
		bswap eax		
	}	
}
#pragma warning( default : 4035 )
#else
unsigned int __inline bswap(unsigned int a)
{
	return ((a&0xff)<<24) | ((a&0xff00)<<8) | ((a>>8)&0xff00)| ((a>>24)&0xff);
}
#endif


int __inline GetBits(int n)
{	unsigned int val=(bswap(*(unsigned int*)(MBufPtr+(mBufPos>>3)))>>(-n-(mBufPos&7)))&bitmasks[n];mBufPos+=n;

	return val;
}

int __inline ShowBits(int n)
{	return (bswap(*(unsigned int*)(MBufPtr+(mBufPos>>3)))>>(-n-(mBufPos&7)))&bitmasks[n];
}

void __inline FlushBits(int n)
{	mBufPos+=n;
}




// zig-zag scan patterns
static unsigned char scan[64]=
{  
    0,1,8,16,9,2,3,10,17,24,32,25,18,11,4,5,
    12,19,26,33,40,48,41,34,27,20,13,6,7,14,21,28,
    35,42,49,56,57,50,43,36,29,22,15,23,30,37,44,51,
    58,59,52,45,38,31,39,46,53,60,61,54,47,55,62,63
}
;

// default intra quantization matrix 
static unsigned char default_intra_quantizer_matrix[64]=
{
  8, 16, 19, 22, 26, 27, 29, 34,
  16, 16, 22, 24, 27, 29, 34, 37,
  19, 22, 26, 27, 29, 34, 34, 38,
  22, 22, 26, 27, 29, 34, 37, 40,
  22, 26, 27, 29, 32, 35, 40, 48,
  26, 27, 29, 32, 35, 40, 48, 58,
  26, 27, 29, 34, 38, 46, 56, 69,
  27, 29, 35, 38, 46, 56, 69, 83
};


/* Table B-1, macroblock_address_increment, codes 00010 ... 011xx */
static VLCtab MBAtab1[16] =
{ {ERROR,0}, {ERROR,0}, {7,5}, {6,5}, {5,4}, {5,4}, {4,4}, {4,4},
  {3,3}, {3,3}, {3,3}, {3,3}, {2,3}, {2,3}, {2,3}, {2,3}
};

/* Table B-1, macroblock_address_increment, codes 00000011000 ... 0000111xxxx */
static VLCtab MBAtab2[104] =
{
  {33,11}, {32,11}, {31,11}, {30,11}, {29,11}, {28,11}, {27,11}, {26,11},
  {25,11}, {24,11}, {23,11}, {22,11}, {21,10}, {21,10}, {20,10}, {20,10},
  {19,10}, {19,10}, {18,10}, {18,10}, {17,10}, {17,10}, {16,10}, {16,10},
  {15,8},  {15,8},  {15,8},  {15,8},  {15,8},  {15,8},  {15,8},  {15,8},
  {14,8},  {14,8},  {14,8},  {14,8},  {14,8},  {14,8},  {14,8},  {14,8},
  {13,8},  {13,8},  {13,8},  {13,8},  {13,8},  {13,8},  {13,8},  {13,8},
  {12,8},  {12,8},  {12,8},  {12,8},  {12,8},  {12,8},  {12,8},  {12,8},
  {11,8},  {11,8},  {11,8},  {11,8},  {11,8},  {11,8},  {11,8},  {11,8},
  {10,8},  {10,8},  {10,8},  {10,8},  {10,8},  {10,8},  {10,8},  {10,8},
  {9,7},   {9,7},   {9,7},   {9,7},   {9,7},   {9,7},   {9,7},   {9,7},
  {9,7},   {9,7},   {9,7},   {9,7},   {9,7},   {9,7},   {9,7},   {9,7},
  {8,7},   {8,7},   {8,7},   {8,7},   {8,7},   {8,7},   {8,7},   {8,7},
  {8,7},   {8,7},   {8,7},   {8,7},   {8,7},   {8,7},   {8,7},   {8,7}
};

/* Table B-12, dct_dc_size_luminance, codes 00xxx ... 11110 */
static VLCtab DClumtab0[32] =
{ {1, 2}, {1, 2}, {1, 2}, {1, 2}, {1, 2}, {1, 2}, {1, 2}, {1, 2},
  {2, 2}, {2, 2}, {2, 2}, {2, 2}, {2, 2}, {2, 2}, {2, 2}, {2, 2},
  {0, 3}, {0, 3}, {0, 3}, {0, 3}, {3, 3}, {3, 3}, {3, 3}, {3, 3},
  {4, 3}, {4, 3}, {4, 3}, {4, 3}, {5, 4}, {5, 4}, {6, 5}, {ERROR, 0}
};

/* Table B-12, dct_dc_size_luminance, codes 111110xxx ... 111111111 */
static VLCtab DClumtab1[16] =
{ {7, 6}, {7, 6}, {7, 6}, {7, 6}, {7, 6}, {7, 6}, {7, 6}, {7, 6},
  {8, 7}, {8, 7}, {8, 7}, {8, 7}, {9, 8}, {9, 8}, {10,9}, {11,9}
};

/* Table B-13, dct_dc_size_chrominance, codes 00xxx ... 11110 */
static VLCtab DCchromtab0[32] =
{ {0, 2}, {0, 2}, {0, 2}, {0, 2}, {0, 2}, {0, 2}, {0, 2}, {0, 2},
  {1, 2}, {1, 2}, {1, 2}, {1, 2}, {1, 2}, {1, 2}, {1, 2}, {1, 2},
  {2, 2}, {2, 2}, {2, 2}, {2, 2}, {2, 2}, {2, 2}, {2, 2}, {2, 2},
  {3, 3}, {3, 3}, {3, 3}, {3, 3}, {4, 4}, {4, 4}, {5, 5}, {ERROR, 0}
};

/* Table B-13, dct_dc_size_chrominance, codes 111110xxxx ... 1111111111 */
static VLCtab DCchromtab1[32] =
{ {6, 6}, {6, 6}, {6, 6}, {6, 6}, {6, 6}, {6, 6}, {6, 6}, {6, 6},
  {6, 6}, {6, 6}, {6, 6}, {6, 6}, {6, 6}, {6, 6}, {6, 6}, {6, 6},
  {7, 7}, {7, 7}, {7, 7}, {7, 7}, {7, 7}, {7, 7}, {7, 7}, {7, 7},
  {8, 8}, {8, 8}, {8, 8}, {8, 8}, {9, 9}, {9, 9}, {10,10}, {11,10}
};

/* Table B-14, DCT coefficients table zero,
 * codes 0100 ... 1xxx (used for first (DC) coefficient)
 */
IDEC_DCTtab IDEC_DCTtabfirst[12] =
{
  {0,2,4}, {2,1,4}, {1,1,3}, {1,1,3},
  {0,1,1}, {0,1,1}, {0,1,1}, {0,1,1},
  {0,1,1}, {0,1,1}, {0,1,1}, {0,1,1}
};

/* Table B-14, DCT coefficients table zero,
 * codes 0100 ... 1xxx (used for all other coefficients)
 */
IDEC_DCTtab IDEC_DCTtabnext[12] =
{
  {0,2,4},  {2,1,4},  {1,1,3},  {1,1,3},
  {64,0,2}, {64,0,2}, {64,0,2}, {64,0,2}, /* EOB */
  {0,1,2},  {0,1,2},  {0,1,2},  {0,1,2}
};

/* Table B-14, DCT coefficients table zero,
 * codes 000001xx ... 00111xxx
 */
IDEC_DCTtab IDEC_DCTtab0[60] =
{
  {65,0,6}, {65,0,6}, {65,0,6}, {65,0,6}, /* Escape */
  {2,2,7}, {2,2,7}, {9,1,7}, {9,1,7},
  {0,4,7}, {0,4,7}, {8,1,7}, {8,1,7},
  {7,1,6}, {7,1,6}, {7,1,6}, {7,1,6},
  {6,1,6}, {6,1,6}, {6,1,6}, {6,1,6},
  {1,2,6}, {1,2,6}, {1,2,6}, {1,2,6},
  {5,1,6}, {5,1,6}, {5,1,6}, {5,1,6},
  {13,1,8}, {0,6,8}, {12,1,8}, {11,1,8},
  {3,2,8}, {1,3,8}, {0,5,8}, {10,1,8},
  {0,3,5}, {0,3,5}, {0,3,5}, {0,3,5},
  {0,3,5}, {0,3,5}, {0,3,5}, {0,3,5},
  {4,1,5}, {4,1,5}, {4,1,5}, {4,1,5},
  {4,1,5}, {4,1,5}, {4,1,5}, {4,1,5},
  {3,1,5}, {3,1,5}, {3,1,5}, {3,1,5},
  {3,1,5}, {3,1,5}, {3,1,5}, {3,1,5}
};

/* Table B-15, DCT coefficients table one,
 * codes 000001xx ... 11111111
*/
IDEC_DCTtab IDEC_DCTtab0a[252] =
{
  {65,0,6}, {65,0,6}, {65,0,6}, {65,0,6}, /* Escape */
  {7,1,7}, {7,1,7}, {8,1,7}, {8,1,7},
  {6,1,7}, {6,1,7}, {2,2,7}, {2,2,7},
  {0,7,6}, {0,7,6}, {0,7,6}, {0,7,6},
  {0,6,6}, {0,6,6}, {0,6,6}, {0,6,6},
  {4,1,6}, {4,1,6}, {4,1,6}, {4,1,6},
  {5,1,6}, {5,1,6}, {5,1,6}, {5,1,6},
  {1,5,8}, {11,1,8}, {0,11,8}, {0,10,8},
  {13,1,8}, {12,1,8}, {3,2,8}, {1,4,8},
  {2,1,5}, {2,1,5}, {2,1,5}, {2,1,5},
  {2,1,5}, {2,1,5}, {2,1,5}, {2,1,5},
  {1,2,5}, {1,2,5}, {1,2,5}, {1,2,5},
  {1,2,5}, {1,2,5}, {1,2,5}, {1,2,5},
  {3,1,5}, {3,1,5}, {3,1,5}, {3,1,5},
  {3,1,5}, {3,1,5}, {3,1,5}, {3,1,5},
  {1,1,3}, {1,1,3}, {1,1,3}, {1,1,3},
  {1,1,3}, {1,1,3}, {1,1,3}, {1,1,3},
  {1,1,3}, {1,1,3}, {1,1,3}, {1,1,3},
  {1,1,3}, {1,1,3}, {1,1,3}, {1,1,3},
  {1,1,3}, {1,1,3}, {1,1,3}, {1,1,3},
  {1,1,3}, {1,1,3}, {1,1,3}, {1,1,3},
  {1,1,3}, {1,1,3}, {1,1,3}, {1,1,3},
  {1,1,3}, {1,1,3}, {1,1,3}, {1,1,3},
  {64,0,4}, {64,0,4}, {64,0,4}, {64,0,4}, /* EOB */
  {64,0,4}, {64,0,4}, {64,0,4}, {64,0,4},
  {64,0,4}, {64,0,4}, {64,0,4}, {64,0,4},
  {64,0,4}, {64,0,4}, {64,0,4}, {64,0,4},
  {0,3,4}, {0,3,4}, {0,3,4}, {0,3,4},
  {0,3,4}, {0,3,4}, {0,3,4}, {0,3,4},
  {0,3,4}, {0,3,4}, {0,3,4}, {0,3,4},
  {0,3,4}, {0,3,4}, {0,3,4}, {0,3,4},
  {0,1,2}, {0,1,2}, {0,1,2}, {0,1,2},
  {0,1,2}, {0,1,2}, {0,1,2}, {0,1,2},
  {0,1,2}, {0,1,2}, {0,1,2}, {0,1,2},
  {0,1,2}, {0,1,2}, {0,1,2}, {0,1,2},
  {0,1,2}, {0,1,2}, {0,1,2}, {0,1,2},
  {0,1,2}, {0,1,2}, {0,1,2}, {0,1,2},
  {0,1,2}, {0,1,2}, {0,1,2}, {0,1,2},
  {0,1,2}, {0,1,2}, {0,1,2}, {0,1,2},
  {0,1,2}, {0,1,2}, {0,1,2}, {0,1,2},
  {0,1,2}, {0,1,2}, {0,1,2}, {0,1,2},
  {0,1,2}, {0,1,2}, {0,1,2}, {0,1,2},
  {0,1,2}, {0,1,2}, {0,1,2}, {0,1,2},
  {0,1,2}, {0,1,2}, {0,1,2}, {0,1,2},
  {0,1,2}, {0,1,2}, {0,1,2}, {0,1,2},
  {0,1,2}, {0,1,2}, {0,1,2}, {0,1,2},
  {0,1,2}, {0,1,2}, {0,1,2}, {0,1,2},
  {0,2,3}, {0,2,3}, {0,2,3}, {0,2,3},
  {0,2,3}, {0,2,3}, {0,2,3}, {0,2,3},
  {0,2,3}, {0,2,3}, {0,2,3}, {0,2,3},
  {0,2,3}, {0,2,3}, {0,2,3}, {0,2,3},
  {0,2,3}, {0,2,3}, {0,2,3}, {0,2,3},
  {0,2,3}, {0,2,3}, {0,2,3}, {0,2,3},
  {0,2,3}, {0,2,3}, {0,2,3}, {0,2,3},
  {0,2,3}, {0,2,3}, {0,2,3}, {0,2,3},
  {0,4,5}, {0,4,5}, {0,4,5}, {0,4,5},
  {0,4,5}, {0,4,5}, {0,4,5}, {0,4,5},
  {0,5,5}, {0,5,5}, {0,5,5}, {0,5,5},
  {0,5,5}, {0,5,5}, {0,5,5}, {0,5,5},
  {9,1,7}, {9,1,7}, {1,3,7}, {1,3,7},
  {10,1,7}, {10,1,7}, {0,8,7}, {0,8,7},
  {0,9,7}, {0,9,7}, {0,12,8}, {0,13,8},
  {2,3,8}, {4,2,8}, {0,14,8}, {0,15,8}
};

/* Table B-14, DCT coefficients table zero,
 * codes 0000001000 ... 0000001111
 */
IDEC_DCTtab IDEC_DCTtab1[8] =
{
  {16,1,10}, {5,2,10}, {0,7,10}, {2,3,10},
  {1,4,10}, {15,1,10}, {14,1,10}, {4,2,10}
};

/* Table B-15, DCT coefficients table one,
 * codes 000000100x ... 000000111x
 */
IDEC_DCTtab IDEC_DCTtab1a[8] =
{
  {5,2,9}, {5,2,9}, {14,1,9}, {14,1,9},
  {2,4,10}, {16,1,10}, {15,1,9}, {15,1,9}
};

/* Table B-14/15, DCT coefficients table zero / one,
 * codes 000000010000 ... 000000011111
 */
IDEC_DCTtab IDEC_DCTtab2[16] =
{
  {0,11,12}, {8,2,12}, {4,3,12}, {0,10,12},
  {2,4,12}, {7,2,12}, {21,1,12}, {20,1,12},
  {0,9,12}, {19,1,12}, {18,1,12}, {1,5,12},
  {3,3,12}, {0,8,12}, {6,2,12}, {17,1,12}
};

/* Table B-14/15, DCT coefficients table zero / one,
 * codes 0000000010000 ... 0000000011111
 */
IDEC_DCTtab IDEC_DCTtab3[16] =
{
  {10,2,13}, {9,2,13}, {5,3,13}, {3,4,13},
  {2,5,13}, {1,7,13}, {1,6,13}, {0,15,13},
  {0,14,13}, {0,13,13}, {0,12,13}, {26,1,13},
  {25,1,13}, {24,1,13}, {23,1,13}, {22,1,13}
};

/* Table B-14/15, DCT coefficients table zero / one,
 * codes 00000000010000 ... 00000000011111
 */
IDEC_DCTtab IDEC_DCTtab4[16] =
{
  {0,31,14}, {0,30,14}, {0,29,14}, {0,28,14},
  {0,27,14}, {0,26,14}, {0,25,14}, {0,24,14},
  {0,23,14}, {0,22,14}, {0,21,14}, {0,20,14},
  {0,19,14}, {0,18,14}, {0,17,14}, {0,16,14}
};

/* Table B-14/15, DCT coefficients table zero / one,
 * codes 000000000010000 ... 000000000011111
 */
IDEC_DCTtab IDEC_DCTtab5[16] =
{
  {0,40,15}, {0,39,15}, {0,38,15}, {0,37,15},
  {0,36,15}, {0,35,15}, {0,34,15}, {0,33,15},
  {0,32,15}, {1,14,15}, {1,13,15}, {1,12,15},
  {1,11,15}, {1,10,15}, {1,9,15}, {1,8,15}
};

/* Table B-14/15, DCT coefficients table zero / one,
 * codes 0000000000010000 ... 0000000000011111
 */
IDEC_DCTtab IDEC_DCTtab6[16] =
{
  {1,18,16}, {1,17,16}, {1,16,16}, {1,15,16},
  {6,3,16}, {16,2,16}, {15,2,16}, {14,2,16},
  {13,2,16}, {12,2,16}, {11,2,16}, {31,1,16},
  {30,1,16}, {29,1,16}, {28,1,16}, {27,1,16}
};


static int Get_macroblock_address_increment()
{
  int code, val;


  val = 0;

  while ((code = ShowBits(11))<24)
  {
    if (code!=15) /* if not macroblock_stuffing */
    {
      if (code==8) /* if macroblock_escape */
      {

        val+= 33;
      }
      else
      {
      
        return 1;
      }
    }
    else /* macroblock suffing */
    {
    }

    FlushBits(11);
  }

  if (code>=1024)
  {
    FlushBits(1);
    return val + 1;
  }

  if (code>=128)
  {
    code >>= 6;
    FlushBits(MBAtab1[code].len);
    
    return val + MBAtab1[code].val;
  }
  
  code-= 24; /* remove common base */
  FlushBits(MBAtab2[code].len);


  return val + MBAtab2[code].val;
}


static int Get_Luma_DC_dct_diff()
{
  int code, size, dct_diff;


  /* decode length */
  code = ShowBits(5);

  if (code<31)
  {
    size = DClumtab0[code].val;
    FlushBits(DClumtab0[code].len);
  }
  else
  {
    code = ShowBits(9) - 0x1f0;
    size = DClumtab1[code].val;
    FlushBits(DClumtab1[code].len);

  }


  if (size==0)
    dct_diff = 0;
  else
  {
    dct_diff = GetBits(size);
    if ((dct_diff & (1<<(size-1)))==0)
      dct_diff-= (1<<size) - 1;
  }


  return dct_diff;
}


static int Get_Chroma_DC_dct_diff()
{
  int code, size, dct_diff;

  code = ShowBits(5);

  if (code<31)
  {
    size = DCchromtab0[code].val;
    FlushBits(DCchromtab0[code].len);

  }
  else
  {
    code = ShowBits(10) - 0x3e0;
    size = DCchromtab1[code].val;
    FlushBits(DCchromtab1[code].len);

  }


  if (size==0)
    dct_diff = 0;
  else
  {
    dct_diff = GetBits(size);
    if ((dct_diff & (1<<(size-1)))==0)
      dct_diff-= (1<<size) - 1;
  }


  return dct_diff;
}

//
// IDCT
//

#define W1 2841 /* 2048*sqrt(2)*cos(1*pi/16) */
#define W2 2676 /* 2048*sqrt(2)*cos(2*pi/16) */
#define W3 2408 /* 2048*sqrt(2)*cos(3*pi/16) */
#define W5 1609 /* 2048*sqrt(2)*cos(5*pi/16) */
#define W6 1108 /* 2048*sqrt(2)*cos(6*pi/16) */
#define W7 565  /* 2048*sqrt(2)*cos(7*pi/16) */



static void idctrow(short *blk)
{
  int x0, x1, x2, x3, x4, x5, x6, x7, x8;

  /* shortcut */
  if (!((x1 = blk[4]<<11) | (x2 = blk[6]) | (x3 = blk[2]) |
        (x4 = blk[1]) | (x5 = blk[7]) | (x6 = blk[5]) | (x7 = blk[3])))
  {
    blk[0]=blk[1]=blk[2]=blk[3]=blk[4]=blk[5]=blk[6]=blk[7]=blk[0]<<3;
    return;
  }

  x0 = (blk[0]<<11) + 128; /* for proper rounding in the fourth stage */

  /* first stage */
  x8 = W7*(x4+x5);
  x4 = x8 + (W1-W7)*x4;
  x5 = x8 - (W1+W7)*x5;
  x8 = W3*(x6+x7);
  x6 = x8 - (W3-W5)*x6;
  x7 = x8 - (W3+W5)*x7;
  
  /* second stage */
  x8 = x0 + x1;
  x0 -= x1;
  x1 = W6*(x3+x2);
  x2 = x1 - (W2+W6)*x2;
  x3 = x1 + (W2-W6)*x3;
  x1 = x4 + x6;
  x4 -= x6;
  x6 = x5 + x7;
  x5 -= x7;
  
  /* third stage */
  x7 = x8 + x3;
  x8 -= x3;
  x3 = x0 + x2;
  x0 -= x2;
  x2 = (181*(x4+x5)+128)>>8;
  x4 = (181*(x4-x5)+128)>>8;
  
  /* fourth stage */
  blk[0] = (x7+x1)>>8;
  blk[1] = (x3+x2)>>8;
  blk[2] = (x0+x4)>>8;
  blk[3] = (x8+x6)>>8;
  blk[4] = (x8-x6)>>8;
  blk[5] = (x0-x4)>>8;
  blk[6] = (x3-x2)>>8;
  blk[7] = (x7-x1)>>8;
}


static void idctcol(short *blk)
{
  int x0, x1, x2, x3, x4, x5, x6, x7, x8;

  ///* shortcut */
  if (!((x1 = (blk[8*4]<<8)) | (x2 = blk[8*6]) | (x3 = blk[8*2]) |
        (x4 = blk[8*1]) | (x5 = blk[8*7]) | (x6 = blk[8*5]) | (x7 = blk[8*3])))
  {
    blk[8*0]=blk[8*1]=blk[8*2]=blk[8*3]=blk[8*4]=blk[8*5]=blk[8*6]=blk[8*7]=
      iclip[((blk[8*0]+32)>>6)+512];
    return;
  }

  x0 = (blk[8*0]<<8) + 8192;

  /* first stage */
  x8 = W7*(x4+x5) + 4;
  x4 = (x8+(W1-W7)*x4)>>3;
  x5 = (x8-(W1+W7)*x5)>>3;
  x8 = W3*(x6+x7) + 4;
  x6 = (x8-(W3-W5)*x6)>>3;
  x7 = (x8-(W3+W5)*x7)>>3;
  
  /* second stage */
  x8 = x0 + x1;
  x0 -= x1;
  x1 = W6*(x3+x2) + 4;
  x2 = (x1-(W2+W6)*x2)>>3;
  x3 = (x1+(W2-W6)*x3)>>3;
  x1 = x4 + x6;
  x4 -= x6;
  x6 = x5 + x7;
  x5 -= x7;
  
  /* third stage */
  x7 = x8 + x3;
  x8 -= x3;
  x3 = x0 + x2;
  x0 -= x2;
  x2 = (181*(x4+x5)+128)>>8;
  x4 = (181*(x4-x5)+128)>>8;
  
  /* fourth stage */
  blk[8*0] = iclip[((x7+x1)>>14)+512];
  blk[8*1] = iclip[((x3+x2)>>14)+512];
  blk[8*2] = iclip[((x0+x4)>>14)+512];
  blk[8*3] = iclip[((x8+x6)>>14)+512];
  blk[8*4] = iclip[((x8-x6)>>14)+512];
  blk[8*5] = iclip[((x0-x4)>>14)+512];
  blk[8*6] = iclip[((x3-x2)>>14)+512];
  blk[8*7] = iclip[((x7-x1)>>14)+512];
}

// Fast two dimensional inverse discrete cosine transform

static void Fast_IDCT(short *block)
{
    idctrow(block+8*0);
    idctrow(block+8*1);
    idctrow(block+8*2);
    idctrow(block+8*3);
    idctrow(block+8*4);
    idctrow(block+8*5);
    idctrow(block+8*6);
    idctrow(block+8*7);

    idctcol(block+0);
    idctcol(block+1);
    idctcol(block+2);
    idctcol(block+3);
    idctcol(block+4);
    idctcol(block+5);
    idctcol(block+6);
    idctcol(block+7);

}


// decode intra block 
static void Decode_Block_MPEG1(int comp)
{
  int val, i, j, sign;
  unsigned int code;
  IDEC_DCTtab *tab;
  short *bp= &block[comp][0];

  /* ISO/IEC 11172-2 section 2.4.3.7: Block layer. */
  /* decode DC coefficients */
  if (comp<4)
    bp[0] = (dc_dct_pred[0]+=Get_Luma_DC_dct_diff()) << 3;
  else if (comp==4)
    bp[0] = (dc_dct_pred[1]+=Get_Chroma_DC_dct_diff()) << 3;
  else
    bp[0] = (dc_dct_pred[2]+=Get_Chroma_DC_dct_diff()) << 3;


  /* decode AC coefficients */
  for (i=1; ; i++)
  {
    code = ShowBits(16);
    if (code>=16384)
      tab = &IDEC_DCTtabnext[(code>>12)-4];
    else if (code>=1024)
      tab = &IDEC_DCTtab0[(code>>8)-4];
    else if (code>=512)
      tab = &IDEC_DCTtab1[(code>>6)-8];
    else if (code>=256)
      tab = &IDEC_DCTtab2[(code>>4)-16];
    else if (code>=128)
      tab = &IDEC_DCTtab3[(code>>3)-16];
    else if (code>=64)
      tab = &IDEC_DCTtab4[(code>>2)-16];
    else if (code>=32)
      tab = &IDEC_DCTtab5[(code>>1)-16];
    else if (code>=16)
      tab = &IDEC_DCTtab6[code-16];
    else
    {
   
      return;
    }

    FlushBits(tab->len);

    if (tab->run==64) /* end_of_block */
      return;

    if (tab->run==65) /* escape */
    {
      i+= GetBits(6);

      val = GetBits(8);
      if (val==0)
        val = GetBits(8);
      else if (val==128)
        val = GetBits(8) - 256;
      else if (val>128)
        val -= 256;

      if((sign = (val<0)))
        val = -val;
    }
    else
    {
      i+= tab->run;
      val = tab->level;
      sign = GetBits(1);
    }

    if (i>=64)
    {
      return;
    }

    j = scan[i];
    val = (val*quantizer_scale*default_intra_quantizer_matrix[j]) >> 3;

 
    /* saturation */
    if (!sign)
      bp[j] = (val>2047) ?  2047 :  val; /* positive */
    else
      bp[j] = (val>2048) ? -2048 : -val; /* negative */
  }
}


static void Decode_Block_MPEG2(int comp,int intra_vlc_format)
{
  int val, i, j, sign,run;
  unsigned int code;
  IDEC_DCTtab *tab;
  short *bp= &block[comp][0];

  /* ISO/IEC 11172-2 section 2.4.3.7: Block layer. */
  /* decode DC coefficients */
  if (comp<4)
    bp[0] = (dc_dct_pred[0]+=Get_Luma_DC_dct_diff()) << 3;
  else if (comp==4)
    bp[0] = (dc_dct_pred[1]+=Get_Chroma_DC_dct_diff()) << 3;
  else
    bp[0] = (dc_dct_pred[2]+=Get_Chroma_DC_dct_diff()) << 3;


  /* decode AC coefficients */
  for (i=1; ; i++)
  {
	code = ShowBits(16);
    if (code>=16384 && !intra_vlc_format)
      tab = &IDEC_DCTtabnext[(code>>12)-4];
    else if (code>=1024)
    {
      if (intra_vlc_format)
        tab = &IDEC_DCTtab0a[(code>>8)-4];
      else
        tab = &IDEC_DCTtab0[(code>>8)-4];
    }
    else if (code>=512)
    {
      if (intra_vlc_format)
        tab = &IDEC_DCTtab1a[(code>>6)-8];
      else
        tab = &IDEC_DCTtab1[(code>>6)-8];
    }
    else if (code>=256)
      tab = &IDEC_DCTtab2[(code>>4)-16];
    else if (code>=128)
      tab = &IDEC_DCTtab3[(code>>3)-16];
    else if (code>=64)
      tab = &IDEC_DCTtab4[(code>>2)-16];
    else if (code>=32)
      tab = &IDEC_DCTtab5[(code>>1)-16];
    else if (code>=16)
      tab = &IDEC_DCTtab6[code-16];
    else
    {
      return;
    }

    FlushBits(tab->len);


    if (tab->run==64) /* end_of_block */
    {
      return;
    }

    if (tab->run==65) /* escape */
    {

		run = GetBits(6);
		i+= run;

      val = GetBits(12);
      if ((val&2047)==0)
      {
        return;
      }
      if((sign = (val>=2048)))
        val = 4096 - val;
    }
    else
    {
      i+= run = tab->run;
      val = tab->level;
      sign = GetBits(1);

	}
    if (i>=64)
    {
      return;
    }

    j = scan[i];
    val = (val*quantizer_scale*default_intra_quantizer_matrix[j]) >> 3;

    /* saturation */
    if (!sign)
      bp[j] = (val>2047) ?  2047 :  val; /* positive */
    else
      bp[j] = (val>2048) ? -2048 : -val; /* negative */
  }
}





// YUV to RGB

static void yuv_to_rgb( ARGB* data, int stride, int width,int index, int n, int m )
{  
    int     i,j;

	for( i = 0 ; i < 8 ; i+=2 )
	{	ARGB *data2 = data + (stride>>2);

		for( j = 0 ; j < 8 ; j+=2 )
		{	short *blk = &block[index][i*8+j];
			int u =block[4][((i>>1)+n)*8+(j>>1)+m];
			int v =block[5][((i>>1)+n)*8+(j>>1)+m];		
			int vr = v * 104595;
			int vg = v * 53281;
			int ug = u * 25625;
			int ub = u * 132252;
			int y = (76284*(blk[0])-16);
			data[j].a =255;
			data[j].r = Clip[(((y + vr)+32768)>>16)+512]; 
			data[j].b = Clip[(((y + ub)+32768)>>16)+512];
			data[j].g = Clip[(((y - ug - vg)+32768)>>16)+512];
			y = (76284*(blk[1])-16);
			data[j+1].a =255;
			data[j+1].r = Clip[(((y + vr)+32768)>>16)+512]; 
			data[j+1].b = Clip[(((y + ub)+32768)>>16)+512];
			data[j+1].g = Clip[(((y - ug - vg)+32768)>>16)+512];
			y = (76284*(blk[8])-16);
			data2[j].a =255;
			data2[j].r = Clip[(((y + vr)+32768)>>16)+512]; 
			data2[j].b = Clip[(((y + ub)+32768)>>16)+512];
			data2[j].g = Clip[(((y - ug - vg)+32768)>>16)+512];
			y = (76284*(blk[9])-16);
			data2[j+1].a =255;
			data2[j+1].r = Clip[(((y + vr)+32768)>>16)+512]; 
			data2[j+1].b = Clip[(((y + ub)+32768)>>16)+512];
			data2[j+1].g = Clip[(((y - ug - vg)+32768)>>16)+512];
		}

		data += ((stride>>2)*2);
	}
   
}

static void yuv_to_rgb_alpha4( ARGB* data, int stride, int width, int index, int n, int m,unsigned char *alpha )
{
    int     i,j;

	for( i = 0 ; i < 8 ; i+=2 )
	{	ARGB *data2 = data + (stride>>2);
		unsigned char *alpha2 = alpha + (width>>1);
		for( j = 0 ; j < 8 ; j+=2 )
		{	short *blk = &block[index][i*8+j];
			int u =block[4][((i>>1)+n)*8+(j>>1)+m];
			int v =block[5][((i>>1)+n)*8+(j>>1)+m];
			int vr = v * 104595;
			int vg = v * 53281;
			int ug = u * 25625;
			int ub = u * 132252;
			int y = (76284*(blk[0])-16);
			data[j+1].a =alpha[j>>1] << 4;
			data[j].r = Clip[(((y + vr)+32768)>>16)+512];
			data[j].b = Clip[(((y + ub)+32768)>>16)+512];
			data[j].g = Clip[(((y - ug - vg)+32768)>>16)+512];
			y = (76284*(blk[1])-16);
			data[j].a =alpha[j>>1] & 0xf0;
			data[j+1].r = Clip[(((y + vr)+32768)>>16)+512];
			data[j+1].b = Clip[(((y + ub)+32768)>>16)+512];
			data[j+1].g = Clip[(((y - ug - vg)+32768)>>16)+512];
			y = (76284*(blk[8])-16);
			data2[j+1].a =alpha2[j>>1] <<4;
			data2[j].r = Clip[(((y + vr)+32768)>>16)+512];
			data2[j].b = Clip[(((y + ub)+32768)>>16)+512];
			data2[j].g = Clip[(((y - ug - vg)+32768)>>16)+512];
			y = (76284*(blk[9])-16);
			data2[j].a =alpha2[j>>1] & 0xf0;
			data2[j+1].r = Clip[(((y + vr)+32768)>>16)+512];
			data2[j+1].b = Clip[(((y + ub)+32768)>>16)+512];
			data2[j+1].g = Clip[(((y - ug - vg)+32768)>>16)+512];
		}
		alpha += ((width>>1)*2);
		data += ((stride>>2)*2);
	}

}


static void yuv_to_rgb_alpha8( ARGB* data, int stride, int width,int index, int n, int m,unsigned char *alpha )
{
    int     i,j;

	for( i = 0 ; i < 8 ; i+=2 )
	{	ARGB *data2 = data + (stride>>2);
		unsigned char *alpha2 = alpha + width;
		for( j = 0 ; j < 8 ; j+=2 )
		{	short *blk = &block[index][i*8+j];
			int u =block[4][((i>>1)+n)*8+(j>>1)+m];
			int v =block[5][((i>>1)+n)*8+(j>>1)+m];		
			int vr = v * 104595;
			int vg = v * 53281;
			int ug = u * 25625;
			int ub = u * 132252;
			int y = (76284*(blk[0])-16);
			data[j].a =alpha[j];
			data[j].r = Clip[(((y + vr)+32768)>>16)+512];
			data[j].b = Clip[(((y + ub)+32768)>>16)+512];
			data[j].g = Clip[(((y - ug - vg)+32768)>>16)+512];
			y = (76284*(blk[1])-16);
			data[j+1].a =alpha[j+1];
			data[j+1].r = Clip[(((y + vr)+32768)>>16)+512]; 
			data[j+1].b = Clip[(((y + ub)+32768)>>16)+512];
			data[j+1].g = Clip[(((y - ug - vg)+32768)>>16)+512];
			y = (76284*(blk[8])-16);
			data2[j].a =alpha2[j];
			data2[j].r = Clip[(((y + vr)+32768)>>16)+512]; 
			data2[j].b = Clip[(((y + ub)+32768)>>16)+512];
			data2[j].g = Clip[(((y - ug - vg)+32768)>>16)+512];
			y = (76284*(blk[9])-16);
			data2[j+1].a =alpha2[j+1];
			data2[j+1].r = Clip[(((y + vr)+32768)>>16)+512]; 
			data2[j+1].b = Clip[(((y + ub)+32768)>>16)+512];
			data2[j+1].g = Clip[(((y - ug - vg)+32768)>>16)+512];
		}
		alpha += (width*2);
		data += ((stride>>2)*2);
	}
   
}



static void __inline Clear_Block(int comp)
{	int *Block_Ptr = (int *)block[comp];
	Block_Ptr[0 ] = Block_Ptr[0 +1]  = Block_Ptr[0 +2] = Block_Ptr[0 +3] = Block_Ptr[0 +4] = Block_Ptr[0 +5]  = Block_Ptr[0 +6] = Block_Ptr[0 +7] = 
	Block_Ptr[8 ] = Block_Ptr[8 +1]  = Block_Ptr[8 +2] = Block_Ptr[8 +3] = Block_Ptr[8 +4] = Block_Ptr[8 +5]  = Block_Ptr[8 +6] = Block_Ptr[8 +7] = 
	Block_Ptr[16] = Block_Ptr[16+1]  = Block_Ptr[16+2] = Block_Ptr[16+3] = Block_Ptr[16+4] = Block_Ptr[16+5]  = Block_Ptr[16+6] = Block_Ptr[16+7] = 
	Block_Ptr[24] = Block_Ptr[24+1]  = Block_Ptr[24+2] = Block_Ptr[24+3] = Block_Ptr[24+4] = Block_Ptr[24+5]  = Block_Ptr[24+6] = Block_Ptr[24+7] = 0;

}



static void idec_decodeblock( int lastblock,int mpeg2,int intra_vlc_format )
{
    int  i,get_quant = 0;

	if (!GetBits(1))
	{	if (GetBits(1))
		{	get_quant = 1;
		}
	}

	if (get_quant)
		quantizer_scale = GetBits(5);				// QSC

#ifdef DEBUG
//	printf("quantizer_scale %d\n",quantizer_scale);
#endif


    for( i = 0 ; i < 6 ; ++i )
    {	Clear_Block(i);
		if (!mpeg2)
			Decode_Block_MPEG1(i);
		else
			Decode_Block_MPEG2(i,intra_vlc_format);

		Fast_IDCT( &block[i][0] );
    }

	if (!lastblock)
	{	Get_macroblock_address_increment();
	}
	
}





static void Initialize_Decoder()
{	int i;

	/* Clip table */
	for (i=0; i<1023; i++)
        Clip[i] = (i<384) ? 0 : ((i>(255+384)) ? 255 : i-384);

	for (i= -512; i<512; i++)
		iclip[i+512] = (i<-256) ? -256 : ((i>255) ? 255 : i);
}



// IDEC_decode
// returns 1 for success, 0 for failure.


int CDECL IDEC_decode( void *destbuffer, int destrowbytes,const void *sourcebuffer,int width, int height, int flags)
{	static int init = 0;
    int i,j;
	int real_width,real_height;
	int macroblock_count;
	ARGB *image = (ARGB*) destbuffer;
	IDEC_HEADER *header = (IDEC_HEADER *) sourcebuffer;
	unsigned char *alpha_channel;

	if (!init)
	{	Initialize_Decoder();
		init = 1;
	}

	if (header)
	{

		if (flags & IDECFLAG_HASALPHA )
		{
			unsigned int source_size = header->size;
			unsigned int alpha_channel_size = width*height;

			if (header->alpha_type == IDEC_ALPHA4BITRAW)
			{	alpha_channel_size>>=1;
			}

			source_size -= alpha_channel_size;
			alpha_channel = (unsigned char *)sourcebuffer+source_size;
		}

		real_width = width;
		real_height = height;

		width =  ((width)+15)   & 0xfffffff0;
		height = ((height)+15) & 0xfffffff0;
		macroblock_count = (width>>4) * (height>>4);

		MBufPtr = ((unsigned char *)sourcebuffer)+8;
		mBufPos = 0;
		dc_dct_pred[0] = dc_dct_pred[1] = dc_dct_pred[2] = 0;

		if (flags & IDECFLAG_SONYIPU)
        {
  			for( i = 0 ; i < height ; i += 16 )
  			{
                for( j = 0 ; j < width ; j += 16 )
        		{
    				ARGB*     dst;

   				    dst = &image[i*(destrowbytes>>2)+j];
    				int lastblock = (macroblock_count == 1);
					idec_decodeblock( lastblock,(flags & IDECFLAG_MPEG2) ? 1 : 0,(flags & IDECFLAG_INTRAVLC) ? 1 : 0);
    				yuv_to_rgb( dst,           destrowbytes, width, 0, 0, 0 );
    				yuv_to_rgb( dst+8,         destrowbytes, width, 1, 0, 4 );
    				yuv_to_rgb( dst+(8*(destrowbytes>>2)),   destrowbytes, width, 2, 4, 0 );
    				yuv_to_rgb( dst+(8*(destrowbytes>>2))+8, destrowbytes, width, 3, 4, 4 );
    				macroblock_count--;
    		    }
		    }
        }
        else
        {
            for( j = 0 ; j < width ; j += 16 )
    		{
    			for( i = 0 ; i < height ; i += 16 )
    			{
    				ARGB*     dst;

  				    dst = &image[i*(destrowbytes>>2)+j];
    				int lastblock = (macroblock_count == 1);
    				idec_decodeblock( lastblock,0,0 );
    				if (flags & IDECFLAG_HASALPHA )
    				{
    					if (header->alpha_type == IDEC_ALPHA4BITRAW)
    					{	unsigned char *alphasrc= &alpha_channel[i*(width>>1)+(j>>1)];

    						yuv_to_rgb_alpha4( dst,          destrowbytes, width, 0, 0, 0,alphasrc );
    						yuv_to_rgb_alpha4( dst+8,        destrowbytes, width, 1, 0, 4,alphasrc+4  );
    						yuv_to_rgb_alpha4( dst+(8*(destrowbytes>>2)),  destrowbytes, width, 2, 4, 0,alphasrc+(8*(width>>1))  );
    						yuv_to_rgb_alpha4( dst+(8*(destrowbytes>>2))+8,destrowbytes, width, 3, 4, 4,alphasrc+4+(8*(width>>1))  );
    					} else
    					{
    						unsigned char *alphasrc= &alpha_channel[i*(width)+j];
    						yuv_to_rgb_alpha8( dst,          destrowbytes, width, 0, 0, 0,alphasrc  );
    						yuv_to_rgb_alpha8( dst+8,        destrowbytes, width, 1, 0, 4,alphasrc+8  );
    						yuv_to_rgb_alpha8( dst+(8*(destrowbytes>>2)),  destrowbytes, width, 2, 4, 0,alphasrc+(8*width)  );
    						yuv_to_rgb_alpha8( dst+(8*(destrowbytes>>2))+8,destrowbytes, width, 3, 4, 4,alphasrc+8+(8*width) );
    					}
    				} else
    				{
    					yuv_to_rgb( dst,           destrowbytes, width, 0, 0, 0 );
    					yuv_to_rgb( dst+8,         destrowbytes, width, 1, 0, 4 );
    					yuv_to_rgb( dst+(8*(destrowbytes>>2)),   destrowbytes, width, 2, 4, 0 );
    					yuv_to_rgb( dst+(8*(destrowbytes>>2))+8, destrowbytes, width, 3, 4, 4 );
    				}

    				macroblock_count--;
    		    }
		    }
        }

		return 1;
	}

	return 0;

}



#endif


// IDEC_is_idec
// returns 1 for success, 0 for failure.

int CDECL IDEC_is_idec(const void *sourcebuffer)
{	IDEC_HEADER *header = (IDEC_HEADER *) sourcebuffer;
	return (header->id == IDEC_HEADER_ID);

}

// IDEC_decodeinfo
// returns 1 for success, 0 for failure.

int CDECL IDEC_decodeinfo(const void *sourcebuffer, int *width, int *height,int *alpha_depth,int *quant, int *compressedsize)
{	IDEC_HEADER *header = (IDEC_HEADER *) sourcebuffer;
	if (header->id == IDEC_HEADER_ID)
	{	*width = ((int) header->width) << 4;
		*height = ((int) header->height) << 4;
		*compressedsize = ((int) header->size);
		if (header->flags & IDECFLAG_HASALPHA)
		{	*alpha_depth = (header->alpha_type == IDEC_ALPHA8BITRAW) ? 8 : 4;
		} else
			*alpha_depth = 0;

		*quant = ((*(((unsigned char *) sourcebuffer) + 8)) >> 1) & 0x1f;
		return 1;
	} else
		return 0;
}

