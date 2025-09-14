// (c) 1994-2007 Electronic Arts Inc.

#include "nilgimex.h"
#include "lbmgimex.h"

#include "pshgimex.h"
#include "fshgimex.h"
#include "xshgimex.h"
#include "xssgimex.h"
#include "gssgimex.h"
#include "sssgimex.h"
#include "fssgimex.h"
#include "mssgimex.h"
#include "gshgimex.h"
#include "mshgimex.h"
#include "yshgimex.h"
#include "zshgimex.h"
#include "sshgimex.h"
#include "ashgimex.h"

#include "oldpshgimex.h"
#include "oldfshgimex.h"
#include "oldxshgimex.h"
#include "oldxssgimex.h"
#include "oldgssgimex.h"
#include "oldsssgimex.h"
#include "oldfssgimex.h"
#include "oldmssgimex.h"
#include "oldgshgimex.h"
#include "oldmshgimex.h"
#include "oldsshgimex.h"
#include "oldashgimex.h"

#include "bmpgimex.h"
#include "tgagimex.h"
#include "anmgimex.h"
#include "flmgimex.h"
//#include "twngimex.h"
#include "gmxgimex.h"
#include "hsigimex.h"
#include "sctgimex.h"
//#include "wctgimex.h"
#include "simgimex.h"
#include "actgimex.h"
#include "ffngimex.h"
#include "pfngimex.h"
//#include "vshgimex.h"
#include "sfngimex.h"
//#include "nfngimex.h"
#include "gfngimex.h"
#include "xfngimex.h"
#include "mfngimex.h"
#include "yfngimex.h"
#include "zfngimex.h"
//#include "ffsgimex.h"

#include "picgimex.h"
#include "tgvgimex.h"
#include "pcxgimex.h"
#include "gifgimex.h"
//#include "3dfgimex.h"
//#include "vqtgimex.h"
#include "fligimex.h"
#include "sgigimex.h"
#include "ppmgimex.h"
#include "rlagimex.h"
#include "psdgimex.h"
#include "strgimex.h"
#include "movgimex.h"
#include "avigimex.h"
#include "vp6gimex.h"
#include "pixgimex.h"
//#include "mvigimex.h"
#include "timgimex.h"
#include "tplgimex.h"
#include "tm2gimex.h"
#include "bnrgimex.h"
//#include "txpgimex.h"
#include "bifgimex.h"
//#include "cmpgimex.h"
#include "epsgimex.h"
#include "rawgimex.h"
//#include "rgsgimex.h"
//#include "invgimex.h"
//#include "vrmgimex.h"
//#include "nshgimex.h"
//#include "bamgimex.h"
#include "yuvgimex.h"
#include "icogimex.h"
//#include "s3tgimex.h"
#include "ddsgimex.h"
#include "xprgimex.h"
#include "3prgimex.h"
#include "gtfgimex.h"
//#include "mggimex.h"
//#include "ipugimex.h"
//#include "laygimex.h"
#include "newgimex.h"
#include "jp2gimex.h"
#include "exegimex.h"
#include "ramgimex.h"
#include "madgimex.h"

#if (defined(_MSC_VER) && !defined(_XBOX)) || defined(__unix__) || defined(__APPLE__)
//#include "celgimex.h"
#include "jpggimex.h"
#include "mpggimex.h"
#include "tifgimex.h"
//#include "vangimex.h"
//#include "omfgimex.h"
//#include "uvgimex.h"
#include "pnggimex.h"
#include "qmtgimex.h"
//#include "pvrgimex.h"
//#include "matgimex.h"
#endif

#if (defined(_MSC_VER) && !defined(_XBOX))
#include "ttfgimex.h"
#include "ijlgimex.h"
#include "bikgimex.h"
//#include "djvgimex.h"
#include "qt3gimex.h"
#include "clpgimex.h"
//want this one #include "aigimex.h"
//#include "tqigimex.h"
#endif

