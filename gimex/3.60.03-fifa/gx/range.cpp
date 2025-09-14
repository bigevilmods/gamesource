// (c) 1995-2006 Criterion Software Limited and its licensors. All Rights Reserved

/*------------------------------------------------------------------*/
/*                                                                  */
/*                         Range Routines                           */
/*                                                                  */
/*                     by FrANK G. Barchard, EAC                    */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History:                                       */
/* ------- ----   -- --------                                       */
/*    1.00 95xxxx FB initial version in G? tools                    */
/*    1.01 950628 GS extracted code to seperate source module       */
/*    1.02 950831 FB getrangefield public                           */
/*    1.03 950911 FB RANGE_check and getrangeopt                    */
/*    1.04 960430 FB quick check for full range ""                  */
/*    1.05 961008 FB simple string support                          */
/*    1.06 961009 FB wildcards, min/max, numerical strings          */
/*    1.07 980218 SK no SPIT. parse min/max/step all at once        */
/*    1.08 980224 SK added RANGE_constrain (modified parser)        */
/*    1.09 020208 FB identify numerical strings                     */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Range API:                                                       */
/*                                                                  */
/* int RANGE_constrain(const char *rangestr, int num, int option)   */
/* int RANGE_match(const char *rangestr, char *name, int num)       */
/* int RANGE_min(const char *rangestr, int minrange)                */
/* int RANGE_max(const char *rangestr, int maxrange)                */
/* int RANGE_check(const char *rangestr, int minrange,int maxrange) */
/* int RANGE_count(const char *rangestr, int minrange,int maxrange) */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/*                                                                  */
/* RANGE_min is a lower bound on the range, but may be unbounded.   */
/* ie ..10 has no minimum.  The min and max will be restricted to   */
/* the min,max passed and this range of indexes should be           */
/* iterated over.                                                   */
/* RANGE_count is an upper bound on the count, because it does not  */
/* include string matching.                                         */
/* RANGE_constrain constrains num to the nearest element in the     */
/* range.  Option < 0 rounds down, option > 0 rounds up.            */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Range syntax:                                                    */
/* -------------                                                    */
/* -f!min..max+=step                                                */
/* ie  -f0..9                                                       */
/*     -f1,3                                                        */
/*     -f!5                                                         */
/*     -f0..9+=2                                                    */
/*     -fapple..zebra                                               */
/*     -fbob*                                                       */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Example of use:                                                  */
/* --------------                                                   */
/*                                                                  */
/* int first = RANGE_min(rangestr, 0, 10000);                        */
/* int last  = RANGE_max(rangestr, 0, 10000);                        */
/* int count = RANGE_count(rangestr, 0, 10000);                      */
/* int ok = RANGE_check(rangestr, 0, 10000);                         */
/* int i;                                                           */
/*                                                                  */
/* if (!ok)                                                         */
/*   printf("a value in the specified range is out of               */
/* for (i=first; i<=last; ++i)                                      */
/* {                                                                */
/*     GetInfo(&name,i);                                            */
/*     if (RANGE_match(rangestr, name, i))                           */
/*         ProcessItem(i);                                          */
/* }                                                                */
/*                                                                  */
/*------------------------------------------------------------------*/

#ifndef __RANGE_C
#define __RANGE_C 1


#include <ctype.h>
#include "range.h"
#include "spit.h"

