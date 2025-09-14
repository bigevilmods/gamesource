// (c) 2000-2006 Criterion Software Limited and its licensors. All Rights Reserved

/*------------------------------------------------------------------*/
/*                                                                  */
/*                         Ranger Routines                          */
/*                                                                  */
/*                     by FrANK G. Barchard, EAC                    */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History:                                       */
/* ------- ----   -- --------                                       */
/*    1.00 000322 FB based on range api, new implementation         */
/*    1.01 000331 FB linux port                                     */
/*    1.02 000613 FB isdigit bug fix                                */
/*    1.03 000928 FB OSX                                            */
/*    1.04 010821 FB cast galloc for C++ friendly compile           */
/*    1.05 020314 FB bool, expression error handling                */
/*    1.06 020315 FB error return requires non const handles        */
/*    1.07 031001 FB respect EXPR_setbase, new getfield, simplified */
/*                                                                  */
/*------------------------------------------------------------------*/


/*!
************************************************************
 \file ranger.cpp
 \brief Ranger is a more sophisticated and order preserving version of the range api

 \par Ranger API
 \code                                                                  
 RANGER *RANGER_open(expr, rangerstr, min, max);
 void    RANGER_close(ranger);
 bool    RANGER_info(ranger, &min, &max, &count);
 bool    RANGER_next(ranger, &next);
 double  RANGER_closest(ranger, val, round);
 \endcode
                                                                  
 \par Range syntax
 \box
 -f!min..max+=step
 ie  -f0..9
     -f1,3
     -f!5
     -f0..9+=2
     -fapple..zebra
 \endbox

 \par Example of use
 \code
 {                                                                
     double v;
     RANGER *r=RANGER_open(NULL,"10..20+=.5",0.0,10000.0);
     while (RANGER_next(r,&v)
         printf("%.1f\n",v);
     RANGER_close(r);
 }
 \endcode
*************************************************************/

#ifndef __RANGE_C
#define __RANGE_C 1

#include "ranger.h"
#include <math.h>
#include "spit.h"

/****************************************************************/
/*  Internal Functions                                          */
/****************************************************************/

static const int RANGERMAXITEM=512;

#if !defined(_MSC_VER)
#define __forceinline __inline
#endif
#if defined(DEBUG)
#define __inline
#endif

#define EPSILON 0.000000001

static __inline double ROUNDDOUBLE(double f)
{
    double r;

#if defined(_MSC_VER)
    __asm
    {
        fld     f
        frndint
        fstp    r
    }
#else
    if (f<0.0)
        r = -floor(-f+0.5);
    else
        r = floor(f+0.5);
#endif
    return(r);
}


static __inline INT64 DOUBLE2INT64(double f)
{
    INT64 i;

#if defined(_MSC_VER)
    __asm
    {
        fld     f
        fistp   i
    }
#else
    i = (INT64) f;
#endif
    return(i);
}

#define FLOOR(f) floor(f)

/* private functions */

static char *RANGER_getfield(char *dest,const char *source)
{
    const char *s;
    char *d;
    char lim=0;
    char delim=0;
    int  depth;

    s = source;
    d = dest;
    depth = 0;

    while (*s)
    {
        if (!depth)   // within delimiters, only stop on matching delimiter or null
        {
            /* check for terminating field values */

            if (*s==(char)','
             || (*s=='.' && *(s+1)=='.')
             || (*s=='+' && *(s+1)=='=')
             || (*s=='-' && *(s+1)=='='))
            {
                break;
            }
            else
            {
                if (*s==(char)'(' || *s==(char)'{' || *s==(char)'[' || *s==(char)'"' || *s==(char)'`' || *s==(char)'\'')
                {
                    lim = delim = *s;
                    if (lim==(char)'(') delim = (char)')';
                    if (lim==(char)'{') delim = (char)'}';
                    if (lim==(char)'[') delim = (char)']';
                    ++depth;
                }
            }
        }
        else  // not delimited, check for start of delimited section
        {
            if (*s==delim)    --depth;
            else if (*s==lim) ++depth;
        }
        if (d-dest<RANGERMAXITEM-1)
            *d++ = *s;
        ++s;
    }
    *d = '\0';
    return((char *)s);
}


#define RANGER_isdigit(c) ((unsigned int)(c)-(unsigned int)'0'<=(unsigned int)9)

