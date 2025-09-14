// (c) Electronic Arts. All Rights Reserved.

#ifndef __HASH_C
#define __HASH_C

#include <string.h>
#include <stdlib.h>

#include "hash.h"

struct MEMHUNK;
typedef struct MEMHUNK
{
    struct MEMHUNK *next;
    struct MEMHUNK *prev;
} MEMHUNK;

typedef struct MEMHUNKHEADER
{
    int structsize;
    int allocblocks;
    MEMHUNK allochead;
    MEMHUNK alloctail;

    MEMHUNK freehead;
    MEMHUNK freetail;

    MEMHUNK head;
    MEMHUNK tail;
} MEMHUNKHEADER;

typedef struct HASH_RECORD
{
    struct HASH_RECORD *left;
    struct HASH_RECORD *right;
    struct HASH_RECORD *next;   /* forward unsorted link */
    char *name;
    int code;
} HASH_RECORD;

struct HASH
{
    int hashsize;
    int numrecords; /* number of hash records added */
    HASH_RECORD *records;
    HASH_RECORD *list;
    MEMHUNKHEADER hashhunk;
};

static int HUNK_allocate(MEMHUNKHEADER *header,int numrecords);
static void HUNK_purge(MEMHUNKHEADER *header);
static MEMHUNK *HUNK_newrecord(MEMHUNKHEADER *header);
static void HUNK_init(MEMHUNKHEADER *header,int structsize,int allocblocks);

static int HUNK_allocate(MEMHUNKHEADER *header,int numrecords)
{
    int i;
    unsigned char *block;
    MEMHUNK *sr;
    MEMHUNK *snext;

    do{
        block=(unsigned char *)malloc((numrecords*(sizeof(MEMHUNK)+header->structsize))+sizeof(MEMHUNK));
        if(!block)          /* out of memory, try less */
            numrecords=numrecords>>1;
    }while(!block && numrecords);
    if(!block)
        return(0);

    /* add the first block to the allocated blocks list */

    sr=(MEMHUNK *)block;
    snext=header->allochead.next;
    header->allochead.next=sr;
    sr->prev=&header->allochead;
    sr->next=snext;
    snext->prev=sr;
    block+=sizeof(MEMHUNK);

    i=numrecords;
    while(i)
    {
        sr=(MEMHUNK *)block;
        snext=header->freehead.next;        /* add this record to the free list */
        sr->prev=&header->freehead;
        sr->next=snext;
        snext->prev=sr;
        header->freehead.next=sr;

        block+=(sizeof(MEMHUNK)+header->structsize);
        i--;
    }

    return(numrecords);
}

static void HUNK_init(MEMHUNKHEADER *header,int structsize,int allocblocks)
{
    header->structsize=structsize;
    header->allocblocks=allocblocks;

    header->allochead.next=&header->alloctail;
    header->alloctail.prev=&header->allochead;

    header->freehead.next=&header->freetail;
    header->freetail.prev=&header->freehead;

    header->head.next=&header->tail;
    header->tail.prev=&header->head;
    HUNK_allocate(header,header->allocblocks);
}

static void HUNK_purge(MEMHUNKHEADER *header)
{
    MEMHUNK *sr;
    MEMHUNK *next;

    /* purge all records from memory */
    sr=header->allochead.next;
    while(sr!=&header->alloctail)
    {
        next=sr->next;
        free((signed char *)sr);
        sr=next;
    }

    header->allochead.next=&header->alloctail;
    header->alloctail.prev=&header->allochead;

    header->freehead.next=&header->freetail;
    header->freetail.prev=&header->freehead;

    header->head.next=&header->tail;
    header->tail.prev=&header->head;
}

static MEMHUNK *HUNK_newrecord(MEMHUNKHEADER *header)
{
    MEMHUNK *record;
    MEMHUNK *next;

    if(header->freehead.next==&header->freetail)
    {
        if(!HUNK_allocate(header,header->allocblocks))
            return(0);
    }

    record=header->freehead.next;       /* record to return */

    next=record->next;                  /* remove record from the free list */
    header->freehead.next=next;
    next->prev=&header->freehead;

    return(record);
}