/*!
  \file range.cpp
  \brief provides support for the range syntax (x..y) to cop programs

  RANGE_min is a lower bound on the range, but may be unbounded.   
  ie ..10 has no minimum.  The min and max will be restricted to   
  the min,max passed and this range of indexes should be           
  iterated over.                                                   
  RANGE_count is an upper bound on the count, because it does not  
  include string matching.                                         
  RANGE_constrain constrains num to the nearest element in the     
  range.  Option < 0 rounds down, option > 0 rounds up.            

  \par Range syntax                                                   
  \box
  -f!min..max+=step
  ie  -f0..9                                                       
      -f1,3                                                        
      -f!5                                                         
      -f0..9+=2                                                    
      -fapple..zebra                                               
      -fbob*                                                       
  \endbox                                                            
  \par Example of use
  \code
   int first = RANGE_min(rangestr, 0, 10000);                        
   int last  = RANGE_max(rangestr, 0, 10000);                        
   int count = RANGE_count(rangestr, 0, 10000);                      
   int ok    = RANGE_check(rangestr, 0, 10000);                         
   int i;                                                           
                                                                  
   if (!ok)                                                         
     printf("a value in the requested range is out of the allowed range\n");
   for (i=first; i<=last; ++i)                                      
   {                                                                
       GetInfo(&name,i);                                            
       if (RANGE_match(rangestr, name, i))                           
           ProcessItem(i);                                          
   }
  \endcode                                                            
*/


typedef struct RANGE
{
    int  num;
    char name[SPITMAXLINE];
} RANGE;

#define SPITBIGNUM 0x7fffffff
#define SPITUNDEF (SPITBIGNUM-1)

/****************************************************************/
/*  Internal Functions                                          */
/****************************************************************/

#define HARDSPACE 28

/* Checks if an entire string is numeric */
static int rangeisnumeric(const char *str)
{
    if (*str == '\0')
        return 0;
    if (*str == '-')
        ++str;
    if (*str == '\0')
        return 0;
    while(isdigit(*str) && *str != '\0')
        ++str;
    return (*str == '\0');
}


char *SPIT_getrangefield(char *dest,const char *optptr,int strip,int rangestop)
{
    const char *s;
    char *d;
    char lim=0;
    char delim=0;
    int  depth;
    int  more;
    int  stripdelim=0;

    s = optptr;
    d = dest;
    depth = 0;
    more = SPITYES;

    while (more)
    {
        if (*s)
        {
            if (!depth)
            {
                /* check for start of delimited section of field */

                if (*s==(char)'(' || *s==(char)'{' || *s==(char)'[' || *s==(char)'"' || *s==(char)'`' || *s==(char)'\'')
                {
                    lim = *s++;
                    delim = lim;
                    if (lim==(char)'(') delim = (char)')';
                    if (lim==(char)'{') delim = (char)'}';
                    if (lim==(char)'[') delim = (char)']';
                    ++depth;
                    if (d==dest && strip)
                    {
                        stripdelim = SPITYES;
                    }
                    else
                    {
                        *d++ = lim;
                        stripdelim = SPITNO;
                    }
                }

                /* check for white space delimiters */

                else
                {
                    if (((unsigned char)*s<=(unsigned char)' ' && *s!=HARDSPACE) || (rangestop && ((*s==(char)',') || (*s==(char)'.' && *(s+1)==(char)'.') ||  (*s==(char)'+' && *(s+1)==(char)'='))))
                    {
                        more = SPITNO;
                    }
                    else
                    {
                        *d++ = *s++;
                    }
                }
            }
            else
            {
                if (*s==delim || *s==lim)
                {
                    if (*s==delim)
                    {
                        --depth;
                        if (!depth && stripdelim && strip)
                        {
                            ++s;
                            more = SPITNO;
                        }
                        else
                            *d++ = *s++;
                    }
                    else
                    {
                        ++depth;
                    }
                }
                else
                {
                    *d++ = *s++;
                }
            }
        }
        else
            more = SPITNO;

    }
    *d = '\0';
    return((char *)s);
}


char *SPIT_getrangeopt(char *optvalptr,const char *optptr, const char *optname, int optmin, int optmax)
{
    char *s;

    s = SPIT_getrangefield(optvalptr,optptr, 1,0);

#ifdef __SPIT_H
    if (optmax!=0x7fffffff)
    {
        if (!RANGE_check(optvalptr, optmin, optmax))
        {
            printf("getrangeopt - RANGE OPTION VALUE OUT OF RANGE.  %s.\nSPECIFIED %s. VALID RANGE IS %ld..%ld\n", optname, optvalptr, optmin, optmax);
            exit(10);
        }
    }
#endif

    return(s);
}


