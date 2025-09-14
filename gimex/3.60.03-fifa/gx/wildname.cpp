// (c) 1996-2004 Electronic Arts Inc.

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "wildname.h"

char *  SPIT_getfield( char *dest,const char *optptr, int strip, int list);
char *  SPIT_nextfield(const char *optptr);

#if defined(_MSC_VER)
#pragma warning(disable: 4996) // warning C4996: 'fscanf' was declared deprecated
#endif

int wildname(char *dest, char *frameformat, EXPR *symbols, char *WILDname, int WILDindex)
{
    char *s=frameformat;
    int scanstate=0;
    int scantype=0;
    char nextformat[512];
    char nextstr[512];
    int n;
    int ok=1;

    dest[0] = 0;
    while (*s)
    {
        switch (*s)
        {
            case '%':
                scanstate ^= 1;   /* % to enter, or %% is just a lit */
                break;

            case ' ':   /* space for sign */
            case '#':   /* octal and hex prefix */
            case '*':   /* use argment for field width */
            case '-':   /* left adjust */
            case '+':   /* print + for positive numbers */
            case '.':   /* field precision */
            case '0':   /* zero pad */
            case '1': case '2': case '3': case '4':     /* field width */
            case '5': case '6': case '7': case '8': case '9':
            case 'h':   /* short int arg */
            case 'l':   /* int int arg */
            case 'w':   /* wide string */
            case 'F':   /* far */
            case 'N':   /* near */
            case 'L':   /* int float */
                break;  /* modifiers dont change state */

            case 's':   /* string arg */
                if (scanstate)
                {
                    scantype = 1;
                }

            case 'n':
            case 'e':   /* exp */
            case 'E':   /* float */
            case 'g':   /* float */
            case 'G':   /* float */
            case 'f':   /* float point */
            case 'c':   /* character */
            case 'D':   /* int decimal arg */
            case 'd':   /* decimal integer */
            case 'i':   /* integer */
            case 'O':   /* int octal arg */
            case 'o':   /* octal arg */
            case 'b':   /* binary arg */
            case 'p':   /* pointer arg */
            case 'P':   /* pointer arg */
            case 'U':   /* int unsigned int */
            case 'u':   /* unsigned int */
            case 'X':   /* upper case hex arg */
            case 'x':   /* hex arg */
                if (scanstate)
                {
                    n = s-frameformat+1;
                    memcpy(nextformat,frameformat,n);
                    nextformat[n] = 0;
                    if (scantype && WILDname)
                        sprintf(nextstr,nextformat,WILDname);
                    else
                        sprintf(nextstr,nextformat,WILDindex);

                    strcat(dest,nextstr);
                    frameformat = s+1;
                    scantype = 0;
                }

            default:
                scanstate = 0;
                break;  /* formatters determine type and return to 0 state */

        }
        ++s;
    }
    strcat(dest,frameformat);
    return(ok);
}

