// (c) 1994-2007 Electronic Arts Inc.

#if defined(_MSC_VER) && _MSC_VER>=1400
#define ENABLE_OMP 0
//#include <omp.h>
#endif

const char *SPIT_history[] =
{
"------------------------------------------------------------------",
"                                                                  ",
"                             GX v5.89                             ",
"                                                                  ",
"                     by FrANK G. Barchard, EAC                    ",
"                                                                  ",
"                    Main program - June 06, 2007                  ",
"                                                                  ",
"------------------------------------------------------------------",
"                                                                  ",
" Version Date   SE History:                                       ",
" ------- ----   -- --------                                       ",
"    1.00 940708 FB based on gshow,runpack,cel2shp, and huff       ",
"    1.01 940719 FB cel, -p to pack                                ",
"    1.02 940730 FB -4 reduce to 4 bit, memory bug fix             ",
"    1.03 940809 FB -n any bits. set argb bits appropriately/help  ",
"    1.04 940811 FB toxxx variations of gx                         ",
"    1.05 940830 FB HASHID_apply                                   ",
"    1.06 940912 FB build multishape shapefiles                    ",
"    1.07 940914 FB -s to set subtype. dont change ext if user set ",
"    1.08 940920 FB -t transparent colour                          ",
"    1.09 941007 FB -l translucient colour (50% alpha),-q quality  ",
"    1.10 941017 FB -d dither, -565                                ",
"    1.11 941020 FB -c to specify number of colours                ",
"    1.12 941027 FB file not found, 2 memory leaks                 ",
"    1.13 941225 FB -32 option parse                               ",
"    1.14 941226 FB wu colour quantizer by default for 32to8       ",
"    1.15 941228 FB -i0 no inverse table                           ",
"    1.16 950203 FB -x/-y/-cx/-cy options                          ",
"    1.17 950222 FB -r rotate                                      ",
"    1.18 950226 FB -crop                                          ",
"    1.19 950311 FB -f frame start/end delimit,!,msg on first out  ",
"    1.20 950316 FB -l take range                                  ",
"    1.21 950320 FB -w0 dont call writebmp                         ",
"    1.22 950403 FB -l ranges                                      ",
"    1.23 950403 FB -t ranges,-<n> affect originalbpp,-clut<x>,<y> ",
"    1.24 950406 FB auto depth 32 only if original is 8            ",
"    1.25 950413 FB -p palette file,-pack for packing (was -p)     ",
"    1.26 950417 FB -hflip,-n name                                 ",
"    1.27 950418 FB -p regenerate inversetable                     ",
"    1.28 950419 FB -f use ranges syntax                           ",
"    1.29 950426 FB -cropx,y,w,h or -cropw,h                       ",
"    1.30 950427 FB ? help file types updated                      ",
"    1.31 950501 FB frames-1 max frame                             ",
"    1.32 950511 FB -hotadjust                                     ",
"    1.33 950515 FB removed expermental file types                 ",
"    1.34 950515 FB zoom hardcoded                                 ",
"    1.35 950613 FB updated command line help types                ",
"    1.36 950623 FB zoom force conversion to 32 bit                ",
"    1.37 950706 FB -s anything for bamf                           ",
"    1.38 950712 FB fsh export !pals                               ",
"    1.39 950816 FB -sort (filenames)                              ",
"    1.40 950823 FB local options when accumulating many to 1      ",
"    1.41 950831 FB zoom sharpness optional 3rd arg (100 default)  ",
"    1.42 950907 FB prevent overwrite, warns old gimex versions    ",
"    1.43 950911 FB inversetable functions renamed,parse composite ",
"    1.44 950912 FB get composite frame                            ",
"    1.45 950915 FB crop center, composite by x/y pixel and clip   ",
"    1.46 950928 FB spit allow .\\ in destination                  ",
"    1.47 950929 FB auto 8 bit conversion only if necessary        ",
"    1.48 951002 FB -opaque                                        ",
"    1.49 951004 FB resample message terser                        ",
"    1.50 951006 FB -8/-p combined                                 ",
"    1.51 951008 FB -noremap to force no auto remapping            ",
"    1.52 951010 FB -8 -p blend 2 palettes                         ",
"    1.53 951012 FB -charname (font index),-sub/-add composite     ",
"    1.54 951017 FB many to 1, processimage function               ",
"    1.55 951018 FB %s in destination use framename,-startindex    ",
"    1.56 951020 FB -charmactodos (not complete)                   ",
"    1.57 951022 FB shape naming when going from tga to fsh        ",
"    1.58 951024 FB -black to black out a range in the palette     ",
"    1.59 951025 FB -charname,-acopy,-blur,-invert,-remove         ",
"    1.60 951026 FB -tblack,-blend<n>,-setcolour<colours>,val,mask ",
"    1.61 951027 FB -math filter                                   ",
"    1.62 951030 FB -grey filter                                   ",
"    1.63 951031 FB change extension if not in acceptable list     ",
"    1.64 951101 FB -wrap<x>,<y>,-rot<n>,-twhite                   ",
"    1.65 951102 FB -palsort<method>,<force>                       ",
"    1.66 951104 FB -rectx,y,w,h,c                                 ",
"    1.67 951106 FB -cropx,y,w,h,c filter, allows upsizing         ",
"    1.68 951107 FB -cropby to crop inward; -fill<x>,<y><c> or <c> ",
"    1.69 951108 FB -channel<n>                                    ",
"    1.70 951109 FB -comment<str>; -shrinktofit                    ",
"    1.71 951112 FB Ported to MrC for Mac; set mactype             ",
"    1.72 951114 FB output names only changed is -tga specified    ",
"    1.73 951119 FB outtype, composite, -blend<f>,<b>, -background ",
"    1.74 951121 FB -sharpen<amt>,colours                          ",
"    1.75 951122 FB -vflip (based on filter)                       ",
"    1.76 951212 FB handle no extension                            ",
"    1.77 960109 FB edge filter                                    ",
"    1.78 960112 FB blend channel mask                             ",
"    1.79 960118 FB -hotadd to adjust a hotspot                    ",
"    1.80 960201 FB dont hash id for -name option                  ",
"    1.81 960201 FB -dx,-dy                                        ",
"    1.82 960204 FB -hsv                                           ",
"    1.83 960205 FB -paste                                         ",
"    1.84 960207 FB -preten                                        ",
"    1.85 960209 FB -hsv take brightness, followed by v optional   ",
"    1.86 960209 FB -line                                          ",
"    1.87 960217 FB -colour/bpp logic                              ",
"    1.88 960226 FB disable export if not supported                ",
"    1.89 960304 FB sct gimex                                      ",
"    1.90 960314 FB -palsort4 for inverse histogram                ",
"    1.91 960318 FB -contrast<contrast>,<brightness>               ",
"    1.92 960409 FB -swap<colour1>,<colour2>                       ",
"    1.93 960416 FB -scale<width>,<height>, -d2                    ",
"    1.94 960423 FB spit does sort                                 ",
"    1.95 960424 FB -scale110%,90%                                 ",
"    1.96 960425 FB -zoom110%,90%,-deband                          ",
"    1.97 960506 FB -deint<0=odd,1=even>                           ",
"    1.98 960508 FB -grey<n> n colours 2..256                      ",
"    1.99 960509 FB -autocenter for golf center point triangulate  ",
"    2.00 960510 FB shrink last, center first                      ",
"    2.01 960612 YK -mipmap, processimage 1:many, -reset           ",
"    2.02 960620 YK -mipmap width, height, sharpness               ",
"    2.03 960621 FB FSH 32 bit cluts                               ",
"    2.04 960626 FB -name%s.  no prefix ellimination               ",
"    2.05 960627 FB -edge<colour>,<pixels>,<pits>                  ",
"    2.06 960701 FB -565 on 8 bit art                              ",
"    2.07 960709 YK do not zoom mipmap0 if size as desired         ",
"    2.08 960709 FB blend on 8 bit                                 ",
"    2.09 960719 YK mipmap level selection default logic           ",
"    2.10 960723 FB size filter                                    ",
"    2.11 960726 FB maintain subtype for SHPx                      ",
"    2.12 960730 FB blend use source alpha, not dest               ",
"    2.13 960813 YK mipmap uses size filter by default             ",
"    2.14 960813 FB -noremap for -p option too                     ",
"    2.15 960815 FB -edge on 8 bit                                 ",
"    2.16 960822 FB fsh 32 bit clut no remap 255                   ",
"    2.17 960911 FB arbitary rotation -rotate<degrees>             ",
"    2.18 960918 FB rotate use -background                         ",
"    2.19 960919 FB -subtype                                       ",
"    2.20 960920 FB -palsort trans                                 ",
"    2.21 961002 FB frames named bob.shp.ext                       ",
"    2.22 961006 FB frame names, 3 pass                            ",
"    2.23 961007 FB -avoid                                         ",
"    2.24 961009 FB new range api                                  ",
"    2.25 961011 FB palsort0,3 t255 better                         ",
"    2.26 961015 FB transparency respect,palette file,crop %       ",
"    2.27 961023 FB cropto,by,amt                                  ",
"    2.28 961101 FB - for local options, dont confuse ../          ",
"    2.29 961106 FB transparent 0 and 16 colour,-pal+shape         ",
"    2.30 961108 FB -comment to turn off                           ",
"    2.31 961114 FB -8 set alphabits depending on transparent      ",
"    2.32 961117 FB --opt error, use crapi via gcolour             ",
"    2.33 961120 FB output filename use framename                  ",
"    2.34 961121 FB -t,-opaque,-translucient affect -8, -8 on 8    ",
"    2.35 961122 FB -p use indexhistogram find unused transparent  ",
"    2.36 961205 YK spit fix for -h                                ",
"    2.37 961206 YK -grey bug fix if no argument given             ",
"    2.38 961209 YK version rev for LBM import bug fix             ",
"    2.39 961210 YK OPTstatic,-n1,-p use numcolours from palfile   ",
"    2.40 961211 YK removed OPTstatic, dynamic avoid               ",
"    2.41 961216 YK besttransparent,-perfect,no 1-bit SSH, -88 NSH ",
"    2.42 970109 FB -t for a transparent colour                    ",
"    2.43 970128 FB -acopy1 to copy to alpha from max(r,g,b)       ",
"    2.44 970130 FB -555 -d1 dont randomize alpha                  ",
"    2.45 970206 FB gsocket log added                              ",
"    2.46 970217 FB unlimited hash entries                         ",
"    2.47 970221 FB -shadow option                                 ",
"    2.48 970227 FB -center mid,max x,y                            ",
"    2.49 970305 FB -pack0..3 for van                              ",
"    2.50 970307 FB -shadowx1,x2,y,dx,dy                           ",
"    2.51 970324 FB -setcolor                                      ",
"    2.52 970327 FB gfilter module seperated                       ",
"    2.53 970401 FB wildname used and default ignore framename     ",
"    2.54 970416 FB expression API for -cx option                  ",
"    2.55 970417 FB gfilter init/restore                           ",
"    2.56 970505 FB -replace any colour with any other             ",
"    2.57 970506 FB -invert<m>, -twhite etc macros                 ",
"    2.58 970509 FB frame in expressions for -x/-y                 ",
"    2.59 970526 FB -extract, remove -perfect                      ",
"    2.60 970610 FB -hsl                                           ",
"    2.61 970613 FB -doubledither                                  ",
"    2.62 970618 FB wildname to compare for -=%s.pic               ",
"    2.63 970619 FB -blurp and output naming fix -=%s.pic          ",
"    2.64 970708 FB -replaces,r,sm,rm,not not=0 or 1               ",
"    2.65 970819 FB -pixela,r,g,b expressions                      ",
"    2.67 970909 FB -clutx for consistancy                         ",
"    2.68 970916 FB -fps,-framesize for madcow                     ",
"    2.69 970921 FB -dct set subtype and filename extension        ",
"    2.70 970926 FB -mipmapb box filter method for mipmapnearest   ",
"    2.71 971113 FB close if really done                           ",
"    2.72 971113 KO -semitrans for shapes                          ",
"    2.73 971117 FB -tga? prints about for file format             ",
"    2.74 971201 FB -pixela,r,g,b,i index expression for 8 bit     ",
"    2.75 980121 KO GIMEX_set before abnormal close                ",
"    2.76 980129 KO if resizable set width/height before read      ",
"    2.77 980210 FB framecount til next destination                ",
"    2.78 980222 KO New GIMEX API.                                 ",
"    2.79 980317 FB -gamma                                         ",
"    2.80 980429 FB -globalframe to make -f use global frame count ",
"    2.81 980504 KO fixed -globalframe                             ",
"    2.82 980507 KO -flattened to extract flattened PSD image      ",
"    2.83 980526 FB -line? and -line allow 0 and negatives         ",
"    2.84 980527 KO -globalpalette builds single palette           ",
"    2.85 980527 SK -abstracted -line? into SPIT                   ",
"    2.86 980528 SK -uses SPIT_strstr instead of strstr            ",
"    2.87 980612 FB -t forces -i0                                  ",
"    2.88 980626 FB change extension only if filename is same      ",
"    2.89 980929 FB -twiddle                                       ",
"    2.90 981024 FB Visual C port                                  ",
"    2.91 981103 FB -mipmap0,0,0,minsize (ie 8 = 8x8)              ",
"    2.92 981106 FB multiple source using single backup detect     ",
"    2.93 990329 FB -sethot sets list of hotspots                  ",
"    2.94 990407 FB -grb 8 bit palettizer                          ",
"    2.95 990423 KO -pfile.act where histogram == 1                ",
"    2.96 990426 KO -gif auto global palette                       ",
"    2.97 990514 LC changed palsort method to be string            ",
"    2.98 990526 FB -m9 for wus 555                                ",
"    2.99 990609 LC warning msg if format doesnt support bit depth ",
"    3.00 990617 FB preten after -8 and -pixelr,g,b                ",
"    3.01 990712 LC warning msg if multiple target directories     ",
"    3.02 990712 LC -m-1 auto mode                                 ",
"    3.03 990722 FB -gridx,y,w,h,c & quicktime fix                 ",
"    3.04 990723 FB -ff for filter frames, -fps expr & ginfo       ",
"    3.05 990806 FB -flatten                                       ",
"    3.06 990806 FB -alphacomment                                  ",
"    3.07 990811 FB -dpi (for gmx initially)                       ",
"    3.08 990823 FB -dilate, -erode, fast blur from Alister Milne  ",
"    3.09 990823 FB blur force 32 bug. dpi in more modules         ",
"    3.10 990825 FB info32to8 vars static (-globalpalette bug)     ",
"    3.11 990915 FB -size chromakey                                ",
"    3.12 990929 LC fixed bug in gxinitinversetable in gcolour.c   ",
"    3.13 991001 LC remap.c: lastindex reset for each frame        ",
"    3.14 991005 FB -setvar=expression                             ",
"    3.15 991027 FB halloween version (text string macros)         ",
"    3.16 991027 FB -screencolour,orientation                      ",
"    3.17 991215 FB -time for timing diagnostics                   ",
"    3.18 991224 FB -i0 -m -d auto & message. crapi alpha 5666     ",
"    3.19 000110 FB -globalpalette works with ; (resets itself)    ",
"    3.20 000113 FB -cropmax/-sizemax/min                          ",
"    3.21 000126 FB unix generic build system/code (linux first)   ",
"    3.22 000203 FB -halfalpha (-pixela/2,r,g,b)                   ",
"    3.23 000203 FB -halfpixel, crapi fast, -pixel fast, -Oxa      ",
"    3.24 000219 FB -halfsize                                      ",
"    3.25 000223 FB -deint                                         ",
"    3.26 000225 FB -halfalpha after colour reduction              ",
"    3.27 000306 FB -palgrad                                       ",
"    3.28 000306 FB -csm2                                          ",
"    3.29 000306 FB -extract wont do backups                       ",
"    3.30 000328 FB -preten use opt2 default 0                     ",
"    3.31 000330 FB -pixel help show functions available           ",
"    3.32 000410 FB -hash4 to hash shape names to 4 explicitely    ",
"    3.33 000417 FB -size320*200pix for pixels,-palgradi,j         ",
"    3.34 000519 FB -gop15,3 for mpeg i & p frame intervals        ",
"    3.35 000524 FB -echo<string>                                  ",
"    3.36 000524 FB -size/scale/crop/zoom expression               ",
"    3.37 000524 FB -echoformat,expr                               ",
"    3.38 000606 FB remap error diffusion bug fix, nfn, cropto     ",
"    3.39 000608 FB hash name add framenum                         ",
"    3.40 000612 FB composite seperated from flatten               ",
"    3.41 000613 FB -dxt1 -dxt3 shortcuts                          ",
"    3.42 000616 FB -flattened fixed                               ",
"    3.43 000712 FB -if, -while                                    ",
"    3.44 000713 FB -shell to launch system commands               ",
"    3.45 000718 FB -pass2 and maxwidth/maxheight fields           ",
"    3.46 000719 FB -dx use ranger                                 ",
"    3.47 000721 FB psnr, one msg for write error                  ",
"    3.48 000814 FB tgv -t255                                      ",
"    3.49 000830 FB -tedge extrude edge RGB onto transparent pixel ",
"    3.50 000831 FB -wrap expressions                              ",
"    3.51 000908 FB tedge bump depth to 32                         ",
"    3.52 000911 FB crop clamp,wrap,mirror.  -sethot expr          ",
"    3.53 000912 FB fast size and linux -flatten                   ",
"    3.54 000917 FB -line pixel,expr rect/line/grid/fill,pixel x/y ",
"    3.55 000918 FB GX_getopt                                      ",
"    3.56 000922 FB transparent disguard inverse table             ",
"    3.57 000925 FB -fif,-pack,-q,-framesize,-w expressions        ",
"    3.58 000928 FB OSX and unix clamp size                        ",
"    3.59 001004 FB c_ vars for composite file                     ",
"    3.60 001010 FB clamp options parsing                          ",
"    3.61 001106 FB composite fixed                                ",
"    3.62 001115 FB -bpp? fixed                                    ",
"    3.63 001117 FB GX_VERSION,GX_DATE,GIMEX_VERSION predefines    ",
"    3.64 001121 FB post filter convert to 32 if not export 8      ",
"    3.65 001123 FB 64 bit seeks                                   ",
"    3.66 001128 FB -noremap warning                               ",
"    3.67 001130 FB -halfalpha fixed for 32 bit                    ",
"    3.68 001201 FB -m7 on 24 bit fixed                            ",
"    3.69 001203 FB -halfpixel working on 8 bit again              ",
"    3.70 001204 FB stdgimex header version check                  ",
"    3.71 001206 FB -doublealpha                                   ",
"    3.72 001211 FB -shrinktofit<method>                           ",
"    3.73 001213 FB -static -fPIC for linux/sgi friendlier builds  ",
"    3.74 001219 LC palsort & name with no args, now does nothing  ",
"    3.75 001219 FB -pixel remove nextfield                        ",
"    3.76 001222 FB mipmap variable and mipmap clear count         ",
"    3.77 010118 ID added -layer option and fixed filters.         ",
"    3.78 010126 ID added -warp option                             ",
"    3.79 010129 FB -swizzle aka twiddle                           ",
"    3.80 010206 FB -isdot3,-isembm,-iscubemap subtype metal info  ",
"    3.81 010209 FB -5a3,-openas                                   ",
"    3.82 010212 FB -openas error checking                         ",
"    3.83 010216 FB -dxt5                                          ",
"    3.84 010227 ID -cubemap for dds only                          ",
"    3.85 010301 FB -pack? show avi compression                    ",
"    3.86 010305 FB -subtype                                       ",
"    3.87 010315 FB -halfsize/-size clamp to 1 pixel               ",
"    3.88 010316 ID -flattencram added, saves xy,wh in hotspot     ",
"    3.89 010318 FB -shrinktofit0=3                                ",
"    3.90 010328 FB -safe                                          ",
"    3.91 010329 ID -fdxt option runs image through dxt filter     ",
"    3.92 010401 FB global palette dont force alphawu              ",
"    3.93 010403 FB -flattencram add shape names to comment field  ",
"    3.94 010404 FB -aauxfile.tga for an image to refer to         ",
"    3.95 010405 ID -flatten2                                      ",
"    3.96 010406 FB alpha wu fix, -a, maxcolours                   ",
"    3.97 010410 FB -palcopy,-palmove,-palswap, UDP                ",
"    3.98 010411 FB remap source alpha sensitive                   ",
"    3.99 010411 ID added cram not yet flatten able                ",
"    4.00 010411 ID cram now can cram when we flatten              ",
"    4.01 010412 FB unix/gsocket builds                            ",
"    4.02 010416 FB transparent test, crop layer hacked out        ",
"    4.03 010417 ID crash problem fixed with CRAPI                 ",
"    4.04 010418 FB 1x1 special case avoid transparent warning     ",
"    4.05 010420 FB -flatten2 maintain hotspots properly           ",
"    4.06 010421 FB -log respected                                 ",
"    4.07 010424 FB widthp2/heightp2 expression variables          ",
"    4.08 010424 ID no mem leaks in gxfilter.c                     ",
"    4.09 010425 FB -dxt auto, log working to garp                 ",
"    4.10 010425 ID -cram fixed                                    ",
"    4.11 010426 ID -flatten2 now does -paste as well              ",
"    4.12 010427 FB expr fill in more API versions                 ",
"    4.13 010430 ID fixed spit issue with response files           ",
"    4.14 010501 FB -bpp option, gcmp tool                         ",
"    4.15 010503 FB -ismipmont,metalbin                            ",
"    4.16 010506 FB -stereo                                        ",
"    4.17 010507 FB -thumb                                         ",
"    4.18 010509 FB -embm, -d0 default                             ",
"    4.19 010510 ID -embm<n> added subtype support as well         ",
"    4.20 010515 FB -pixel -a work together setting a_ vars        ",
"    4.21 010515 ID -unembm<n> fixed up                            ",
"    4.22 010522 FB -n depth rounding fix                          ",
"    4.23 010529 ID transpose filter                               ",
"    4.24 010530 FB transparent check at end of filtering          ",
"    4.25 010531 FB -comment%s to format with old comment          ",
"    4.26 010531 ID crippled embm to only support UVL internally   ",
"    4.27 010601 ID added the new -gshembm and -gshunembm formats  ",
"    4.28 010601 ID preserve subtypes on interations of SPIT_opt   ",
"    4.29 010602 FB -comment%s                                     ",
"    4.30 010607 ID fix of -fbr?? range checking.                  ",
"    4.31 010607 ID fixed small bug with pagetype multipass        ",
"    4.32 010611 LC -p takes optional framenumber or framename     ",
"    4.33 010613 ID small fix of OPTlayered                        ",
"    4.34 010620 FB -? delimited                                   ",
"    4.35 010623 FB hot10..19                                      ",
"    4.36 010625 ID new dropshadow filter                          ",
"    4.37 010625 ID added support for 8bpp dropshadowing           ",
"    4.38 010627 ID added support for psd blendmethods and visible ",
"    4.39 010628 FB GIMEX_COMMENT_SIZE etc                         ",
"    4.40 010706 ID dot3 filter support                            ",
"    4.41 010709 FB gcmp movies                                    ",
"    4.42 010709 ID fixed -ff option to work with wildcards        ",
"    4.43 010709 ID added undot3 needs some improvement            ",
"    4.44 010711 ID split gx.c into two parts gx.c and gxopt.c     ",
"    4.45 010712 FB -equalize                                      ",
"    4.46 010718 ID range fix with negatives in a list             ",
"    4.47 010723 FB -blendbehind                                   ",
"    4.48 010724 FB -m11 fu                                        ",
"    4.49 010727 ID small fix in wildcard rangematching.           ",
"    4.50 010728 FB -m2 fu renumbered                              ",
"    4.51 010801 FB -m3 wu777 and fu working and default           ",
"    4.52 010803 FB revert to p4 ver 279 to fix *.tga =bob.xsh     ",
"    4.53 010808 FB -time minutes, -threshhold                     ",
"    4.54 010810 ID -dot3 now has height scale                     ",
"    4.55 010810 ID allow -dot30 and negative dot3 scales          ",
"    4.56 010817 ID remove of -cubemap for dds only use -isCubemap ",
"    4.57 010824 ID remove auto logic for layered textures.        ",
"    4.58 010824 LC added cramspace option                         ",
"    4.59 010824 FB -advanced                                      ",
"    4.60 010827 ID added antialiasing supersampling for dropshad  ",
"    4.61 010831 ID added global palette option to gfilter         ",
"    4.62 010918 FB switched to intel c and some c++               ",
"    4.63 010930 FB dropped shadow feature (fly)                   ",
"    4.64 011009 FB scale then size                                ",
"    4.65 011010 FB -safe2 for k loken pc to ntsc                  ",
"    4.66 011012 FB -time minutes                                  ",
"    4.67 011016 FB -align for shapes                              ",
"    4.68 011019 ID fixed galloc problems for no file specified    ",
"    4.69 011109 ID allow -8000 and -a8 items                      ",
"    4.70 011122 ID fixed cropto functionality                     ",
"    4.71 011129 ID fixed -globalpalette to work with 8bit         ",
"    4.72 011130 ID two globalpalette files don't cause colour re. ",
"    4.73 011130 ID -16 (depth) should now reset default           ",
"    4.74 011207 ID fix divide by zero problem                     ",
"    4.75 011207 ID break up gxmain a bit to try to help with maint",
"    4.76 011213 ID fixed up frame count for multiple input files  ",
"    4.77 020117 ID fixed infinite loop for istext function        ",
"    4.78 020122 ID fixed range problem with negative ranges       ",
"    4.79 020131 ID changed the helpstr to autogen gimex formats   ",
"    4.80 020205 ID fixed the -f option to allow multiple files    ",
"    4.81 020205 FB -grey set bpc moved before bpp options         ",
"    4.82 020212 FB -p on numbered frames fixed                    ",
"    4.83 020212 ID -f fix for multiple files with -globalframe    ",
"    4.84 020213 FB -dot3 use unsigned char for height field       ",
"    4.85 020214 ID -dropshadow no longer supports 8bpp            ",
"    4.86 020215 FB -unpack alias for -pack0, 64 bit file io       ",
"    4.87 020216 FB -mipmap max option force 32 bpp, invert/dot3   ",
"    4.88 020218 FB -dot3(scale,filter (1=3x3)                     ",
"    4.89 020219 FB -framename and faster crapi exact/map          ",
"    4.90 020219 ID stop opening stream with invalid framecount    ",
"    4.91 020220 FB overwrite ram without rename                   ",
"    4.92 020221 FB dont open 'non files' on each frame            ",
"    4.93 020221 ID '' force -f to work as text                    ",
"    4.94 020222 FB dot3 detect ginfo, not OPTsubtype              ",
"    4.95 020225 FB embm similar to dot3 options                   ",
"    4.96 020226 ID don't use hardspaces for gxlib                 ",
"    4.97 020303 FB free memory on error                           ",
"    5.00 020304 FB mythrill 5.0 release                           ",
"    5.01 020308 ID don't flush stdout for gxlib, bad in MFC app   ",
"    5.02 020309 ID cleanup of gxfilter. reduce code size          ",
"    5.03 020312 FB -p dont get rid of USED transparent colours    ",
"    5.04 020314 ID OPTfilterframes was not cleared each iteration ",
"    5.05 020319 ID don't scan the file when we have non multiframe",
"    5.06 020322 AE -text option to render font onto image         ",
"    5.07 020408 FB -n -8 for colours to what -n said              ",
"    5.08 020409 FB -text take code page to offset ascii to unicode",
"    5.09 020409 ID globpal force 4DWu and do it only when necess. ",
"    5.10 020410 FB -char16 subtype for fonts                      ",
"    5.11 020411 FB spit getfield update for -pixel, -size options ",
"    5.12 020415 FB -swizzle etc combine, mipmap documentation     ",
"    5.13 020417 ID many to many =gif =ffn longer globalcol reduce ",
"    5.14 020427 ID validate the width height for swizzled shapes  ",
"    5.15 020506 FB unswizzle etc same as noswizzle                ",
"    5.16 020520 FB -name expression                               ",
"    5.17 020522 FB -name vs -name'' and -nopal fixed              ",
"    5.18 020522 ID mipmaps retain the name of the base texture    ",
"    5.19 020522 ID hotspots able to overrun the end of spit buffer",
"    5.20 020526 FB -isopaque, -istransparent                      ",
"    5.21 020529 FB -mipmap -nomipmapcollapse logic                ",
"    5.22 020602 FB -ishalfalpha                                   ",
"    5.23 020610 FB -pointsize -f0 is no4 'text'                   ",
"    5.24 020618 FB -antialias                                     ",
"    5.25 020624 ID 8bpp->globapallete not happen on single image  ",
"    5.26 020626 FB style                                          ",
"    5.27 020626 ID crammer no infinite loop on invalid texture    ",
"    5.28 020702 FB charset,ascii,typeface for fonts               ",
"    5.29 020703 FB -italic, -bold, -aspect                        ",
"    5.30 020703 ID globalpalette 4DWu, -tighten                   ",
"    5.31 020707 FB -outline                                       ",
"    5.32 020708 FB mem leak detect on PC using GlobalMemoryStatus ",
"    5.33 020709 FB -palgrad mask parameter                        ",
"    5.34 020716 FB -ffn 1 pass fix. -antialias2 fixed             ",
"    5.35 020717 FB SPIT_tempname replaces .old convention         ",
"    5.36 020720 FB SPIT_changeext when new format                 ",
"    5.37 020731 FB -style? documentation and whats new on web     ",
"    5.38 020731 ID mergepalette to respect -n -8 if it can        ",
"    5.39 020802 FB C_BLACK and CA_BLACK are ok too                ",
"    5.40 020808 FB RT build, -smallcaps0.8 for TTF                ",
"    5.41 020911 FB multi global to 1 global 2 pass auto off       ",
"    5.42 020918 FB cram reset variables between envokations       ",
"    5.44 020918 FB -return & -log                                 ",
"    5.45 021015 ID -pscolred photoshop colour reduce option       ",
"    5.46 021019 FB -fyi to adjust yiq and alpha for ps2/xbox      ",
"    5.47 021029 ID two pass on outline option                     ",
"    5.48 021102 FB -binsize                                       ",
"    5.49 021101 ID -usersize                                      ",
"    5.50 021105 ID -iscubemap now works with shape2 format        ",
"    5.51 021105 ID globalpalette not invoked on multipl size      ",
"    5.52 021112 ID -safe2_grey<n> option                          ",
"    5.53 021121 FB ShpF/SHPF,FntF/FNTF signature match            ",
"    5.54 021201 FB -tracking for adjusting font spacing           ",
"    5.55 021205 FB -newfsh map to fsh using firstextension about  ",
"    5.56 021218 ID -mipmapc preserves colour depth when mipmapping",
"    5.57 021220 ID -mipmapc retains channel depths                ",
"    5.58 030109 FB -maxwidth256 font subtype, -mmc bug fix        ",
"    5.59 030207 FB -blendmethod10 alpha test                      ",
"    5.60 030220 FB debug and icl shown as variables and help      ",
"    5.61 030224 ID added warnings for frames out of range         ",
"    5.62 030309 FB added -info                                    ",
"    5.63 030402 FB -name%s strips path                            ",
"    5.64 030423 FB PSNRY total calc for gcmp                      ",
"    5.65 030504 FB -h5 for vfw codecs, remove holiday             ",
"    5.66 030624 ID text versions of the quantizes -wu5 etc        ",
"    5.67 030626 FB mipmapb default for colour reduction           ",
"    5.68 030703 ID mkftexmap reports an error on odd sized images ",
"    5.69 030714 ID -mipmap minsize fix                            ",
"    5.70 030724 ID -gst option added                              ",
"    5.71 030807 ID added -text option                             ",
"    5.72 030829 FB -text use colour function, -ma                 ",
"    5.73 030911 FB -m10 does not add transparent                  ",
"    5.74 030916 FB -halfalpha using fshade filter                 ",
"    5.75 031020 FB -nonameelement to remove long name in old shp  ",
"    5.76 031103 FB -h4 show full extension                        ",
"    5.77 031201 FB -pq option (-m4) added, default, documented    ",
"    5.78 040110 FB -n256 -m9 transparent image numcolours bug fix ",
"    5.79 040113 FB first extension only if type specified         ",
"    5.80 051006 FB quicktime 7 fix, sleep in main loop            ",
"    5.81 051116 FB -vp6 (avi dottype) and quicktime import fix    ",
"    5.82 060320 FB -datarate<KB/s>                                ",
"    5.83 060424 FB -mpv for mpeg video on ps3                     ",
"    5.84 060508 FB -low -size1280x720, sleep 30 ms per frame      ",
"    5.85 060801 FB -sort fix, -layer, -temporalfix                ",
"    5.86 061116 FB -fdxt                                          ",
"    5.87 070106 FB -high priority, openmp dxt/halfsize            ",
"    5.88 070503 FB OMPNUMTHREADS, eadxt 1.0                       ",
"    5.89 070606 FB halfsize optimized                             ",
"    5.90 071017 FB -omp option                                    ",
"    5.91 071017 FB -omp, bloodhound disabled                      ",
"                                                                  ",
"------------------------------------------------------------------",
"                                                                  ",
" Note as merging and hitting finding transparent and black        ",
" should remove from dynamic list                                  ",
" -translucient/-opaque should attempt to map a colour from        ",
" dynamic list into that slot                                      ",
"                                                                  ",
" Description of GX tool goals                                     ",
"    file format conversion                                        ",
"    colour reduction                                              ",
"    cropping, resizing                                            ",
"    change center point, hot spots, position, name                ",
"    compression via compression based gimex modules               ",
"    bundling/unbundling of multiframe image formats               ",
"                                                                  ",
" File formats are either single frame, or multiframe.  GX will    ",
" support conversion from either of these types to either of the   ",
" types.   That is:                                                ",
"                                                                  ",
"  Class           Example                                         ",
"  -----           -------                                         ",
"  1 to 1          tga -> bmp                                      ",
"  1 to many       tga -> movie                                    ",
"  many to 1       movie -> tga                                    ",
"  many to many    movie -> movie                                  ",
"                                                                  ",
"------------------------------------------------------------------",
"\0"
};



