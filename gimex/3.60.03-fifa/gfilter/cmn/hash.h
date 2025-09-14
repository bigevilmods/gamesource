// (c) 1997-2007 Electronic Arts Inc.

#ifndef __HASH_H
#define __HASH_H 1

#if defined(_MSC_VER)
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef LOCAL_HASH_FUNCTIONS
#define HASH_DECAL  static
#else
#define HASH_DECAL
#endif

typedef struct HASH HASH;

HASH_DECAL HASH * HASH_init(int hashsize);
HASH_DECAL void   HASH_add(HASH *header,const char *name,int code);
HASH_DECAL int    HASH_check(HASH *header,const char *name);
HASH_DECAL void   HASH_purge(HASH *header);

#ifdef __cplusplus
}
#endif
#endif

