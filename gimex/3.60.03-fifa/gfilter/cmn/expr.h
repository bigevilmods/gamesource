// (c) 1990-2007 Electronic Arts Inc.

/*------------------------------------------------------------------*/
/*                                                                  */
/*                    EXPRession Parser API v1.40                   */
/*                                                                  */
/*                     by FrANK G. Barchard, EAC                    */
/*                                                                  */
/*                       Header - May 20, 2005                      */
/*                                                                  */
/*------------------------------------------------------------------*/

#ifndef EXPR_H
#define EXPR_H

#if defined(_MSC_VER)
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define EXPR_VERSION 140

typedef struct EXPR EXPR;

EXPR *  EXPR_open(void);
void    EXPR_set(EXPR *expr, const char *symbol, double value);
double  EXPR_get(EXPR *expr, const char *expression);
void    EXPR_close(EXPR *expr);

int     EXPR_error(EXPR *expr);
void    EXPR_addfunc(EXPR *expr, const char *fname, double (*fptr)(...), int ftype);
void    EXPR_setmode(EXPR *expr, int mode); // set fast mode
void    EXPR_setbase(EXPR *expr, int base);
int     EXPR_getbase(EXPR *expr);

#ifdef __cplusplus
}
#endif
#endif // EXPR_H