struct GFUNCTIONS gfunctions[]=
{

// ***************************************************************************
// These formats are on all platforms: PC, linux, sgi, consoles
// ***************************************************************************

    {
        NIL_about,
        NIL_is,
        NIL_open,
        NIL_info,
        NIL_read,
        NIL_close,
        NIL_wopen,
        NIL_write,
        NIL_wclose,
    },

// ***************************************************************************
// JPeg import only
// ***************************************************************************

#if defined(J)
    // JPG is bigger, but self contained
    { JPG_about,JPG_is,JPG_open,JPG_info,JPG_read,JPG_close,JPG_wopen,JPG_write,JPG_wclose },
//    IJL_about,IJL_is,IJL_open,IJL_info,IJL_read,IJL_close,
//    IJL_wopen,IJL_write,IJL_wclose, // export
    // NULL,NULL,NULL, // no export
#else

// ***************************************************************************
// All platforms + RT library
// ***************************************************************************

    { BMP_about,BMP_is,BMP_open,BMP_info,BMP_read,BMP_close,BMP_wopen,BMP_write,BMP_wclose },
    { TGA_about,TGA_is,TGA_open,TGA_info,TGA_read,TGA_close,TGA_wopen,TGA_write,TGA_wclose },
//    WCT_about,WCT_is,WCT_open,WCT_info,WCT_read,WCT_close,WCT_wopen,WCT_write,WCT_wclose, */
    { GMX_about,GMX_is,GMX_open,GMX_info,GMX_read,GMX_close,GMX_wopen,GMX_write,GMX_wclose },
    { MOV_about,MOV_is,MOV_open,MOV_info,MOV_read,MOV_close,MOV_wopen,MOV_write,MOV_wclose },

    { LBM_about,LBM_is,LBM_open,LBM_info,LBM_read,LBM_close,LBM_wopen,LBM_write,LBM_wclose },
    { HSI_about,HSI_is,HSI_open,HSI_info,HSI_read,HSI_close,HSI_wopen,HSI_write,HSI_wclose },