char *SPIT_getrangeoptnostrip(char *optvalptr,const char *optptr, const char *optname, int optmin, int optmax)
{
    char *s;

    s = SPIT_getrangefield(optvalptr,optptr, 0,0);

#ifdef __SPIT_H
    if (optmax!=0x7fffffff)
    {
        if (!RANGE_check(optvalptr, optmin, optmax))
        {
            printf("getrangeopt - RANGE OPTION VALUE OUT OF RANGE.  %s.\nSPECIFIED %s. VALID RANGE IS %ld..%ld\n", optname, optvalptr, optmin, optmax);
            exit(10);
        }
    }
#endif

    return(s);
}



/* returns lit-wild   0=match, -n=less, +n=greater */
static int wildstrcmp(const char *lit, const char *wild)
{
    int r=0;
    while (!r)
    {
        if (*wild=='*')
        {
            ++wild;
            do
            {
                int w;
                w = wildstrcmp(lit,wild);      /* compare trailing strings */
                if (!r) r=w;
                if (!w)
                    return(0);
            } while (*lit++);                   /* keep trying */
            return(r);
        }
        else if (*wild=='?')
        {
            if (*lit)
                ++lit;
            else
                r = tolower(*lit)-tolower(*wild);
            ++wild;
        }
        else if (*wild=='#')
        {
            if (isdigit(*lit))
            {
                ++wild;                          /* skip '#' */
                ++lit;                          /* skip a character of dest */
            }
            else
                return(*lit-'0');
        }
        else
        {
            if (rangeisnumeric(wild) && rangeisnumeric(lit))
            {
                int litnum;
                int wildnum;
                litnum = atoi(lit);
                wildnum = atoi(wild);
                r = litnum-wildnum;
                if (r)
                    return(r);
            }

            r = tolower(*lit)-tolower(*wild);
            if (r || !*wild)
                return(r);

            ++wild;
            ++lit;
        }
    }
    return(r);
}


static int rangecompare(const RANGE *a,const RANGE *b)
{
    int r;
    if (a->name[0] && b->name[0])
        r = wildstrcmp(a->name,b->name);  /* a is lit, b is wildcard */
    else
    {
        if (a->num==b->num)  /* a-b but allow for signed diff of 32 bits */
            r = 0;
        else if (a->num>b->num)
            r = 1;
        else
            r = -1;
    }
    return(r);

}


/* Finds the .. in the range string */
static char *rangefinddots(const char *rangestr)
{
    const char *dots = NULL;
    while (dots == NULL && *rangestr != '\0')
    {
        if (*rangestr == '.' && *(rangestr+1) == '.')
            dots = rangestr;
        ++rangestr;
    }
    return((char *) dots);
}


/* Finds the += in the range string */
static char *rangefindplusequal(const char *rangestr)
{
    const char *plusequal = NULL;
    while (plusequal == NULL && *rangestr != '\0')
    {
        if (*rangestr == '+' && *(rangestr+1) == '=')
            plusequal = rangestr;
        ++rangestr;
    }
    return((char *)plusequal);
}


/* Finds the minimum range string */
static char *rangeminstr(char *dest, const char *rangestr)
{
    char *dots;
    char *plusequal;


    dots = rangefinddots(rangestr);
    plusequal = rangefindplusequal(rangestr);
    if (dots != NULL)
    {
        strncpy(dest, rangestr, dots - rangestr);
        dest[dots - rangestr] = '\0';
    }
    else
    {
        if (plusequal != NULL)
        {
            strncpy(dest, rangestr, plusequal - rangestr);
            dest[plusequal - rangestr] = '\0';
        }
        else
        {
            strcpy(dest, rangestr);
        }
    }

    return dest;
}


