// (c) 1994-2004 Electronic Arts Inc.

/*------------------------------------------------------------------*/
/*                                                                  */
/*                             GXopts                               */
/*                                                                  */
/*                     by FrANK G. Barchard, EAC                    */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/*    stripped out of gx.c to reduce redundancy                     */
/*    contains all option variables for gx all variables should     */
/*    live in gx.c so gx.c and only gx.c should define GXMAIN       */
/*    around the include statement.                                 */
/*------------------------------------------------------------------*/

#ifndef _GXOPT_H_
#define _GXOPT_H_ 1

#ifndef GXMAIN
#define EXTERN extern
#else
#define EXTERN
#endif

#include "gxcram.h"
#include "stdgimex.h"
#include "spit.h"

char *SPIT_opt(const char *optptr);
/*
#define GIMEXTYPES \
     "TYPES:    .3df.act.ai .anm.avi.bam.bif.bmp.cel.clp.cmp.dct.dcx.dds.eps.ffn\n" \
     "          .ffs.fli.flm.fsh.gfn.gif.gmx.gs .gsh.hsi.ico.inv.ipu.jp2.jpg.jpg\n" \
     "          .lay.lbm.m2v.mad.mat.mdl.mfn.mg .mov.mov.mpc.msh.mvi.new.nfn.nsh\n" \
     "          .omf.pal.pcx.pfn.pic.pix.png.ppm.psd.psh.pvr.ras.raw.rgb.rla.rle\n" \
     "          .s3d.s3t.sct.sfn.sgi.ssh.str.tga.tgv.tif.tim.tpl.tqi.twn.txt.uv\n" \
     "          .van.vqt.vrm.vsh.xa.xfn.xsh.yuv.idec.pict\n"
*/
EXTERN int writemsgarg;
EXTERN int errorlevel;

EXTERN int NAMEsubtype;


