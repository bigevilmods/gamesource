// (c) Electronic Arts. All Rights Reserved.

/*-------------------------------------------------------------------------*/
/*                                                                         */
/*    WildMatch Utility for Wildcard Matching in Strings and Filenames     */
/*                                                                         */
/*                  by Steve Kilthau (coop)                                */
/*                                                                         */
/*                  Main Code - Jan 09, 1998                               */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*                                                                         */
/* Version Date   SE History:                                              */
/* ------- ----   -- --------                                              */
/*    1.00 980209 SK Matches wild cards                                    */
/*    1.01 980303 SK Has partial-string matching and case-insensitivity    */
/*    1.02 980303 SK Added wild*c* functions to intelligently match case   */
/*    1.03 010930 FB Disable # selectively                                 */
/*    1.04 011025 ID fixed bob forwardslash+* on windows platform matching */
/*                                                                         */
/* To do:                                                                  */
/*   Fix multiple adjacent wildcards.  Right now *#* doesn't work properly */
/*                                                                         */
/*-------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------*/
/*                         WildCard API                                    */
/*-------------------------------------------------------------------------*/
/*  int WILD_matchpath(dest, src, wildin, wildout);                        */
/*      - return 1 if src matches wildin, 0 otherwise                      */
/*      - use wildout, to create new filename from src, iff it matched     */
/*                                                                         */
/*  int WILD_match(dest, src, wildin, wildout);                            */
/*      - return 1 if src matches wildin, 0 otherwise                      */
/*      - use wildout to create new string from src, iff it matched        */
/*                                                                         */
/*-------------------------------------------------------------------------*/


#include <stdio.h>
#include <string.h>
#include "wildmatch.h"
#include "range.h"
#include "expr.h"
#include "spit.h"

/* Define NSPIT if not being used in a SPIT tool. */
/* Otherwise errors are controlled by value of OPTverbose. */
#if defined(NSPIT)
int OPTverbose = 0;
#else
extern int OPTverbose;
#endif

int WILDcharstar='*';
int WILDcharquestion='?';
int WILDcharnumber='#';
int WILDcharexclamation='!';
int WILDcharleftbracket='[';
int WILDcharrightbracket=']';
int WILDtoupper=0; /* count of # of wildcards to make upper case */

static int matchstring(char *dest, const char *src, const char *wildin, char *wildout, int *index, int casesensitive, int casematch);
static int matchpath(char *dest, const char *src, const char *wildin, char *wildout, int *index, int casesensitive, int casematch);
static void compressWildCard(char *newwild, const char *wildin);
static int isNumeric(const char *str);
static char **reallocateBuffer(char **oldbuf, const char *strin, int numelems);
static void freeBuffer(char **buffer, int numelems);
static char *getIndex(char *buffer, const char *str);
static int isValidIndex(char *buffer, char *index1, char *index2);
static int charsequal(char c1, char c2, int casesensitive);
static char *caseStrStr(const char *s1, const char *s2, int casesensitive);
static void matchCase(char **dest_1, const char **in_1, char **out_1, const char **src_1);
static void wild_itoa(int num, char *dest, int radix);

#define WILDMAXLINE    1024

static int current_unmatched = 0;

/* Exported globals */
char **OPTmathrules = NULL;
int nummathrules = 0;


/*-------------------------------------------------------------------------*/
/*-- Path matching functions ----------------------------------------------*/
/*-------------------------------------------------------------------------*/

int WILD_imatchpath(char *dest, const char *src, const char *wildin, char *wildout)
{
    return matchpath(dest, src, wildin, wildout, (int *)0, 0, 0);
}

/*-------------------------------------------------------------------------*/
/*-- String matching functions --------------------------------------------*/
/*-------------------------------------------------------------------------*/

int WILD_match(char *dest, const char *src, const char *wildin, char *wildout)
{
    return matchstring(dest, src, wildin, wildout, (int *)0, 1, 0);
}

int WILD_nmatch(char *dest, const char *src, const char *wildin, char *wildout, int *index)
{
    return matchstring(dest, src, wildin, wildout, index, 1, 0);
}

int WILD_imatch(char *dest, const char *src, const char *wildin, char *wildout)
{
    return matchstring(dest, src, wildin, wildout, (int *)0, 0, 0);
}