/****************************************************************/
/*  Spit Setup                                                  */
/****************************************************************/

#define TOOL_VERSION 592
#define GX_DATE 20200728
#if !defined(GXRT)
const char TextVersion[]="5.92 (Distant) 2020.7.28";
#else
const char TextVersion[]="5.92 2020.7.28";
#endif

#define  GXMAIN             // for gxopt.h make this the home of the gxoptions variables.

#if defined(_MSC_VER)
#pragma warning(push,1)
#include "process.h"
#include "windows.h"
#include "winbase.h"
#pragma warning(pop)
#endif
#include <math.h>
#include "spit.h"
#include "gx.h"
#include "expr.h"
#include "gopen.h"
#include "wildname.h"
#include "gcolour.h"
#include "crapi.h"
#include "copen.h"
#include "hashidi.h"
#include "pshgimex.h"
#include "ttfgimex.h"
#include "gfilter.h"
#include "range.h"
#include "gsocket.h"
#include "gxfilter.h"
#include "gxvideo.h"
#include "expr.h"
#include "stdgimex.h"
#include "gxcram.h"
#include "ustr.h"
#include "codex.h"
//#include "bloodhound.h"
#include "fblend.h"

#include "gtoolerror.h"
#include "gxopt.h"


// do all version checking for each module. Abort compile on failure.
#if GIMEX_VERSION < 355
#error GX needs GIMEX_VERSION 355 or later
#endif
#if STDGIMEX_VERSION < 151
#error GX needs STDGIMEX_VERSION 151 or later
#endif
#if SPIT_VERSION < 396
#error GX needs SPIT 396 or later
#endif
#if GFILTER_VERSION < 109
#error GX needs GFILTER_VERSION 109 or later
#endif
#if CRAPI_VERSION < 169
#error GX needs CRAPI_VERSION 169 or later
#endif
#if CODEX_VERSION < 203
#error GX wants CODEX_VERSION 203 or later.  GX does not use CODEX, but Gimex does
#endif
#if EXPR_VERSION < 140
#error GX needs EXPR_VERSION 140 or later
#endif
#if GOPEN_VERSION < 107
#error GX needs GOPEN_VERSION 107 or later
#endif