/* Finds the maximum range string */
static char *rangemaxstr(char *dest, const char *rangestr)
{
    char *dots;
    char *plusequal;

    dots = rangefinddots(rangestr);
    plusequal = rangefindplusequal(rangestr);
    if (dots == NULL && plusequal == NULL)
        strcpy(dest, rangestr);
    else if (dots != NULL && plusequal == NULL)
        strcpy(dest, dots+2);
    else if (dots == NULL && plusequal != NULL)
        *dest = '\0';
    else if (dots != NULL && plusequal != NULL)
    {
        if (dots == plusequal-2)
        {
            *dest = '\0';
        }
        else
        {
            strncpy(dest, dots + 2, plusequal - dots - 2);
            dest[plusequal - dots - 2] = '\0';
        }
    }

    return dest;
}


/* Finds step range string */
static char *rangestepstr(char *dest, const char *rangestr)
{
    char *plusequal;

    plusequal = rangefindplusequal(rangestr);
    if (plusequal == NULL)
    {
        *dest = '\0';
    }
    else
    {
        strcpy(dest, plusequal + 2);
    }
    return dest;
}


static void rangeswap(RANGE *r1, RANGE *r2)
{
    RANGE temp;
    temp.num = r1->num;
    strcpy(temp.name, r1->name);
    r1->num = r2->num;
    strcpy(r1->name, r2->name);
    r2->num = temp.num;
    strcpy(r2->name, temp.name);
    return;
}


