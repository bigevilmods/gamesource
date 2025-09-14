// (c) 2000-2007 Electronic Arts Inc.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#if defined(_MSC_VER)
// for Sleep()
#include <windows.h>
#endif
#include "gimex.h"
#include "mpggimex.h"

#define GLOBAL
#include "mpeg2enc.h"


#define SIGNATURE GMAKEID('M','P','E','G')

typedef struct
{
    int         subtype;
    int         frames;
    int         outframe;
    int         buffered;
    char        *buffer[2];
    long        patchpos;
	int			dataSize;
	int			maxChunkSize;
	AviIndex*	index;
} ENCODEINFO;


static int initialized = 0;

static char chunkheader[8] = { 'M', 'P', 'C', 'h', 0, 0, 0, 0 };
static char avichunkheader[8] = { '0', '0', 'd', 'c', 0, 0, 0, 0 };


void error(char *text);

static void initparms(const GINFO *ginfo)
{
  int i;
  int h,m,s,f;
  double newerror,olderror;
  static double ratetab[8]=
    {24000.0/1001.0, 24.0, 25.0, 30000.0/1001.0, 30.0, 50.0, 60000.0/1001.0, 60.0};

  MPEhorizontal_size = ginfo->width;
  MPEvertical_size = ginfo->height;

  MPEn = (ginfo->subtype>>16)&255;
  if (!MPEn) MPEn = 15;

  MPEm = (ginfo->subtype>>8)&255;
  if (!MPEm) MPEm = 3;

  if (MPEm > 3) MPEm = 3;
  if ((MPEn % MPEm) != 0) MPEm = 1;

  if (ginfo->fps <= 0.0)
  {
    MPEframe_rate_code = 5;
    MPEframe_rate_extension_d = 0;
    MPEframe_rate = 30.0;
  }
  else
  {
    if (ginfo->fps < 20.0)
    {
      olderror = 1.0e10;
      for (i=0; i<5; ++i)
      {
        newerror = fabs(ratetab[i]/2.0 - ginfo->fps);
        if (newerror < olderror)
        {
          MPEframe_rate_code = i+1;
          MPEframe_rate_extension_d = 1;
          MPEframe_rate = ratetab[i]/2.0;
          olderror = newerror;
        }
      }
    }
    else
    {
      olderror = 1.0e10;
      for (i=0; i<8; ++i)
      {
        newerror = fabs(ratetab[i] - ginfo->fps);
        if (newerror < olderror)
        {
          MPEframe_rate_code = i+1;
          MPEframe_rate_extension_d = 0;
          MPEframe_rate = ratetab[i];
          olderror = newerror;
        }
      }
    }
  }

  if (ginfo->framesize == 0)
      MPEbit_rate = (int) (240.0 * sqrt((float)MPEhorizontal_size*MPEvertical_size) * MPEframe_rate);
  else
      MPEbit_rate = (8*ginfo->framesize) * MPEframe_rate;

  strcpy(MPEid_string, "Encoded with MPEG Gimex module");
  strcpy(MPEiqname, "-");
  strcpy(MPEniqname, "-");
  strcpy(MPEstatname, "statfile.txt");

  h = 0; m = 0; s = 0; f = 0;

  MPEmpeg1 = 0;
  MPEfieldpic = 0;
  MPEaspectratio = 1;
  MPEvbv_buffer_size = 112;
  MPElow_delay = 0;
  MPEconstrparms = 0;
  MPEprofile = 4;
  MPElevel = 8;
  MPEprog_seq = 1;
  MPEchroma_format = 1;
  MPEvideo_format = 0;
  MPEcolor_primaries = 5;
  MPEtransfer_characteristics = 5;
  MPEmatrix_coefficients = 4;
  MPEdisplay_horizontal_size = MPEhorizontal_size;
  MPEdisplay_vertical_size = MPEvertical_size;
  MPEdc_prec = 0;
  MPEtopfirst = 0;

  MPEframe_pred_dct_tab[0] = 1;
  MPEframe_pred_dct_tab[1] = 1;
  MPEframe_pred_dct_tab[2] = 1;
  MPEconceal_tab[0] = 0;
  MPEconceal_tab[1] = 0;
  MPEconceal_tab[2] = 0;
  MPEqscale_tab[0] = 0;
  MPEqscale_tab[1] = 0;
  MPEqscale_tab[2] = 0;
  MPEintravlc_tab[0] = 1;
  MPEintravlc_tab[1] = 0;
  MPEintravlc_tab[2] = 0;
  MPEaltscan_tab[0] = 0;
  MPEaltscan_tab[1] = 0;
  MPEaltscan_tab[2] = 0;

  MPErepeatfirst = 0;
  MPEprog_frame = 1;

  MPEmotion_data = (struct MPEmotion_data *)galloc(MPEm*sizeof(struct MPEmotion_data));
  if (!MPEmotion_data)
    error("mpgwrite: galloc failed\n");

  MPEmotion_data[0].forw_hor_f_code = 2;
  MPEmotion_data[0].forw_vert_f_code = 2;
  MPEmotion_data[0].sxf = 15;
  MPEmotion_data[0].syf = 15;

  for (i=1; i<MPEm; i++)
  {
    MPEmotion_data[i].forw_hor_f_code = 1;
    MPEmotion_data[i].forw_vert_f_code = 1;
    MPEmotion_data[i].sxf = 7;
    MPEmotion_data[i].syf = 7;

    MPEmotion_data[i].back_hor_f_code = 1;
    MPEmotion_data[i].back_vert_f_code = 1;
    MPEmotion_data[i].sxb = 7;
    MPEmotion_data[i].syb = 7;
  }

  /* make flags boolean (x!=0 -> x=1) */
  MPEmpeg1 = !!MPEmpeg1;
  MPEfieldpic = !!MPEfieldpic;
  MPElow_delay = !!MPElow_delay;
  MPEconstrparms = !!MPEconstrparms;
  MPEprog_seq = !!MPEprog_seq;
  MPEtopfirst = !!MPEtopfirst;
  for (i=0; i<3; i++)
  {
    MPEframe_pred_dct_tab[i] = !!MPEframe_pred_dct_tab[i];
    MPEconceal_tab[i] = !!MPEconceal_tab[i];
    MPEqscale_tab[i] = !!MPEqscale_tab[i];
    MPEintravlc_tab[i] = !!MPEintravlc_tab[i];
    MPEaltscan_tab[i] = !!MPEaltscan_tab[i];
  }
  MPErepeatfirst = !!MPErepeatfirst;
  MPEprog_frame = !!MPEprog_frame;

  /* make sure MPEG specific parameters are valid */
  range_checks();

  /* timecode -> frame number */
  MPEtc0 = h;
  MPEtc0 = 60*MPEtc0 + m;
  MPEtc0 = 60*MPEtc0 + s;
  MPEtc0 = (int)(MPEframe_rate+0.5)*MPEtc0 + f;

  if (!MPEmpeg1)
  {
    profile_and_level_checks();
  }
  else
  {
    /* MPEG-1 */
    if (MPEconstrparms)
    {
      if (MPEhorizontal_size>768
          || MPEvertical_size>576
          || ((MPEhorizontal_size+15)/16)*((MPEvertical_size+15)/16)>396
          || ((MPEhorizontal_size+15)/16)*((MPEvertical_size+15)/16)*MPEframe_rate>396*25.0
          || MPEframe_rate>30.0)
      {
        if (!MPEquiet)
          fprintf(stderr,"Warning: setting MPEconstrparms = 0\n");
        MPEconstrparms = 0;
      }
    }

    if (MPEconstrparms)
    {
      for (i=0; i<MPEm; i++)
      {
        if (MPEmotion_data[i].forw_hor_f_code>4)
        {
          if (!MPEquiet)
            fprintf(stderr,"Warning: setting MPEconstrparms = 0\n");
          MPEconstrparms = 0;
          break;
        }

        if (MPEmotion_data[i].forw_vert_f_code>4)
        {
          if (!MPEquiet)
            fprintf(stderr,"Warning: setting MPEconstrparms = 0\n");
          MPEconstrparms = 0;
          break;
        }

        if (i!=0)
        {
          if (MPEmotion_data[i].back_hor_f_code>4)
          {
            if (!MPEquiet)
              fprintf(stderr,"Warning: setting MPEconstrparms = 0\n");
            MPEconstrparms = 0;
            break;
          }

          if (MPEmotion_data[i].back_vert_f_code>4)
          {
            if (!MPEquiet)
              fprintf(stderr,"Warning: setting MPEconstrparms = 0\n");
            MPEconstrparms = 0;
            break;
          }
        }
      }
    }
  }

  /* relational checks */

  if (MPEmpeg1)
  {
    if (!MPEprog_seq)
    {
      if (!MPEquiet)
        fprintf(stderr,"Warning: setting MPEprog_seq = 1\n");
      MPEprog_seq = 1;
    }

    if (MPEchroma_format!=CHROMA420)
    {
      if (!MPEquiet)
        fprintf(stderr,"Warning: setting MPEchroma_format = 1 (4:2:0)\n");
      MPEchroma_format = CHROMA420;
    }

    if (MPEdc_prec!=0)
    {
      if (!MPEquiet)
        fprintf(stderr,"Warning: setting MPEdc_prec = 0\n");
      MPEdc_prec = 0;
    }

    for (i=0; i<3; i++)
      if (MPEqscale_tab[i])
      {
        if (!MPEquiet)
          fprintf(stderr,"Warning: setting MPEqscale_tab[%d] = 0\n",i);
        MPEqscale_tab[i] = 0;
      }

    for (i=0; i<3; i++)
      if (MPEintravlc_tab[i])
      {
        if (!MPEquiet)
          fprintf(stderr,"Warning: setting MPEintravlc_tab[%d] = 0\n",i);
        MPEintravlc_tab[i] = 0;
      }

    for (i=0; i<3; i++)
      if (MPEaltscan_tab[i])
      {
        if (!MPEquiet)
          fprintf(stderr,"Warning: setting MPEaltscan_tab[%d] = 0\n",i);
        MPEaltscan_tab[i] = 0;
      }
  }

  if (!MPEmpeg1 && MPEconstrparms)
  {
    if (!MPEquiet)
      fprintf(stderr,"Warning: setting MPEconstrparms = 0\n");
    MPEconstrparms = 0;
  }

  if (MPEprog_seq && !MPEprog_frame)
  {
    if (!MPEquiet)
      fprintf(stderr,"Warning: setting MPEprog_frame = 1\n");
    MPEprog_frame = 1;
  }

  if (MPEprog_frame && MPEfieldpic)
  {
    if (!MPEquiet)
      fprintf(stderr,"Warning: setting MPEfieldpic = 0\n");
    MPEfieldpic = 0;
  }

  if (!MPEprog_frame && MPErepeatfirst)
  {
    if (!MPEquiet)
      fprintf(stderr,"Warning: setting MPErepeatfirst = 0\n");
    MPErepeatfirst = 0;
  }

  if (MPEprog_frame)
  {
    for (i=0; i<3; i++)
      if (!MPEframe_pred_dct_tab[i])
      {
        if (!MPEquiet)
          fprintf(stderr,"Warning: setting MPEframe_pred_dct_tab[%d] = 1\n",i);
        MPEframe_pred_dct_tab[i] = 1;
      }
  }

  if (MPEprog_seq && !MPErepeatfirst && MPEtopfirst)
  {
    if (!MPEquiet)
      fprintf(stderr,"Warning: setting MPEtopfirst = 0\n");
    MPEtopfirst = 0;
  }

  /* search windows */
  for (i=0; i<MPEm; i++)
  {
    if (MPEmotion_data[i].sxf > (4<<MPEmotion_data[i].forw_hor_f_code)-1)
    {
      if (!MPEquiet)
        fprintf(stderr,
          "Warning: reducing forward horizontal search width to %d\n",
          (4<<MPEmotion_data[i].forw_hor_f_code)-1);
      MPEmotion_data[i].sxf = (4<<MPEmotion_data[i].forw_hor_f_code)-1;
    }

    if (MPEmotion_data[i].syf > (4<<MPEmotion_data[i].forw_vert_f_code)-1)
    {
      if (!MPEquiet)
        fprintf(stderr,
          "Warning: reducing forward vertical search width to %d\n",
          (4<<MPEmotion_data[i].forw_vert_f_code)-1);
      MPEmotion_data[i].syf = (4<<MPEmotion_data[i].forw_vert_f_code)-1;
    }

    if (i!=0)
    {
      if (MPEmotion_data[i].sxb > (4<<MPEmotion_data[i].back_hor_f_code)-1)
      {
        if (!MPEquiet)
          fprintf(stderr,
            "Warning: reducing backward horizontal search width to %d\n",
            (4<<MPEmotion_data[i].back_hor_f_code)-1);
        MPEmotion_data[i].sxb = (4<<MPEmotion_data[i].back_hor_f_code)-1;
      }

      if (MPEmotion_data[i].syb > (4<<MPEmotion_data[i].back_vert_f_code)-1)
      {
        if (!MPEquiet)
          fprintf(stderr,
            "Warning: reducing backward vertical search width to %d\n",
            (4<<MPEmotion_data[i].back_vert_f_code)-1);
        MPEmotion_data[i].syb = (4<<MPEmotion_data[i].back_vert_f_code)-1;
      }
    }
  }
}

