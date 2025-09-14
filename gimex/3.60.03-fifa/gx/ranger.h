// (c) 2000-2007 Electronic Arts Inc.

/*------------------------------------------------------------------*/
/*                                                                  */
/*                         Ranger Routines                          */
/*                                                                  */
/*                     by FrANK G. Barchard, EAC                    */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* See ranger.c for version history, module notes, examples and     */
/* ranger syntax.                                                   */
/*                                                                  */
/*------------------------------------------------------------------*/

#ifndef RANGER_H
#define RANGER_H

#if defined(_MSC_VER)
#pragma once
#endif

#include "spit.h"
#include "expr.h"

#ifdef __cplusplus
extern "C" {
#endif

#define RANGER_VERSION 108

/* round modes for RANGER_closest */
#define RANGER_ROUND_CLOSEST    0
#define RANGER_ROUND_FLOOR      1
#define RANGER_ROUND_CEILING    2

/* datatypes */
typedef struct RANGER RANGER;

RANGER *RANGER_open(EXPR *expr, const char *rangerstr, double min, double max);
void    RANGER_close(RANGER *ranger);

bool    RANGER_next(RANGER *ranger, double *next);
bool    RANGER_info(RANGER *ranger, double *min, double *max, INT64 *count);
double  RANGER_closest(RANGER *ranger, double val, int round);


/* private functions */

struct RANGER
{
    double next;
    double step;
    double first;
    double last;
    double min;
    double max;
    INT64  count;
    EXPR *expr;
    const char *nextstr;
    const char *rangerstr;
    bool error;
};

bool RANGER_getrangelet(RANGER *ranger);

#define RANGER_next RANGERINLINE_next

static bool __inline RANGERINLINE_next(RANGER *ranger, double *next)
{
    bool more=true;

    ranger->next += ranger->step;
    if (--ranger->count < 0)
    {
        more = RANGER_getrangelet(ranger);
        --ranger->count;
    }
    *next = ranger->next;
    return more;
}

#ifdef __cplusplus
}
#endif
#endif // RANGER_H


