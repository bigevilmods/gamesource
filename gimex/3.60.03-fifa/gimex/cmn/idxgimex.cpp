// (c) 1994-2007 Electronic Arts Inc.
/*------------------------------------------------------------------*/
/*                                                                  */
/*                 Frame Index manager for gimex                    */
/*                                                                  */
/*                       by Kaz Okuda, EAC                          */
/*                                                                  */
/*                     Code Module - Oct 08, 1997                   */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History:                                       */
/* ------- ----   -- --------                                       */
/*    1.00 971008 KO Initial version                                */
/*    1.01 980513 KO Added IDXgetsize, index.userdata               */
/*    1.02 990310 NG Modified IDXseekkey into IDXseekspecial,retspe.*/
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/*                                                                  */
/* This module contains some simple frame index manager useful for  */
/* multiframe gimex formats.  It stores a frame index with frame    */
/* info like position in the file, size and some flags.             */
/* Originally used with madcow gimex but could be reused for many   */
/* Multi frame formats                                              */
/*                                                                  */
/* IDXnewtable      Creates a new index table                       */
/* IDXfreetable     Frees an index table                            */
/* IDXaddindex      Adds an index to the table (Copies it)          */
/* IDXlookup        Finds a particular index in the table           */
/* IDXseekframe     Seeks in a file for a frame number              */
/* IDXseekkey       Seeks in a file for the last key frame before   */
/*                  a frame number                                  */
/* IDXseekspecial	Seeks in a file for the last frame with			*/
/*					appropriate IDXFlags before a frame number		*/
/* IDXgetsize       Return the number of frames stored in the index */
/*                                                                  */
/*------------------------------------------------------------------*/


#include <stdlib.h>
#include <string.h>
#include "idxgimex.h"

/**********************
 * Exported Functions *
 **********************/

IDXTable *IDXnewtable(int numframes)
{
    IDXTable *table = 0;

    table = (IDXTable *) galloc(sizeof(IDXTable));
    if (table)
    {
        table->table = (IDXStruct *) galloc(sizeof(IDXStruct) * numframes);
        table->count = numframes;
        table->currentpos = 0;
    }

    return(table);
}


void IDXfreetable(IDXTable *table)
{
    if (table)
    {
        if (table->table)
        {
            gfree(table->table);
        }
        gfree(table);
    }
}


void IDXaddindex(IDXTable *table, IDXStruct *newstruct)
{
    IDXStruct *temp;

    if (table->currentpos == table->count)
    {
        temp = (IDXStruct *) galloc(table->count * 2 * sizeof(IDXStruct));
        memcpy(temp, table->table, table->count * sizeof(IDXStruct));
        gfree(table->table);
        table->table = temp;
        table->count = table->count * 2;
    }

    memcpy(&table->table[table->currentpos], newstruct, sizeof(IDXStruct));
    table->currentpos++;
}


IDXStruct *IDXlookup(IDXTable *table, int framenum)
{
    IDXStruct *element = NULL;

    if ( (framenum < table->currentpos) && (framenum >= 0) )
    {
        if (table)
            if (table->table)
                element = &table->table[framenum];
    }

    return(element);
}


int IDXseekframe(IDXTable *table, GSTREAM *g, int framenum)
{
    IDXStruct *temp;
    int ok = 0;

    temp = IDXlookup(table, framenum);
    if (temp)
    {
        gseek(g, temp->offset);
        ok = 1;
    }

    return(ok);
}

int IDXseekkey(IDXTable *table, GSTREAM *g, int framenum)
{
	return IDXseekspecial(table,g,framenum,IDXKEY);
}

int IDXretspecial(IDXTable *table, int framenum, IDXFlags code)
{
    IDXStruct *temp;
    int frame;

    frame = framenum;

    temp = IDXlookup(table, frame);
    while ( (temp != NULL) && !(temp->flags & code) )
    {
        frame--;
        temp = IDXlookup(table, frame);
    }    

    return(frame);
}

int IDXseekspecial(IDXTable *table, GSTREAM *g, int framenum, IDXFlags code)
{
    IDXStruct *temp;
    int frame;

    frame=IDXretspecial(table,framenum,code);    
    
    if (frame!=-1)
    {		
        gseek(g, IDXlookup(table, frame)->offset);
    }
    else
    {		
        frame = 0;
        temp = IDXlookup(table, frame);
        if (temp)
            gseek(g, temp->offset);
        else
            frame = -1;
    }

    return(frame);
}

int IDXgetsize(IDXTable *table)
{
    return(table->currentpos);
}