// located in gxopts.cpp contains all help information on options.
extern char *optionhelp[];
// located in spit.cpp this is for forcing spit not to use hardspaces for the gxlib.
extern int OPTlibnohardspace;

EXPR *expr=0;

#define OPTAUTO 0x80000001

extern int galloccount;

#if !(defined(_MSC_VER) && !defined(_XBOX)) || defined(GXRT)

TTFINFO *TTFinfo=0;

#endif

void RestoreTemporal(void); // free up temporalfix gfilter
extern int gNumTemporalFix; // number of temporal fixed frames

/****************************************************************************/
/* Internal functions                                                       */
/****************************************************************************/

//------------------------------------------------------------------------
// check if a file exists by attempting to open the file and checking
// success.
static int GX_fileexist(char *nextname)
{
    int ok=0;
    GSTREAM *g;
    g = gopen(nextname);
    if (g)
    {
        ok = 1;
        gclose(g);
    }
    return(ok);
}

#define ISPOWER2(v) (!((v)&((v)-1)))
//------------------------------------------------------------------------
// calculate the correct width and height for any swizzled GSH.
// these come from gimex. We should really find a way of sharing
// the code. Frank and I will have to discuss it.
static int IsValidGSHSwizzleWidthHeight(int w,int h,int bpp)
{
    int realwidth  = w;
    int realheight = h;
    int lowheight  = 4;
    switch (bpp)
    {
        case 4:// size need to be div by 8x8 for 4 bit else can not twiddle
        {
            realwidth  = (w + 7)&~7;
            realheight = (h + 7)&~7;
            lowheight = 8;
            break;
        }
        case 8:// size needs to be div by 8x4 for 8 bit else can not twiddle
        {
            realwidth  = (w + 7)&~7;
            realheight = (h + 3)&~3;
            lowheight  = 8;
            break;
        }
        case 15:
        case 16:
        case 32://size needs to be div by 4x4 for higher bit else can not twiddle
        {
            realwidth  = (w + 3)&~3;
            realheight = (h + 3)&~3;
            break;
        }
    }
    if((realwidth != w || realheight != h) && (h > lowheight || w > lowheight))
        return 0;
    else
        return 1;
}

//------------------------------------------------------------------------
// calculate the correct width and height for any swizzled SSH.
// these come from gimex. We should really find a way of sharing
// the code. Frank and I will have to discuss it.

// ------------------------------------------------------------
static int IsValidSSHSwizzleWidthHeight(int w,int h,int bpp)
{
    int realwidth  = w;
    int realheight = h;
    switch (bpp)
    {
        case 4:// size need to be div by 16x32 for 4 bit else can not swizzle
        {
            realwidth  = (w + 15)&~15; // 32x32, 64x64, 128x128 or multiple of 256 (rectangular okay)
            realheight = (h + 31)&~31;
            break;
        }
        case 8:// size needs to be div by 16x16 for 8 bit else can not swizzle
        {
            realwidth  = (w + 15)&~15;
            realheight = (h + 15)&~15;
            break;
        }
        case 15:
        case 16:// size needs to be div by 8x16 for 15 bit else can not swizzle
        {
            realwidth  = (w + 7)&~7;
            realheight = (h + 15)&~15;
            break;
        }
    }
    if ((realwidth == w && realheight == h)
     || (ISPOWER2(w) && ISPOWER2(h)))
        return 1;
    return 0;
}


// ------------------------------------------------------------
// calculate the correct width and height for any swizzled SSH.
// these come from gimex. We should really find a way of sharing
// the code. Frank and I will have to discuss it.
static int IsValidXSHSwizzleWidthHeight(int w,int h,int bpp)
{
    if(ISPOWER2(w) && ISPOWER2(h))
        return 1;
    else
        return 0;
}

// ------------------------------------------------------------
// calculate the correct width and height for any swizzled SSH.
// these come from gimex. We should really find a way of sharing
// the code. Frank and I will have to discuss it.
static int IsValidFSHSwizzleWidthHeight(int w,int h,int bpp)
{
    if(ISPOWER2(w) && ISPOWER2(h))
        return 1;
    else
        return 0;
}

// ------------------------------------------------------------
static bool IsShape(unsigned int signature)
{
    return
        (SignatureCompare(signature,GMAKEID('S','H','P','X')) ||
         SignatureCompare(signature,GMAKEID('S','h','p','F')) ||
         SignatureCompare(signature,GMAKEID('S','h','p','X')) ||
         SignatureCompare(signature,GMAKEID('S','h','p','G')) ||
         SignatureCompare(signature,GMAKEID('S','h','p','S')) );
}

// ------------------------------------------------------------
static bool IsShape(unsigned int signature, unsigned char EndChar)
{
    return
        (SignatureCompare(signature,GMAKEID('S','H','P',EndChar)) ||
         SignatureCompare(signature,GMAKEID('S','h','p',EndChar)) );
}

// ------------------------------------------------------------
static bool IsGstFourCC(unsigned int packed)
{
  if((packed&0xFFFFFF00) == 0x47535400) // if the fourCC == 'GST '
    return true;
  return false;
}

// ------------------------------------------------------------
static bool IsMovie(GABOUT * about)
{
  return(about->multiframe && about->movie && !about->multisize);
}

// ------------------------------------------------------------
bool GX_ismipmap(char * shape_ID,int len)
{
    // if the name is the old one.
    if (shape_ID[0] == 'm'
     && shape_ID[1] == 'm'
     && shape_ID[2] >= '0' && shape_ID[2] <= '9'
     && ((shape_ID[3] >= '1' && shape_ID[3] <= '9') || shape_ID[2] >= '1'))
        return true;

    // if the name is the new one.
    if (len == 0 || len > 4)
    {
        int    i = 0;
        char * s = shape_ID;

        if (len == 0)
            len = strlen(shape_ID) - 3;

        for (i = 0; i < len; i++)
        {
            if ((s[0] == '_') &&
                (s[1] == 'm') &&
                (s[2] == 'm') &&
                (s[3] >= '0') && (s[3] <= '9'))
                    return true;
            s++;
        }

    }

    // otherwise not a mipmap.
    return false;
}

//------------------------------------------------------------------------
// validate shape width and height for swizzleing.
static void GXVerifyFrame(GINFO * ginfo,GABOUT *wabout)
{
    int validMatch;

    // lets check shapes for valid sizes when we want to do swizzleing.
    {
        static bool movieMipmapDoneOnce = false;
        if(IsMovie(wabout) && GX_ismipmap(ginfo->framename,strlen(ginfo->framename)) && !movieMipmapDoneOnce)
        {
            movieMipmapDoneOnce = true;
            gprintf("WARNING: [%s] exporting mipmaps to a movie can result in corrupt data\n",ginfo->framename);
        }
        else if (ginfo->subtype&SUBSWIZZLE && IsShape(wabout->signature,'X'))
        {
            static int xdoneonce = 0;
            validMatch = IsValidXSHSwizzleWidthHeight(ginfo->width,ginfo->height,ginfo->originalbpp);
            if(OPTverbose && !validMatch && !xdoneonce)
            {
                gprintf("WARNING: [%s] swizzle flag turned on but image width and height\n         are not power of 2. Image data may not be valid.\n",ginfo->framename);
                xdoneonce = 1;
            }
        }
        // GST compression
        else if (IsGstFourCC(ginfo->packed) && IsShape(wabout->signature))
        {
            static int gstdoneonce  = 0;
            static int gstmiponce   = 0;
            int width_align         = 32;
            int height_align        = 16;
            int newwidth            = (ginfo->width +(width_align-1))  & ~(width_align-1);
            int newheight           = (ginfo->height+(height_align-1)) & ~(height_align-1);
            validMatch              = (newwidth == ginfo->width && newheight == ginfo->height);
            if(OPTverbose)
            {
              if(!gstdoneonce)
              {
                // no swizzle for ssh
                if(IsShape(wabout->signature,'S') && !(ginfo->subtype&SUBSWIZZLE))
                {
                  gprintf("WARNING:  [%s] GST compressing SSH without the SWIZZLE flag turned on.\n",ginfo->framename);
                  gstdoneonce = 1;
                }
                // Dimension check
                if(!validMatch)
                {
                  gprintf("WARNING:  [%s] Invalid GST texture size (%dx%d), Image dimensions must be a multiple of %dx%d\n",ginfo->framename,ginfo->width,ginfo->height,width_align,height_align);
                  gstdoneonce = 1;
                }
              }
              // GST with mipmap check
              // Mipmaps have to check the second frame
              // but I don't want my other warnings to
              // mask this warning since it's fairly important
              // thus it gets it's own status var.
              if(!gstmiponce)
              {
                if(GX_ismipmap(ginfo->framename,strlen(ginfo->framename)))
                {
                  gprintf("WARNING:  [%s] Attempt to GST compress mipmaps, GST should only be used with -mipmont.\n",ginfo->framename);
                  gstmiponce = 1;
                }
              }
            }
        }
        else if (ginfo->subtype&SUBSWIZZLE && IsShape(wabout->signature,'S'))
        {
            static int sdoneonce  = 0;
            static int sdoneonce2 = 0;

            validMatch = IsValidSSHSwizzleWidthHeight(ginfo->width,ginfo->height,ginfo->originalbpp);
            if(OPTverbose && !validMatch && !sdoneonce)
            {
                gprintf("WARNING:  [%s] swizzle flag turned on but the image width and height\n         are not the correct dimensions. Image data may not be valid.\n",ginfo->framename);
                sdoneonce = 1;
            }
            if(OPTverbose && ginfo->originalbpp == 24 && !sdoneonce2)
            {
                gprintf("WARNING: [%s] swizzle flag turned on but image is 24bit\n         Image will not be swizzled.",ginfo->framename);
                sdoneonce2 = 1;
            }
        }
        else if (ginfo->subtype&SUBSWIZZLE && IsShape(wabout->signature,'G'))
        {
            static int gdoneonce = 0;
            validMatch = IsValidGSHSwizzleWidthHeight(ginfo->width,ginfo->height,ginfo->originalbpp);
            if(OPTverbose && !validMatch && !gdoneonce)
            {
                gprintf("WARNING: [%s] swizzle flag turned on but image width and height\n         are not proper dimensions. Image data may not be valid.\n",ginfo->framename);
                gdoneonce = 1;
            }
        }
        else if (ginfo->subtype&SUBSWIZZLE && IsShape(wabout->signature,'F'))
        {
            static int fdoneonce = 0;
            validMatch = IsValidFSHSwizzleWidthHeight(ginfo->width,ginfo->height,ginfo->originalbpp);
            if(OPTverbose && !validMatch && !fdoneonce)
            {
                gprintf("WARNING: [%s] swizzle flag turned on but image width and height\n         are not power of 2. Image data may not be valid.\n",ginfo->framename);
                fdoneonce = 1;
            }
        }
        else if (ginfo->packed == 4         &&
                 ginfo->bpp    != 8         &&
                 IsShape(wabout->signature) &&
                 (ginfo->width  != ((ginfo->width+15)&~15) ||
                  ginfo->height != ((ginfo->height+15)&~15)
                  ))
        {
            static int iframedoneonce = 0;
            if(OPTverbose && !iframedoneonce)
            {
                gprintf("WARNING: [%s] attempt to export to an IFrame shape that is not a multiple of 16x16\n         texture will not be compressed\n",ginfo->framename);
                iframedoneonce = 1;
            }
        }
    }
}



