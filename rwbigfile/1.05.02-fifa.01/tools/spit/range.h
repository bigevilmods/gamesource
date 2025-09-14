// (c) Electronic Arts. All Rights Reserved.

/*------------------------------------------------------------------*/
/*																	*/
/*						   Range Routines							*/
/*																	*/
/*					   by FrANK G. Barchard, EAC					*/
/*																	*/
/*------------------------------------------------------------------*/
/*																	*/
/* See range.c for version history, module notes, examples and		*/
/* range sytanx.													*/
/*																	*/
/*------------------------------------------------------------------*/

#ifndef RANGE_H
#define RANGE_H

#if defined(_MSC_VER)
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define RANGE_VERSION 109

char *SPIT_getrangeopt(char *optvalptr,const char *optptr, const char *optname, int optmin, int optmax);
char *SPIT_getrangeoptnostrip(char *optvalptr,const char *optptr, const char *optname, int optmin, int optmax);
char *SPIT_getrangefield(char *dest,const char *optptr,int strip,int rangestop);

int RANGE_constrain(const char *rangestr, int num, int option);
int RANGE_match(const char *rangestr, char *name, int num);
int RANGE_min(const char *rangestr, int minrange);
int RANGE_max(const char *rangestr, int maxrange);
int RANGE_check(const char *rangestr, int minrange, int maxrange);
int RANGE_count(const char *rangestr, int minrange, int maxrange);

#ifdef __cplusplus
}
#endif
#endif // RANGE_H