static void readquantmat(void)
{
  int i,v;
  FILE *fd;

  if (MPEiqname[0]=='-')
  {
    /* use default intra matrix */
    MPEload_iquant = 0;
    for (i=0; i<64; i++)
      MPEintra_q[i] = MPEdefault_intra_quantizer_matrix[i];
  }
  else
  {
    /* read customized intra matrix */
    MPEload_iquant = 1;
    if (!(fd = fopen(MPEiqname,"r")))
    {
      sprintf(MPEerrortext,"Couldn't open quant matrix file %s",MPEiqname);
      error(MPEerrortext);
    }

    for (i=0; i<64; i++)
    {
      fscanf(fd,"%d",&v);
      if (v<1 || v>255)
        error("invalid value in quant matrix");
      MPEintra_q[i] = v;
    }

    fclose(fd);
  }

  if (MPEniqname[0]=='-')
  {
    /* use default non-intra matrix */
    MPEload_niquant = 0;
    for (i=0; i<64; i++)
      MPEinter_q[i] = 16;
  }
  else
  {
    /* read customized non-intra matrix */
    MPEload_niquant = 1;
    if (!(fd = fopen(MPEniqname,"r")))
    {
      sprintf(MPEerrortext,"Couldn't open quant matrix file %s",MPEniqname);
      error(MPEerrortext);
    }

    for (i=0; i<64; i++)
    {
      fscanf(fd,"%d",&v);
      if (v<1 || v>255)
        error("invalid value in quant matrix");
      MPEinter_q[i] = v;
    }

    fclose(fd);
  }
}