//------------------------------------------------------------------------
// if we are working on a windows machine then set console title to
// string.
static void gsettitle(char *title)
{
#if defined(_MSC_VER) || defined(__WATCOMC__)
    if (OPTverbose >= 3)
        SetConsoleTitle(title);
#endif
}

// on windows get the current console title 
static int ggettitle(char* title, int length)
{
#if defined(_MSC_VER) || defined(__WATCOMC__)
    return GetConsoleTitleA(title, length);
#endif
    return 0;
}



/* if user specifies a type, but no extension, this should */
/* force a change of extension.  otherwise, use whats there. */

//------------------------------------------------------------------------
// verify that any given extension is valid with GIMEX. returns non
// zero on success.
static int isvalidextension(GABOUT *about,char *ext,int type)
{
    int exttype;
    int ok=0;

    if (ext)
    {
        exttype = GIMEX_find(ext,0);
        if (!exttype || exttype==type)
            ok = 1;
    }

    return(ok);
}

//------------------------------------------------------------------------
// find and return a pointer to the begining of the extension within
// a string. returns 0 if no extension found.
static char *GXextension(char *filename)
{
    char *ext=filename+(strlen(filename))-1; /* find extension */
    while (ext>filename)
    {
        if (*ext=='.')
            break;
        if (*ext==(char)':' || *ext==(char)'/' || *ext==(char)'\\')
            break;
        --ext;
    }
    if (*ext!='.')
        ext = 0;
    return(ext);
}

static void TTF_pointsize(const char *OPTpointsize, const char *OPTantialias, const char *OPTstyle, int OPTitalic, int OPTbold, const char *OPTcharset, const char *OPTtypeface, const char *OPTaspect, const char *OPTtighten, const char *OPTtracking, const char *OPTsmallcaps, const char *OPTascii, const char *OPTbaseline, int frame)
{
    TTFINFO *ttf = (TTFINFO *) TTFinfo;
    if (TTFinfo)
    {
        EXPR_set(expr,"frame",frame);   // available for font pointsize
        EXPR_set(expr,"framenum",frame);

        // point size

        float pointsize = ttf->pointsize;
        if (OPTpointsize[0])
            pointsize = (float)GX_constrain(OPTpointsize,(double)ttf->pointsize);
        ttf->pointsize = pointsize;

        // bit depth control

        int antialias = 1;
        if (OPTantialias[0])
            antialias  = (int)GX_constrain(OPTantialias,(double)((ttf->flags&1)?0:1));
        if (antialias==0)       ttf->flags = 1;
        else if (antialias==1)  ttf->flags = 0;
        else                    ttf->flags = 2;

        // aspect ratio. ie 0.5 is for 640x240 screens
        float aspect = 1.0f;
        if (OPTaspect[0])
            aspect = (float)GX_constrain(OPTaspect,(double)ttf->aspect);
        ttf->aspect = aspect;

        // adjust spacing.  <1.0 is tight.  >1 is widen
        float tighten = 1.0f;
        if (OPTtighten[0])
            tighten = (float)GX_constrain(OPTtighten,(double)ttf->tighten);
        ttf->tighten = tighten;

        // adjust spacing.  <-0 is tight.  >0 is widen
        float tracking = 0.0f;
        if (OPTtracking[0])
            tracking = (float)GX_constrain(OPTtracking,(double)ttf->tracking);
        ttf->tracking = tracking;

        // small caps feature.  <0 is none. 1.0 is all caps. 0.8 is small caps default>
        float smallcaps = 0.8f;
        if (OPTsmallcaps[0])
            smallcaps = (float)GX_constrain(OPTsmallcaps,(double)ttf->smallcaps);
        ttf->smallcaps = smallcaps;


        // bold, italic, strikethrough
        int style = 0;
        if (OPTstyle[0])
            style = (int)GX_constrain(OPTstyle,(double)ttf->style);
        if (OPTitalic)
            style |= 2;
        if (OPTbold)
            style |= 1;
        ttf->style = style;

        // charset is languages. 129=asian.
        int charset = 0;
        if (OPTcharset[0])
            charset = (int)GX_constrain(OPTcharset,(double)ttf->charset);
        ttf->charset = charset;

        // ascii limit to 32..255
        int ascii = 1;
        if (OPTascii[0])
            ascii = (int)GX_constrain(OPTascii,(double)ttf->ascii);
        ttf->ascii = ascii;

        // baseline flag 0 or 1
        int baseline = 1;
        if (OPTbaseline[0])
            baseline = (int)GX_constrain(OPTbaseline,(double)ttf->baseline);
        ttf->baseline = baseline;

        // name of font
        if (OPTtypeface[0])
            USTR_strcpy(ttf->fontname,OPTtypeface);
    }
}


//---------------------------------------------------------------------
// handle the messy business of setting the tools title bar.
// watch out for the 3 different tools.
static void GX_settooltitle(GABOUT * about,char *fnamebuf,int pass)
{
    if (about)
    {
#if 0

        // set window title

        char titlebuf[SPITMAXLINE];
        if (OPTverbose>=3)
            sprintf(titlebuf,"%s - GX Reading Pass %d",fnamebuf,pass);
        else
            sprintf(titlebuf,"%s - GX Reading",fnamebuf);
        gsettitle(titlebuf);
#endif

        // print out what file I am currently reading.

        if (OPTverbose && (pass==1 || OPTverbose>=4 || OPTglobalpalette || OPTflatten || OPTcram))
            gprintf("%s '%s'\n",about->longtypestr,fnamebuf);

    }
}


//------------------------------------------------------------------------
// check to see if the gimex module supports exporing and verify
// bitdepth. Also checks GIMEX version number.
// Returns FALSE if we have an invalid input and gx should
// goto ErrorExit in this case.
// Also returns a flag (int * w) that specifies if the user
// have been warned or not during function call.
// w can be set to true if user should not be warned of anything.
static bool GX_verifyoutputfile(GABOUT * wabout,int SPITargnum,int * w,int morethanonefile)
{
    int warned = *w;                                                              // don't warn the user twice about anything.
    if (wabout)
    {
        if (OPTwrite)                                                              // only flag things as errors if the user
        {                                                                          // requesting to write out. This is on by default.
            if (!wabout->canexport)                                                // does the module selected have the ability to export?
            {
                if (OPTverbose)
                    gprintf(TOOL_NAME " - CAN NOT EXPORT '%s' FORMAT.\n",wabout->wordtypestr);
                return false;                                                                // not gx should goto it's ErrorExit code on failure.
            }
                                                                                             // can I export a high enough bit depth and the user only
            if (!wabout->export32 && (OPTdepth<=32) && (OPTdepth>8) && ((SPITargnum-2)<=0))  // specified this one file?
            {
                if (OPTverbose)
                    gprintf(TOOL_NAME " - '%s' DOES NOT SUPPORT %d BPP.\n",wabout->wordtypestr, OPTdepth);
                return false;                                                                 // note gx should goto it's ErrorExit code on failure.
            }
                                                                                              // the user wants to export to 32 bit, format does not support it
            if (!wabout->export32 && (OPTdepth<=32) && (OPTdepth>8) && OPTverbose && !warned) // and they exported some other number of files then warn them.
            {
                gprintf(TOOL_NAME " - WARNING '%s' DOES NOT SUPPORT %d BPP.\n",wabout->wordtypestr, OPTdepth);
                warned = 1;
            }
                                                                                              // do the same for 8bpp.
            if (!wabout->export8 && (OPTdepth>=1) && (OPTdepth<=8) && OPTverbose && !warned)
            {
                gprintf(TOOL_NAME " - WARNING '%s' DOES NOT SUPPORT %d BPP.\n",wabout->wordtypestr, OPTdepth);
                warned = 1;
            }
        }
        if (wabout->globalpalette && wabout->export8 && !OPTglobalpalette)         // only if I'm going to export to 8 bit format needs a global palette
        {                                                                          // and it's not already on.
            int willbe8=0;                                                         // not sure if I'm going to 8 bit yet.

            // going to export to 8 bit globalpalette AND we have more than one file OR
            // one file in which the input format doesn't support globalpalettes.
            // the latter is done later once we have loaded the input file.
            if ((!wabout->export32
                || ((OPTdepth>=1) && (OPTdepth<=8))
                || OPTcolours)
                && morethanonefile > 1)
            {
                willbe8 = 1;                                                       // definately will be 8
                OPTglobalpalette = 1;
            }

            if (OPTverbose>=2)                                                     // let the user know what we are doing.
                if (OPTverbose>=(4-willbe8))                                       // spit out the message with the assurance we have
                    gprintf(TOOL_NAME " - NOTE '%s' FORMAT ENABLING GLOBAL PALETTE.\n",wabout->wordtypestr);  // that texture will indeed end up as 8bpp.
        }

        {
            static int warnexportversion=0;                                        // only do this once an iteration.
            if (wabout->version<234 && !warnexportversion && OPTverbose)           // what is the minimum gimex version that I am willing to tolerate nicely.
            {
                gprintf(TOOL_NAME " - WARNING '%s' GIMEX IS VERSION %.2f.  GX WANTS 2.30\n",wabout->wordtypestr,(float)wabout->version/100.0f);
                warnexportversion = 1;
            }
        }
    } // end if wabout
    *w = warned;                                                                   // don't warn the user twice.
    return 1;
}

//------------------------------------------------------------------------
// is this a simple decimal number, not preceeded by zeros (octal or framename)
// used to determine if -f is a named frame or numerical

static bool GX_isdecimal(const char *data)
{
    bool isdigits=false;
    const char *p = data;

    while (*p>='0' && *p<='9')  // search for first non-digit (or end of string)
        ++p;

    if (!*p && (*data!='0' || p!=data)) // all digits and not zero padded?
        isdigits = true;

    return isdigits;
}

/****************************************************************/
/*  SPIT Routines                                               */
/****************************************************************/

//------------------------------------------------------------------------
// setup all basic spit, internal expression handling items,
// spit uses this to reset the defaults you want to use.
static void SPIT_reset(void)
{
    GXFILTER_reset();
#ifdef GCMP
    EXPR_set(expr,"GCMP_VERSION",TOOL_VERSION);
    EXPR_set(expr,"GCMP_DATE",GX_DATE);
    OPTpsnr = 1;
    OPTwrite = 0;
#else
    EXPR_set(expr,"GX_VERSION",TOOL_VERSION);
    EXPR_set(expr,"GX_DATE",GX_DATE);
#endif
#ifdef GXLIB
    EXPR_set(expr,"GXLIB",1);
    OPTautoquote = 0;
    OPTbanner = 0;
    OPTverbose = 1;
#endif
    EXPR_set(expr,"TOOL_VERSION",TOOL_VERSION);
    EXPR_set(expr,"GIMEX_VERSION",GIMEX_VERSION);
    EXPR_set(expr,"EXPR_VERSION",EXPR_VERSION);
    EXPR_set(expr,"SPIT_VERSION",SPIT_VERSION);
    EXPR_set(expr,"STDGIMEX_VERSION",STDGIMEX_VERSION);

#ifdef DEBUG
    EXPR_set(expr,"DEBUG",1.0);
#else
    EXPR_set(expr,"DEBUG",0.0);
#endif
#ifdef __ICL
    EXPR_set(expr,"INTEL",1.0);
#else
    EXPR_set(expr,"INTEL",0.0);
#endif

    EXPR_set(expr,"year",2018);

    EXPR_set(expr,"GIMEX_FRAMENAME_SIZE",    GIMEX_FRAMENAME_SIZE);
    EXPR_set(expr,"GIMEX_COMMENT_SIZE",      GIMEX_COMMENT_SIZE);
    EXPR_set(expr,"GIMEX_COLOURTBL_SIZE",    GIMEX_COLOURTBL_SIZE);
    EXPR_set(expr,"GIMEX_HOTSPOTTBL_SIZE",   GIMEX_HOTSPOTTBL_SIZE);
    EXPR_set(expr,"GIMEX_HOTSPOTTBL_VALUES", GIMEX_HOTSPOTTBL_VALUES);

    NAMEsubtype=0;


    return;
}

//------------------------------------------------------------------------
// take an ID packed into an integer and convert it to a string
// representation. strips trailing or starting zero values off
// of string.
static char *GXid2str(char *buf, unsigned int id)
{
    int i;
    int n;

    for (i=0,n=0; i<4; ++i)
        if ((id>>(24-i*8))&0xff)
            buf[n++] = (char) (id>>(24-i*8));
    buf[n] = 0;
    return(buf);
}

//------------------------------------------------------------------------
// check if a character is alphanumeric or _ returns non zero if true.
static int iswhitestart(const char c)
{
    return(!(isalnum(c) || c=='_'));
}

//------------------------------------------------------------------------
// check if a character is non alphabetic or _ returns non zero if true.
static int iswhiteend(const char c)
{
    return(!(isalpha(c) || c=='_'));
}


//------------------------------------------------------------------------
// Case insensitive strstr
static char *GX_strstrwhite(const char *str, const char *substr)
{
    int len1;
    int len2;
    int i = 0;
    char a = substr[0];
    char b;

    if (islower(a))
        b = (char)toupper(a);
    else
        b = (char)tolower(a);

    len2 = strlen(substr);
    len1 = strlen(str) - len2;

    while (i < len1)
    {
        if ((str[i] == a) || (str[i] == b))
        {
            if (!strnicmp(&(str[i]), substr, len2))
            {
                int iswhite=1;
                if (i>0 && !iswhitestart(str[i-1]))
                    iswhite = 0;
                if (!iswhiteend(str[i+len2]))
                    iswhite = 0;
                if (iswhite)
                    return((char *)str+i);
            }
        }
        ++i;
    }

    return(NULL);
}