int WILD_nimatch(char *dest, const char *src, const char *wildin, char *wildout, int *index)
{
    return matchstring(dest, src, wildin, wildout, index, 0, 0);
}

int WILD_icmatch(char *dest, const char *src, const char *wildin, char *wildout)
{
    return matchstring(dest, src, wildin, wildout, (int *)0, 0, 1);
}

int WILD_nicmatch(char *dest, const char *src, const char *wildin, char *wildout, int *index)
{
    return matchstring(dest, src, wildin, wildout, index, 0, 1);
}


/*-------------------------------------------------------------------------*/


static int matchpath(char *dest, const char *src, const char *wildin, char *wildout, int *index, int casesensitive, int casematch)
{
    char win1[WILDMAXLINE];
    char win2[WILDMAXLINE];
    char wout1[WILDMAXLINE];
    char wout2[WILDMAXLINE];
    char src1[WILDMAXLINE];
    char src2[WILDMAXLINE];
    char result[WILDMAXLINE];
    const char *ptr;
/*
    strcpy(wildin,wildinsrc);
    for(i = 0; i < winl;i++)
    {
#if defined(MSDOS) || defined(_WIN32) || defined(__WATCOMC__) || defined(_MSC_VER)
        if(wildin[i] == '/')
            wildin[i] = '\\';
    #else
        if(wildin[i] == '\\')
            wildin[i] = '/';
    #endif
    }
*/

    *dest = '\0';
    ptr = wildin + strlen(wildin);
    while (*ptr != '\\' && *ptr != '/' && ptr != wildin)
        --ptr;
    if (ptr == wildin)
        return matchstring(dest, src, wildin, wildout, index, casesensitive, casematch);
    else
    {
        strncpy(win1, wildin, (int)(ptr - wildin + 1));
        win1[ptr-wildin+1] = '\0';
        strcpy(win2, ptr + 1);

        ptr = src + strlen(src);
        while (*ptr != '\\' && *ptr != '/' && ptr != src)
            --ptr;
        if (ptr == src)
        {
            *src1 = '\0';
            strcpy(src2, src);
        }
        else
        {
            strncpy(src1, src, (int)(ptr - src + 1));
            src1[ptr-src+1] = '\0';
            strcpy(src2, ptr + 1);
        }

        ptr = wildout + strlen(wildout);
        while (*ptr != '\\' && *ptr != '/' && ptr != wildout)
            --ptr;
        if (ptr == wildout)
        {
            *wout1 = '\0';
            strcpy(wout2, wildout);
        }
        else
        {
            strncpy(wout1, wildout, (int)(ptr - wildout + 1));
            wout1[ptr-wildout+1] = '\0';
            strcpy(wout2, ptr + 1);
        }

        if (!matchstring(result, src1, win1, wout1, index, casesensitive, casematch))
        {
            *dest = '\0';
            return 0;
        }
        else
        {
            strcpy(dest, result);
            if (!matchstring(result, src2, win2, wout2, index, casesensitive, casematch))
            {
                *dest = '\0';
                return 0;
            }
            strcat(dest, result);
        }
    }

    return 1;
}