static void init(ENCODEINFO *info)
{
  int i, size;
  static int block_count_tab[3] = {6,8,12};

  initbits();
  init_fdctref();
  Initialize_Fast_IDCT();

  /* round picture dimensions to nearest multiple of 16 or 32 */
  MPEmb_width = (MPEhorizontal_size+15)/16;
  MPEmb_height = MPEprog_seq ? (MPEvertical_size+15)/16 : 2*((MPEvertical_size+31)/32);
  MPEmb_height2 = MPEfieldpic ? MPEmb_height>>1 : MPEmb_height; /* for field pictures */
  MPEwidth = 16*MPEmb_width;
  MPEheight = 16*MPEmb_height;

  MPEchrom_width = (MPEchroma_format==CHROMA444) ? MPEwidth : MPEwidth>>1;
  MPEchrom_height = (MPEchroma_format!=CHROMA420) ? MPEheight : MPEheight>>1;

  MPEheight2 = MPEfieldpic ? MPEheight>>1 : MPEheight;
  MPEwidth2 = MPEfieldpic ? MPEwidth<<1 : MPEwidth;
  MPEchrom_width2 = MPEfieldpic ? MPEchrom_width<<1 : MPEchrom_width;
  
  MPEblock_count = block_count_tab[MPEchroma_format-1];

  /* clip table */
  if (!(MPEclp = (unsigned char *)galloc(1024)))
    error("mpgwrite: galloc failed\n");
  MPEclp += 384;
  for (i=-384; i<640; i++)
    MPEclp[i] = (i<0) ? 0 : ((i>255) ? 255 : i);

  for (i=0; i<3; i++)
  {
    size = (i==0) ? MPEwidth*MPEheight : MPEchrom_width*MPEchrom_height;

    if (!(MPEnewrefframe[i] = (unsigned char *)galloc(size)))
      error("mpgwrite: galloc failed\n");
    if (!(MPEoldrefframe[i] = (unsigned char *)galloc(size)))
      error("mpgwrite: galloc failed\n");
    if (!(MPEauxframe[i] = (unsigned char *)galloc(size)))
      error("mpgwrite: galloc failed\n");
    if (!(MPEneworgframe[i] = (unsigned char *)galloc(size)))
      error("mpgwrite: galloc failed\n");
    if (!(MPEoldorgframe[i] = (unsigned char *)galloc(size)))
      error("mpgwrite: galloc failed\n");
    if (!(MPEauxorgframe[i] = (unsigned char *)galloc(size)))
      error("mpgwrite: galloc failed\n");
    if (!(MPEpredframe[i] = (unsigned char *)galloc(size)))
      error("mpgwrite: galloc failed\n");
  }

  MPEmbinfo = (struct MPEmbinfo *)galloc(MPEmb_width*MPEmb_height2*sizeof(struct MPEmbinfo));
  if (!MPEmbinfo)
    error("mpgwrite: galloc failed\n");

  MPEblocks = (short (*)[64])galloc(MPEmb_width*MPEmb_height2*MPEblock_count*sizeof(short [64]));
  if (!MPEblocks)
    error("mpgwrite: galloc failed\n");

  for (i=0; i<MPEm-1; ++i)
  {
    if (!(info->buffer[i] = (char*)galloc(MPEvertical_size*MPEhorizontal_size*sizeof(ARGB))))
      error("mpgwrite: galloc failed\n");
  }

  /* open statistics output file */
  MPEstatfile = stdout;
#ifdef STATFILE
  if (MPEstatname[0] != '-')
  {
    if (!(MPEstatfile = fopen(MPEstatname,"w")))
    {
      sprintf(MPEerrortext,"Couldn't create statistics output file %s",MPEstatname);
      error(MPEerrortext);
    }
  }
#endif
}