//------------------------------------------------------------------------
// show GIMEX module about information.
void GXshowabout(GABOUT *about)
{
    int j;
    char idbuf[8];
    static bool shown=false;

    if (shown)
        return;
    shown = true;


    if (OPTverbose==2)
    {
        printf("Format Version:    %s\n",about->versionstr);
        printf("gimex.dll Version: %.2f\n",(float)about->version/100.0f);
        printf("Short Name:    %s\n",about->shorttypestr);
        printf("Long Name:     %s\n",about->longtypestr);
        printf("Extensions:    ");
        for (j=0; j<MAXEXTENSIONS; ++j)
            if (about->extensions[j][0])
                printf("%s",about->extensions[j]);
        printf("\n");
        printf("Features:      ");

        if ((about->import8 || about->export8) && (about->import32 || about->export32))
            gprintf("8/32 ");
        else if ((about->import8 || about->export8))
            gprintf("8 ");
        else if ((about->import32 || about->export32))
            gprintf("32 ");
        else
            gprintf("- ");
        if (about->canimport && about->canexport)
            gprintf("import/export ");
        else if (about->canimport)
            gprintf("import only ");
        else if (about->canexport)
            gprintf("export only ");
        else
            gprintf(" ");

        if (about->multiframe)
        {
            if (about->movie)
                gprintf("movie");
            else if (about->font)
                gprintf("font");
            else if (about->mipmap)
                gprintf("texture");
            else
                gprintf("pages");
        }
        else
            gprintf("image");

        if (about->file64)
            gprintf("64");

        gprintf("\n");
    }
    else if (OPTverbose>=3)
    {
        printf("ABOUT %s\n",about->longtypestr);
        printf("version          %.2f\n",(float)about->version/100.0f);
        printf("canimport    %8d        canexport    %8d\n",about->canimport,about->canexport);
        printf("importpacked %8d        exportpacked %8d\n",about->importpacked,about->exportpacked);
        printf("import8      %8d        export8      %8d\n",about->import8,about->export8);
        printf("import32     %8d        export32     %8d\n",about->import32,about->export32);
        printf("multiframe   %8d        multifile    %8d\n",about->multiframe,about->multifile);
        printf("framebuffer  %8d        external     %8d\n",about->framebuffer,about->external);
        for (j=0; j<MAXMACTYPES; ++j)
            if (about->mactype[j])
                printf("mactype[%d]       %s\n",j,GXid2str(idbuf,about->mactype[j]));
        for (j=0; j<MAXEXTENSIONS; ++j)
            if (about->extensions[j][0])
                printf("extensions[%d]    %s\n",j,about->extensions[j]);
        printf("authorstr        %s\n",about->authorstr);
        printf("versionstr       %s\n",about->versionstr);
        printf("shorttypestr     %s\n",about->shorttypestr);
        printf("wordtypestr      %s\n",about->wordtypestr);
        printf("longtypestr      %s\n",about->longtypestr);
    }
}


// move this later when I am sure this stuff works.
typedef struct TYPESTRS
{
    char * extension;
    char * longname;
}TYPESTRS;

//------------------------------------------------------------------------
// generate a list of structures holding my extensions and their
// Long Type Strings. This should be used for the help string
// information.
static TYPESTRS * GX_generateGimexTypes()
{
    GABOUT *about;
    TYPESTRS * list = 0;
    unsigned int type = 0;
    unsigned int count = GIMEX_count();
    unsigned int i = 0;
    unsigned int m = 0;
    list = (TYPESTRS *)galloc((sizeof(TYPESTRS)*count)+1*sizeof(TYPESTRS));
    memset(list,0,(sizeof(TYPESTRS)*count)+1*sizeof(TYPESTRS));
    if(list)
    {
#ifdef DEBUG
        if (OPTverbose>=5)
            printf("gimex types %d\n",count);
#endif

        for (type=0; type<count-1; ++type)
        {

            GIMEX_set(type+1);
            about = GIMEX_about();
            if (about)
            {
                list[type].extension = (char *)galloc(strlen(about->shorttypestr)+5);     // create memory for my two new entries
                list[type].longname  = (char *)galloc(strlen(about->longtypestr)+5);
                strcpy(list[type].extension,about->shorttypestr);                         // copy the items over into my buffers
                strcpy(list[type].longname,about->longtypestr);
                for(i = 0; i < strlen(list[type].extension);++i)                          // keep the extensions uniform, all lowercase.
                    list[type].extension[i] = tolower(list[type].extension[i]);
                char *temp1 = list[type].extension;                                             // copy the pointers so I can do a dirty insertion sort.
                char *temp2 = list[type].longname;

#ifdef DEBUG
                if (OPTverbose>=5)
                    printf("type[%d] %s\n",type,temp1);
#endif


                for (i = 0; i < type; ++i)                                                  // insertion sort them alphabetically.
                {
                    if (strcmp(list[type].extension,list[i].extension) < 0)
                    {
                        for(m = type; m > i; m--)
                        {
                            list[m].extension = list[m-1].extension;
                            list[m].longname  = list[m-1].longname;
                        }
                        list[i].extension = temp1;
                        list[i].longname  = temp2;
                        break;
                    }
                }
            }
            gfree((char *)about);
        }
    }
    return(list);
}


//------------------------------------------------------------------------
// kill all the buffers I created in my types structures and then
// kill the list itself.
static void GX_killTypeStrs(TYPESTRS * t)
{
    int i = 0;
    if(t)
    {
        while(t[i].extension)
        {
            if(t[i].extension)
                gfree(t[i].extension);
            if(t[i].longname)
                gfree(t[i].longname);
            i++;
        }
        gfree(t);
    }
}

//------------------------------------------------------------------------
// print out all the GIMEXTYPES to the screen.
static void GX_printGIMEXTYPES(TYPESTRS * t)
{
    int count = 0;
    int i     = 0;
    int len   = 0;
    if(t)
    {
        gprintf("TYPES:    ");
        while(t[i].extension)
        {
            len = strlen(t[i].extension);
            len = len < 4?4:len;
            if((count + len + 1) > 64)
            {
                count = 0;
                gprintf("\n          ");

            }
            gprintf(".%-4s",t[i].extension);
            count += len + 1;
            i++;
        }
        gprintf("\n");
    }
}


//------------------------------------------------------------------------
// print out the gimex types string in alphabetical order.
// wrapper for GX_printGIMEXTYPES(TYPESTRS * t)
static void GX_printAllGimexTypes()
{
    TYPESTRS * typestrs = GX_generateGimexTypes();
    GX_printGIMEXTYPES(typestrs);
    GX_killTypeStrs(typestrs);
    typestrs = 0;
}




//------------------------------------------------------------------------
// print out the help string on the screen.
static void GX_printHelpString()
{
    TYPESTRS * typestrs = 0;
    char ** hlp = optionhelp;
    int index = 0;
    typestrs = GX_generateGimexTypes();
    while (*hlp)
    {
        if(!strncmp(*hlp,"COLOUR SYNTAX:\n",13))
        {
            gprintf("FILE FORMATS:\n");
            while(typestrs[index].extension)
            {
                gprintf("    -%-6s%-27s\n",typestrs[index].extension,typestrs[index].longname);
                index++;
            }
        }
        gprintf("%s",*hlp);
        ++hlp;
    }
    GX_printGIMEXTYPES(typestrs);
    GX_killTypeStrs(typestrs);
    typestrs = 0;
}


//------------------------------------------------------------------------
// dump out gx usage information to the command line.
static void SPIT_usage(const char *prgname,const char *envname,const char *urlname,int utype)
{
    char **hlp;
    TYPESTRS * typestrs = 0;
    int index = 0;

#if defined(GXRT)
    gprintf( TOOL_NAME " v%s (c) 1996-2003 Electronic Arts Inc.\n",TextVersion);
#else
    gprintf( TOOL_NAME " v%s by Frank Barchard,EAC\n",TextVersion);
#endif
#ifdef GCMP
    if (utype)
        gprintf("USAGE: %s -options image1 image2\n", prgname);
#else
    if (utype)
        gprintf("USAGE: %s -options imagein [=imageout]\n", prgname);
#endif
    switch (utype)
    {
        case 2:
#ifndef GCMP
            gprintf("    =file output file\n");
            gprintf("    @file response file\n");
            gprintf("    -ext  file format (see Types below)\n");
            gprintf("    -n    convert to n bits (1..32)\n");
#endif
            gprintf("    -h    more help (or ?)\n");
#if !defined(GXRT)
            gprintf("    -help web page (%s)\n",urlname);
#endif
            GX_printAllGimexTypes();
            break;

       case 3:
          gprintf("\n");
          gprintf("VERSION: %s v%s, Spit v%.2f\n",SPITmachine,TextVersion,(float)SPIT_VERSION/100.0f);
#ifdef GCMP
            gprintf("SUMMARY: GCMP graphics file comparison tool\n");
#else
            gprintf("SUMMARY: GX graphics file format conversion tool\n");
#endif
            gprintf("         SPIT environment variable is %s\n",envname);
            gprintf("\n");
            GX_printHelpString();
            break;

        case 4:
            gprintf("\n");
            if (strstr(envname,"_OPTS"))
            {
                int type;
                int i;
                if (OPTverbose>=2)
                    gprintf("FILE FORMATS:\n");

                for (type=1; type<GIMEX_count(); ++type)
                {
                    GABOUT *about;

                    GIMEX_set(type);
                    about = GIMEX_about();
                    if (about)
                    {

                        gprintf("%-6s", about->shorttypestr);

                        const int MAXGIMEXEXTENSIONS=4;
                        const int MAXEXTENSIONBUFFER=128;
                        char ExtensionsBuffer[MAXEXTENSIONBUFFER];
                        ExtensionsBuffer[0] = '\0';

                        for (i=0; i < MAXGIMEXEXTENSIONS; i++)
                        {
                            char NextExtensionsBuffer[MAXEXTENSIONBUFFER];

                            sprintf(NextExtensionsBuffer,"%-5s", about->extensions[i]?about->extensions[i]:"");
                            strcat(ExtensionsBuffer,NextExtensionsBuffer);
                        }

                        gprintf("%-20.20s", ExtensionsBuffer);

                        gprintf("%-27.27s", about->longtypestr);
                        if ((about->import8 || about->export8) && (about->import32 || about->export32))
                            gprintf("8/32 ");
                        else if ((about->import8 || about->export8))
                            gprintf("8    ");
                        else if ((about->import32 || about->export32))
                            gprintf("32   ");
                        else
                            gprintf("-    ");
                        if (about->canimport && about->canexport)
                            gprintf("import/export ");
                        else if (about->canimport)
                            gprintf("import only   ");
                        else if (about->canexport)
                            gprintf("export only   ");
                        else
                            gprintf("              ");

                        if (about->multiframe)
                        {
                            if (about->movie)
                                gprintf("movie");
                            else if (about->font)
                                gprintf("font");
                            else if (about->mipmap)
                                gprintf("texture");
                            else
                                gprintf("pages");
                        }
                        else
                            gprintf("image");

                        if (about->file64)
                            gprintf("64");

                        gprintf("\n");

                        gfree((char *)about);
                    }
                }
            }
            else // Help on a particular option is also h4 via spit.
            {
                hlp = optionhelp;
                while (*hlp)
                {
                    if (GX_strstrwhite(*hlp,envname))
                        gprintf("%s",*hlp);
                    ++hlp;
                }
                gprintf("\n");

                if (OPTverbose>=2)
                {
                    int type=0;
                    int NAMEsubtype;

                    type = GIMEX_find(envname,&NAMEsubtype);
                    if (type)
                    {
                        GABOUT *about;

                        about = GIMEX_about();
                        if (about)
                        {
                            GXshowabout(about);
                            SPIThelpgiven = 1;
                            gfree((char *)about);
                        }
                    }
                }
            }
            break;

        case 5:
#if defined(_MSC_VER)
            GX_ShowCodecs();
#endif
            break;

        default:
            break;
    }
}

//------------------------------------------------------------------------
// determins if a token is a path by looking for \/:. and so forth
static int GX_ispath(const char *s)
{
    int l;
    int ok=0;

    l = strlen(s);
    if (l>=1)
    {
        if (s[0]!='-' && (s[l-1]=='\\' || s[l-1]=='/' || s[l-1]==':' || s[l-1]=='.'))
        {
            if (l>=2)
            {
                /* c: */
                if ((s[l-1]==':' && isalpha(s[0])
#if defined(_MSC_VER)
                   && l==2
#endif
                )
                /* c:. . .. c:\dir\. */
                || (s[l-1]=='.' && (s[l-2]=='\\' || s[l-2]=='/' || s[l-2]==':' || s[l-2]=='.'))
                || s[l-1]=='/'
                || s[l-1]=='\\')
                    ok = 1;
            }
            else
                ok = 1;
        }
    }
    return(ok);
}