	{ PSD_about,PSD_is,PSD_open,PSD_info,PSD_read,PSD_close,PSD_wopen,PSD_write,PSD_wclose },
    { ANM_about,ANM_is,ANM_open,ANM_info,ANM_read,ANM_close,ANM_wopen,ANM_write,ANM_wclose },
    { FLM_about,FLM_is,FLM_open,FLM_info,FLM_read,FLM_close,FLM_wopen,FLM_write,FLM_wclose },
    { BIF_about,BIF_is,BIF_open,BIF_info,BIF_read,BIF_close,BIF_wopen,BIF_write,BIF_wclose },
    { PCX_about,PCX_is,PCX_open,PCX_info,PCX_read,PCX_close,PCX_wopen,PCX_write,PCX_wclose },
    { PPM_about,PPM_is,PPM_open,PPM_info,PPM_read,PPM_close,PPM_wopen,PPM_write,PPM_wclose },
    { FLI_about,FLI_is,FLI_open,FLI_info,FLI_read,FLI_close,FLI_wopen,FLI_write,FLI_wclose },
    { FFN_about,FFN_is,FFN_open,FFN_info,FFN_read,FFN_close,FFN_wopen,FFN_write,FFN_wclose },
    { AVI_about,AVI_is,AVI_open,AVI_info,AVI_read,AVI_close,AVI_wopen,AVI_write,AVI_wclose },
    { MAD_about,MAD_is,MAD_open,MAD_info,MAD_read,MAD_close,MAD_wopen,MAD_write,MAD_wclose },

// ***************************************************************************
// Only on Windows you say?  Pity
// ***************************************************************************

#if (defined(_MSC_VER) && !defined(_XBOX))
    { TTF_about,TTF_is,TTF_open,TTF_info,TTF_read,TTF_close,TTF_wopen,TTF_write,TTF_wclose },
    { IJL_about,IJL_is,IJL_open,IJL_info,IJL_read,IJL_close,IJL_wopen,IJL_write,IJL_wclose },
    { BIK_about,BIK_is,BIK_open,BIK_info,BIK_read,BIK_close,BIK_wopen,BIK_write,BIK_wclose },
//    DJV_about,DJV_is,DJV_open,DJV_info,DJV_read,DJV_close,DJV_wopen,DJV_write,DJV_wclose,
//    TWN_about,TWN_is,TWN_open,TWN_info,TWN_read,TWN_close,NIL_wopen,NIL_write,NIL_wclose,
    { QT3_about,QT3_is,QT3_open,QT3_info,QT3_read,QT3_close,QT3_wopen,QT3_write,QT3_wclose },
//    TQI_about,TQI_is,TQI_open,TQI_info,TQI_read,TQI_close,TQI_wopen,TQI_write,TQI_wclose,
    { CLP_about,CLP_is,CLP_open,CLP_info,CLP_read,CLP_close,CLP_wopen,CLP_write,CLP_wclose },
#if !defined(RT)
//    AI_about,AI_is,AI_open,AI_info,AI_read,AI_close,AI_wopen,AI_write,AI_wclose,
//    OMF_about,OMF_is,OMF_open,OMF_info,OMF_read,OMF_close,OMF_wopen,OMF_write,OMF_wclose,
    { JP2_about,JP2_is,JP2_open,JP2_info,JP2_read,JP2_close,JP2_wopen,JP2_write,JP2_wclose },
    { EXE_about,EXE_is,EXE_open,EXE_info,EXE_read,EXE_close,EXE_wopen,EXE_write,EXE_wclose },
    { RAM_about,RAM_is,RAM_open,RAM_info,RAM_read,RAM_close,RAM_wopen,RAM_write,RAM_wclose },
#endif
#endif

// ***************************************************************************
// All platforms, but NOT RT library
// ***************************************************************************

#if !defined(RT)    /* not included for copyright/patent/license reasons */
    { VP6_about,VP6_is,VP6_open,VP6_info,VP6_read,VP6_close,VP6_wopen,VP6_write,VP6_wclose },
    { PIX_about,PIX_is,PIX_open,PIX_info,PIX_read,PIX_close,PIX_wopen,PIX_write,PIX_wclose },
    { RLA_about,RLA_is,RLA_open,RLA_info,RLA_read,RLA_close,RLA_wopen,RLA_write,RLA_wclose },
    { ACT_about,ACT_is,ACT_open,ACT_info,ACT_read,ACT_close,ACT_wopen,ACT_write,ACT_wclose },
    { SGI_about,SGI_is,SGI_open,SGI_info,SGI_read,SGI_close,SGI_wopen,SGI_write,SGI_wclose },
    { SCT_about,SCT_is,SCT_open,SCT_info,SCT_read,SCT_close,SCT_wopen,SCT_write,SCT_wclose },
    { TPL_about,TPL_is,TPL_open,TPL_info,TPL_read,TPL_close,TPL_wopen,TPL_write,TPL_wclose },
    { TM2_about,TM2_is,TM2_open,TM2_info,TM2_read,TM2_close,TM2_wopen,TM2_write,TM2_wclose },
    { BNR_about,BNR_is,BNR_open,BNR_info,BNR_read,BNR_close,BNR_wopen,BNR_write,BNR_wclose },
    { TIM_about,TIM_is,TIM_open,TIM_info,TIM_read,TIM_close,TIM_wopen,TIM_write,TIM_wclose },
    { ICO_about,ICO_is,ICO_open,ICO_info,ICO_read,ICO_close,ICO_wopen,ICO_write,ICO_wclose },
//    S3T_about,S3T_is,S3T_open,S3T_info,S3T_read,S3T_close,S3T_wopen,S3T_write,S3T_wclose,
    { DDS_about,DDS_is,DDS_open,DDS_info,DDS_read,DDS_close,DDS_wopen,DDS_write,DDS_wclose },
    { XPR_about,XPR_is,XPR_open,XPR_info,XPR_read,XPR_close,XPR_wopen,XPR_write,XPR_wclose },
    { TPR_about,TPR_is,TPR_open,TPR_info,TPR_read,TPR_close,TPR_wopen,TPR_write,TPR_wclose },
    { GTF_about,GTF_is,GTF_open,GTF_info,GTF_read,GTF_close,GTF_wopen,GTF_write,GTF_wclose },
	{ PSH_about,PSH_is,PSH_open,PSH_info,PSH_read,PSH_close,PSH_wopen,PSH_write,PSH_wclose },
    { SIM_about,SIM_is,SIM_open,SIM_info,SIM_read,SIM_close,SIM_wopen,SIM_write,SIM_wclose },
    { TGV_about,TGV_is,TGV_open,TGV_info,TGV_read,TGV_close,TGV_wopen,TGV_write,TGV_wclose },
    { GIF_about,GIF_is,GIF_open,GIF_info,GIF_read,GIF_close,GIF_wopen,GIF_write,GIF_wclose },
//    VQT_about,VQT_is,VQT_open,VQT_info,VQT_read,VQT_close,VQT_wopen,VQT_write,VQT_wclose,
    { STR_about,STR_is,STR_open,STR_info,STR_read,STR_close,STR_wopen,STR_write,STR_wclose },
    { PFN_about,PFN_is,PFN_open,PFN_info,PFN_read,PFN_close,PFN_wopen,PFN_write,PFN_wclose },
//    VSH_about,VSH_is,VSH_open,VSH_info,VSH_read,VSH_close,VSH_wopen,VSH_write,VSH_wclose,
    { SFN_about,SFN_is,SFN_open,SFN_info,SFN_read,SFN_close,SFN_wopen,SFN_write,SFN_wclose },
//    NFN_about,NFN_is,NFN_open,NFN_info,NFN_read,NFN_close,NFN_wopen,NFN_write,NFN_wclose,
    { GFN_about,GFN_is,GFN_open,GFN_info,GFN_read,GFN_close,GFN_wopen,GFN_write,GFN_wclose },
    { XFN_about,XFN_is,XFN_open,XFN_info,XFN_read,XFN_close,XFN_wopen,XFN_write,XFN_wclose },
    { MFN_about,MFN_is,MFN_open,MFN_info,MFN_read,MFN_close,MFN_wopen,MFN_write,MFN_wclose },
    { YFN_about,YFN_is,YFN_open,YFN_info,YFN_read,YFN_close,YFN_wopen,YFN_write,YFN_wclose },
    { ZFN_about,ZFN_is,ZFN_open,ZFN_info,ZFN_read,ZFN_close,ZFN_wopen,ZFN_write,ZFN_wclose },
//    MVI_about,MVI_is,MVI_open,MVI_info,MVI_read,MVI_close,MVI_wopen,MVI_write,MVI_wclose,
//    TXP_about,TXP_is,TXP_open,TXP_info,TXP_read,TXP_close,TXP_wopen,TXP_write,TXP_wclose,
//    CMP_about,CMP_is,CMP_open,CMP_info,CMP_read,CMP_close,NIL_wopen,NIL_write,NIL_wclose,
    { EPS_about,EPS_is,EPS_open,EPS_info,EPS_read,EPS_close,EPS_wopen,EPS_write,EPS_wclose },
    { RAW_about,RAW_is,RAW_open,RAW_info,RAW_read,RAW_close,RAW_wopen,RAW_write,RAW_wclose },
//    RGS_about,RGS_is,RGS_open,RGS_info,RGS_read,RGS_close,RGS_wopen,RGS_write,RGS_wclose,
//    INV_about,INV_is,INV_open,INV_info,INV_read,INV_close,INV_wopen,INV_write,INV_wclose,
//    VRM_about,VRM_is,VRM_open,VRM_info,VRM_read,VRM_close,VRM_wopen,VRM_write,VRM_wclose,
//    FFS_about,FFS_is,FFS_open,FFS_info,FFS_read,FFS_close,FFS_wopen,FFS_write,FFS_wclose,
//    NSH_about,NSH_is,NSH_open,NSH_info,NSH_read,NSH_close,NSH_wopen,NSH_write,NSH_wclose,
//    BAM_about,BAM_is,BAM_open,BAM_info,BAM_read,BAM_close,BAM_wopen,BAM_write,BAM_wclose,
    { YUV_about,YUV_is,YUV_open,YUV_info,YUV_read,YUV_close,YUV_wopen,YUV_write,YUV_wclose },

