// (c) 1994-2007 Electronic Arts Inc.

#ifndef __IDXGIMEX
#define __IDXGIMEX 1

#include "gimex.h"

typedef enum
{
    IDXKEY      = 0x01,
    IDXMOTION   = 0x02,
    IDXEXTRA    = 0x04

} IDXFlags;


typedef struct
{
    char        identifier[4];
    IDXFlags    flags;
    int         offset;
    int         length;
    unsigned int userdata;

} IDXStruct;


typedef struct
{
    int         count;
    int         currentpos;
    IDXStruct   *table;

} IDXTable;


IDXTable    *IDXnewtable(int numframes);
void        IDXfreetable(IDXTable *table);

void        IDXaddindex(IDXTable *table, IDXStruct *newstruct);
IDXStruct   *IDXlookup(IDXTable *table, int framenum);

int			IDXretspecial(IDXTable *table, int framenum, IDXFlags code);
int         IDXseekframe(IDXTable *table, GSTREAM *g, int framenum);
int         IDXseekkey(IDXTable *table, GSTREAM *g, int framenum);
int			IDXseekspecial(IDXTable *table, GSTREAM *g, int framenum, IDXFlags code);

int         IDXgetsize(IDXTable *table);

#endif /* __IDXGIMEX */