static void freemem(ENCODEINFO *info)
{
    int i;

    gfree(MPEmotion_data);
    MPEclp -= 384;
    gfree(MPEclp);

    for (i=0; i<3; i++)
    {
        gfree(MPEnewrefframe[i]);
        gfree(MPEoldrefframe[i]);
        gfree(MPEauxframe[i]);
        gfree(MPEneworgframe[i]);
        gfree(MPEoldorgframe[i]);
        gfree(MPEauxorgframe[i]);
        gfree(MPEpredframe[i]);
    }

    gfree(MPEmbinfo);
    gfree(MPEblocks);

    for (i=0; i<MPEm-1; ++i) gfree(info->buffer[i]);
}

static void putchunkheader(ENCODEINFO *info)
{
    if ((((info->subtype&SUBTYPE_TYPEMASK)==SUBTYPE_MPC) || ((info->subtype&SUBTYPE_TYPEMASK)==SUBTYPE_MPV)) && (info->patchpos == 0))
    {
        info->patchpos = (int)gtell(MPEoutfile) + 4;
        gwrite(MPEoutfile, chunkheader, 8);
    }
	else if (((info->subtype&SUBTYPE_TYPEMASK)==SUBTYPE_MPA) && (info->patchpos == 0))
	{
		info->patchpos = (int)gtell(MPEoutfile) + 4;
		gwrite(MPEoutfile, avichunkheader, 8);
		gwrite(MPEoutfile, chunkheader, 8);

	}

}

static void closechunk(ENCODEINFO *info)
{
    char buf[4];
    long pos;

    alignbits();

	pos = (int)gtell(MPEoutfile);
    
	while ((pos % 4) != 0)
    {
        buf[0] = 0;
        gwrite(MPEoutfile, buf, 1);
        ++pos;
    }
	gseek(MPEoutfile, (GPOS)info->patchpos);


    if ((info->subtype&SUBTYPE_TYPEMASK)==SUBTYPE_MPC
     || (info->subtype&SUBTYPE_TYPEMASK)==SUBTYPE_MPV)
    {       
        gputi(buf, pos-info->patchpos+4, 4);
        gwrite(MPEoutfile, buf, 4); 
    }
	else if ((info->subtype&SUBTYPE_TYPEMASK)==SUBTYPE_MPA)
	{
		int chunkLength = pos - info->patchpos - 4;

		gputi(buf, chunkLength, 4);
		gwrite(MPEoutfile, buf, 4);
		gseek(MPEoutfile, gtell(MPEoutfile) + 4);
		gwrite(MPEoutfile, buf, 4);

		/* update the AVI index */
		
		gputm(buf, GMAKEID('0','0','d','c'), 4);
		((info->index)[info->outframe]).Identifier  = ggeti(buf,4);
		((info->index)[info->outframe]).Flags		= (info->outframe % MPEn) ? 0 : 0x10;	// 0x10: keyframe
		((info->index)[info->outframe]).Offset		= 4 + info->dataSize;
		((info->index)[info->outframe]).Length		= chunkLength;
		 
		/* dataSize starts from the first '00dc', and ends at end of last movi chunk */
		info->dataSize += chunkLength + 8;  // add 8 since didn't include '00dcsize' (8 bytes)
   										    // for the chunk length 

		if( chunkLength > info->maxChunkSize )
		{
			info->maxChunkSize = chunkLength;
		}
		

	}													

	gseek(MPEoutfile, (GPOS)pos);
    info->patchpos = 0;
}