//------------------------------------------------------------------------
// gx application code, called by both the lib and the command line
// versions.
int gxmain(int argc,char **argv)
{
    int more=0;
    int i;
    int frame;
    int inframe;
    int outframe;
    int pass;
    int n;
    int j;
    int ok;
    int lframes;
    int totalframes=0;
    int frames=0;
    int framestart;
    int frameend;
    int type;
    int wtype=0;
    int isopt;
    int *framecount=0;
    int *filecount =0;
    char filename[SPITMAXLINE];
    char *f;
    int rowbytes;
    int warned = 0;
    int alreadyopen = 0;
    int infilecount = 0;
    int SourceArtContains8bpp = 0;
    GABOUT *about=0;
    GABOUT *wabout=0;
    GINSTANCE *grx=0;
    GINSTANCE *gwx=0;
    GSTREAM *g;
    GSTREAM *gw=0;
    GINFO *ginfo;
    char fnamebuf[SPITMAXLINE];
#ifdef GCMP
    int CMPargnum=-1;
#endif
    writemsgarg = -1;
    errorlevel = 0;

    double totaltime = SPIT_gettime();

    if (!GIMEX_init())
    {
        if (OPTverbose)
        {
            gprintf(TOOL_NAME ERROR_GIMEX_NOT_FOUND);
        }
        ++errorlevel;
        goto ErrorExit;
    }

    GXFILTER_init();
    errorlevel = 0;
    SPIT_init(SPIT_reset, SPIT_usage, SPIT_opt, SPIT_history);                 // Initialize my command line parser.
    do
    {
        gsettitle("gx - Parsing");

        more = SPIT_parse(argc, argv, TOOL_NAME, TOOL_VERSION);                  // get spit to parse all my command line options.

        gsettitle("gx - Parsing.");

#if !defined(GXRT) && !defined(GXLIB)
        //BLOODHOUND_Message(TOOL_NAME,TOOL_VERSION);
#endif

        if (!SPITargnum) goto ErrorExit;                                       // if the user didn't specify anything at the command line

#if defined(_MSC_VER) /* pc, xbox, xenon */
        SetThreadPriority(GetCurrentThread(), OPTpriorityhigh?THREAD_PRIORITY_ABOVE_NORMAL : (OPTprioritylow?THREAD_PRIORITY_BELOW_NORMAL : THREAD_PRIORITY_NORMAL) );
#endif
                                                                               // then go give them some help.
        framecount = (int *) galloc((SPITargnum+1)*sizeof(int));
        filecount  = (int *) galloc((SPITargnum+1)*sizeof(int));               // file count is for many to many information. I need to know
                                                                               // how many files are going to a single output file. If it's only
                                                                               // one then I need not turn on global colour reduction etc.
        if ((!framecount  || !filecount) && OPTverbose)
            gprintf("error - out of memory allocating frame counts\n");

        memset(framecount,0,SPITargnum*sizeof(int));
        memset(filecount,0,SPITargnum*sizeof(int));

        framecount[SPITargnum] = -1; /* tells write code to flush */
        filecount[SPITargnum]  = -1;

        if (OPTcolours && !OPTdepth)
        {
            OPTdepth = gmax(1,GXintlog2(OPTcolours-1)+1);
            if (OPTverbose>=4)
                gprintf("implied bpp %d\n",OPTdepth);
        }

        n = 0;

        totalframes = 512; /* approx for hasher */

#ifdef GCMP
        // remember which is last argument

        if (!OPTcompositefile[0])
        {
            int i;

            for (i=0; i<SPITargnum; ++i)
            {
                if (stricmp(SPITsargv[i],SPITdargv[i]))
                    strcpy(OPTcompositefile,SPITsargv[i]);
            }
        }

        if (!OPTcompositefile[0])
        {
            int i;

            for (i=0; i<SPITargnum; ++i)
            {
                isopt = 0;
                if (SPITsargv[i][0]=='-' || (SPITsargv[i][0]=='.' && SPITsargv[i][1]!='.' && SPITsargv[i][1]!='/' && SPITsargv[i][1]!='\\'))
                {
                    isopt = Zop(SPITsargv[i]+1)?1:0;
                }
                else if (SPITsargv[i][0]=='=' || SPITsargv[i][0]=='+')
                {
                    isopt = Zop(SPITsargv[i])?1:0;
                }
                if (!isopt)
                    CMPargnum = i;
            }
            if (CMPargnum>=0)
            {
                strcpy(OPTcompositefile,SPITsargv[CMPargnum]);
                if (CMPargnum==0) // only one frame
                    CMPargnum = -1;
            }
        }
#endif

        // count the number of real files I have for later use when determining whether I should turn on globalpalette or not
        {
            int last = 0;
            int find = 0;
            int lastinfilec = 0;
            // loop and try to find how many input files go to this output file.
            // stuff the count in all guys going to the specified output file so I can check if
            // I'm many to many, many to one, or one to many.
            for (i=0; i<SPITargnum; ++i)
            {

                if(strcmp(SPITdargv[i],SPITdargv[last]) != 0)
                {
                    for(find = last;find < i; find++)
                    {
                        filecount[find] = infilecount-lastinfilec;
                    }
                    last = i;
                    lastinfilec = infilecount;
                }
                else if(i == (SPITargnum-1))
                {
                    int add = 0;
                    if (!GX_ispath(SPITsargv[i]) &&
                        !(SPITsargv[i][0]=='-' || (SPITsargv[i][0]=='.' && SPITsargv[i][1]!='.' && SPITsargv[i][1]!='/' && SPITsargv[i][1]!='\\')) &&
                        !(SPITsargv[i][0]=='=' || SPITsargv[i][0]=='+'))
                        add++;

                    for(find = last;find <= i; find++)
                    {
                        filecount[find] = infilecount+add-lastinfilec;
                    }
                    last = i;
                    lastinfilec = infilecount;
                }
                if (!GX_ispath(SPITsargv[i]) &&
                    !(SPITsargv[i][0]=='-' || (SPITsargv[i][0]=='.' && SPITsargv[i][1]!='.' && SPITsargv[i][1]!='/' && SPITsargv[i][1]!='\\')) &&
                    !(SPITsargv[i][0]=='=' || SPITsargv[i][0]=='+'))
                    infilecount++;

            }
            // we had an = first and then the files so have to stuff the array correctly.
            if(filecount[0] == 0)
            {
                for(i=0; i < SPITargnum; ++i)
                {
                    filecount[i] = infilecount;
                }
            }
        }

        gsettitle("gx - Parsing..");

        for (pass=0; pass<2; ++pass)
        {
            int temptotalframes = totalframes;
            int totalcount      = 0;          // keep track of my total frame number within the whole range of files on each pass.
            inframe = 0;
            outframe = 0;
            if(temptotalframes < 1)           // the ruse is to stop the annoying error messages that happen if we attempt
                temptotalframes = 1;          // to use gx without a valid input file. The hasher will get initialized to -1
            if(OPThashlen < 1)                // on pass 1. We are going to abort later in this case so fake that we
                OPThashlen = 1;               // have more frames here to the hasher to avoid gallocing a negative amount.

            HASHID_init(temptotalframes,OPThashlen,0,0); // nopad, noext

            for (i=0; i<SPITargnum; ++i)
            {
                if (OPTverbose>=6)
                {
                    gprintf("DEBUG pass %d, arg %d of %d is %s -> %s\n",
                        pass,i,SPITargnum,
                        SPITsargv[i],SPITdargv[i]);
                }

                if (GX_ispath(SPITsargv[i]))
                {
                    if (pass==0 && OPTverbose)
                        gprintf(TOOL_NAME " - WARNING multiple target directories; '%s' ignored\n", SPITsargv[i]);
                }
                else
                {
                    isopt = 0;
                    if (SPITsargv[i][0]=='-' || (SPITsargv[i][0]=='.' && SPITsargv[i][1]!='.' && SPITsargv[i][1]!='/' && SPITsargv[i][1]!='\\'))
                    {
                        isopt = Zop(SPITsargv[i]+1)?1:0;
                    }
                    else if (SPITsargv[i][0]=='=' || SPITsargv[i][0]=='+')
                    {
                        isopt = Zop(SPITsargv[i])?1:0;
                    }
#ifdef GCMP
                    else if (i==CMPargnum)
                        isopt = 1;
#endif

                    if (!isopt)
                    {
                        wtype = OPTtype;
                        if (!wtype && OPTwrite)
                        {
                            if (GXextension(SPITdargv[i]))
                                wtype = GIMEX_find(GXextension(SPITdargv[i])+1,&NAMEsubtype);

                            if (!wtype)
                            {
                                if (OPTverbose)
                                    gprintf(TOOL_NAME " - NO OUTPUT TYPE SPECIFIED FOR '%s'.\n",SPITdargv[i]);
                                goto ErrorExit;
                            }
                        }
                        GIMEX_set(wtype);

                        wabout = GIMEX_about();
                        if (wabout)
                        {
                            int samename = 0;
                            char bakname[SPITMAXLINE];
                            // verify if the exporting format is writeable and other similar issues.
                            if (!GX_verifyoutputfile(wabout,SPITargnum,&warned,filecount[i]))
                                goto ErrorExit;
                            strcpy(filename,SPITdargv[i]);
                            {
                                char sname[SPITMAXLINE];
                                char dname[SPITMAXLINE];
                                SPIT_getname( dname,SPITdargv[i]);
                                SPIT_getname( sname,SPITsargv[i]);
                                if (!stricmp(dname,sname))
                                {
                                    if (wabout->firstextension && OPTtype)
                                    {
                                        NAMEsubtype = 0;
                                    }
                                    SPIT_changeext(filename,SPITdargv[i],wabout->extensions[NAMEsubtype]+1);
                                }
                            }


                            if (OPTsubtype<0 && wabout->dotsubtype)
                            {
                                if (SignatureCompare(wabout->signature,GMAKEID('M','A','D','C')))
                                {
                                    OPTsubtype = ((1<<NAMEsubtype)>>1)<<24;
                                }
                                else
                                {
                                    OPTsubtype = (1<<NAMEsubtype)>>1;
                                }
                            }

                            strcpy(bakname,SPITsargv[i]);

                            if (OPTwrite && wabout->usesfile) // don't worry about overwrite of non-files
                            {
                                if (!stricmp(bakname,filename) && !OPTextract)
                                    samename = 1;
                                else
                                {
                                    /* if not the same, it could be that one of the files has '.\' */
                                    if ((filename[0] == '.') && (filename[1] == '\\'))
                                    {
                                        if (!stricmp(bakname,&filename[2]))
                                            samename = 1;
                                    }
                                    else if ((bakname[0] == '.') && (bakname[1] == '\\'))
                                    {
                                        if (!stricmp(&bakname[2],filename))
                                            samename = 1;
                                    }
                                }
                            }

                            if (samename)
                            {
                                if (OPToverwrite)
                                {
                                    SPIT_tempname(bakname,bakname);
                                    if (pass==0)
                                    {
                                        int j;
                                        int first=1;
                                        for (j=0; j<i; ++j)
                                        {
                                            if (!stricmp(filename,SPITsargv[j]))
                                                first = 0;
                                        }
                                        if (first)
                                        {
                                            if (!GX_fileexist(SPITsargv[i]))
                                            {
                                                if (OPTverbose)
                                                    gprintf(TOOL_NAME " - FILE NOT FOUND '%s'!\n",SPITsargv[i]);
                                                goto ErrorExit;
                                            }

                                            remove(bakname);
                                            if (GX_fileexist(bakname))
                                            {
                                                if (OPTverbose)
                                                    gprintf(TOOL_NAME " - COULD NOT REMOVE OLD BACK UP BEFORE OVERWRITING. READ ONLY? '%s'\n",bakname);
                                                goto ErrorExit;
                                            }

                                            if (OPTverbose>=4)
                                                gprintf("overwrite backing up source from '%s' to '%s'\n",SPITsargv[i],bakname);


                                            if (rename(SPITsargv[i],bakname))
                                            {
                                                if (OPTverbose)
                                                    gprintf(TOOL_NAME " - COULD NOT BACK UP SOURCE BEFORE OVERWRITING. READ ONLY? '%s'\n",SPITsargv[i]);
                                                goto ErrorExit;
                                            }
                                            if (OPTverbose>=2 && OPToverwrite==2)
                                                gprintf("Temporary backup '%s'\n",bakname);
                                            if (OPTverbose>=2 && OPToverwrite==1)
                                                gprintf("Source backup '%s'\n",bakname);
                                        }
                                        else
                                        {
                                            if (OPTverbose>=5)
                                                gprintf("using existing backup '%s'\n",bakname);
                                        }
                                    }
                                }
                                else
                                {
                                    if (OPTverbose)
                                        gprintf(TOOL_NAME " - MUST NAME OUTPUT FILE DIFFERENT THAN INPUT FILE '%s'!\n",filename);
                                    goto ErrorExit;
                                }
                            }
                            if (OPTopenastype)
                            {
                                g = gopen(bakname);
                                strcpy(fnamebuf,bakname);
                                GIMEX_set(OPTopenastype);
                            }
                            else
                            {
                                g = GIMEX_gopen(bakname,fnamebuf);
                            }
                            type = GIMEX_get();
                            ok = GIMEX_is(g);

                            if (type && ok)
                            {
                                if (OPTverbose>=2)
                                {
                                    if (!pass)
                                    {
                                        if ( (OPTverbose>=4) || OPTglobalpalette)
                                            gprintf("Parsing ");
                                    }
                                    else
                                        gprintf("Reading ");
#ifndef GXLIB
                                   // note that if we try to flush stdout from inside a windows app all hell breaks loose!
                                   // lets avoid the issue inside the gxlib and let windows flush stdout when it wants to
                                   // since most likely the user doesn't want any output to the console anyways.
                                   SPIT_flush();
#endif
                                }


                                about = GIMEX_about();
                                if (about)
                                {
                                    GX_settooltitle(about,fnamebuf,pass);           // print out what we are currently parsing or doing to
                                    ok = 0;                                         // reset the general ok flag for next iteration.

                                    TTF_pointsize(OPTpointsize,OPTantialias,OPTstyle,OPTitalic,OPTbold,OPTcharset,OPTtypeface,OPTaspect,OPTtighten,OPTtracking,OPTsmallcaps,OPTascii,OPTbaseline,frame);

                                    frames = GIMEX_open(&grx,g,bakname,1);
                                    EXPR_set(expr,"frames",frames);

                                    if (!RANGE_check(OPTfilterframes, 0,frames-1))
                                    {
                                        if (OPTverbose)
                                            gprintf(TOOL_NAME " - '-ff' OPTION SPECIFIED FRAMES OUT OF RANGE.\nSpecified %s. Valid range is %d..%d\n",OPTfilterframes,0,frames-1);
                                        goto ErrorExit;
                                    }

                                    if (OPTpsflattened)
                                    {
                                        {
                                            int fframestart = 0;
                                            int fframeend   = 0;
                                            int flframes    = 0;
                                            fframestart = RANGE_min(OPTfilterframes,0);
                                            if (fframestart>(frames-1))
                                                fframestart = frames-1;
                                            fframeend = RANGE_max(OPTfilterframes,frames-1);
                                            if (fframeend<fframestart)
                                                fframeend = fframestart;

                                            flframes = RANGE_count(OPTfilterframes,0,frames-1);

                                            frames = frames - flframes + 1;
                                            if(frames < 1)
                                                frames = 1;
                                        }
                                    }
                                    // note: I cannot do this check for OPTglobalframe since I have no idea of the total framecount at this point.
                                    if (!RANGE_check(OPTframes, 0,frames-1) && !OPTglobalframe)
                                    {
                                        if (OPTverbose)
                                            gprintf(TOOL_NAME " - '-f' OPTION SPECIFIED FRAMES OUT OF RANGE.\nSpecified %s. Valid range is %d..%d\n",OPTframes,0,frames-1);
                                        if(grx)
                                        {
                                            GIMEX_close(grx);
                                            grx = 0;
                                            gclose(g);
                                            g = 0;
                                        }

                                        goto ErrorExit;
                                    }


                                    framestart = 0;
                                    frameend = frames-1;
                                    lframes = frames;
// PAULDING START
                                    // for now, make -layer take precedent over -f
                                    if (OPTlayers[0])
                                    {
                                        GINFO * tginfo     = 0;
                                        int checkframe     = 0;
                                        int layerstart     = -1;
                                        int layerend       = -1;
                                        int ind            = 0;

                                        lframes = 0;
                                        for(ind = framestart; ind<=frameend;ind++)
                                        {
                                            checkframe=OPTglobalframe?totalcount+framestart:ind;
                                            tginfo = GIMEX_info(grx, ind);
                                            if( (!strcmp("</Layer group>",tginfo->framename)) ||
                                                (!strcmp("</Layer set>",tginfo->framename)) )
                                            {
                                                // track the starting point of the most recent layer group
                                                layerstart = ind;
                                            }
                                            else if(!strcmp(OPTlayers,tginfo->framename))
                                            {
                                                // try to confirm that this is a layer group definition ($$TODO - find a more proven method)
                                                if(tginfo->height == 1 && tginfo->width == 1)
                                                {
                                                    // layer group, so add all frames from layerstart to now
                                                    if(layerstart == -1)
                                                    {
                                                        if (OPTverbose)
                                                            gprintf(TOOL_NAME " - WARNING - layer group %s found with no matching start\n",OPTlayers);
                                                        if(grx)
                                                        {
                                                            gfree(tginfo);

                                                            GIMEX_close(grx);
                                                            grx = 0;
                                                            gclose(g);
                                                            g = 0;
                                                        }
                                                        goto ErrorExit;
                                                    }

                                                    framestart = layerstart + 1;
                                                    frameend = ind - 1;
                                                    lframes = frameend - framestart + 1;

                                                    gprintf(TOOL_NAME " - layer group %s contains layers %d-%d\n",OPTlayers,framestart,frameend);
                                                }
                                                else
                                                {
                                                    // just specifying a single layer
                                                    framestart = ind;
                                                    frameend = ind;
                                                    lframes = 1;

                                                    gprintf(TOOL_NAME " - layer %s is layer %d\n",OPTlayers,framestart);
                                                }

                                                // -layer only accepts the first matching layer/group
                                                gfree(tginfo);                 // cleanup my count.

                                                if(lframes < 2 && OPTflatten)
                                                {
                                                    OPTflatten = false;
                                                }
                                                break;
                                            }
                                            gfree(tginfo);                 // cleanup my count.
                                            if(about->multiframe || OPTglobalframe)
                                                totalcount += 1;
                                        }
                                        for(ind = framestart; ind<=frameend;ind++)
                                        {
                                            tginfo = GIMEX_info(grx, ind);
                                            gprintf("**** Layer #%d - center (%d, %d), default (%d, %d)\n", ind, tginfo->centerx, tginfo->centery, tginfo->defaultx, tginfo->defaulty);
                                            gfree(tginfo);                 // cleanup my count.
                                        }
                                    }
                                    else
// PAULDING END
                                    if (OPTframes[0])
                                    {
                                        int isFrameRangeWild = SPIT_haswild(OPTframes) || !GX_isdecimal(OPTframes);  // check if range string has wilds in which case we do not know how many frames
                                                                                                                 // we are going to need to write out for a given iteration.
                                        framestart = RANGE_min(OPTframes,0);
                                        if (framestart>(frames-1))
                                            framestart = frames-1;
                                        frameend = RANGE_max(OPTframes,frames-1);
                                        if (frameend<framestart)
                                            frameend = framestart;

                                        lframes = RANGE_count(OPTframes,0,frames-1);
                                        if (lframes<1) lframes = 1;


                                        if (!GX_isdecimal(OPTframes))
                                        {
                                            framestart = 0;
                                            frameend   = frames-1;
                                        }
                                        // This block does two main things.
                                        // 1. it sets the index information for range matches.
                                        // 2. it determines if I have source art that is 8bpp if
                                        //    I am going out to a format that supports multiple depths
                                        //    and thus might not have to re-colour reduce at all.
                                        //    ie this handles the 8->8 case on AVI where I have 24 bit as well.
                                        if (about->multiframe || OPTglobalframe ||
                                             (wabout->globalpalette && !(!wabout->export32 || ((OPTdepth>=1) && (OPTdepth<=8)) || OPTcolours))  // if 8 -> 8 so we won't trigger col red do this
                                           )
                                        {
                                            if (isFrameRangeWild)                  // figure out how many matches we have for this
                                            {                                      // wild card match. Have to loop and find them.
                                                GINFO * tginfo     = 0;
                                                int ind                  = 0;
                                                int foundmin         = 0;
                                                int startingframe = 0;
                                                int endingframe    = 0;
                                                int checkframe     = 0;
                                                lframes = 0;                       // set my frame count to zero and begin counting.
                                                for(ind = framestart; ind<=frameend;ind++)
                                                {
                                                    checkframe=OPTglobalframe?totalcount+framestart:ind;
                                                    tginfo = GIMEX_info(grx, ind);
                                                    if(RANGE_match(OPTframes,tginfo->framename,checkframe))
                                                    {
                                                        // we are doing two things here, if I might need to
                                                        // do global palette colour reduction I will check
                                                        // here if my source has any 8bpp images and if I'm not
                                                        // doing anything else (like colour reduction) that will
                                                        // trigger the global palette colour reduction.
                                                        if(wabout->globalpalette && !(!wabout->export32 || ((OPTdepth>=1) && (OPTdepth<=8)) || OPTcolours))
                                                        {
                                                            if(tginfo->bpp == 8)
                                                                SourceArtContains8bpp = 1;
                                                        }
                                                        // here I'm doing the range check to find out just
                                                        // what frames will be included in the final output.
                                                        if(about->multiframe || OPTglobalframe)
                                                        {
                                                            if(!foundmin)              // find my starting and ending frame for
                                                            {                          // the wildcard matching.
                                                                startingframe = ind;
                                                                foundmin      = 1;     // we found our first frame.
                                                            }
                                                            endingframe = ind;         // keep going till I find my ending frame.
                                                            lframes++;                 // increment my found frame count.
                                                        }
                                                    }
                                                    gfree(tginfo);                 // cleanup my count.
                                                    if(about->multiframe || OPTglobalframe)
                                                        totalcount += 1;
                                                }
                                                // if I didn't even have one match then I better warn the user that
                                                // they will not be getting any output.
                                                if(!foundmin)
                                                {
                                                    if (OPTverbose)
                                                        gprintf(TOOL_NAME " - WARNING -f%s does not match any frames\n",OPTframes,0,frames-1);
                                                    if(grx)
                                                    {
                                                        GIMEX_close(grx);
                                                        grx = 0;
                                                        gclose(g);
                                                        g = 0;
                                                    }
                                                    goto ErrorExit;
                                                }

                                                // if I was doing a range match then
                                                // update my range information.
                                                if(about->multiframe || OPTglobalframe)
                                                {
                                                    framestart = startingframe;        // set my endpoints.
                                                    frameend   = endingframe;
                                                }

                                            }
                                        }
                                    }
                                    if (pass==0)
                                    {
                                        framecount[i] = lframes;
                                        if (OPTmipmap)
                                            framecount[i] *= OPTmipmap;
                                        if (OPTflatten)
                                        {
                                            if(about->multiframe || OPTglobalframe)    // if the input format doesn't support multiframe then no need to try to
                                            {                                          // scan the file for frame matches.
                                                int ind         = 0;
                                                int flframes    = 0;
                                                int ismatched   = 0;
                                                int fisFrameRangeWild = SPIT_haswild(OPTfilterframes);  // check if range string has wilds in which case we do not know how many frames
                                                if(fisFrameRangeWild)                  // figure out how many matches we have for this
                                                {                                      // wild card match. Have to loop and find them.
                                                    int checkframe    = 0;
                                                    GINFO * tginfo    = 0;
                                                    flframes = frames;                                 // set my frame count to zero and begin counting.
                                                    for(ind = framestart; ind<=frameend;ind++)
                                                    {
                                                        checkframe=OPTglobalframe?totalcount+framestart:ind;
                                                        tginfo = GIMEX_info(grx, ind);
                                                        if(RANGE_match(OPTfilterframes,tginfo->framename,checkframe))
                                                        {
                                                            flframes--;                // this one will get composited.
                                                            ismatched = 1;
                                                        }
                                                        gfree(tginfo);                 // cleanup my count.
                                                    }

                                                    framecount[i] = flframes + ismatched; // have to add the one flattened frame to the total.

                                                }
                                                else
                                                {
                                                    flframes = RANGE_count(OPTfilterframes,0,frames-1);
                                                    framecount[i] = frames - flframes + 1;
                                                }

                                                if(framecount[i] < 1)
                                                    framecount[i] = 1;
                                            }
                                            else
                                            {
                                                framecount[i] = 1;//i==0?1:0;
                                            }
                                        }
                                        if (OPTverbose>=5)
                                            gprintf("arg %d, count %d\n",i,lframes);
                                    }

                                    //framecount[i] = framecount[i] <= 0? 1 :framecount[i];   // no invalid frame counts floating around.
                                    // cripple double pass on pages type due to strange behaviour.
                                    int pagesmultipass = OPTlayered > 0;

                                    // if destination is globalette type AND we are exporting 8 bit:
                                    //     then if we are going to 8 bit AND we have more than one image going out to this file.
                                    //     we better colour reduce just to be sure.
                                    if (!OPTglobalpalette && wabout->globalpalette  && wabout->export8 &&
                                      ( // if due to a file attribute then we need multiple files.
                                          (!wabout->export32 || ((OPTdepth>=1) && (OPTdepth<=8)) || OPTcolours || (!about->globalpalette && SourceArtContains8bpp))
                                           && filecount[i] > 1
                                       ) ||
                                      ( // a filter that bumps up to 32bpp should happen if we have multiple files OR
                                        // multiple frames.
                                       ( OPTzoomx[0] || OPTzoomy[0] || OPTzoomxby || OPTzoomyby ||
                                         OPTthumbx[0] || OPTthumby[0] || OPTthumbxby || OPTthumbyby ||
                                         OPTsizex[0] || OPTsizey[0] || OPTsizeyby || OPTsizeyby ||
                                         OPTtedge || OPToutlinew[0] || OPToutlineh[0] || OPToutlinex[0] || OPToutliney[0] ||
                                         OPTundot3 || OPTunembm || OPTgshunembm ||  OPTglobalpalette ||
                                         (OPTmipmap && (OPTmipmapmethod!=OPTMIPMAPBOX || OPTmipmapwidth || OPTmipmapheight)) ||
                                         OPTchroma || OPTsharpen || OPTblur[0] || OPTerode[0] || OPTdilate[0] || OPTequalize[0] ||
                                         OPTblendb!=100 || OPTpalfile[0] || OPTavoid[0]
                                         // if we are doing a filter, have multiple frames and we are bumping down to 8bpp
                                        ) && (filecount[i] > 1 || (frameend - framestart >= 1))
                                          && (!wabout->export32 ||((OPTdepth>=1) && (OPTdepth<=8)) || OPTcolours)
                                      ))
                                    {
                                        if(OPTverbose >= 4)
                                            printf("Setting option GlobalPalette due to filter operation\n");
                                        OPTglobalpalette       = 1;
                                    }


                                    if ((pass==1) || OPTglobalpalette || (wabout->globalpalette&&!about->globalpalette) || OPTflatten || OPTcram || OPTpass>=2 || pagesmultipass)
                                    {
                                        int currentoutputframe = -1;           // keep track of this so I can only output processing information once (important for flattened images)
                                        inframe += framestart;

                                        for (frame=framestart; frame<=frameend; ++frame)
                                        {
                                            GBITMAP *gbitmap;
                                            GBITMAP **gbitmaps;
                                            char titlebuf[SPITMAXLINE];

                                            if (writemsgarg<0)
                                            {
                                                if (pass==0)
                                                    sprintf(titlebuf,"%s - GX Parsing Frame: %d of %d",fnamebuf,frame+1,frameend+1); // indexed from zero so inc for human readability.
                                                else
                                                    sprintf(titlebuf,"%s - GX Reading Frame: %d of %d",fnamebuf,frame+1,frameend+1); // indexed from zero so inc for human readability.
                                                gsettitle(titlebuf);
                                            }

#if defined(_MSC_VER)
                                            Sleep(10);  // give some CPU to other processes
#endif

                                            gbitmap = (GBITMAP *) galloc(sizeof(GBITMAP));
                                            if (gbitmap)
                                            {
                                                int n;
                                                int noutthisframe = 1;

                                                memset(gbitmap,0,sizeof(GBITMAP));

                                                // .ttf -pointsize

                                                TTF_pointsize(OPTpointsize,OPTantialias,OPTstyle,OPTitalic,OPTbold,OPTcharset,OPTtypeface,OPTaspect,OPTtighten,OPTtracking,OPTsmallcaps,OPTascii,OPTbaseline,frame);

                                                ginfo = gbitmap->ginfo = GIMEX_info(grx, OPTpsflattened ? -1 : frame);

#if defined(_MSC_VER)
                                                Sleep(10);  // give some CPU to other processes
#endif
                                                if (ginfo)
                                                {
                                                    int checkframe=OPTglobalframe?inframe:frame;
                                                    {
                                                        static int ginfomsg=0;

                                                        if (!ginfomsg && OPTverbose && ginfo->version<GIMEX_VERSION)
                                                        {
                                                            gprintf(TOOL_NAME " - WARNING GIMEX VERSION %.2f USED. GX EXPECTED %.2f\n",(float)ginfo->version/100.0f,(float)GIMEX_VERSION/100.0f);
                                                            ginfomsg = 1;
                                                            ++errorlevel;
                                                        }
                                                    }

                                                    if (RANGE_match(OPTframes,ginfo->framename,checkframe))
                                                    {
                                                        if (about->resizable && (OPTsizex || OPTsizey))
                                                        {
                                                            int x;
                                                            int y;

                                                            /* Code copied from gxfilter.applysize */
                                                            x = GXscaleset(ginfo->width,OPTsizex,OPTsizexby,GX_sizeratio(OPTzoomx,ginfo->width*ginfo->height));
                                                            y = GXscaleset(ginfo->height,OPTsizey,OPTsizeyby,GX_sizeratio(OPTzoomx,ginfo->width*ginfo->height));
                                                            y = GXscaledefault(y,ginfo->height,x,ginfo->width);
                                                            x = GXscaledefault(x,ginfo->width,y,ginfo->height);

                                                            ginfo->width = x;
                                                            ginfo->height = y;
                                                        }

                                                        rowbytes = gbitmap->rowbytes = ginfo->width*ginfo->bpp/8;

                                                        f = (char *) (gbitmap->image = galloc(rowbytes * ginfo->height));

                                                        if (OPTpschannel>=0)
                                                            ginfo->subtype = OPTpschannel;


                                                        if (f)
                                                        {
                                                            ok = GIMEX_read(grx,ginfo,f,rowbytes);
                                                            if (!ok)
                                                            {
                                                                gprintf(TOOL_NAME " - ERROR READING FILE %s (GIMEX_read)\n",fnamebuf);
                                                            }
                                                            else
                                                            {
                                                                int     framecounti;
                                                                char    framefilename[SPITMAXLINE];
                                                                char    framename[SPITMAXLINE];

#if defined(_MSC_VER)
                                                                Sleep(10);  // give some CPU to other processes
#endif

                                                                /* close importer asap so exporter can close and rename\backup */
                                                                if (frame==frameend)
                                                                {
                                                                    if (grx)
                                                                    {
                                                                        GIMEX_close(grx);
                                                                        grx = 0;
                                                                    }
                                                                    if (g)
                                                                    {
                                                                        gclose(g);
                                                                        g = 0;
                                                                    }
                                                                }

                                                                strcpy(framename,ginfo->framename);

                                                                if (RANGE_match(OPTfilterframes,ginfo->framename,checkframe))
                                                                {
                                                                    gbitmaps = ProcessImage(gbitmap,about,wabout,i,outframe,pass,&noutthisframe);
                                                                }
                                                                else
                                                                {
                                                                    noutthisframe = 1;
                                                                    gbitmaps = (GBITMAP **)galloc(sizeof(GBITMAP *));
                                                                    gbitmaps[0] = gbitmap;
                                                                }

                                                                GIMEX_set(wtype);

                                                                ++inframe;
                                                                for (n = 0; n < noutthisframe; ++n)
                                                                {
                                                                    if ((wabout->multiframe && !OPTextract) || (framecount[i]<=1 && !strchr(filename,'%')))
                                                                    {
                                                                        strcpy(framefilename,filename);
                                                                        framecounti = framecount[i];
                                                                        // this is a special case, we selectively picking off frames inside the list,
                                                                        // it's the first frame visited but not the first frame in the list,
                                                                        // and nothing else is open and we have already accumulated all our framecounts into the
                                                                        // first slot. We have to grab that frame count for use here.
                                                                        if(pass ==1 && framecounti == 0 && !alreadyopen)
                                                                            framecounti = framecount[0];
                                                                    }
                                                                    else
                                                                    {
                                                                        framecounti = gmin(1,framecount[i]);

                                                                        wildname(framefilename, filename, (EXPR *)0, framename, outframe+OPTstartindex);
                                                                        if (!stricmp(framefilename,filename))
                                                                        {
                                                                            char *ext=GXextension(filename);
                                                                            if (ext)
                                                                                *ext = 0;

                                                                            sprintf(framefilename,"%s.%d",filename,outframe+OPTstartindex);

                                                                            if (ext)
                                                                            {
                                                                                *ext = '.';
                                                                                strcat(framefilename,ext);
                                                                            }
                                                                        }
                                                                    }

                                                                    if ((pass == 1) && (framecounti > 0) && (OPTverbose==2 || OPTverbose==3)
                                                                       && (OPTverbose>=3 || !SignatureCompare(wabout->signature,GMAKEID('N','U','L','L'))))
                                                                    {
                                                                        char titlebuf[SPITMAXLINE];

                                                                        sprintf(titlebuf,"%s - GX Writing Frame: %d of %d",framefilename,frame+1,frameend+1); // indexed from zero so increase human readability.
                                                                        gsettitle(titlebuf);

                                                                    }

                                                                    if ((pass == 1) && (framecounti > 0) && (OPTverbose==2 || OPTverbose==3) && ((writemsgarg!=i /*&& framecount[i+1]*/) || framecounti==1)
                                                                       && (OPTverbose>=3 || !SignatureCompare(wabout->signature,GMAKEID('N','U','L','L'))))
                                                                    {
#ifndef GCMP
                                                                        if (OPTwrite) gprintf("Writing %s '%s'\n",wabout->longtypestr,framefilename);
                                                                        else          gprintf("Not writing %s '%s'\n",wabout->longtypestr,framefilename);
#endif
                                                                        writemsgarg=i;
                                                                    }
                                                                    if (OPTwrite && (framecounti > 0 || !alreadyopen) && framecounti > -1)
                                                                    {
                                                                        if (pass == 1)
                                                                        {
                                                                            if (!gwx)
                                                                            {
                                                                                if (OPTverbose>=4)
                                                                                    gprintf("Writing %s '%s'\n",wabout->longtypestr,framefilename);

                                                                                if (wabout->usesfile)
                                                                                    gw = gwopen(framefilename);
                                                                                alreadyopen = 1;

                                                                                if (gw || !wabout->usesfile)
                                                                                {
                                                                                    GIMEX_wopen(&gwx,gw,framefilename,((framecounti<1)?1:framecounti));
                                                                                }
                                                                            }
                                                                            else
                                                                            {
                                                                                if (OPTverbose>=4)
                                                                                    gprintf("Writing %s '%s'\n",wabout->longtypestr,framefilename);
                                                                            }
                                                                        }
                                                                    }
                                                                    else
                                                                    {
                                                                        if (OPTverbose>=4)
                                                                            gprintf("Writing %s '%s'\n",wabout->longtypestr,framefilename);
                                                                    }

#ifndef GXLIB
                                   // note that if we try to flush stdout from inside a windows app all hell breaks loose!
                                   // lets avoid the issue inside the gxlib and let windows flush stdout when it wants to
                                   // since most likely the user doesn't want any output to the console anyways.

                                                                    if (OPTverbose)
                                                                        SPIT_flush();
#endif

                                                                    if (OPTverbose>=3)
                                                                    {
                                                                        if(currentoutputframe != outframe)           // only show the new frame count as framecounti reflects number of frames written out.
                                                                        {
                                                                            gprintf(" processing frame %d of %d\n",outframe+1,framecounti);
                                                                        }
                                                                        currentoutputframe = outframe;

                                                                    }
                                                                    else if (OPTverbose>=5)
                                                                        gprintf("arg %d, frame %d, frames %d, framecount %d\n",i,outframe+1,frames,framecount[i]);

                                                                    if (gwx)
                                                                    {

                                                                        GXVerifyFrame(gbitmaps[n]->ginfo,wabout);
                                                                        ok = GIMEX_write(gwx,gbitmaps[n]->ginfo,gbitmaps[n]->image,gbitmaps[n]->rowbytes);
                                                                        if (!ok)
                                                                        {
                                                                            static int writeerrmsg=-1;

                                                                            if (writeerrmsg!=framecounti && OPTverbose)
                                                                            {
                                                                                gprintf(TOOL_NAME " - WRITE FAILED on frame %d in file %s\n",framecounti,framefilename);
                                                                                if (wabout->movie || wabout->font || wabout->mipmap)
                                                                                    gprintf(" NOTE %s is a %s format and may have frame size constraints\n",
                                                                                        wabout->longtypestr,
                                                                                        wabout->movie?"movie":(wabout->font?"font":"texture"));

                                                                                writeerrmsg = framecounti;
                                                                            }
                                                                        }
//  else - removing this else allows write errorlevel to be noted but the file to be closed as normal.
                                                                        {
                                                                            ++outframe;

                                                                            if ((framecounti==1) ||
                                                                                ((n==noutthisframe-1) && (frame==frameend) && (framecount[i+1] || framecount[i+1] == -1)))
                                                                            {
                                                                                alreadyopen = 0;                          // I don't want to be opening another file if I already
                                                                                GIMEX_wclose(gwx);                        // have one open. So keep track of that status here.
                                                                                gwx = 0;
                                                                                if (gw)
                                                                                {
                                                                                    gclose(gw);
                                                                                    gw = 0;
                                                                                }
                                                                                if (!(framecounti==1))
                                                                                {
                                                                                    outframe = 0;
                                                                                    //inframe = 0;    /* reset frame counts */
                                                                                }

    #if defined(applec) || defined(powerc) || defined (__powerc)
                                                                                if (wabout->mactype[0])
                                                                                {
                                                                                    if (OPTverbose>=5)
                                                                                        gprintf("Setting mac type %s %x %x\n",framefilename,GMAKEID('P','a','n','o'),wabout->mactype[0]);
                                                                                    fsetfileinfo(framefilename,GMAKEID('8','B','I','M'),wabout->mactype[0]);
                                                                                }
    #endif

                                                                            }
                                                                        }
                                                                    }
                                                                }

                                                                GIMEX_set(type);
#ifndef GXLIB
                                   // note that if we try to flush stdout from inside a windows app all hell breaks loose!
                                   // lets avoid the issue inside the gxlib and let windows flush stdout when it wants to
                                   // since most likely the user doesn't want any output to the console anyways.
                                                                SPIT_flush();
#endif
                                                                if (gbitmaps)
                                                                {
                                                                    for (n=0; n<noutthisframe; ++n)
                                                                        GIMEX_freebitmap(gbitmaps[n]);
                                                                    gfree(gbitmaps);
                                                                    gbitmaps = 0;
                                                                }
                                                            }
                                                        }
                                                        else
                                                        {
                                                            GIMEX_freebitmap(gbitmap);
                                                            gbitmap = 0;
                                                        }
                                                    }
                                                    else
                                                    {
                                                        ++inframe;
                                                        gfree(ginfo);
                                                        gfree(gbitmap);
                                                        gbitmap = 0;
                                                        ginfo   = 0;
                                                    }
                                                }
                                                else
                                                    gfree(gbitmap);

                                            }
#if defined(_MSC_VER)
                                            if (OPTverbose>=6)
                                            {
                                                MEMORYSTATUS stat;
                                                GlobalMemoryStatus(&stat);
                                                printf("freemem %d %d, galloc %d\n",(int)stat.dwAvailPhys,(int)stat.dwAvailVirtual,galloccount);
                                            }
#endif
                                        }
                                    }
                                    gfree((char *)about);
                                    about = 0;
                                }

                                /* should already be closed, but just incase */

                                if (grx)
                                {
                                    GIMEX_close(grx);
                                    grx = 0;
                                }
                                if (g)
                                {
                                    gclose(g);
                                    g = 0;
                                }

                                if (pass==1 && samename) //stricmp(bakname,SPITsargv[i]))
                                {
                                    GSTREAM *tmp;

                                    tmp = gopen(SPITsargv[i]);
                                    if (!tmp)
                                    {
                                        tmp = gopen(bakname);
                                        if (tmp)
                                        {
                                            gclose(tmp);
                                            rename(bakname, SPITsargv[i]);
                                        }
                                    }
                                    else
                                    {
                                        gclose(tmp);

                                        if (OPToverwrite==2)
                                            remove(bakname);
                                    }
                                }
                            }
                            else
                            {
                                if (pass==1)
                                {
                                    if (OPTverbose)
                                    {
                                        if (!GX_fileexist(SPITsargv[i]))
                                            gprintf(TOOL_NAME " - FILE NOT FOUND '%s'!\n",SPITsargv[i]);
                                        else if (OPTopenastype)
                                            gprintf(TOOL_NAME " - UNABLE TO OPEN IMAGE '%s' AS SPECIFIED TYPE!\n",SPITsargv[i]);
                                        else
                                            gprintf(TOOL_NAME " - UNKNOWN IMAGE FORMAT '%s'!\n",SPITsargv[i]);
                                    }
                                    ++errorlevel;
                                }
                            }

#ifndef GXLIB
                            // note that if we try to flush stdout from inside a windows app all hell breaks loose!
                            // lets avoid the issue inside the gxlib and let windows flush stdout when it wants to
                            // since most likely the user doesn't want any output to the console anyways.
                            SPIT_flush();
#endif
                            ++n;
                            gfree((char *)wabout);
                            wabout = 0;
                        }
                    }
                }
            }

            if (gwx)
            {
                int oldtype;
                if(i > 0 && framecount[i] == -1 && framecount[i-1] == 0)   // assume that we were selectively picking off
                {                                                          // frames within a range and didn't get a chance
                                                                           // to close the stream properly.
                    alreadyopen = 0;                                       // do it here and don't warn since this is normal behaviour.
                    oldtype = GIMEX_get();
                    GIMEX_set(wtype);
                    GIMEX_wclose(gwx);
                    GIMEX_set(oldtype);
                    gwx = 0;
                    if (gw)
                    {
                        gclose(gw);
                        gw = 0;
                    }
                }
                else
                {
                    if (OPTverbose)
                    {
                        gprintf(TOOL_NAME " - WARNING closing outfile file abnormally\n");
                        ++errorlevel;
                    }
                    oldtype = GIMEX_get();
                    GIMEX_set(wtype);
                    GIMEX_wclose(gwx);
                    GIMEX_set(oldtype);
                    gwx = 0;
                    if (gw)
                    {
                        gclose(gw);
                        gw = 0;
                    }
                }
            }

            /* at end of pass 1, accumulate counts for new files */

            {
                int last;
                j = 0;

                while (!framecount[j])
                    ++j;

                totalframes = 0;
                last = j;
                totalframes = framecount[j];
                for (i=j+1; i<=SPITargnum; ++i)
                {
                    if (framecount[i])
                    {
                        if (framecount[i]>0)
                            totalframes = framecount[i];
                        if (i<SPITargnum && !stricmp(SPITdargv[i],SPITdargv[j]) && !strchr(SPITdargv[i],'%'))
                        {
                            framecount[j] += framecount[i];
                            framecount[i] = 0;
                        }
                        else
                        {
                            if ((last+1)!=i)
                                framecount[last+1] = -1;
                            j = i;
                        }
                        last = i;
                    }
                }
            }

            if (OPTverbose>=5 && pass==0)
            {
                for (i=0; i<SPITargnum; ++i)
                    gprintf("arg %d, count %d\n",i,framecount[i]);
                gprintf("total frames %d\n",totalframes);
            }
            HASHID_restore();
        }

ErrorExit:

        if (gstdout!=stdout)
            fclose(gstdout);

        HASHID_restore();
        gxfreeinversetable();

        if (about)
        {
            gfree((char *)about);
            about = NULL;
        }
        if (wabout)
        {
            gfree((char *)wabout);
            wabout = NULL;
        }

        if (framecount)
        {
            gfree(framecount);
            framecount = NULL;
        }
        if (filecount)
        {
            gfree(filecount);
            filecount = NULL;
        }
    } while (more > 0);

    RestoreTemporal();  // free up temporalfix gfilter
    if (gNumTemporalFix && OPTverbose>=3)
        gprintf( TOOL_NAME " %d frames temporal fixed\n",gNumTemporalFix);

    GXFILTER_restore();
    SPIT_restore();
    GIMEX_restore();


    if (galloccount)
    {
        if (OPTverbose>=1)
        {
            gprintf(TOOL_NAME " - WARNING MEMORY NOT FREED TO SYSTEM %d\n",galloccount);
            ++errorlevel;
        }
    }

    if (OPTtime)
    {
        totaltime = SPIT_gettime()-totaltime;

        if (totaltime>=60000.0)
            gprintf("TIME " TOOL_NAME " totaltime %.2f ms (%.2f seconds) (%.2f minutes)\n",totaltime,totaltime/1000.0,totaltime/60000.0);
        else if (totaltime>=1000.0)
            gprintf("TIME " TOOL_NAME " totaltime %.2f ms (%.2f seconds)\n",totaltime,totaltime/1000.0);
        else
            gprintf("TIME " TOOL_NAME " totaltime %.2f ms\n",totaltime);
    }
#ifdef GCMP

    if (OPTverbose==1)
    {
//        extern double BLENDpsnr;

        gprintf("PSNR is %.2f\n",BLENDpsnr);
    }
    if (OPTverbose>=3 || (OPTverbose>=2 && totalframes>=2))
    {
//        extern double BLENDpsnrtotal;

        gprintf("PSNR total is %.2f, PSNRY total is %.2f (%d frames)\n",BLENDpsnrtotal,BLENDpsnrYtotal,totalframes);
    }

#endif

#ifndef GCMP
    if (OPTverbose>1 && OPTbanner && OPTtime>10000.0 && !SPIThelpgiven)
        gprintf("done.\n");
#endif

    return(errorlevel);
}

