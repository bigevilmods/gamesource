// (c) 1990-2006 Criterion Software Limited and its licensors. All Rights Reserved

/*------------------------------------------------------------------*/
/*                                                                  */
/*       FindFile Utility for OS-Specific FileName Retrieval        */
/*                                                                  */
/*                  by Steve Kilthau (coop)                         */
/*                                                                  */
/*                  Main Code - Jan 10, 1998                        */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History:                                       */
/* ------- ----   -- --------                                       */
/*    1.00 980210 SK Compiles on PC,SGI                             */
/*    1.01 980309 SK Expanded functionality (attribs, size, date)   */
/*    1.02 000928 FB Mac OSX ifdefs added                           */
/*    1.03 001017 LC fix recurse on unix, use |= not &=             */
/*    1.04 030911 FB removed DOS, renamed SGI to UNIX               */
/*                                                                  */
/*------------------------------------------------------------------*/


/*------------------------------------------------------------------*/
/* Exported Functions                                               */
/*    RHandle RFindFirst(RFindData *);                              */
/*    int RFindNext(RHandle, RFindData *);                          */
/*    int RFindClose(RHandle, RFindData *);                         */
/*    int RChDir(char *);                                           */
/*    char *RGetCWD(char *, size_t);                                */
/*                                                                  */
/* Exported Macros                                                  */
/*    RName(RFindData)      - accesses name field of RFindData      */
/*    RAttrib(RFindData)    - access attrib field of RFindData      */
/*                                                                  */
/* Exported Datatypes                                               */
/*    RFindData           - structure required by find functions    */
/*    RHandle             - data returned from find functions       */
/*                                                                  */
/* Exported Constants                                               */
/*    RNOTFOUND       - & with RHandle to test RFindFirst success   */
/*    RSUBDIR         - & with Rattrib to see if directory          */
/*------------------------------------------------------------------*/

#include "findfile.h"
#include "stdgimex.h"
#include "spit.h"

/*   DAY     0x001F */
/*   MONTH   0x000F */
/*   TIME    0x003F */
/*   SEC     0x001F */

#if defined(_MSC_VER) || defined(__WATCOMC__)

time_t WIN32_finddate(WIN32_FIND_DATA *fd)
{
    FILETIME loc;
    struct tm t;
    WORD date;
    WORD time;

    if (FileTimeToLocalFileTime(&fd->ftLastWriteTime, &loc))
    {
        if (FileTimeToDosDateTime(&loc, &date, &time))
        {
            t.tm_sec   = ((time)    & (0x001F)) << 1;
            t.tm_min   = (time>>5)  & (0x003F);
            t.tm_hour  = (time>>11) & (0x001F);
            t.tm_mday  = ((date)    & (0x001F));
            t.tm_mon   = ((date>>5) & (0x000F)) - 1;
            t.tm_year  = ((date>>9) & (0x007F)) + 80;
            t.tm_isdst = -1;
            return mktime(&t);
        }
    }
    return (time_t)0;
}

#else

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>

#include <stdio.h>

/*DIR *UNIXfindfirst(struct UNIX_finddata *tmp); */
/*int UNIXfindnext(DIR *dirp, struct UNIX_finddata *tmp); */
/*int UNIXfindclose(DIR *dirp); */


DIR *UNIXfindfirst(struct UNIX_finddata *tmp)
{
    DIR *dirp;               /* New directory handle */
    struct dirent *dp;      /* Directory entry */
    struct stat st;         /* Statistics buffer */

    /* Clear the structure */
    memset((void *)tmp, 0, sizeof(struct UNIX_finddata));

    dirp = opendir(".");
    if ((dp = readdir(dirp)) != NULL)
    {
        strcpy(tmp->name, dp->d_name);
        if (stat(dp->d_name, &st) != -1)
        {
            tmp->size = (int)(st.st_size);
#if defined(__unix__) || defined(__PPU__) || defined(__BEOS__) || defined(__APPLE__)
            tmp->date = st.st_ctime;
#else
            tmp->date = st.st_ctim.tv_sec;
#endif
#if !defined(__PPU__)
            if (S_ISDIR(st.st_mode))
                tmp->attrib |= RSUBDIR;
#endif
#if defined(__unix__) || defined(__APPLE__)
            if ((st.st_mode & S_IREAD) && !(st.st_mode & S_IWRITE))
                tmp->attrib |= RREADONLY;
#endif
        }
    }
    else
    {
        closedir(dirp);
        dirp = NULL;
    }

    return dirp;
}


int UNIXfindnext(DIR *dirp, struct UNIX_finddata *tmp)
{
    struct dirent *dp;      /* Directory entry */
    struct stat st;         /* Statistics buffer */
    int ok = 0;

    /* Clear the structure */
    memset((void *)tmp, 0, sizeof(struct UNIX_finddata));

    if ((dp = readdir(dirp)) != NULL)
    {
        strcpy(tmp->name, dp->d_name);
        if (stat(dp->d_name, &st) != -1)
        {
            tmp->size = (int)(st.st_size);
#if defined(__unix__) || defined(__PPU__) || defined(__BEOS__) || defined(__APPLE__)
            tmp->date = st.st_ctime;
#else
            tmp->date = st.st_ctim.tv_sec;
#endif
#if !defined(__PPU__)
			if (S_ISDIR(st.st_mode))
                tmp->attrib |= RSUBDIR;
#endif
#if defined(__unix__) || defined(__APPLE__)
            if ((st.st_mode & S_IREAD) && !(st.st_mode & S_IWRITE))
                tmp->attrib |= RREADONLY;
#endif
            ok = 1;
        }
    }

    return ok;
}


int UNIXfindclose(DIR *dirp)
{
    closedir(dirp);
    return 1;
}


#endif