static void encodeframe(ENCODEINFO *info, char *source, int rowbytes)
{
    int i, j, k, f, f0, n, np, nb;
    int sxf = 0;
    int syf = 0;
    int sxb = 0;
    int syb = 0;
    int nframes;
    int ipflag;
    unsigned char *neworg[3], *newref[3];
    static char ipb[5] = {' ','I','P','B','D'};

#if defined(_MSC_VER)
	Sleep(1); // allow other threads some time, since encoding is slow
#endif

    nframes = info->frames;
    i = info->outframe;

    if (!MPEquiet)
    {
      fprintf(stderr,"Encoding frame %d ",i);
      fflush(stderr);
    }

    /* f0: lowest frame number in current GOP
     *
     * first GOP contains N-(M-1) frames,
     * all other GOPs contain N frames
     */
    f0 = MPEn*((i+(MPEm-1))/MPEn) - (MPEm-1);
    if (f0<0) f0=0;

    if (i==0 || (i-1)%MPEm==0)
    {
      /* I or P frame */
      for (j=0; j<3; j++)
      {
        /* shuffle reference frames */
        neworg[j] = MPEoldorgframe[j];
        newref[j] = MPEoldrefframe[j];
        MPEoldorgframe[j] = MPEneworgframe[j];
        MPEoldrefframe[j] = MPEnewrefframe[j];
        MPEneworgframe[j] = neworg[j];
        MPEnewrefframe[j] = newref[j];
      }

      /* f: frame number in display order */
      f = (i==0) ? 0 : i+MPEm-1;
      if (f>=nframes) f = nframes - 1;

      if (i==f0) /* first displayed frame in GOP is I */
      {
        /* I frame */
        MPEpict_type = I_TYPE;
        MPEforw_hor_f_code = MPEforw_vert_f_code = 15;
        MPEback_hor_f_code = MPEback_vert_f_code = 15;

        /* n: number of frames in current GOP
         *
         * first GOP contains (M-1) less (B) frames
         */
        n = (i==0) ? MPEn-(MPEm-1) : MPEn;

        /* last GOP may contain less frames */
        if (n > nframes-f0) n = nframes-f0;

        /* number of P frames */
        if (i==0)
          np = (n + 2*(MPEm-1))/MPEm - 1; /* first GOP */
        else
          np = (n + (MPEm-1))/MPEm - 1;

        /* number of B frames */
        nb = n - np - 1;

        rc_init_GOP(np,nb);

        putchunkheader(info);
        putgophdr(f0,i==0); /* set closed_GOP in first GOP only */
      }
      else
      {
        /* P frame */
        MPEpict_type = P_TYPE;
        MPEforw_hor_f_code = MPEmotion_data[0].forw_hor_f_code;
        MPEforw_vert_f_code = MPEmotion_data[0].forw_vert_f_code;
        MPEback_hor_f_code = MPEback_vert_f_code = 15;
        sxf = MPEmotion_data[0].sxf;
        syf = MPEmotion_data[0].syf;
      }
    }
    else
    {
      /* B frame */
      for (j=0; j<3; j++)
      {
        neworg[j] = MPEauxorgframe[j];
        newref[j] = MPEauxframe[j];
      }

      /* f: frame number in display order */
      f = i - 1;
      MPEpict_type = B_TYPE;
      n = (i-2)%MPEm + 1; /* first B: n=1, second B: n=2, ... */
      MPEforw_hor_f_code = MPEmotion_data[n].forw_hor_f_code;
      MPEforw_vert_f_code = MPEmotion_data[n].forw_vert_f_code;
      MPEback_hor_f_code = MPEmotion_data[n].back_hor_f_code;
      MPEback_vert_f_code = MPEmotion_data[n].back_vert_f_code;
      sxf = MPEmotion_data[n].sxf;
      syf = MPEmotion_data[n].syf;
      sxb = MPEmotion_data[n].sxb;
      syb = MPEmotion_data[n].syb;
    }

    MPEtemp_ref = f - f0;
    MPEframe_pred_dct = MPEframe_pred_dct_tab[MPEpict_type-1];
    MPEq_scale_type = MPEqscale_tab[MPEpict_type-1];
    MPEintravlc = MPEintravlc_tab[MPEpict_type-1];
    MPEaltscan = MPEaltscan_tab[MPEpict_type-1];

    STATPRINT(MPEstatfile,"\nFrame %d (#%d in display order):\n",i,f);
    STATPRINT(MPEstatfile," picture_type=%c\n",ipb[MPEpict_type]);
    STATPRINT(MPEstatfile," temporal_reference=%d\n",MPEtemp_ref);
    STATPRINT(MPEstatfile," frame_pred_frame_dct=%d\n",MPEframe_pred_dct);
    STATPRINT(MPEstatfile," q_scale_type=%d\n",MPEq_scale_type);
    STATPRINT(MPEstatfile," intra_vlc_format=%d\n",MPEintravlc);
    STATPRINT(MPEstatfile," alternate_scan=%d\n",MPEaltscan);

    if (MPEpict_type!=I_TYPE)
    {
      STATPRINT(MPEstatfile," forward search window: %d...%d / %d...%d\n",
        -sxf,sxf,-syf,syf);
      STATPRINT(MPEstatfile," forward vector range: %d...%d.5 / %d...%d.5\n",
        -(4<<MPEforw_hor_f_code),(4<<MPEforw_hor_f_code)-1,
        -(4<<MPEforw_vert_f_code),(4<<MPEforw_vert_f_code)-1);
    }

    if (MPEpict_type==B_TYPE)
    {
      STATPRINT(MPEstatfile," backward search window: %d...%d / %d...%d\n",
        -sxb,sxb,-syb,syb);
      STATPRINT(MPEstatfile," backward vector range: %d...%d.5 / %d...%d.5\n",
        -(4<<MPEback_hor_f_code),(4<<MPEback_hor_f_code)-1,
        -(4<<MPEback_vert_f_code),(4<<MPEback_vert_f_code)-1);
    }

    readframe(source, rowbytes, neworg);

    if (MPEfieldpic)
    {
      if (!MPEquiet)
      {
        fprintf(stderr,"\nfirst field  (%s) ",MPEtopfirst ? "top" : "bot");
        fflush(stderr);
      }

      MPEpict_struct = MPEtopfirst ? TOP_FIELD : BOTTOM_FIELD;

      motion_estimation(MPEoldorgframe[0],MPEneworgframe[0],
                        MPEoldrefframe[0],MPEnewrefframe[0],
                        neworg[0],newref[0],
                        sxf,syf,sxb,syb,MPEmbinfo,0,0);

      predict(MPEoldrefframe,MPEnewrefframe,MPEpredframe,0,MPEmbinfo);
      dct_type_estimation(MPEpredframe[0],neworg[0],MPEmbinfo);
      transform(MPEpredframe,neworg,MPEmbinfo,MPEblocks);

      putchunkheader(info);
      putpict(neworg[0]);
      closechunk(info);

      for (k=0; k<MPEmb_height2*MPEmb_width; k++)
      {
        if (MPEmbinfo[k].mb_type & MB_INTRA)
          for (j=0; j<MPEblock_count; j++)
            iquant_intra(MPEblocks[k*MPEblock_count+j],MPEblocks[k*MPEblock_count+j],
                         MPEdc_prec,MPEintra_q,MPEmbinfo[k].mquant);
        else
          for (j=0;j<MPEblock_count;j++)
            iquant_non_intra(MPEblocks[k*MPEblock_count+j],MPEblocks[k*MPEblock_count+j],
                             MPEinter_q,MPEmbinfo[k].mquant);
      }

      itransform(MPEpredframe,newref,MPEmbinfo,MPEblocks);
      calcSNR(neworg,newref);
      stats();

      if (!MPEquiet)
      {
        fprintf(stderr,"second field (%s) ",MPEtopfirst ? "bot" : "top");
        fflush(stderr);
      }

      MPEpict_struct = MPEtopfirst ? BOTTOM_FIELD : TOP_FIELD;

      ipflag = (MPEpict_type==I_TYPE);
      if (ipflag)
      {
        /* first field = I, second field = P */
        MPEpict_type = P_TYPE;
        MPEforw_hor_f_code = MPEmotion_data[0].forw_hor_f_code;
        MPEforw_vert_f_code = MPEmotion_data[0].forw_vert_f_code;
        MPEback_hor_f_code = MPEback_vert_f_code = 15;
        sxf = MPEmotion_data[0].sxf;
        syf = MPEmotion_data[0].syf;
      }

      motion_estimation(MPEoldorgframe[0],MPEneworgframe[0],
                        MPEoldrefframe[0],MPEnewrefframe[0],
                        neworg[0],newref[0],
                        sxf,syf,sxb,syb,MPEmbinfo,1,ipflag);

      predict(MPEoldrefframe,MPEnewrefframe,MPEpredframe,1,MPEmbinfo);
      dct_type_estimation(MPEpredframe[0],neworg[0],MPEmbinfo);
      transform(MPEpredframe,neworg,MPEmbinfo,MPEblocks);

      putchunkheader(info);
      putpict(neworg[0]);

      for (k=0; k<MPEmb_height2*MPEmb_width; k++)
      {
        if (MPEmbinfo[k].mb_type & MB_INTRA)
          for (j=0; j<MPEblock_count; j++)
            iquant_intra(MPEblocks[k*MPEblock_count+j],MPEblocks[k*MPEblock_count+j],
                         MPEdc_prec,MPEintra_q,MPEmbinfo[k].mquant);
        else
          for (j=0;j<MPEblock_count;j++)
            iquant_non_intra(MPEblocks[k*MPEblock_count+j],MPEblocks[k*MPEblock_count+j],
                             MPEinter_q,MPEmbinfo[k].mquant);
      }

      itransform(MPEpredframe,newref,MPEmbinfo,MPEblocks);
      calcSNR(neworg,newref);
      stats();
    }
    else
    {
      MPEpict_struct = FRAME_PICTURE;

      /* do motion_estimation
       *
       * uses source frames (...orgframe) for full pel search
       * and reconstructed frames (...refframe) for half pel search
       */

      motion_estimation(MPEoldorgframe[0],MPEneworgframe[0],
                        MPEoldrefframe[0],MPEnewrefframe[0],
                        neworg[0],newref[0],
                        sxf,syf,sxb,syb,MPEmbinfo,0,0);

      predict(MPEoldrefframe,MPEnewrefframe,MPEpredframe,0,MPEmbinfo);
      dct_type_estimation(MPEpredframe[0],neworg[0],MPEmbinfo);
      transform(MPEpredframe,neworg,MPEmbinfo,MPEblocks);

      putchunkheader(info);
      putpict(neworg[0]);

      for (k=0; k<MPEmb_height*MPEmb_width; k++)
      {
        if (MPEmbinfo[k].mb_type & MB_INTRA)
          for (j=0; j<MPEblock_count; j++)
            iquant_intra(MPEblocks[k*MPEblock_count+j],MPEblocks[k*MPEblock_count+j],
                         MPEdc_prec,MPEintra_q,MPEmbinfo[k].mquant);
        else
          for (j=0;j<MPEblock_count;j++)
            iquant_non_intra(MPEblocks[k*MPEblock_count+j],MPEblocks[k*MPEblock_count+j],
                             MPEinter_q,MPEmbinfo[k].mquant);
      }

      itransform(MPEpredframe,newref,MPEmbinfo,MPEblocks);
      calcSNR(neworg,newref);
      stats();
    }

    if (i == info->frames-1) putseqend();
    closechunk(info);

    info->outframe = i+1;
}