/*  !min..max+=step,.. */
/* return 1 if name or num is in range */
/* countret -SPITBIGNUM - dont know, n - numbers in range */
/* Ranges of "" or "..+=" are considered equivalent and return 1, with min = -oo, max = oo, and count = oo */
/* Ranges of ..x set min to lowerbound if not = -oo and 0 otherwise */
static int rangeparse(const char *rangestr, const char *name, int num, int *error, int *count, int *absmin, int *absmax, int *constraint, int lowerbound, int upperbound)
{
    const char *ptr1 = rangestr;
    const char *ptr2 = rangestr;
    char rangepiece[1024];
    char min[1024];
    char max[1024];
    char step[1024];
    RANGE begin;
    RANGE end;
    RANGE check;
    int ret = 0;
    int i_step = 0;
    int i_count = 0;
    int abs_max = -SPITBIGNUM;
    int abs_min = SPITBIGNUM;
    int negative_range = 0;
    int i_error = 0;
    int success = 0;
    int rule = 0;           /* greater than 0 => roundup, less than 0 => rounddown, else pick closest */
    int i_constraint;

    /* If a constraint is being checked then assign a constraint rule (as int as its not 0) */
    if (constraint)
        rule = *constraint;

    if (num >= 0)
        i_constraint = SPITBIGNUM;
    else
        i_constraint = -SPITBIGNUM;

    check.num = num;
    *check.name = '\0';
    if (name)
        strcpy(check.name,name);

    if (*ptr1 == '\0')
    {
        abs_min = -SPITBIGNUM;
        abs_max = SPITBIGNUM;
        i_count = SPITBIGNUM;
        ret = 1;
    }

    while (*ptr1 != '\0')
    {
        if (*ptr2 == ',' || *ptr2 == '\0')
        {
            if ((*ptr1 == '!') && (ptr1 + 1 != ptr2))
            {
                strncpy(rangepiece, ptr1 + 1, ptr2-ptr1-1);
                rangepiece[ptr2-ptr1-1] = '\0';
                negative_range = 1;
            }
            else
            {
                strncpy(rangepiece, ptr1, ptr2-ptr1);
                rangepiece[ptr2-ptr1] = '\0';
            }

            if (*rangepiece != '\0')
            {
                /* Reset begin and end */
                *begin.name = *end.name = '\0';
                begin.num = end.num = 0;

                /* Get min, max and step as strings */
                rangeminstr(min, rangepiece);
                rangemaxstr(max, rangepiece);
                rangestepstr(step, rangepiece);

                /* Get min and absolute min values */
                if (*min == '\0')
                {
                    /* Assume that in the normal case, bounds start from 0 */
                    if (lowerbound != -SPITBIGNUM)
                        begin.num = lowerbound;
                    else
                        begin.num = 0;
                }
                else if (rangeisnumeric(min))
                {
                    begin.num = atoi(min);
                }
                else
                {   /* strings cause absmin and absmax to be infinite */
                    strcpy(begin.name, min);
                    abs_min = -SPITBIGNUM;
                    abs_max = SPITBIGNUM;
                }

                /* Get max and absolute max values */
                if (*max == '\0')
                {
                    end.num = upperbound;
                }
                else if (rangeisnumeric(max))
                {
                    end.num = atoi(max);
                }
                else
                {   /* strings cause absmin and absmax to be infinite */
                    strcpy(end.name, max);
                    abs_min = -SPITBIGNUM;
                    abs_max = SPITBIGNUM;
                }

                /* Get step value */
                if (rangeisnumeric(step))
                    i_step = atoi(step);
                else
                    i_step = 1;

               /* Look for special case of max, min, and step all "" */
                if (*max == '\0' && *min == '\0' && *step == '\0')
                {
                    abs_min = -SPITBIGNUM;
                    abs_max = SPITBIGNUM;
                    i_count = SPITBIGNUM;
                    ret = 1;
                }

                /* If max or min are strings and not equal then unknown count */
                if (*begin.name != '\0' || *end.name != '\0')
                {
                    if (strcmp(begin.name, end.name))
                        i_count = SPITBIGNUM;
                }

                /* if one string and one number then unknown max, min, and count */
                if ((*begin.name != '\0' && *end.name == '\0' && *max != '\0') ||
                    (*min != '\0' && *begin.name == '\0' && *end.name != '\0'))
                {
                    ++i_error;
                    i_count = SPITBIGNUM;
                    abs_min = -SPITBIGNUM;
                    abs_max = SPITBIGNUM;
                }

                if (end.num < begin.num)
                    rangeswap(&begin, &end);

                /* If it is not a negative range... */
                if (!negative_range && !i_error)
                {
                    /* If you've seen a string or an unbounded below range with i_step > 1 then make count infinite */
                    if (i_count != SPITBIGNUM && count)
                    {
                        i_count += (end.num - begin.num) / i_step + 1;
                    }

                    /* Update absolute maximum and absolute minimum */
                    if (i_count != SPITBIGNUM)
                    {
                        int last;
                        if (end.num == begin.num)
                            last = end.num;
                        else
                            last = end.num - ((end.num - begin.num) % i_step);
                        if (abs_max < last)
                            abs_max = last;
                        if (abs_min > begin.num)
                            abs_min = begin.num;
                    }
                }
                /* If it is a negative range... */
                else if (negative_range && !i_error)
                {
                    /* Update count of items */
                    if (i_count != SPITBIGNUM && count)
                    {
                        i_count += (upperbound - lowerbound + 1) - (((end.num - begin.num ) / i_step) + 1);
                    }

                    /* Update absolute maximum value */
                    if (i_count != SPITBIGNUM && absmax)
                    {
                        if (end.num != upperbound)
                        {
                            if (abs_max < upperbound)
                                abs_max = upperbound;
                        }
                        else if (i_step != 1)
                        {
                            if ((((end.num - begin.num) / i_step) * (end.num - begin.num)) != end.num)
                            {
                                if (abs_max < end.num)
                                    abs_max = end.num;
                            }
                            else
                            {
                                if (abs_max < end.num - 1)
                                    abs_max = end.num - 1;
                            }
                        }
                        else if (begin.num != lowerbound)
                        {
                            if (abs_max < begin.num - 1)
                                abs_max = begin.num - 1;
                        }
                    }

                    /* Update absolute minimum value */
                    if (i_count != SPITBIGNUM && absmin)
                    {
                        if (begin.num != lowerbound)
                        {
                            if (abs_min > lowerbound)
                                abs_min = lowerbound;
                        }
                        else if (i_step != 1)
                        {
                            if (abs_min > begin.num + 1)
                                abs_min = begin.num + 1;
                        }
                        else if (end.num != upperbound)
                        {
                            if (abs_min > end.num + 1)
                                abs_min = end.num + 1;
                        }
                    }
                }

                /* Determine whether given value is within range */
                if (rangecompare(&check,&begin)>=0 && rangecompare(&check,&end)<=0 && !ret)
                {
                    if (*begin.name != '\0')
                        success = 1;
                    if (!ret)
                        success = !((num-begin.num)%i_step);
                }

                if (negative_range)
                    success = success^1;

                if (success == 1)
                    ret = 1;

                if (constraint)
                {
                    int tmp_constraint=0;
                    int roundup;
                    int rounddown;
                    if (!negative_range && !i_error)
                    {
                        if (num >= begin.num && num <= end.num && ret == 0)
                        {
                            rounddown = (((int)((num - begin.num) / i_step)) * i_step) + begin.num;
/*                            roundup = ((((int)((num - begin.num) \ i_step)) + 1) * i_step) + begin.num; */
                            roundup = rounddown + i_step;
                            if (roundup > end.num)
                                roundup -= i_step;
                            if (rule > 0)
                                tmp_constraint = roundup;
                            else if (rule < 0)
                                tmp_constraint = rounddown;
                            else
                            {
                                if (abs(roundup - num) > abs(rounddown - num))
                                    tmp_constraint = rounddown;
                                else
                                    tmp_constraint = roundup;
                            }
                        }
                        else if (num < begin.num)
                            tmp_constraint = begin.num;
                        else if (num > end.num)
                            tmp_constraint = (((int)((end.num - begin.num) / i_step)) * i_step) + begin.num;
                        else
                            tmp_constraint = num;
                    }
                    else if (negative_range && !i_error)
                    {
                        if (num >= begin.num && num <= end.num && ret == 0)
                        {
                            if (i_step == 1)
                            {
                                if (end.num != SPITBIGNUM)
                                    roundup = end.num + 1;
                                else
                                    roundup = SPITBIGNUM;
                                if (begin.num != -SPITBIGNUM)
                                    rounddown = begin.num - 1;
                                else
                                    rounddown = -SPITBIGNUM;
                            }
                            else
                            {
                                roundup = num + 1;
                                rounddown = num - 1;
                            }
                            if (rule > 0)
                                tmp_constraint = roundup;
                            else if (rule < 0)
                                tmp_constraint = rounddown;
                            else
                            {
                                if (rounddown == -SPITBIGNUM)
                                    tmp_constraint = roundup;
                                else if (roundup == SPITBIGNUM)
                                    tmp_constraint = rounddown;
                                else if (abs(roundup - num) > abs(rounddown - num))
                                    tmp_constraint = rounddown;
                                else
                                    tmp_constraint = roundup;
                            }
                        }
                        else
                            tmp_constraint = num;
                    }

                    if (i_constraint == SPITBIGNUM || i_constraint == -SPITBIGNUM)
                        i_constraint = tmp_constraint;
                    else if (abs(tmp_constraint - num) < abs(i_constraint - num))
                    {
                        if (rule > 0 && i_constraint < tmp_constraint)
                            i_constraint = tmp_constraint;
                        else if (rule < 0 && i_constraint > tmp_constraint)
                            i_constraint = tmp_constraint;
                    }
                }

                success = 0;
                negative_range = 0;
            }
            else
            {
                abs_min = -SPITBIGNUM;
                abs_max = SPITBIGNUM;
                i_count = SPITBIGNUM;
                ret = 1;
            }

            if (*ptr2 != '\0')
                ptr1 = ptr2 + 1;
            else
                ptr1 = ptr2;
        }

        if (*ptr2 != '\0')
            ++ptr2;
    }

    if (absmax) *absmax = abs_max;
    if (absmin) *absmin = abs_min;
    if (count) *count = i_count;
    if (constraint) *constraint = i_constraint;
    if (error) *error = i_error;
    if (i_error) ret = 0;

    return(ret);
}