static int matchstring(char *dest, const char *src, const char *wildin, char *wildout, int *index, int casesensitive, int casematch)
{
    char wildinbuf[WILDMAXLINE] = "";
    const char *src_1 = src;
    const char *src_2 = src;
    const char *in_1 = wildinbuf;
    const char *in_2 = NULL;
    char *out_1 = wildout;
    char *dest_1 = dest;
    int done = 0;
    char **stars = NULL;
    char **numbers = NULL;
    char **questions = NULL;
    char indexbuf[WILDMAXLINE] = "";
    int numstars = 0;
    int numnumbers = 0;
    int numquestions = 0;
    int wildtoupper = WILDtoupper;

    char index1[WILDMAXLINE];
    char index2[WILDMAXLINE];
    int curstar = 0;
    int curnumber = 0;
    int curquestion = 0;
    int ok = 1;
    int unmatchednumber = 0;

    /* Remove multiple adjacent *'s from input wildcards */
    compressWildCard(wildinbuf, wildin);

    /* Find first wildcard in wildin */
    while (*in_1 != WILDcharstar && *in_1 != WILDcharquestion && *in_1 != WILDcharnumber && *in_1 != '\0')
    {
        if (!charsequal(*in_1, *src_1, casesensitive))
            return 0;
        ++in_1;
        if (*src_1 == '\0')
            return 0;
        else
            ++src_1;
    }

    if (*in_1 != '\0')
        in_2 = in_1 + 1;
    else
        done = 1;

    /* Find rest of the wildcards generating a buffer full of (wildcard, wildcard string)'s */
    while (!done)
    {
        /* Find next wildcard in wildin */
        while (*in_2 != WILDcharstar && *in_2 != WILDcharquestion && *in_2 != WILDcharnumber && *in_2 != '\0')
            ++in_2;
        if (*in_1 == WILDcharquestion)
        {
            char buf[2];
            strncpy(buf, src_1, 1);
            if (*buf != '\0')
                questions = reallocateBuffer(questions, buf, ++numquestions);
            else
                ok = 0;
            ++src_1;
        }
        else if (*in_1 == WILDcharstar)
        {
            char buf[WILDMAXLINE];
            strncpy(buf, in_1 + 1, in_2 - in_1 - 1);
            buf[in_2 - in_1 - 1] = '\0';
            src_2 = caseStrStr(src_1, buf, casesensitive);

            if (src_2)
            {
                /* Deal with adjacent wildcards */
                if (in_2 - in_1 == 1)
                {
                    if (*in_2 == '\0')
                    {
                        strcpy(buf, src_1);
                        src_1 += (int)(strlen(src_1));
                    }
                    else if (*in_2 == WILDcharnumber)
                    {
                        src_2 = src_1;
                        while (((*src_2 < 48) || (*src_2 > 57)) && (*src_2 != '\0'))
                            ++src_2;

                        strncpy(buf, src_1, src_2 - src_1);
                        buf[src_2 - src_1] = '\0';
                        src_1 = src_2;
                    }
                    /* Doesn't yet handle case of *????#* just *??????ddf... */
                    else if (*in_2 == WILDcharquestion)
                    {
                        const char *ptr = in_2 + 1;
                        int num = 1;
                        while (*ptr == WILDcharquestion)
                        {
                            ++ptr;
                            ++num;
                        }
                        while ((*ptr != '\0') && (*ptr != WILDcharstar) && (*ptr != WILDcharnumber) && (*ptr != WILDcharquestion))
                            ++ptr;
                        strncpy(buf, in_2 + num, ptr - in_2 - num);
                        buf[ptr - in_2 - num] = '\0';
                        if ((int)strlen(src_1) >= num)
                        {
                            src_2 = caseStrStr(src_1 + num, buf, casesensitive);
                            if (src_2)
                            {
                                src_2 -= num;
                                if (src_2 != src_1)
                                {
                                    strncpy(buf, src_1, src_2 - src_1);
                                    buf[src_2 - src_1] = '\0';
                                    if (src_2 != src)
                                        src_1 = src_2;
                                }
                                else
                                {
                                    int tmp = strlen(src_1) - num;
                                    strncpy(buf, src_1, tmp);
                                    buf[tmp] = '\0';
                                    src_1 += tmp;
                                }
                            }
                            else
                                ok = 0;
                        }
                        else
                            ok = 0;
                    }
                }
                else
                {
                    strncpy(buf, src_1, src_2 - src_1);
                    buf[src_2 - src_1] = '\0';
                    if (src_2 != src)
                        src_1 = src_2;
                }
                stars = reallocateBuffer(stars, buf, ++numstars);
            }
            else
                ok = 0;
        }
        else if (*in_1 == WILDcharnumber)
        {
            char buf[WILDMAXLINE];
            strncpy(buf, in_1 + 1, in_2 - in_1 - 1);
            buf[in_2 - in_1 - 1] = '\0';
            src_2 = caseStrStr(src_1, buf, casesensitive);

            if (src_2)
            {
                if ((in_2 - in_1 == 1))
                {
                    if (*in_2 == '\0')
                    {
                        strcpy(buf, src_1);
                        src_1 += (int)(strlen(src_1));
                    }
                    else if (*in_2 == WILDcharstar || *in_2 == WILDcharquestion)
                    {
                        int i = 0;
                        while (*src_1 > 47 && *src_1 < 58)
                        {
                            buf[i] = *src_1;
                            ++src_1;
                            ++i;
                        }
                        buf[i] = '\0';
                    }
                }
                else
                {
                    strncpy(buf, src_1, src_2 - src_1);
                    buf[src_2 - src_1] = '\0';
                    if (src_2 != src)
                        src_1 = src_2;
                }
                if (isNumeric(buf))
                    numbers = reallocateBuffer(numbers, buf, ++numnumbers);
                else
                    ok = 0;
            }
            else
                ok = 0;
        }

        do
        {
            ++in_1;
            if ((!charsequal(*in_1, *src_1, casesensitive)) && (in_1 != in_2))
                ok = 0;
            if (in_1 != in_2)
            {
                if (*src_1 == '\0')
                    ok = 0;
                else
                   ++src_1;
            }
        }while (in_1 < in_2);

        if (*in_1 != '\0' && ok)
            in_2 = in_1 + 1;
        else
            done = 1;
    }

    if (*src_1 != '\0' && !index)
        ok = 0;

    if (index)
    {
        int i = 0;
        while(&src[i] != src_1)
            ++i;
        *index = i;
    }

    src_1 = src;
    in_1 = wildinbuf;

    /* Construct destination filename according to wildcard specifications */
    while (*out_1 != '\0' && ok)
    {
        if (*out_1 == WILDcharstar)
        {
            out_1 = getIndex(indexbuf, out_1 + 1);
            if (isValidIndex(indexbuf, index1, index2))
            {
                int starnum;
                int first, last;
                int i;

                if (*index1 == '\0')
                    starnum = curstar++;
                else
                    starnum = atoi(index1);
                if (starnum < 0 || starnum >= numstars)
                {
                    if (OPTverbose>6)
                        printf("WILD_match - Unmatched wildcard *[%i]\n - replacing with \"\"", starnum);
                }
                else
                {
                    first = RANGE_min(index2, 0);
                    if (strlen(stars[starnum]) == 0)
                        last = 0;
                    else
                        last = min(RANGE_max(index2, 32768), (int)strlen(stars[starnum]) - 1);
                    for (i = first; i <= last; ++i)
                    {
                        char emptyString[] = "";
                        if (RANGE_match(index2, emptyString, i) && *stars[starnum] != '\0')
                        {
                            if (wildtoupper)
                            {
                                *dest_1 = (char) toupper(stars[starnum][i]);
                            }
                            else
                                *dest_1 = stars[starnum][i];
                            ++dest_1;
                        }
                    }
                    if (wildtoupper)
                        --wildtoupper;

                    if (casematch)
                        src_1 += strlen(stars[starnum]);
                }
            }
            else
            {
                if (OPTverbose)
                    printf("WILD_match - invalid index format: %s\n", indexbuf);
                *dest_1 = '\0';
                ok = 0;
            }
        }
        else if (*out_1 == WILDcharquestion)
        {
            out_1 = getIndex(indexbuf, out_1 + 1);
            if (isValidIndex(indexbuf, index1, index2))
            {
                int questionnum;
                int minfieldwidth = 0;
                int i;

                if (*index1 == '\0')
                    questionnum = curquestion++;
                else
                    questionnum = atoi(index1);
                if (questionnum < 0 || questionnum >= numquestions)
                {
                    if (OPTverbose>6)
                        printf("WILD_match - Unmatched wildcard ?[%i]\n - replacing with ' '", questionnum);
                    *dest_1 = ' ';
                    ++dest_1;
                }
                else
                {
                    minfieldwidth = RANGE_min(index2, 0);
                    for (i = 1; i < minfieldwidth; ++i)
                    {
                      *dest_1 = ' ';
                      ++dest_1;
                    }
                    if (wildtoupper)
                    {
                        *dest_1 = (char) toupper(questions[questionnum][0]);
                    }
                    else
                        *dest_1 = questions[questionnum][0];
                    ++dest_1;
                    if (casematch)
                        ++src_1;
                    if (wildtoupper)
                        --wildtoupper;

                }
            }
            else
            {
                if (OPTverbose)
                    printf("WILD_match - invalid index format: %s\n", indexbuf);
                *dest_1 = '\0';
                ok = 0;
            }
        }
        else if (*out_1 == WILDcharnumber)
        {
            int numbernum;
            int minfieldwidth = 0;
            int minimum;
            int maximum;
            int i = 0;
            int num;

            while (*out_1 == WILDcharnumber)
            {
                ++minfieldwidth;
                ++out_1;
            }
            out_1 = getIndex(indexbuf, out_1);
            if (isValidIndex(indexbuf, index1, index2))
            {
                if (*index1 == '\0')
                    numbernum = curnumber++;
                else
                    numbernum = atoi(index1);
                if (numbernum < 0 || numbernum >= numnumbers)
                {
                    char buffer[40];
                    if (OPTverbose>6)
                        printf("WILD_match - Unmatched wildcard #[%i]\n - replacing with %i", numbernum, current_unmatched);
                    wild_itoa(current_unmatched, buffer, 10);
                    strcpy(dest_1, buffer);
                    dest_1 += strlen(buffer);
                    ++unmatchednumber;
                }
                else
                {
                    if (*index2 == '\0')
                    {
                        if (OPTmathrules)
                        {
                            char cresult[40];
                            num = WILD_applymathrules(numbers, numbernum, numnumbers);
                            if (num != -32768)
                            {
                                wild_itoa(num, cresult, 10);
                                minfieldwidth -= strlen(cresult);
                                for (i = 0; i < minfieldwidth; ++i)
                                {
                                    *dest_1 = '0';
                                    ++dest_1;
                                }
                                strcpy(dest_1, cresult);
                                dest_1 += strlen(cresult);
                            }
                            else
                            {
                                minfieldwidth -= strlen(numbers[numbernum]);
                                for (i = 0; i < minfieldwidth; ++i)
                                {
                                    *dest_1 = '0';
                                    ++dest_1;
                                }
                                strcpy(dest_1, numbers[numbernum]);
                                dest_1 += strlen(numbers[numbernum]);
                            }
                        }
                        else
                        {
                            minfieldwidth -= strlen(numbers[numbernum]);
                            for (i = 0; i < minfieldwidth; ++i)
                            {
                                *dest_1 = '0';
                                ++dest_1;
                            }
                            strcpy(dest_1, numbers[numbernum]);
                            dest_1 += strlen(numbers[numbernum]);
                        }
                    }
                    else
                    {
                        minimum = RANGE_min(index2, 0);
                        maximum = RANGE_max(index2, 32768);
                        if (OPTmathrules)
                            num = WILD_applymathrules(numbers, numbernum, numnumbers);
                        else
                            num = atoi(numbers[numbernum]);
                        
                        char emptyString[] = "";
                        if (RANGE_match(index2, emptyString, num))
                        {
                            for (i = 0; i < minfieldwidth; ++i)
                            {
                                *dest_1 = '0';
                                ++dest_1;
                            }
                            strcpy(dest_1, numbers[numbernum]);
                            dest_1 += strlen(numbers[numbernum]);
                        }
                        else
                        {
                            for (i = 0; i < minfieldwidth + (int)strlen(numbers[numbernum]); ++i)
                            {
                                *dest_1 = '0';
                                ++dest_1;
                            }
                        }
                    }
                    if (casematch)
                        src_1 += (strlen(numbers[numbernum]) + minfieldwidth);
                }
            }
            else
            {
                if (OPTverbose)
                    printf("WILD_match - invalid index format: %s\n", indexbuf);
                *dest_1 = '\0';
                ok = 0;
            }
            if (*out_1 == ',' && *(out_1+1)==WILDcharnumber)
                ++out_1;
        }
/*        else if (*out_1 == ',') */
/*        { */
/*            if (*(out_1-1) == WILDcharnumber && *(out_1+1) == WILDcharnumber) */
/*            { */
/*                ++out_1; */
/*            } */
/*            else */
/*            { */
/*                *dest_1 = *(out_1-1); */
/*                ++dest_1; */
/*            } */
/*        } */
/* This piece of code will not work for % becuase %%dog => %dog and gx will confuse %dog */
/* as meaning #og.  Doh!! */
/*        else if (*out_1 == '%') */
/*        { */
/*            char t = *(out_1 + 1); */
/*            ++out_1; */
/*            if (t == WILDcharstar || t == WILDcharnumber || t == WILDcharquestion || t == ',' || t == '%') */
/*            { */
/*                *dest_1 = t; */
/*                ++dest_1; */
/*                ++out_1; */
/*            } */
/*        } */
        else
        {
            if (casematch)
                matchCase(&dest_1, &in_1, &out_1, &src_1);
            else
            {
                while (*out_1 != WILDcharstar && *out_1 != WILDcharnumber && *out_1 != WILDcharquestion && *out_1 != '\0')
                {
//                    if(*out_1 == '\\' || *out_1 == '/')
//                  {
//#if defined(MSDOS) || defined(_WIN32) || defined(__WATCOMC__) || defined(_MSC_VER)
//                      *dest_1 = '\\';
//#else
//                      *dest_1 = '/';
//#endif
//                  }
//                  else
                        *dest_1 = *out_1;
                    ++dest_1;
                    ++out_1;
                }
            }
        }
    }

    freeBuffer(stars, numstars);
    freeBuffer(questions, numquestions);
    freeBuffer(numbers, numnumbers);

    *dest_1 = '\0';

    if (unmatchednumber)
        ++current_unmatched;

    return ok;
}