static void bufferframe(ENCODEINFO *info, char *source, int rowbytes)
{
    char    *dest;
    int     n;
    int     i;

    n = info->buffered;
    if (n >= 2) return;

    dest = info->buffer[n];
    for (i=0; i<MPEvertical_size; ++i)
    {
        memcpy(dest, source, MPEhorizontal_size*sizeof(ARGB));
        source += rowbytes;
        dest += MPEhorizontal_size*sizeof(ARGB);
    }

    info->buffered = n+1;
}

static void outputframes(ENCODEINFO *info, char *source, int rowbytes)
{
    int i;

    encodeframe(info, source, rowbytes);

    for (i=0; i<info->buffered; ++i)
        encodeframe(info, info->buffer[i], MPEhorizontal_size*sizeof(ARGB));

    info->buffered = 0;
}

static void flushframes(ENCODEINFO *info)
{
    int i;

    if (info->buffered > 0)
    {
        /* last buffered frame must go first because it won't be a B frame */
        encodeframe(info, info->buffer[info->buffered-1], MPEhorizontal_size*sizeof(ARGB));

        for (i=0; i<info->buffered-1; ++i)
            encodeframe(info, info->buffer[i], MPEhorizontal_size*sizeof(ARGB));
    }
}

static int outputAviHeaders(const GINFO* ginfo, const ENCODEINFO *info)
{
	int ok=0;
    char buf[56];
    int hsize;
    int bpp;
    int TimeBetweenFrames;
    int datahandler;
	int maxDataSize;
	int maxBps;

    GSTREAM *g = MPEoutfile;
        
    datahandler = 0; /* default */
    bpp = 32;  
	TimeBetweenFrames = (int)((double)1000000.0f/MPEframe_rate);
	maxDataSize = ginfo->width * ginfo->height * 4;
	maxBps =  (int) ((float) MPEbit_rate/8.0f);

    /* Output initial file chunk header (Leaving buf+4 for later) */
    gputm(buf,GMAKEID('R','I','F','F'),4);
    gputm(buf+8,GMAKEID('A','V','I',' '),4);
    ok = gwrite(g,buf,12);

    /* Output AVI header chunk header */
    hsize = 192;
    gputm(buf,GMAKEID('L','I','S','T'),4);
    gputi(buf+4,hsize,4);
    gputm(buf+8,GMAKEID('h','d','r','l'),4);
    ok = gwrite(g,buf,12);

    /* Output AVI header */
    gputm(buf,GMAKEID('a','v','i','h'),4);
    gputi(buf+4,56,4);
    ok = gwrite(g,buf,8);

    gputi(buf, TimeBetweenFrames, 4);   /* TimeBetweenFrames (1000000/15fps) */
    gputi(buf+4, maxBps , 4);			/* MaxBps */
    gputi(buf+8, 0, 4);                 /* PaddingGranularity */
    gputi(buf+12, 0x00000010, 4);       /* Flags (Index exists) */
    gputi(buf+16, info->frames, 4);     /* TotalFrames */
    gputi(buf+20, 0, 4);                /* InitFrames */
    gputi(buf+24, 1, 4);                /* Streams */
    gputi(buf+28, maxDataSize, 4);      /* SuggestedBufferSize (Max frame data size, Modified later) */
    gputi(buf+32, ginfo->width, 4);     /* Width */
    gputi(buf+36, ginfo->height, 4);    /* Height */
    gputi(buf+40, TimeBetweenFrames,4); /* TimeScale */
    gputi(buf+44, 0, 4);                /* DataRate */
    gputi(buf+48, 0, 4);                /* StartTime */
    gputi(buf+52, 0, 4);                /* DataLength */
    ok = gwrite(g,buf,56);

    /* Output video stream header chunk header */
    hsize -= 76;
    gputm(buf,GMAKEID('L','I','S','T'),4);
    gputi(buf+4,hsize,4);
    gputm(buf+8,GMAKEID('s','t','r','l'),4);
    ok = gwrite(g,buf,12);

    /* Output video stream header */
    gputm(buf,GMAKEID('s','t','r','h'),4);
    gputi(buf+4,56,4);
    ok = gwrite(g,buf,8);

    gputm(buf, GMAKEID('v','i','d','s'), 4); /* DataType */
    gputm(buf+4, datahandler, 4);       /* DataHandler */
    gputi(buf+8,  0x00000000, 4);       /* Flags */
    gputi(buf+12, 0, 4);                /* Priority */
    gputi(buf+16, 0, 4);                /* InitialFrames */
    gputi(buf+20, TimeBetweenFrames, 4);/* TimeScale */
    gputi(buf+24, 0x000f4240, 4);       /* DataRate */
    gputi(buf+28, 0, 4);                /* StartTime */
    gputi(buf+32, info->frames, 4);     /* DataLength */
    gputi(buf+36, 0x00080000, 4);       /* SuggestedBufferSize (See above)*/
    gputi(buf+40, 0, 4);				/* Quality (10000 Max) */
    gputi(buf+44, 0, 4);                /* SampleSize */
    gputi(buf+48, 0, 4);                /* Reserved */
    gputi(buf+52, 0, 4);                /* Reserved */
    ok = gwrite(g,buf,56);

    /* Output Bitmap info header */
    hsize = 40;
    gputm(buf,GMAKEID('s','t','r','f'),4);
    gputi(buf+4,hsize,4);
    ok = gwrite(g,buf,8);

    gputi(buf, 40, 4);							 /* Size */
    gputi(buf+4, ginfo->width, 4);				 /* Width */
    gputi(buf+8, ginfo->height, 4);              /* Height */
    gputi(buf+12, 1, 2);						 /* Planes */
    gputi(buf+14, 32, 2);						 /* BitCount */
    gputi(buf+16, GMAKEID('h','C','P','M'), 4);  /* Compression (RGB) */
    gputi(buf+20, maxDataSize, 4);				 /* ImageSize */ /* KAZ */
    gputi(buf+24, 0, 4);						 /* XpelsMeter */
    gputi(buf+28, 0, 4);						 /* YpelsMeter */
    gputi(buf+32, 0, 4);						 /* NumberOfColours */
    gputi(buf+36, 0, 4);						 /* ImportantColours */
    ok = gwrite(g,buf,40);

    /* Output movie chunk header */
    gputm(buf,GMAKEID('L','I','S','T'),4);
    gputi(buf+4,0,4); /* Correct size to be given later (on close) */
    gputm(buf+8,GMAKEID('m','o','v','i'),4);
    ok = gwrite(g,buf,12);

    return(ok);
}