/****************************************************************/
/*  Range API                                                   */
/****************************************************************/

/*!
************************************************************
 \brief bump the value in num to the nearest allowed value in the range.
  \param rangestr range to use in constraining num
  \param num value to constrain within the range
  \param option > 0 will round up < 0 will round down

 \return
   returns num constrained to the specified range
************************************************************/
int RANGE_constrain(const char *rangestr, int num, int option)
{
    int r = option;
    int min;
    int max;
    rangeparse(rangestr, (char *)0, num, (int *)0, (int *)0, &min, &max, &r, -SPITBIGNUM, SPITBIGNUM);
    return r;
}

/*!
************************************************************
 \brief check if a given value is within the specified range
   \param rangestr range to constrain num to
   \param name name to check if it is in rangestr
   \param num number to check if it is in rangestr

 <b> NB: use one of name or num, do not use both </b>

 \return returns non zero if name or num is within the
         requested range.
************************************************************/
int RANGE_match(const char *rangestr, char *name, int num)
{
    return (rangeparse(rangestr, name, num, (int *)0, (int *)0, (int *)0, (int *)0, (int *)0, -SPITBIGNUM, SPITBIGNUM));
}

/*!
************************************************************
 \brief returns the minimum value requested by the rangestr
   \param rangestr range to extract the minimum value from
   \param minrange absolute minimum value of the range to clamp to
 \return
   returns the greater of minrange or the minimum value in rangestr.
\sa 
   RANGE_max()
************************************************************/
int RANGE_min(const char *rangestr, int minrange)
{
    int r;

    rangeparse(rangestr, (char *)0, 0, (int *)0, (int *)0, &r, (int *)0, (int *)0, minrange, SPITBIGNUM);
    r = gmax(r,minrange);
    return(r);
}