static __forceinline double RANGER_atof(EXPR *expr, const char *fieldstr)
{
    double f=0.0;
    int neg=0;
    const char *s = fieldstr;
    double Radix=10.0;
    if (expr)
        Radix = (double) EXPR_getbase(expr);

    if (*s=='+')
        ++s;
    else if (*s=='-')
    {
        ++s;
        neg = 1;
    }
    while (RANGER_isdigit(*s))
    {
        f = f*Radix+(double)(*s-'0');
        ++s;
    }
    if (*s=='.')
    {
        double frac=0.1;
        ++s;
        while (RANGER_isdigit(*s))
        {
            f = f+frac*(double)(*s-'0');
            frac = frac/Radix;
            ++s;
        }
    }
    if (neg)
        f = -f;
    if (*s)
    {
        if (expr)
            f = EXPR_get(expr, fieldstr);
        else
            f = atof(fieldstr);
    }
    return(f);
}

static __inline bool RANGER_getnum(EXPR *expr, const char **rangerstr,double *val)
{
    char fieldbuf[RANGERMAXITEM];
    bool more;

    *rangerstr = RANGER_getfield(fieldbuf,*rangerstr);
    if (fieldbuf[0])
    {
        more = true;
        *val = RANGER_atof(expr, fieldbuf);
    }
    else
    {
        more = false;
        *val = 0.0;
    }
    return more;
}

// should be static, but is used by inline
bool RANGER_getrangelet(RANGER *ranger)
{
    const char *rangerstr=ranger->nextstr;
    bool more=false;

    if (*rangerstr)
    {
        double first;
        bool   firstq=false;

        firstq = RANGER_getnum(ranger->expr,&rangerstr,&first);
        if (EXPR_error(ranger->expr))
        {
            ranger->error = true;
            goto ExitError;
        }

        if (*rangerstr==',' || *rangerstr=='\0')
        {
            ranger->next = ranger->first = ranger->last =  first;
            ranger->step = 1.0;
            ranger->count = (INT64) 1;
            more = firstq;
            if (*rangerstr==',')
                ++rangerstr;
        }
        else
        {
            double last, step, next;
            bool   lastq=false, stepq=false, dotq=false;
            int    plusq=0;

            if (*rangerstr=='.' && *(rangerstr+1)=='.')
            {
                rangerstr+=2;
                dotq = true;
            }
            lastq = RANGER_getnum(ranger->expr,&rangerstr,&last);
            if (EXPR_error(ranger->expr))
            {
                ranger->error = true;
                goto ExitError;
            }

            if (*rangerstr=='+' && *(rangerstr+1)=='=')
            {
                rangerstr+=2;
                plusq = 1;
            }
            if (*rangerstr=='-' && *(rangerstr+1)=='=')
            {
                rangerstr+=2;
                plusq = -1;
            }
            stepq = RANGER_getnum(ranger->expr,&rangerstr,&step);
            if (EXPR_error(ranger->expr))
            {
                ranger->error = true;
                goto ExitError;
            }

            if ((*rangerstr=='\0' || *rangerstr==',')
             && (firstq || lastq || stepq))
            {
                INT64  rcount;
                more = true;
                if (*rangerstr==',')
                    ++rangerstr;

                if (!stepq)     // for first alignment
                    step = 1.0;
                else
                {
                    if (step<0) step = -step;
                    else if (!step)
                    {
                        step = 1;
                        ranger->error = true;
                    }
                }

                // ..5 +=2 ..5+=2

                if (!firstq)    // ..5+=2 -> 1..5+=2
                {
                    first = ranger->min;
                    if (lastq)
                        first = FLOOR((ranger->min-last)/-step)*-step+last;
                }
                // 5 5.. +=2 5..+=2
                if (!lastq)
                {
                    last = first;
                    if (dotq || stepq)
                        last = FLOOR((ranger->max-first)/step)*step+first;
                }

                next = first;
                rcount = DOUBLE2INT64((last-first+step/1000000000)/step+1.0-0.5);    // +1 for inclusive end point, 0.5 to truncate, not round
                if (last<first)
                {
                    double temp;
                    temp = last;
                    last = first;
                    first = temp;
                    next = last;
                    step = -step;
                    rcount = 2-rcount;  // +1 was in wrong direction before.  remove and reapply
                }

                ranger->count = rcount;
                ranger->next = next;
                ranger->first= first;
                ranger->last = last;
                ranger->step = step;
            }
            else
                ranger->error = true;
        }
    }

ExitError:
    ranger->nextstr = rangerstr;
    if (ranger->error) more = false;
    return more;
}