static int outputMpvHeaders(const GINFO* ginfo, const ENCODEINFO *info)
{
	int ok=0;
    char buf[32];
	int maxDataSize = ginfo->width * ginfo->height * 4;
    GSTREAM *g = MPEoutfile;
	unsigned int rate=(int)(ginfo->fps*1000);
	if (!rate) rate=29970;


    gputm(buf+0, GMAKEID('M','V','h','d'), 4);	/* type */
    gputi(buf+4, 32 , 4);						/* size */
    gputm(buf+8, GMAKEID('m','p','e','g'), 4);	/* mCompressor */
    gputi(buf+12, ginfo->width, 2);     /* Width */
    gputi(buf+14, ginfo->height, 2);    /* Height */
    gputi(buf+16, info->frames, 4);     /* TotalFrames */
    gputi(buf+20, maxDataSize, 4);      /* SuggestedBufferSize (Max frame data size, Modified later) */
    gputi(buf+24, rate, 4);				/* mFpsNumerator */
    gputi(buf+28, 1000, 2);             /* mFpsDenominator */
    gputi(buf+30, 0,  2);               /* mFlags */
    ok = gwrite(g,buf,32);

    return(ok);
}


static int outputAviIndex(GSTREAM* g, ENCODEINFO* info)
{

	int riffSize;
	int moviSize;
	int sizeIndex;
	int suggestedBufferSize;
	int maxBps;
	char buf[4];


	sizeIndex = info->frames * sizeof(AviIndex);
	gputm(buf,GMAKEID('i','d','x','1'),4);
	gwrite(g, buf, 4);
	
	gputi(buf, sizeIndex ,4);
	gwrite(g, buf, 4);

	gwrite(g, info->index, sizeIndex);
					
	/* Write in size of RIFF chunk*/				
	riffSize = info->dataSize + sizeIndex + 0xe0;
	gseek(g,4);
	gwrite(g, &riffSize, 4);	// 0xe0 is constant offset to movi chunk
	
	/* Write in size of LIST:movi chunk*/
	moviSize = info->dataSize+4;
	gseek(g,0xd8); 
	gwrite(g,&moviSize,4);

	/* Write in updated Avi header info */
	maxBps = (int)( (float) info->maxChunkSize * MPEframe_rate);
	suggestedBufferSize = info->maxChunkSize + 1024; // +1024 for possible overflow
	
	//MaxBps 
	gseek(g,0x24);
	gwrite(g,&maxBps,4);
	
	// Suggested Buffer size
	gseek(g,0x3c);
	gwrite(g,&suggestedBufferSize,4);
	gseek(g,0x90);
	gwrite(g,&suggestedBufferSize,4);


	gfree(info->index);

	return 1;


}