EXTERN char OPTframes[SPITMAXLINE];
EXTERN char OPTfilterframes[SPITMAXLINE];
// PAULDING START
EXTERN char OPTlayers[SPITMAXLINE];
// PAULDING END
EXTERN char OPTopaque[SPITMAXLINE];
EXTERN char OPTblack[SPITMAXLINE];
EXTERN char OPTtransparent[SPITMAXLINE];
EXTERN char OPTtranslucient[SPITMAXLINE];
EXTERN char OPTpalcopydest[SPITMAXLINE];
EXTERN char OPTpalcopysource[SPITMAXLINE];
EXTERN char OPTpalmovedest[SPITMAXLINE];
EXTERN char OPTpalmovesource[SPITMAXLINE];
EXTERN char OPTpalswapdest[SPITMAXLINE];
EXTERN char OPTpalswapsource[SPITMAXLINE];
EXTERN char OPTpalfile[SPITMAXLINE];
EXTERN ARGB OPTpalfilecolourtbl[256];
EXTERN int  OPTpalfilehistogram[256];
EXTERN char OPTframename[SPITMAXLINE];
EXTERN char OPTframenameexpr[SPITMAXLINE];
EXTERN char OPTcompositefile[SPITMAXLINE];
EXTERN char OPTauxillaryfile[SPITMAXLINE];
EXTERN char OPTlogfile[SPITMAXLINE];
EXTERN char OPTpalettefile[SPITMAXLINE];
EXTERN char OPTsetcolour[SPITMAXLINE];
EXTERN char OPTcomment[SPITMAXLINE];
EXTERN char OPTshell[SPITMAXLINE];
EXTERN char OPTecho[SPITMAXLINE];
EXTERN char OPTavoid[SPITMAXLINE];
EXTERN char OPTpalsortmethod[SPITMAXLINE];
EXTERN int OPTsetcolourvalue;
EXTERN int OPTsetcolourmask;
EXTERN int OPTopenastype;
EXTERN int OPTtype;
EXTERN int OPTsubtype;
EXTERN int OPTsubtypewant;
EXTERN int OPTsubtypenowant;
EXTERN int OPTdepth;
EXTERN int OPTcodeddepth;
EXTERN int OPToverwrite;
EXTERN char OPTwritex[SPITMAXLINE];
EXTERN char OPTpack[SPITMAXLINE];
EXTERN char OPTquality[SPITMAXLINE];
EXTERN int OPTdither;
EXTERN int OPTdoubledither;
EXTERN int OPTgrb;
EXTERN int OPTpalgradstartindex;
EXTERN int OPTpalgradendindex;
EXTERN int OPTpalgradargbq;
EXTERN int OPTpalgradstartargb;
EXTERN int OPTpalgradendargb;
EXTERN int OPTpalgradmaskargb;
EXTERN int OPThalfsize;
EXTERN int OPThalfalpha;
EXTERN int OPThalfpixel;
EXTERN int OPTdoublealpha;
EXTERN int OPThflip;
EXTERN int OPTvflip;
EXTERN int OPTgrey;
EXTERN int OPTstereofocus;
EXTERN int OPTstereorange;
EXTERN int OPTembm;
EXTERN char OPTembmscale[SPITMAXLINE];
EXTERN char OPTembmfilter[SPITMAXLINE];
EXTERN char OPTembmwrap[SPITMAXLINE];
EXTERN int OPTunembm;
EXTERN int OPTgshunembm;
EXTERN char OPTdot3[SPITMAXLINE];
EXTERN char OPTdot3filter[SPITMAXLINE];
EXTERN char OPTdot3wrap[SPITMAXLINE];
EXTERN int  OPTundot3;
EXTERN int OPThotadjust;
EXTERN int OPTalphabits;
EXTERN int OPTredbits;
EXTERN int OPTgreenbits;
EXTERN int OPTbluebits;
EXTERN int OPTcolours;
EXTERN int OPTstartindex;
EXTERN int OPTinverse;
EXTERN int OPTquantmethod;
EXTERN int OPTquantmethodalpha;
EXTERN int OPTpass;
EXTERN int OPTpschannel;
EXTERN int OPTpsflattened;
EXTERN int OPTflatten;
EXTERN int OPTalphacomment;
EXTERN char OPTcenterx[SPITMAXLINE];
EXTERN char OPTcentery[SPITMAXLINE];
EXTERN char OPTreturn[SPITMAXLINE];
EXTERN char OPTif[SPITMAXLINE];
EXTERN char OPTfif[SPITMAXLINE];
EXTERN char OPTwhile[SPITMAXLINE];
EXTERN char OPTfps[SPITMAXLINE];
EXTERN char OPTdpi[SPITMAXLINE];
EXTERN char OPTalign[SPITMAXLINE];
EXTERN char OPTUserSize[SPITMAXLINE];
EXTERN char OPTsafe[SPITMAXLINE];
EXTERN char OPTblur[SPITMAXLINE];
EXTERN char OPTerode[SPITMAXLINE];
EXTERN char OPTequalize[SPITMAXLINE];
EXTERN char OPTdilate[SPITMAXLINE];
EXTERN char OPTdefaultx[SPITMAXLINE];
EXTERN char OPTdefaulty[SPITMAXLINE];
EXTERN char OPTdefaultdx[SPITMAXLINE];
EXTERN char OPTdefaultdy[SPITMAXLINE];
EXTERN char OPThotspots[SPITMAXLINE*4];
EXTERN char OPTpixela[SPITMAXLINE];
EXTERN char OPTpixelr[SPITMAXLINE];
EXTERN char OPTpixelg[SPITMAXLINE];
EXTERN char OPTpixelb[SPITMAXLINE];
EXTERN char OPTpixeli[SPITMAXLINE];
EXTERN char OPTpixelx[SPITMAXLINE];
EXTERN char OPTpixely[SPITMAXLINE];
EXTERN char OPTset[SPITMAXLINE];
EXTERN int OPThsvh;
EXTERN int OPThsvs;
EXTERN int OPThsvb;
EXTERN int OPThsvv;
EXTERN int OPThslh;
EXTERN int OPThsls;
EXTERN int OPThsll;
EXTERN int OPTcontrastc;
EXTERN int OPTcontrastb;
EXTERN int OPTreplaces;
EXTERN int OPTreplacer;
EXTERN int OPTreplacesm;
EXTERN int OPTreplacerm;
EXTERN int OPTreplacesn;
EXTERN int OPTgopi;
EXTERN int OPTgopp;
EXTERN char OPTrectx[SPITMAXLINE];
EXTERN char OPTrecty[SPITMAXLINE];
EXTERN char OPTrectw[SPITMAXLINE];
EXTERN char OPTrecth[SPITMAXLINE];
EXTERN int  OPTrectc;
EXTERN char OPToutlinex[SPITMAXLINE];
EXTERN char OPToutliney[SPITMAXLINE];
EXTERN char OPToutlinew[SPITMAXLINE];
EXTERN char OPToutlineh[SPITMAXLINE];
EXTERN int  OPToutlinec;
EXTERN char OPTgridx[SPITMAXLINE];
EXTERN char OPTgridy[SPITMAXLINE];
EXTERN char OPTgridw[SPITMAXLINE];
EXTERN char OPTgridh[SPITMAXLINE];
EXTERN int  OPTgridc;
EXTERN char OPTlinex[SPITMAXLINE];
EXTERN char OPTliney[SPITMAXLINE];
EXTERN char OPTlinex2[SPITMAXLINE];
EXTERN char OPTliney2[SPITMAXLINE];
EXTERN int  OPTlinec;
EXTERN char OPTfillx[SPITMAXLINE];
EXTERN char OPTfilly[SPITMAXLINE];
EXTERN int  OPTfillc;
EXTERN int OPTchroma;
EXTERN int OPTedgec;
EXTERN int OPTedgep;
EXTERN int OPTedges;
EXTERN int OPThotn;
EXTERN int OPThotx;
EXTERN int OPThoty;
//EXTERN int OPTshadowx1;
//EXTERN int OPTshadowx2;
//EXTERN int OPTshadowy;
//EXTERN int OPTshadowdx;
//EXTERN int OPTshadowdy;
EXTERN char OPTcropw[SPITMAXLINE];
EXTERN char OPTcroph[SPITMAXLINE];
EXTERN int OPTcropwby;
EXTERN int OPTcrophby;
EXTERN char OPTcropx[SPITMAXLINE];
EXTERN char OPTcropy[SPITMAXLINE];
EXTERN int OPTcropc;
EXTERN int OPTcropclampx;
EXTERN int OPTcropclampy;
EXTERN int OPTclutx;
EXTERN int OPTcluty;
EXTERN char OPTdatarate[SPITMAXLINE];
EXTERN char OPTframesize[SPITMAXLINE];
EXTERN char OPTthumbx[SPITMAXLINE];
EXTERN char OPTthumby[SPITMAXLINE];
EXTERN int OPTthumbxby;
EXTERN int OPTthumbyby;
EXTERN char OPTzoomx[SPITMAXLINE];
EXTERN char OPTzoomy[SPITMAXLINE];
EXTERN int OPTzoomxby;
EXTERN int OPTzoomyby;
EXTERN int OPTzoomsharp;
EXTERN char OPTsizex[SPITMAXLINE];
EXTERN char OPTsizey[SPITMAXLINE];
EXTERN int OPTsizexby;
EXTERN int OPTsizeyby;
EXTERN int OPTsizemethod;
EXTERN int OPTsizetrans;
EXTERN int OPTsizechromakey;
EXTERN int OPTsizesharpx;
EXTERN int OPTsizesharpy;
EXTERN char OPTscalex[SPITMAXLINE];
EXTERN char OPTscaley[SPITMAXLINE];
EXTERN int OPTscalexby;
EXTERN int OPTscaleyby;
EXTERN int OPTmipmont;
EXTERN int OPTmipmap;
EXTERN int OPTmipmapmethod;
EXTERN int OPTmipmapwidth;
EXTERN int OPTmipmapheight;
EXTERN int OPTmipmapmin;
EXTERN int OPTmipmaptrans;
EXTERN int OPTmipmapchromakey;
EXTERN int OPTmipmapclearlast;
EXTERN char OPTwrapx[SPITMAXLINE];
EXTERN char OPTwrapy[SPITMAXLINE];
EXTERN int OPTdebandrgbdist;
EXTERN int OPTdebandradius;
EXTERN int OPTswap1;
EXTERN int OPTswap2;
EXTERN int OPTpalsortblack;
EXTERN int OPTrot;
EXTERN int OPTrotate;
EXTERN int OPTdeint;
EXTERN int OPTtemporalfix;
EXTERN int OPTnoremap;
EXTERN int OPTcharname;
EXTERN int OPTcharmactodos;
EXTERN int OPTacopy;
EXTERN int OPTinvert;
EXTERN int OPTtedge;
EXTERN int OPTscreencolour;
EXTERN int OPTscreenorientation;
EXTERN int OPTpreten;
EXTERN int OPTblendb;
EXTERN int OPTblendf;
EXTERN int OPTblendmask;
EXTERN int OPTblendmethod;
EXTERN int OPTblendbehind;
EXTERN float OPTgamma;
EXTERN int OPTsharpen;
EXTERN int OPTshrinktofit;
EXTERN int OPTautocenter;
EXTERN int OPTremove;
EXTERN int OPTsub;
EXTERN int OPTadd;
EXTERN int OPTpaste;
EXTERN int OPTbackground;
EXTERN int OPTextract;
EXTERN int OPTsemitrans;
EXTERN int OPTglobalframe;
EXTERN int OPTglobalpalette;
//EXTERN int OPTtime;
EXTERN int OPTpsnr;
EXTERN int OPThashlen;
EXTERN int OPTlayered;
EXTERN int OPTgen;
EXTERN int OPTwarpx2;
EXTERN int OPTwarpy1;
EXTERN int OPTwarpy2;
EXTERN int OPTwarptype;
EXTERN int OPTwarpc;
EXTERN int OPTwarpclampx;
EXTERN int OPTwarpclampy;
EXTERN int OPTwarpcontlines[SPITMAXLINE];
EXTERN int OPTfdxt;
EXTERN int OPTcram;
EXTERN int OPTcramtranspose;
EXTERN int OPTcramrotate;
EXTERN int OPTcramcenterpoint;
EXTERN int OPTcramcheckedge;
EXTERN int OPTcramoddodd;
EXTERN int OPTcrambppsort;
EXTERN int OPTcramclutsort;
EXTERN int OPTcramgroupsort;
EXTERN GXCRAMGROUP OPTcramgroup[20];
EXTERN int OPTcramtpagewidth;
EXTERN int OPTcramtpageheight;
EXTERN int OPTcramtwrapwidth;
EXTERN int OPTcramwidthin;
EXTERN int OPTcramspace;
EXTERN int OPTcramx;
EXTERN int OPTcramy;
EXTERN int OPTcramw;
EXTERN int OPTcramh;
EXTERN int OPTcramnumfixed;
EXTERN int OPTtranspose;
EXTERN int OPTdropshadow;
EXTERN char OPTDSLightLocationx[SPITMAXLINE];
EXTERN char OPTDSLightLocationy[SPITMAXLINE];
EXTERN char OPTDSLightLocationz[SPITMAXLINE];
EXTERN int OPTDSLightType;
EXTERN char OPTDSLightDirectionx[SPITMAXLINE];
EXTERN char OPTDSLightDirectiony[SPITMAXLINE];
EXTERN char OPTDSLightDirectionz[SPITMAXLINE];
EXTERN char OPTDSRadius[SPITMAXLINE];
EXTERN char OPTDSFalloff[SPITMAXLINE];
EXTERN char OPTDSFadePower[SPITMAXLINE];
EXTERN char OPTDSFadeDistance[SPITMAXLINE];
EXTERN char OPTDSArLightAxis1x[SPITMAXLINE];
EXTERN char OPTDSArLightAxis1y[SPITMAXLINE];
EXTERN char OPTDSArLightAxis1z[SPITMAXLINE];
EXTERN char OPTDSArLightAxis2x[SPITMAXLINE];
EXTERN char OPTDSArLightAxis2y[SPITMAXLINE];
EXTERN char OPTDSArLightAxis2z[SPITMAXLINE];
EXTERN int  OPTDSArSizeW;
EXTERN int  OPTDSArSizeH;
EXTERN char OPTDSLTightness[SPITMAXLINE];
EXTERN char OPTDSLLookAtx[SPITMAXLINE];
EXTERN char OPTDSLLookAty[SPITMAXLINE];
EXTERN char OPTDSLLookAtz[SPITMAXLINE];
EXTERN char OPTDSLightColourr[SPITMAXLINE];
EXTERN char OPTDSLightColourg[SPITMAXLINE];
EXTERN char OPTDSLightColourb[SPITMAXLINE];
EXTERN char OPTDSCamLocationx[SPITMAXLINE];
EXTERN char OPTDSCamLocationy[SPITMAXLINE];
EXTERN char OPTDSCamLocationz[SPITMAXLINE];
EXTERN char OPTDSTexPlaneOffset[SPITMAXLINE];
EXTERN char OPTDSShaPlaneOffset[SPITMAXLINE];
EXTERN int  OPTDSAntiAlias;
EXTERN int  OPTnovisiblecheck;
EXTERN int  OPTMkFTexMap;
EXTERN char OPTtextstring[SPITMAXLINE];
EXTERN int  OPTtextx;
EXTERN int  OPTtexty;
EXTERN char OPTtextfont[SPITMAXLINE];
EXTERN int  OPTtextsize;
EXTERN int  OPTtextcolour;
EXTERN int  OPTtextmode;
EXTERN int  OPTpscolred;
EXTERN char OPTFYIYScale[SPITMAXLINE];
EXTERN char OPTFYIYOffset[SPITMAXLINE];
EXTERN char OPTFYICScale[SPITMAXLINE];
EXTERN char OPTFYICOffset[SPITMAXLINE];
EXTERN char OPTFYIAScale[SPITMAXLINE];
EXTERN char OPTFYIAOffset[SPITMAXLINE];
EXTERN int OPTsafe2grey;
EXTERN int OPTbreak;
EXTERN int OPTinfo;
EXTERN int OPTa8;
EXTERN int OPTprioritylow;
EXTERN int OPTpriorityhigh;

// font specific options
EXTERN char OPTpointsize[SPITMAXLINE];
EXTERN char OPTantialias[SPITMAXLINE];
EXTERN char OPTstyle[SPITMAXLINE];
EXTERN char OPTcharset[SPITMAXLINE];
EXTERN char OPTascii[SPITMAXLINE];
EXTERN char OPTtypeface[SPITMAXLINE];
EXTERN char OPTaspect[SPITMAXLINE];
EXTERN char OPTtighten[SPITMAXLINE];
EXTERN char OPTtracking[SPITMAXLINE];
EXTERN char OPTsmallcaps[SPITMAXLINE];
EXTERN char OPTbaseline[SPITMAXLINE];
EXTERN int OPTitalic;
EXTERN int OPTbold;

#endif