/*!
************************************************************
 \brief returns the maximum value requested by the rangestr
   \param rangestr range to extract the maximum value from
   \param maxrange absolute maximum value of the range to clamp to
 \return
   returns the lesser of maxrange or the maximum value in rangestr.
\sa 
   RANGE_min()
************************************************************/
int RANGE_max(const char *rangestr, int maxrange)
{
    int r;

    rangeparse(rangestr, (char *)0, 0, (int *)0, (int *)0, (int *)0, &r, (int *)0, -SPITBIGNUM, maxrange);
    r = gmin(r,maxrange);
    return(r);
}

/*!
************************************************************
  \brief verify that a range is within the required min and max values and
         that the range is properly specified.
    \param rangestr range to be verified
	\param minrange minimum possible value in the range
	\param maxrange maximum possible value in the range
  \return
    returns non zero if the range is not out of bounds and the
	range is properly specified.
************************************************************/
int RANGE_check(const char *rangestr, int minrange, int maxrange)
{
    int r;

    rangeparse(rangestr, (char *)0, 0, &r, (int *)0, (int *)0, (int *)0, (int *)0, minrange, maxrange);
    return(!r);
}

/*!
************************************************************
 \brief returns an upper bound on the number of entries
        requested by the reangestr.

   \param rangestr range to extract the count from
   \param minrange absolute minimum value of the range to clamp to
   \param maxrange absolute maximum value of the range to clamp to

 \return
   returns an upper bound for the number of entries possible
   in rangestr. The function cannot be more accurate than
   this due to text range support.
\sa 
   RANGE_min()
   RANGE_max()
************************************************************/
int RANGE_count(const char *rangestr, int minrange, int maxrange)
{
/*    int i; */
    int count=0;

    rangeparse(rangestr, (char *)0, 0, (int *)0, &count, (int *)0, (int *)0, (int *)0, minrange, maxrange);
    count = gmin(count, abs(maxrange - minrange + 1));
/*    if (!count) */
/*    { */
/*        for (i=minrange; i<=maxrange; ++i) */
/*            if (RANGE_match(rangestr, (char *)0, i)) */
/*                ++count; */
/*    } */
    return(count);
}


#endif