/* Converts case on a resultant string to match the case of the initial string */
static void matchCase(char **dest_1, const char **in_1, char **out_1, const char **src_1)
{
    int caserule = 2;       /* 2 = indetermined, 1 = upper, 0 = lower */

    while (**in_1 == WILDcharstar || **in_1 == WILDcharnumber || **in_1 == WILDcharquestion)
        ++*in_1;
    while (**out_1 == WILDcharstar || **out_1 == WILDcharnumber || **out_1 == WILDcharquestion)
        ++*out_1;

    while (**in_1 != WILDcharstar && **in_1 != WILDcharnumber && **in_1 != WILDcharquestion && **in_1 != '\0')
    {
        if (**src_1 >= 'A' && **src_1 <= 'Z')
            caserule = 1;
        else if (**src_1 >= 'a' && **src_1 <= 'z')
            caserule = 0;

        if (**out_1 != WILDcharstar && **out_1 != WILDcharquestion && **out_1 != WILDcharnumber && **out_1 != '\0')
        {
            **dest_1 = **out_1;
            if (caserule == 1)
                **dest_1 = (char)toupper(**dest_1);
            else if (caserule == 0)
                **dest_1 = (char)tolower(**dest_1);
            ++*dest_1;
            ++*out_1;
        }
        ++*src_1;
        ++*in_1;
    }

    while (**out_1 != WILDcharstar && **out_1 != WILDcharnumber && **out_1 != WILDcharquestion && **out_1 != '\0')
    {
        **dest_1 = **out_1;
        if (caserule == 1)
            **dest_1 = (char)toupper(**dest_1);
        else if (caserule == 0)
            **dest_1 = (char)tolower(**dest_1);
        ++*dest_1;
        ++*out_1;
    }

    return;
}


