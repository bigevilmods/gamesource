// (c) 1994-2007 Electronic Arts Inc.

/* For version info, and examples on how to use see findfile.c */

#if defined(_MSC_VER)
#pragma once
#endif

#define FINDFILE_VERSION 103

#include "spit.h"

#if defined(_WIN32) || defined(WIN32) || defined(WIN)

#if !defined(WIN32_LEAN_AND_MEAN)
#define WIN32_LEAN_AND_MEAN
#endif
#include "windows.h"
#include "winbase.h"
#include <time.h>
#include "spit.h"

time_t WIN32_finddate(WIN32_FIND_DATA *fd);

#define RFindFirst(RFindDataInst)              FindFirstFile("*", RFindDataInst)
#define RFindNext(RHandleInst, RFindDataInst)  static_cast<int>(FindNextFile(RHandleInst, RFindDataInst))
#define RFindClose(RHandleInst, RFindDataInst) static_cast<int>(FindClose(RHandleInst))
#define RChDir(Buffer)                         static_cast<int>(SetCurrentDirectory(Buffer))
#define RMkDir(Buffer)                         static_cast<int>(CreateDirectory(Buffer, NULL))
#define RRmDir(Buffer)                         !rmdir(Buffer)
#define RGetCWD(Buffer, MaxLength)             GetCurrentDirectory(MaxLength, Buffer)
#define RName(RFindDataInst)                   (RFindDataInst).cFileName
#define RAlternateName(RFindDataInst)          (RFindDataInst).cAlternateFileName
#define RSize(RFindDataInst)                   static_cast<int>((RFindDataInst.nFileSizeHigh * MAXDWORD) + RFindDataInst.nFileSizeLow)
#define RDate(RFindDataInst)                   WIN32_finddate(&RFindDataInst)
#define RAttrib(RFindDataInst)                 (RFindDataInst).dwFileAttributes
#define RNOTFOUND                              INVALID_HANDLE_VALUE
#define RSUBDIR                                FILE_ATTRIBUTE_DIRECTORY
#define RREADONLY                              FILE_ATTRIBUTE_READONLY
#define RSYSTEM                                FILE_ATTRIBUTE_SYSTEM
#define RHIDDEN                                FILE_ATTRIBUTE_HIDDEN
typedef WIN32_FIND_DATA                        RFindData;
typedef HANDLE                                 RHandle;

#else

// unix/osx/linux

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

typedef struct UNIX_finddata                   RFindData;
typedef DIR *                                  RHandle;

struct UNIX_finddata
{
    char name[SPITMAXLINE];
    int attrib;
    time_t date;
    int size;
};

DIR *UNIXfindfirst(struct UNIX_finddata *tmp);
int UNIXfindnext(DIR *dirp, struct UNIX_finddata *tmp);
int UNIXfindclose(DIR *dirp);

#define RFindFirst(RFindDataInst)              UNIXfindfirst(RFindDataInst)
#define RFindNext(RHandleInst, RFindDataInst)  UNIXfindnext(RHandleInst, RFindDataInst)
#define RFindClose(RHandleInst, RFindDataInst) UNIXfindclose(RHandleInst)
#define RChDir(Buffer)                         !chdir(Buffer)
#define RMkDir(RDir)                           !mkdir(RDir, 0744)
#define RRmDir(Buffer)                         !rmdir(Buffer)
#define RGetCWD(Buffer, MaxLength)             getcwd(Buffer, MaxLength)
#define RName(RFindDataInst)                   (RFindDataInst).name
#define RAlternateName(RFindDataInst)          (RFindDataInst).name
#define RAttrib(RFindDataInst)                 (RFindDataInst).attrib
#define RSize(RFindDataInst)                   static_cast<int>(RFindDataInst.size)
#define RDate(RFindDataInst)                   (RFindDataInst).date
#define RNOTFOUND                              NULL
#define RSUBDIR                                0x00000001
#define RREADONLY                              0x00000002
#define RSYSTEM                                0x00000004
#define RHIDDEN                                0x00000004

#endif

