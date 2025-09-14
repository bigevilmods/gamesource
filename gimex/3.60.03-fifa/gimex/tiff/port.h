// (c) 2001-2007 Electronic Arts Inc.
/*
 * Warning, this file was automatically created by the TIFF configure script
 * VERSION:	 v3.5.5
 * DATE:	 Tue Mar 28 23:36:39 EST 2000
 * TARGET:	 i586-unknown-linux
 * CCOMPILER:	 /usr/bin/gcc-2.95.2 20000220 (Debian GNU/Linux)
 */
#ifndef _PORT_
#define _PORT_ 1
#ifdef __cplusplus
extern "C" {
#endif
#include <sys/types.h>

// this part modified by hand for gimex
#if defined(__APPLE__) || defined(__mips)

#define HOST_FILLORDER FILLORDER_LSB2MSB
#define HOST_BIGENDIAN	0

#else

#define HOST_FILLORDER FILLORDER_LSB2MSB
#define HOST_BIGENDIAN	0

#endif

#define HAVE_MMAP 1
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
typedef double dblparam_t;
#ifdef __STRICT_ANSI__
#define	INLINE	__inline__
#else
#define	INLINE	inline
#endif


#define GLOBALDATA(TYPE,NAME)	extern TYPE NAME
#ifdef __cplusplus
}
#endif
#endif