/* Determines whether a given character string is a numeric representation */
static int isNumeric(const char *str)
{
    const char *tmp = str;
    int isnumber = 1;
    if (*tmp == '\0')
        isnumber = 0;
    while (*tmp != '\0')
    {
        if (*tmp < 48 || *tmp > 57)
            isnumber = 0;
        ++tmp;
    }
    return isnumber;
}


/* Removes multiple adjacent *'s and #'s from wildin, writing to newwild */
static void compressWildCard(char *newwild, const char *wildin)
{
    const char *w1 = wildin;
    char *w2 = newwild;

    while(*w1 != '\0')
    {
        *w2 = *w1;
        if (*w1 == WILDcharstar)
            while (*w1 == WILDcharstar)
                ++w1;
        else if (*w1 == WILDcharnumber)
            while (*w1 == WILDcharnumber)
                ++w1;
        else
            ++w1;
        ++w2;
    }
    *w2 = '\0';

    return;
}


/* Reallocates an array of strings, adding strin to the last array index */
static char **reallocateBuffer(char **oldbuf, const char *strin, int numelems)
{
    char **buf1;

    if (numelems != 1)
    {
        buf1 = (char **) malloc(numelems * sizeof(char *));
        memcpy(buf1, oldbuf, (numelems - 1) * sizeof(char *));
        free(oldbuf);
        buf1[numelems - 1] = (char *) malloc((strlen(strin) + 1));
        strcpy(buf1[numelems - 1], strin);
    }
    else
    {
        buf1 = (char **) malloc(sizeof(char *));
        buf1[numelems - 1] = (char *) malloc((strlen(strin) + 1));
        strcpy(buf1[numelems - 1], strin);
    }

    return buf1;
}