   	// default to old
    { OLDFSH_about,OLDFSH_is,OLDFSH_open,OLDFSH_info,OLDFSH_read,OLDFSH_close,OLDFSH_wopen,OLDFSH_write,OLDFSH_wclose },
    { OLDGSH_about,OLDGSH_is,OLDGSH_open,OLDGSH_info,OLDGSH_read,OLDGSH_close,OLDGSH_wopen,OLDGSH_write,OLDGSH_wclose },
    { OLDXSH_about,OLDXSH_is,OLDXSH_open,OLDXSH_info,OLDXSH_read,OLDXSH_close,OLDXSH_wopen,OLDXSH_write,OLDXSH_wclose },
    { OLDMSH_about,OLDMSH_is,OLDMSH_open,OLDMSH_info,OLDMSH_read,OLDMSH_close,OLDMSH_wopen,OLDMSH_write,OLDMSH_wclose },
    { OLDSSH_about,OLDSSH_is,OLDSSH_open,OLDSSH_info,OLDSSH_read,OLDSSH_close,OLDSSH_wopen,OLDSSH_write,OLDSSH_wclose },
    { OLDASH_about,OLDASH_is,OLDASH_open,OLDASH_info,OLDASH_read,OLDASH_close,OLDASH_wopen,OLDASH_write,OLDASH_wclose },
    { OLDFSS_about,OLDFSS_is,OLDFSS_open,OLDFSS_info,OLDFSS_read,OLDFSS_close,OLDFSS_wopen,OLDFSS_write,OLDFSS_wclose },
    { OLDXSS_about,OLDXSS_is,OLDXSS_open,OLDXSS_info,OLDXSS_read,OLDXSS_close,OLDXSS_wopen,OLDXSS_write,OLDXSS_wclose },
    { OLDGSS_about,OLDGSS_is,OLDGSS_open,OLDGSS_info,OLDGSS_read,OLDGSS_close,OLDGSS_wopen,OLDGSS_write,OLDGSS_wclose },
    { OLDSSS_about,OLDSSS_is,OLDSSS_open,OLDSSS_info,OLDSSS_read,OLDSSS_close,OLDSSS_wopen,OLDSSS_write,OLDSSS_wclose },
    { OLDMSS_about,OLDMSS_is,OLDMSS_open,OLDMSS_info,OLDMSS_read,OLDMSS_close,OLDMSS_wopen,OLDMSS_write,OLDMSS_wclose },

