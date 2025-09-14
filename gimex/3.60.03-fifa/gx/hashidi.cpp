// (c) 1993-2006 Criterion Software Limited and its licensors. All Rights Reserved

/*------------------------------------------------------------------*/
/*                                                                  */
/*                        Hashing Routines                          */
/*                                                                  */
/*                     by FrANK G. Barchard, EAC                    */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History:                                       */
/* ------- ----   -- --------                                       */
/*    1.00 93???? FB first designed for cel2shp                     */
/*    1.01 930331 GS stand alone module, dynamic allocation         */
/*    1.02 930408 FB remove spaces                                  */
/*    1.03 951127 GS removed warnings, longer MAXLINE (win95)       */
/*    1.04 020303 FB clean variables on restore                     */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/*                                                                  */
/* Converts multiple strings into unique strings.                   */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Example of use:                                                  */
/* --------------                                                   */
/*                                                                  */
/* HASHID_init(maxstrings, maxidlen, 1, 0);                         */
/* for (i = 0; i < maxstrings; ++i)                                 */
/* {                                                                */
/*     char oldname[maxidlen];                                      */
/*     HASHID_apply(newname, oldname);                              */
/*     printf("%s = %s\n", oldname, newname);                       */
/* }                                                                */
/* HASHID_restore();                                                */
/*                                                                  */
/*------------------------------------------------------------------*/


#include "stdgimex.h"
#include "hashidi.h"

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#define MAXLINE 265

static char* hashed;
static int hashcount;
static int hashmaxcount;
static int hashpad;
static int keepextension;
static size_t maxidlen;
static size_t maxnamelen;


static int HASHstricmp(char* s, char* d)
{
    while (*s && ((*s | ' ') == (*d | ' ')))
    {
        ++s;
        ++d;
    }
    if (*s < *d)
        return (-1);
    if (*s > *d)
        return (1);
    return (0);
}

static int isvowel(char u)
{
    int c;

    c = tolower(u);
    if (c == 'a')
        return (1);
    if (c == 'e')
        return (1);
    if (c == 'i')
        return (1);
    if (c == 'o')
        return (1);
    if (c == 'u')
        return (1);
    return (0);
}


static void remove1letter(char* left)
{
    size_t l;
    int i;
    char buf[80];

    l = strlen(left);

    /* if cel_ or msk_ remove it and append c or m */

    strcpy(buf, left);
    buf[4] = 0;
    if (!(HASHstricmp(buf, "cel_")) || !(HASHstricmp(buf, "msk_")))
    {
        strcpy(left, left + 4);
        left[3] = 0;
        buf[1] = 0;
        if (buf[0] == 'c')
            buf[0] = 'f';

        strcat(left, buf);                      /* append m (mask) or f (frame) */
        return;
    }

    /* if _ or ' ' remove last one */

    for (i = l - 1; i >= 0; --i)
    {
        if (left[i] == '_' || left[i] == ' ')
        {
            strcpy(left + i, left + i + 1);
            return;
        }
    }

    /* if any pairs, remove 1 of letters */

    for (i = l - 2; i >= 0; --i)
    {
        if (left[i] == left[i + 1])
        {
            strcpy(left + i, left + i + 1);
            return;
        }
    }

    /* if trailing 'e' remove it */

    if (left[l - 1] == 'e')
    {
        left[l - 1] = 0;
        return;
    }

    /* if inner vowel (2..l-1), remove last 1 */

    for (i = l - 1; i > 1; --i)
    {
        if (isvowel(left[i]))
        {
            strcpy(left + i, left + i + 1);
            return;
        }
    }

    /* if len maxidlen+1 and trailing vowel, remove the vowel */

    if (l == (maxidlen+1) && isvowel(left[l - 1]))
    {
        left[l - 1] = 0;
        return;
    }

    /* if inner vowel (1..l-1), remove last 1 */
    /* unless its 'ea' at start of string */

    for (i = l - 1; i > 0; --i)
    {
        if (isvowel(left[i]) && !(i == 1 && tolower(left[0]) == 'e' && tolower(left[1]) == 'a'))
        {
            strcpy(left + i, left + i + 1);
            return;
        }
    }

    /* remove last character */

    left[l - 1] = 0;
    return;
}

void HASHID_init(size_t maxnames, size_t idlen, int pad, int keepext)
{
    hashcount = 0;
    hashmaxcount = maxnames;
    maxidlen = idlen;
    maxnamelen = idlen+(keepext?4:0);
    hashed = (char *) galloc(maxnames * maxnamelen);
    hashpad = pad;
    keepextension = keepext;
}


void HASHID_restore()
{
    if (hashed)
    {
        gfree(hashed);
        hashed = NULL;
    }
}

/* make a shape name from a filename */

void HASHID_apply(char *shpname, char *pathname)
{
    char left[MAXLINE];
    char right[MAXLINE];
    char ext[MAXLINE];
    int i;
    int j;
    int done;
    int collisionnum;
    char *filename;

    filename = 0;

    for (i=0; pathname[i]; ++i)
    {
        if (pathname[i]==(char)'/' || pathname[i]==(char)'\\' || pathname[i]==(char)':')
            filename = pathname+i+1;
    }
    if (!filename)
        filename = pathname;

    done = 0;
    collisionnum = 0;
    ext[0] = 0;
    if (keepextension)
        if (strrchr(filename,'.'))
            strcpy(ext,(strrchr(filename,'.')));

    while (!done)
    {
        /* split at numerical suffix or extension */
        for (i = 0; filename[i]; ++i)
        {
            if (isdigit(filename[i]) || filename[i] == '.')
                break;
            left[i] = filename[i];
        }
        left[i] = 0;

        /* get right part of id (digits) */

        if (collisionnum)
        {
            sprintf(right, "%d", collisionnum);
        }
        else
        {
            j = 0;
            if (isdigit(filename[i]))
            {
                while (isalnum(filename[i]))
                    right[j++] = filename[i++];
                right[j] = 0;
                if (left[0])
                    right[maxidlen-1] = 0;              /* if left side, then truncate to maxidlen-1 */
                right[maxidlen] = 0;                    /* truncate to maxidlen reguardless */
            }
            else
                right[0] = 0;
        }
        while (strlen(left) + strlen(right) > maxidlen)
        {
            remove1letter(left);
        }

        /* join left, right, and pad */

        strcat(left, right);
        if (keepextension)
            strcat(left,ext);
        if (hashpad)
            strcat(left, "            ");
        left[maxnamelen] = 0;

        /* have id, see if it collides with existing id */

        done = 1;
        for (i = 0; i < hashcount; ++i)
        {
            if (!strncmp(hashed + (i * maxnamelen), left, maxnamelen))
            {
                collisionnum = atoi(right) + 1;
                done = 0;
            }
        }
    }
    strcpy(shpname, left);

    /* add to hash list */

    if (hashcount<hashmaxcount)
    {
        strncpy(hashed + (hashcount * maxnamelen), shpname, maxnamelen);
        hashcount++;
    }
}