/* Frees an array of strings */
static void freeBuffer(char **buffer, int numelems)
{
    int i;
    if (buffer != NULL)
    {
        for (i = 0; i < numelems; ++i)
            free(buffer[i]);
        free(buffer);
    }
    return;
}


/* Gets the index code associated with a * or a # */
static char *getIndex(char *buffer, const char *str)
{
    char *ptr = buffer;

    if (*str != WILDcharleftbracket)
    {
        *buffer = '\0';
        return((char *)str);
    }
    else
    {
        do{
            *ptr = *str;
            ++str;
            ++ptr;
            if (*str == '\0')
            {
                *buffer = '\0';
                return((char *)str);
            }
        }while (*str != WILDcharrightbracket && *str != WILDcharleftbracket);
        *ptr = *str;
        ++str;
        ++ptr;
        if (*str != WILDcharleftbracket)
        {
            *ptr = '\0';
            return((char *)str);
        }
        else
        {
            do{
                *ptr = *str;
                ++str;
                ++ptr;
                if (*str == '\0')
                {
                    *buffer = '\0';
                    return((char *)str);
                }
            }while (*str != WILDcharrightbracket);
            *ptr = *str;
            ++str;
            ++ptr;
        }
    }
    *ptr = '\0';

    return((char *)str);
}