HASH *HASH_init(int hashsize)
{
    int i,size;
    HASH *header;
    HASH_RECORD *hr;

    size=sizeof(HASH)+(hashsize*sizeof(HASH_RECORD));
    header=(HASH *)malloc(size);
    if(header)
    {
        hr=header->records=(HASH_RECORD *)(header+1);
        header->hashsize=hashsize;
        header->numrecords=0;
        header->list=0;
        HUNK_init(&header->hashhunk,sizeof(HASH_RECORD),100);
        for(i=0;i<hashsize;i++)
        {
            hr->left=0;
            hr->right=0;
            hr++;
        }
    }
    return(header);
}

void HASH_purge(HASH *header)
{
    HUNK_purge(&header->hashhunk);
    free((signed char *)header);
}

void HASH_add(HASH *header,const char *name,int code)
{
    int hashcode,hashsize;
    char *tempname;
    HASH_RECORD *nlast;
    HASH_RECORD *n;
    HASH_RECORD *newname;
    int delta;

    tempname=(char *)name;
    hashcode=0;
    hashsize=header->hashsize;

    while(tempname[0])
    {
        if(hashcode)
        {
            hashcode<<=1;
            if(hashcode&hashsize)
                hashcode^=hashsize+1;

            hashcode<<=1;
            if(hashcode&hashsize)
                hashcode^=hashsize+1;

            hashcode<<=1;
            if(hashcode&hashsize)
                hashcode^=hashsize+1;

            hashcode<<=1;
            if(hashcode&hashsize)
                hashcode^=hashsize+1;

            hashcode<<=1;
            if(hashcode&hashsize)
                hashcode^=hashsize+1;
        }
        hashcode^=((tempname[0]-'a')&255);
/*      hashcode^=(unsigned char)tempname[0]; */

        tempname++;
    }
    nlast=&header->records[hashcode];
    n=nlast->left;
    delta=-1;       /* force root names to go on left side */
    if(n)
    {
        do{
            delta=strcmp(name,n->name);
            if(!delta)
                return;                     /* found a match */
            nlast=n;
            if(delta<0)
                n=n->left;
            else
                n=n->right;
        }while(n);
    }

    /* add to nlast */

    newname=(HASH_RECORD *)HUNK_newrecord(&header->hashhunk);
    if(!newname)
        return;

    header->numrecords++;
    if(delta<0)
        nlast->left=newname;
    else
        nlast->right=newname;

    newname->left=0;
    newname->right=0;

    newname->next=header->list;     /* append to the unsorted list */
    header->list=newname;

    newname->name=(char *)name;
    newname->code=code;

/*  fprintf(stderr,"adding name '%s,@%s' %08x,a=%d\n",newname->name,newname->fi->name,newname,newname->allocated); */

    return;
}

int HASH_check(HASH *header,const char *name)
{
    int hashcode,hashsize;
    char *tempname;
    HASH_RECORD *nlast;
    HASH_RECORD *n;
    int delta;

    tempname=(char *)name;
    hashcode=0;
    hashsize=header->hashsize;

    while(tempname[0])
    {
        if(hashcode)
        {
            hashcode<<=1;
            if(hashcode&hashsize)
                hashcode^=hashsize+1;

            hashcode<<=1;
            if(hashcode&hashsize)
                hashcode^=hashsize+1;

            hashcode<<=1;
            if(hashcode&hashsize)
                hashcode^=hashsize+1;

            hashcode<<=1;
            if(hashcode&hashsize)
                hashcode^=hashsize+1;

            hashcode<<=1;
            if(hashcode&hashsize)
                hashcode^=hashsize+1;
        }
        hashcode^=((tempname[0]-'a')&255);

        tempname++;
    }

    nlast=&header->records[hashcode];
    n=nlast->left;
    delta=-1;       /* force root names to go on left side */
    if(n)
    {
        do{
            delta=strcmp(name,n->name);
            if(!delta)
                return(n->code);    /* found a match */
            nlast=n;
            if(delta<0)
                n=n->left;
            else
                n=n->right;
        }while(n);
    }
    return(-1);
}

#endif /* __HASH_C */