// for gx.lib and GXRT.dll

int GCALL gx(const char *commandline)
{
    char *argv[2];
    argv[0] = "gxlib";
    argv[1] = (char*) commandline;
    OPTlibnohardspace = 1;             // tell it not to use hardspaces since I have only one argv with all options in it.
    return(gxmain(2,argv));
}

// || defined(GXRT)

#if !defined(GXLIB)

#if 0 && defined(_MSC_VER)
int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpszCmdLine, int cmdShow)
{
    char *argv[2];
    argv[0] = "gx98";
    argv[1] = lpszCmdLine;
    return gxmain(2,argv);
}
#else

int main(int argc,char **argv)
{
#if ENABLE_OMP
    if (getenv("OMPDISABLE"))   // old way
        omp_set_num_threads(1);
    char *omp = getenv("OMPNUMTHREADS");
    if (omp)
    {
        int ompnum = atoi(omp);
        omp_set_num_threads(ompnum);
    }
#endif
        

    // it is polite to leave things as we found them.
    // store the console title so we can restore it later.
    char titlebuf[SPITMAXLINE];
    int titlesz = ggettitle(titlebuf, SPITMAXLINE);
    
	int err = gxmain(argc, argv);
	
    // restore the console title to what the user had.
    gsettitle(titlebuf);

    return(err);
}

#endif
#endif

#undef   GXMAIN

