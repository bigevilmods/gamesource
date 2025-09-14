// (c) 1994-2007 Electronic Arts Inc.

#ifndef __RLAGIMEX
#define __RLAGIMEX 1

#include "gimex.h"

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************/
/*  RLA Gimex                                                   */
/****************************************************************/

/* Information Functions */

GABOUT * GCALL RLA_about(void);
int      GCALL RLA_is(GSTREAM *g);

/* Import Functions */

int      GCALL RLA_open(GINSTANCE **gx, GSTREAM *g, const char *pathname,bool framecountflag);
GINFO *  GCALL RLA_info(GINSTANCE *gx, int framenum);
int      GCALL RLA_read(GINSTANCE *gx, GINFO *ginfo, char *dest, int rowbytes);
int      GCALL RLA_close(GINSTANCE *gx);

/* Export Functions */

int      GCALL RLA_wopen(GINSTANCE **gx, GSTREAM *g, const char *pathname, int numframes);
int      GCALL RLA_write(GINSTANCE *gx, const GINFO *ginfo, char *source, int rowbytes);
int      GCALL RLA_wclose(GINSTANCE *gx);

/****************************************************************/
/*  Internal                                                    */
/****************************************************************/

typedef struct
{
    char  left[2];
    char  right[2];
    char  bottom[2];
    char  top[2];
} WINDOW_S;

#define RLA_REVISION    0xfffe
#define RLA_TIME_OFFSET 614
#define IMF_C_byte      0
#define RLA_C_INTEGER   IMF_C_byte
#define RLA_C_FLOAT     4

typedef struct
{
    /* RLA FILE HEADER structure 740 bytes */

    WINDOW_S            window;             /* overall frame size */
    WINDOW_S            active_window;      /* minimum rect that contains real data */
    char                frame[2];
    char                storage_type[2];
    char                num_chan[2];
    char                num_matte[2];
    char                num_aux[2];
    char                revision[2];            /* used to be aux_mask */
    char                gamma[16];
    char                red_pri[24];
    char                green_pri[24];
    char                blue_pri[24];
    char                white_pt[24];
    char                job_num[4];
    char                name[128];
    char                desc[128];
    char                program[64];
    char                machine[32];
    char                user[32];
    char                date[20];
    char                aspect[24];
    char                aspect_ratio[8];
    char                chan[32];
                                                /* new fields */
    char                field[2];
    char                time[12];
    char                filter[32];
    char                chan_bits[2];
    char                matte_type[2];
    char                matte_bits[2];
    char                aux_type[2];
    char                aux_bits[2];
    char                aux[32];
    char                space[36];              /* expansion space */
    char                next[4];
} RLAHeaderStruct;

#define RLAGETFIELD(field) ggetm((void *)&field, sizeof(field))
#define RLAPUTFIELD(field,data) gputm((void *)&field,(unsigned int)(data),sizeof(field))

#ifdef __cplusplus
}
#endif
#endif