/* Checks that the index code associated with a * or # is valid */
static int isValidIndex(char *buffer, char *index1, char *index2)
{
    char *ptr = index1;

    *index1 = '\0';
    *index2 = '\0';
    if (*buffer == '\0')
        return 1;
    else if (*buffer == WILDcharleftbracket)
    {
        ++buffer;
        while (*buffer != WILDcharrightbracket && *buffer != '\0')
        {
            *ptr = *buffer;
            ++ptr;
            ++buffer;
        }
        if (*buffer == '\0')
            return 0;
        *ptr = '\0';
        ++buffer;
        if (!isNumeric(index1) && *index1 != '\0')
            return 0;
        if (*buffer == '\0')
            return 1;
        if (*buffer == WILDcharleftbracket)
        {
            ptr = index2;
            ++buffer;
            while (*buffer != WILDcharrightbracket && *buffer != '\0')
            {
                *ptr = *buffer;
                ++ptr;
                ++buffer;
            }
            if (*buffer == '\0')
                return 0;
            *ptr = '\0';
            ++buffer;
            if (!RANGE_check(index2, 0, 32768) && *index2 != '\0')
                return 0;
            if (*buffer != '\0')
                return 0;
        }
        else
            return 0;
    }
    else
        return 0;

    return 1;
}


static int charsequal(char c1, char c2, int casesensitive)
{
    if (casesensitive)
    {
        if(c1 == '/' && c2 == '\\')
            return 1;
        return (c1 == c2);
    }
    else
    {
        c1 = (char)tolower(c1);
        c2 = (char)tolower(c2);
        if(c1 == '/' && c2 == '\\')
            return 1;
        return (c1 == c2);
    }
}


static char *caseStrStr(const char *s1, const char *s2, int casesensitive)
{
    if (casesensitive)
    {
        return (char *) strstr(s1, s2);
    }
    else
    {
        char t1[SPITMAXLINE];
        char t2[SPITMAXLINE];
        char *t;
        strcpy(t1, s1);
        strcpy(t2, s2);
        strlwr(t1);
        strlwr(t2);
        t = strstr(t1, t2);
        if (t != NULL)
            return (char *)s1 + (t - t1);
        else
            return NULL;
    }
}


static void wild_itoa(int num, char *dest, int radix)
{
    int n1 = (int)(num/radix);
    int n2 = num;
    if (radix == 0)
        return;
    if (num == 0)
    {
        *dest = '0';
        *(dest+1) = '\0';
        return;
    }
    n2 = (int)(n2/radix);
    while (n2 != 0)
    {
        n2 = (int)(n2/radix);
        ++dest;
    }
    *(dest+1) = '\0';
    while(num != 0)
    {
        n2 = num - (n1*radix);
        *dest = (char)(48+n2);
        num = (int)(num/radix);
        if (num != 0)
        {
            --dest;
            n1 = (int)(num/radix);
        }
    }
    return;
}