    { FSH_about,FSH_is,FSH_open,FSH_info,FSH_read,FSH_close,FSH_wopen,FSH_write,FSH_wclose },
    { GSH_about,GSH_is,GSH_open,GSH_info,GSH_read,GSH_close,GSH_wopen,GSH_write,GSH_wclose },
    { XSH_about,XSH_is,XSH_open,XSH_info,XSH_read,XSH_close,XSH_wopen,XSH_write,XSH_wclose },
    { MSH_about,MSH_is,MSH_open,MSH_info,MSH_read,MSH_close,MSH_wopen,MSH_write,MSH_wclose },
    { YSH_about,YSH_is,YSH_open,YSH_info,YSH_read,YSH_close,YSH_wopen,YSH_write,YSH_wclose },
    { ZSH_about,ZSH_is,ZSH_open,ZSH_info,ZSH_read,ZSH_close,ZSH_wopen,ZSH_write,ZSH_wclose },
    { SSH_about,SSH_is,SSH_open,SSH_info,SSH_read,SSH_close,SSH_wopen,SSH_write,SSH_wclose },
    { ASH_about,ASH_is,ASH_open,ASH_info,ASH_read,ASH_close,ASH_wopen,ASH_write,ASH_wclose },
    { FSS_about,FSS_is,FSS_open,FSS_info,FSS_read,FSS_close,FSS_wopen,FSS_write,FSS_wclose },
    { XSS_about,XSS_is,XSS_open,XSS_info,XSS_read,XSS_close,XSS_wopen,XSS_write,XSS_wclose },
    { GSS_about,GSS_is,GSS_open,GSS_info,GSS_read,GSS_close,GSS_wopen,GSS_write,GSS_wclose },
    { SSS_about,SSS_is,SSS_open,SSS_info,SSS_read,SSS_close,SSS_wopen,SSS_write,SSS_wclose },
    { MSS_about,MSS_is,MSS_open,MSS_info,MSS_read,MSS_close,MSS_wopen,MSS_write,MSS_wclose },

//    MG_about,MG_is,MG_open,MG_info,MG_read,MG_close,MG_wopen,MG_write,MG_wclose,
//    IPU_about,IPU_is,IPU_open,IPU_info,IPU_read,IPU_close,IPU_wopen,IPU_write,IPU_wclose,
//    LAY_about,LAY_is,LAY_open,LAY_info,LAY_read,LAY_close,LAY_wopen,LAY_write,LAY_wclose,
    { NEW_about,NEW_is,NEW_open,NEW_info,NEW_read,NEW_close,NIL_wopen,NIL_write,NIL_wclose },
#endif /* RT */

// ***************************************************************************
// Sgi, linux or windows, but not consoles
// ***************************************************************************

#if (defined(_MSC_VER) && !defined(_XBOX)) || defined(__unix__) || defined(__APPLE__)
    { JPG_about,JPG_is,JPG_open,JPG_info,JPG_read,JPG_close,JPG_wopen,JPG_write,JPG_wclose },
    { PNG_about,PNG_is,PNG_open,PNG_info,PNG_read,PNG_close,PNG_wopen,PNG_write,PNG_wclose },
#if !defined(RT)
//    CEL_about,CEL_is,CEL_open,CEL_info,CEL_read,CEL_close,NIL_wopen,NIL_write,NIL_wclose,
    { MPG_about,MPG_is,MPG_open,MPG_info,MPG_read,MPG_close,MPG_wopen,MPG_write,MPG_wclose },
#if !defined(__PPU__)
	{ TIF_about,TIF_is,TIF_open,TIF_info,TIF_read,TIF_close,TIF_wopen,TIF_write,TIF_wclose },
#endif
	//    VAN_about,VAN_is,VAN_open,VAN_info,VAN_read,VAN_close,VAN_wopen,VAN_write,VAN_wclose,
    { QMT_about,QMT_is,QMT_open,QMT_info,QMT_read,QMT_close,QMT_wopen,QMT_write,QMT_wclose },
//    MAT_about,MAT_is,MAT_open,MAT_info,MAT_read,MAT_close,MAT_wopen,MAT_write,MAT_wclose,
//    TDF_about,TDF_is,TDF_open,TDF_info,TDF_read,TDF_close,TDF_wopen,TDF_write,TDF_wclose,
//    PVR_about,PVR_is,PVR_open,PVR_info,PVR_read,PVR_close,PVR_wopen,PVR_write,PVR_wclose,
//    UV_about ,UV_is ,UV_open ,UV_info ,UV_read ,UV_close ,UV_wopen ,UV_write ,UV_wclose ,
#endif
#endif
#endif

// ***************************************************************************
// All platforms + RT library
// ***************************************************************************

    { PIC_about,PIC_is,PIC_open,PIC_info,PIC_read,PIC_close,PIC_wopen,PIC_write,PIC_wclose },

    { NULL   ,NULL,  NULL,  NULL,NULL  ,NULL   ,NULL   ,NULL   ,NULL },
};


