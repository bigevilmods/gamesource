// (c) 1994-2006 Criterion Software Limited and its licensors. All Rights Reserved

/*------------------------------------------------------------------*/
/*                                                                  */
/*                        Hashing Routines                          */
/*                                                                  */
/*                     by FrANK G. Barchard, EAC                    */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* See hashidi.c for version history, module notes, and examples.   */
/*                                                                  */
/*------------------------------------------------------------------*/

#ifndef HASHID_H
#define HASHID_H

#if defined(_MSC_VER)
#pragma once
#endif

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

void HASHID_init(size_t maxnames, size_t idlen, int pad, int keepextensions);
void HASHID_restore();
void HASHID_apply(char *shpname,char *filename);

// backward compatibility
#define inithashid HASHID_init
#define deinithashid HASHID_restore
#define hashid HASHID_apply

#ifdef __cplusplus
}
#endif
#endif // HASHID_H