/* Allocates space for a math rule and copies it into the OPTmathrules variable */
void WILD_addmathrule(char *strin)
{
    char **buf1;

    ++nummathrules;
    if (nummathrules != 1)
    {
        buf1 = (char **) malloc(nummathrules * sizeof(char *));
        memcpy(buf1, OPTmathrules, (nummathrules - 1) * sizeof(char *));
        free(OPTmathrules);
        buf1[nummathrules - 1] = (char *) malloc((strlen(strin) + 1));
        strcpy(buf1[nummathrules - 1], strin);
    }
    else
    {
        buf1 = (char **) malloc(sizeof(char *));
        buf1[0] = (char *) malloc((strlen(strin) + 1));
        strcpy(buf1[0], strin);
    }

    OPTmathrules = buf1;
    return;
}


/* Frees the OPTmathrules variable */
void WILD_deletemathrules(void)
{
    int i;
    if (OPTmathrules != NULL)
    {
        for (i = 0; i < nummathrules; ++i)
            free(OPTmathrules[i]);
        free(OPTmathrules);
    }
    return;
}


int WILD_applymathrules(char **numbers, int curnum, int numnumbers)
{
    int i, j=0, k = 0;
    char inRule[40] = "";
    char outRule[40] = "";
    char cnum[40];
    int inum = -2;
    EXPR *expr;
    int l, sum = 0;
    int ok = 1;
    int usedrule = 0;

    for (i = 0; i < nummathrules; ++i)
    {
        strcpy(inRule, OPTmathrules[i]);
        if (inRule[0]==WILDcharnumber && inRule[1]==WILDcharleftbracket)
        {
            j = 2;
            while(inRule[j] != WILDcharrightbracket && inRule[j] != '\0')
            {
                cnum[j-2] = inRule[j];
                ++j;
            }
            if (inRule[j]=='\0')
                ok = 0;
            else
            {
                cnum[j-2] = '\0';
                inum = atoi(cnum);
                ++j;
            }
        }
        else if (inRule[0]==WILDcharnumber)
        {
            j = 1;
            inum = -1;
        }
        else
            ok = 0;
        if ((inum == -1 || inum == curnum) && ok)
        {
            if (inRule[j]=='=')
            {
                ++j;
            }
            else
                ok = 0;
            while (inRule[j] != '\0' && ok)
            {
                if (inRule[j] == WILDcharnumber)
                {
                    ++j;
                    if (inRule[j] == WILDcharleftbracket)
                    {
                        int n = 0;
                        ++j;
                        while(inRule[j] != WILDcharrightbracket && inRule[j] != '\0')
                            cnum[n++] = inRule[j++];
                        if (inRule[j]=='\0')
                            ok = 0;
                        else
                        {
                            ++j;
                            cnum[n] = '\0';
                            if (isNumeric(cnum) || cnum[0] == '\0')
                            {
                                if (atoi(cnum) < numnumbers && cnum[0] != '\0')
                                {
                                    strcpy(&outRule[k], numbers[atoi(cnum)]);
                                    k += strlen(numbers[atoi(cnum)]);
                                }
                                else
                                    ok = 0;
                            }
                            else
                                ok = 0;
                        }
                    }
                    else
                    {
                        strcpy(&outRule[k], numbers[curnum]);
                        k += strlen(numbers[curnum]);
                    }
                }
                else
                    outRule[k++] = inRule[j++];
            }

            if (ok)
            {
                outRule[k] = '\0';
                k = 0;

                expr = EXPR_open();
                l = (int) EXPR_get(expr,outRule);
                if (EXPR_error(expr))
                {
                    if (OPTverbose>2)
                    {
                        printf("Error in expression: %s\n", outRule);
                        return -32768;
                    }
                }
                else
                {
                    usedrule = 1;
                    sum += l;
                }
                EXPR_close(expr);
            }
        }
        if (!ok)
        {
            if (OPTverbose>2)
                printf("Invalid rule format %s\n", OPTmathrules[i]);
            return -32768;
        }
    }
    if (usedrule)
        return (int)(sum);
    else
        return -32768;
}