/* public functions */
/*!
************************************************************
  \brief perpare to work with a range string
************************************************************/
RANGER *RANGER_open(EXPR *expr, const char *rangerstr, double min, double max)
{
    RANGER *ranger = (RANGER *) galloc(sizeof(RANGER));
    if (ranger)
    {
        ranger->expr = expr;
        ranger->nextstr = ranger->rangerstr = rangerstr;
        ranger->next = ranger->first = ranger->last = 0.0;
        ranger->step = 1.0;
        ranger->min = min;
        ranger->max = max;
        ranger->count = (INT64) 0;
        ranger->error = 0;
    }
    return(ranger);
}

/*!
************************************************************
  \brief cleanup from a RANGER_open
************************************************************/
void RANGER_close(RANGER *ranger)
{
    gfree(ranger);
}

#undef RANGER_next
// NOTE: header contains inline version

/*!
************************************************************
  \brief get the next value the user requested in the range
         expression.
************************************************************/
bool RANGER_next(RANGER *ranger, double *next)
{
    return(RANGERINLINE_next(ranger, next));
}

#define RANGER_next RANGERINLINE_next

/*!
************************************************************
  \brief extract min max and count information from a
         users requested range.
************************************************************/
bool RANGER_info(RANGER *ranger, double *min, double *max, INT64 *count)
{
    RANGER inforanger;
    double rmin;
    double rmax;
    INT64 rcount;

    inforanger = *ranger;
    inforanger.nextstr = inforanger.rangerstr;
    rmin = inforanger.max;
    rmax = inforanger.min;
    rcount = 0;

    while (RANGER_getrangelet(&inforanger))
    {
        if (inforanger.last>rmax)  rmax = inforanger.last;
        if (inforanger.first<rmin) rmin = inforanger.first;
        rcount += inforanger.count;
    }
    if (min)   *min = rmin;
    if (max)   *max = rmax;
    if (count) *count = rcount;
    return(!inforanger.error);
}

//@{
#define RANGER_ROUNDDOUBLE_CLOSEST    0
#define RANGER_ROUNDDOUBLE_FLOOR      1
#define RANGER_ROUNDDOUBLE_CEILING    2
//}@

/*!
************************************************************
  \brief round a value to the nearest value within a range.
   \param round one of the ROUNDDOUBLE constants
************************************************************/
double RANGER_closest(RANGER *ranger, double val, int round)
{
    RANGER inforanger;
    double rdiff;
    double rval;
    double bestval=0.0;
    double bestdiff;
    double worstdiff;

    inforanger = *ranger;
    inforanger.nextstr = inforanger.rangerstr;

    bestdiff = worstdiff = 999999999999999.0;

    if (round==RANGER_ROUNDDOUBLE_CLOSEST)
    {
        while (RANGER_getrangelet(&inforanger))
        {
            rval = ROUNDDOUBLE((val-inforanger.next)/inforanger.step)*inforanger.step+inforanger.next;
            if (rval<inforanger.first) rval=inforanger.first;
            if (rval>inforanger.last)  rval=inforanger.last;

            rdiff = val-rval;
            if (rdiff<0.0)
                rdiff = -rdiff-EPSILON;

            if (rdiff<=bestdiff)
            {
                bestval = rval;
                bestdiff = rdiff;
            }
        }
    }
    else if (round==RANGER_ROUNDDOUBLE_FLOOR)
    {
        while (RANGER_getrangelet(&inforanger))
        {
            rval = FLOOR((val-inforanger.next)/inforanger.step)*inforanger.step+inforanger.next;
            if (rval<inforanger.first) rval=inforanger.first;
            if (rval>inforanger.last)  rval=inforanger.last;

            rdiff = val-rval;
            if (rdiff<0.0)
                rdiff = worstdiff;

            if (rdiff<=bestdiff)
            {
                bestval = rval;
                bestdiff = rdiff;
            }
        }
    }
    else // if (round==RANGER_ROUNDDOUBLE_CEILING)
    {
        while (RANGER_getrangelet(&inforanger))
        {
            rval = FLOOR((val-inforanger.next)/inforanger.step+0.999999999)*inforanger.step+inforanger.next;
            if (rval<inforanger.first) rval=inforanger.first;
            if (rval>inforanger.last)  rval=inforanger.last;

            rdiff = rval-val;
            if (rdiff<0.0)
                rdiff = worstdiff;

            if (rdiff<=bestdiff)
            {
                bestval = rval;
                bestdiff = rdiff;
            }
        }
    }
    ranger->error = inforanger.error; // let caller query if error occurred

    return(bestval);
}

#endif