/****************************************************************/
/*  Export Functions                                            */
/****************************************************************/

int GCALL MPG_wopen(GINSTANCE **ginstance, GSTREAM *g, const char *pathname, int numframes)
{
    GINSTANCE   *gx;
    ENCODEINFO  *info;

    *ginstance = NULL;
    if (initialized) return(0); // encoder is not reentrant

    info = (ENCODEINFO*) galloc(sizeof(ENCODEINFO));
    if (info == NULL) return(0);

    gx = (GINSTANCE *) galloc(sizeof(GINSTANCE));
    if (gx)
    {
        memset(gx, 0, sizeof(GINSTANCE));
        gx->signature = SIGNATURE;
        gx->size      = sizeof(GINSTANCE);
        gx->frames    = numframes;
        gx->framenum  = 0;
        gx->gstream   = g;
        gx->gref      = (char*) info;

        info->frames = numframes;
        info->outframe = 0;
        info->buffered = 0;
        info->patchpos = 0;
		info->dataSize = 0;
		info->maxChunkSize = 0;
		info->index    = NULL;

        MPEoutfile = g;
        MPEquiet = 1;

        initialized = 1;
        *ginstance = gx;
        return(1);
    }

    gfree(info);
    return(0);
}

int GCALL MPG_write(GINSTANCE *gx, const GINFO *ginfo, char *source, int rowbytes)
{
    ENCODEINFO  *info;
    int frame;
    int ok=0;

    info = (ENCODEINFO*) gx->gref;
    frame = gx->framenum;
    if (ginfo->bpp>8)
    {
        if (frame == 0)
        {
            info->subtype = ginfo->subtype;
            initparms(ginfo);
            readquantmat();
            init(info);
            initreadframe();

            rc_init_seq(); /* initialize rate control */

			if ((info->subtype&SUBTYPE_TYPEMASK)==SUBTYPE_MPA)
			{
				/* Output AVI headers */
				ok = outputAviHeaders(ginfo,info);
				info->index = (AviIndex *) galloc(info->frames * sizeof(AviIndex));
			}
			else if ((info->subtype&SUBTYPE_TYPEMASK)==SUBTYPE_MPV)
			{
				/* Output MPV header */
				ok = outputMpvHeaders(ginfo,info);
			}

            /* sequence header, sequence extension and sequence display extension */
            putchunkheader(info);
            putseqhdr();
            if (!MPEmpeg1)
            {
              putseqext();
              putseqdispext();
            }

            /* optionally output some text data (description, copyright or whatever) */
            if (strlen(MPEid_string) > 1) putuserdata(MPEid_string);
        }

        if ((frame % MPEm) == 0)
            outputframes(info, source, rowbytes);
        else
            bufferframe(info, source, rowbytes);

        gx->framenum = frame+1;
        ok = 1;
    }
    return(ok);
}

int GCALL MPG_wclose(GINSTANCE *gx)
{
    ENCODEINFO  *info;
	GSTREAM* g;
	
	if (gx)
    {
        info = (ENCODEINFO*) gx->gref;
		g	 =  gx->gstream;

        if (gx->framenum)
        {
            flushframes(info);
            if(MPEstatfile != stdout)
                fclose(MPEstatfile);

			if ((info->subtype&SUBTYPE_TYPEMASK)==SUBTYPE_MPA)
			{
				/* write in the AVI index */
				outputAviIndex(g,info);
			}
		
            restorereadframe();
            freemem(info);

		}
        if (info) gfree(info);
        gfree(gx);
        initialized = 0;
    }

    return(1);
}
