// (c) 1992-2007 Electronic Arts Inc.

/*------------------------------------------------------------------*/
/*                                                                  */
/*                    EXPRession Parser API v1.41                   */
/*                                                                  */
/*                     by FrANK G. Barchard, EAC                    */
/*                                                                  */
/*                       Code   - Feb 01, 2007                      */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History                                        */
/* ------- ------ -- -------                                        */
/*    1.0x 97xxxx FB from azm/spit                                  */
/*    1.10 980529 GS can be used as #include'ed code now			*/
/*    1.20 990205 NG loads functions, uses doubles instead of ints  */
/*    1.21 990210 NG ?: pair works properly, built-in binary funcs. */
/*    1.22 000229 FB perlin noise                                   */
/*    1.23 001031 FB ** power from python                           */
/*    1.24 010317 FB rand(0) do 4294967295                          */
/*    1.25 010918 FB updated for C++                                */
/*    1.26 010925 FB (...) for var args not ()                      */
/*    1.27 011107 FB >? <? gcc min max operators                    */
/*    1.28 020220 FB xbox compatible                                */
/*    1.29 020224 FB gamecube compatible ifdef and ints             */
/*    1.30 020314 FB Error message reporting more readable          */
/*    1.31 021201 FB ceilpow2, floorpow2                            */
/*    1.32 030429 FB cast math functions to double first for .net   */
/*    1.33 030430 FB cast without & for .net                        */
/*    1.34 030508 FB bswap() to endian swap an int                  */
/*    1.35 030908 FB internally simplified for speed                */
/*    1.36 030914 FB 0b 0o defined()                                */
/*    1.37 030929 FB checks for buffer overflow on single term      */
/*    1.38 031201 FB gcc/ps2 fixes for INT64 and cast warnings      */
/*    1.39 031201 FB << 64 and more using pow                       */
/*    1.40 050520 FB added tolower, toupper                         */
/*    1.41 070201 FB spit for intptr_t type                         */
/*                                                                  */
/*------------------------------------------------------------------*/

/****************************************************************/
/*  Includes                                                    */
/****************************************************************/

#ifdef LOCAL_EXPR_FUNCTIONS
#define EXPR_DECAL  static
#else
#define EXPR_DECAL
#endif

#define PRIMARY

#include "expr.h"

#if !defined(_MSC_VER)
#define stricmp strcasecmp
#ifndef max
#define max(a,b)    (((a)>(b))?(a):(b))
#endif
#ifndef min
#define min(a,b)    (((a)<(b))?(a):(b))
#endif
#endif
// spit for stricmp
//#include "spit.h"
// spit for intptr_t
#include "spit.h"

#include "stdgimex.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#if defined(_MSC_VER) && !defined(_XBOX)
#pragma warning(push,1)
#include <windows.h>
#pragma warning(pop)
#endif

#include "hash.h"

#if defined(__PPCBROADWAY__) || defined(_R5900)
#define uintptr_t unsigned int
#define intptr_t  int
#endif

/****************************************************************/
/*  Internal Defines                                            */
/****************************************************************/

static const int EXPRMAXPARAMS=16;       /* Maximum number of parameters */
static const int EXPRMAXDEPTH=64;        /* Maximum number of nested multiparameter functions */
static const int EXPRMAXFUNCTIONS=512;   /* maximum number of user defined functions and libraries*/

/****************************************************************/
/*  EXPR API                                                    */
/****************************************************************/

#ifndef INT64
#if defined(_MSC_VER) || defined(__WATCOMC__)
#define INT64 __int64
#elif defined(_R5900)
#define INT64 long
#else
#define INT64 long long
#endif
#endif

struct EXPR
{
    int Fast; // set to 1 to skip syntax and missing symbol checking

    int              Radix;                  /* default base (10) */
    char             *NextFNS;               /* Next input file path/name */
    struct TermStack *Term;                 /* Term stack pointer */
    struct OpStack   *Ops;                   /* Operator stack pointer */
    struct InFCtl    *InF;                   /* Macro/include file stack pointer */
    int              GetStringLen;           /* len of GetString */
    struct SymTab    *Sym;                   /* ReadSymTab sets this up */
    struct SymTab    *SymCurr;               /* Start of current chunk of data */
    struct SymTab    *SymLim;                /* The symbol table ends here */
    struct SymTab    **Hash;                 /* Pointer to hash table */
    char             *NameLim;              /* Next available name entry */
    struct NameChunk *NameCurr;             /* Start of current name chunk */
    struct SymTab    *SymStart;              /* The symbol table starts here */
    struct NameChunk *NameStart;            /* Start of first name chunk */
    char             *Heap2;                 /* Secondary heap */
    int              Error;                  /* on Error, set to -1 */
    int              paramdepth;             /* nested function depth */
    struct ParamList (*plist)[EXPRMAXDEPTH];/* parameters */
    HASH             *hasher;                /* hash table handle for function */
    int              ExFuncs;                /* number of external libraries */
    struct ExternalFunc (*Functions)[EXPRMAXFUNCTIONS]; /* external functions */

    static char OpPrec[256];            /* Operator precedence look-up table */
    static char FOpPrec[256];           /* First char operator precedence look-up table */
};

char EXPR::OpPrec[256]={0,};            /* Operator precedence look-up table */
char EXPR::FOpPrec[256]={0,};           /* First char operator precedence look-up table */

/* expression binary operators */
#define BOPASL      128
#define BOPASR      129
#define BOPLE       130
#define BOPGE       131
#define BOPEQ       132
#define BOPNE       133
#define BOPAND      134
#define BOPOR       135
#define BOPXOR      136
#define BOPIF       137
#define BOPNIF      138
#define BOPARY      139
#define BOPPOW      140
#define BOPMIN      141
#define BOPMAX      142
#define BOPMOD      143

/* expression unary operators */
#define UOPS        144
#define UOPNEG      144
#define UOPCOM      145
#define UOPNOT      146
#define UOPHIW      147
#define UOPLOW      148
#define UOPASL      149
#define UOPASR      150

/* expression functions */
#define UFNS        151
#define UFNDEFINED  151
#define UFNABS      152
#define FNOTHER     180

/* maximum precedence is assigned to functions */
static const int MAXPREC=16;
static const int PRECUNARY=15;  // UNARY operators are right to left

/* Error codes */
#define OperErr     7
#define RelErr      14
#define FBSizeErr   10
#define NoStrEnd    27
#define Undef       5
#define AddrErr     9
#define BadDigit    29

#define EXPRMAXITEM 512     /* Longest individual item */
#define CHUNKSIZE   65536   /* Size of memory chunks allocated for tables */
static const int HASHSIZE=2048; /* Default number of elements in hash table */
#define DEFHEAP2    1024    /* Default size for secondary heap */

/* High line number for undefined symbols */
#define NODEF 0x7ffffffb

#define TWOPI   6.283184714

#define FloatInCharRange(f) ((f)>=0.0 && (f)<256.0)
#define FloatMod(a,b) a-floor(a/fabs(b))*fabs(b)
#define FloatToBool(f) (f!=0.0F)
#define CharToValue(c) c-='0'; if ((unsigned char) c >= (unsigned char) 10) c = (char)(((c+'0')|' ')-'a'+10)

/****************************************************************/
/*  Internal Structures                                         */
/****************************************************************/

struct TermStack                /* parser's term stack */
{
    double value;               /* value */
    char question;              /* if true the : operator takes the first term, otherwise the second; used to implement the '?' operator */
};

struct InFCtl
{
    int  NArg;                   /* Number of macro arguments or -1 */
};

struct OpStack                   /* parser's operator stack */
{
    char chr;                    /* operator character */
    int  prec;                   /* precedence */
};

struct NameChunk                 /* chunk of labels or macro text */
{
    struct NameChunk        *Link;                      /* link to the next chunk */
    char                    *Data[CHUNKSIZE-sizeof(void *)];   /* data area */
};

typedef double EXPR_func0(EXPR*, int, double *);    /* exprmain */

typedef double EXPR_func1(int, double *);           /* countmain */

typedef double EXPR_func2(                          /* main */
    double,double,double,double,
    double,double,double,double,
    double,double,double,double,
    double,double,double,double);           /* function prototypes for loaded functions */

typedef struct ExternalFunc {
    double (*funcptr)(...);            /* pointer to entry point of function */
    int     functype;               /* prototype number */

} ExternalFunc;

typedef struct ParamList
{
    ExternalFunc *  function;               /* function */
    int             funcparncount;          /* nested parenthesy count at start of function parse */
    int             numparams;                  /* number of pushed parameters */
    double          params[EXPRMAXPARAMS];      /* last parameter does not need to be stored (it's available) */
} ParamList;

static INT64 INF64=0x7ff0000000000000LL;

#define DOUBLE_INFINITY_VALUE (*(double *)&INF64)

struct SymTab                                       /* symbol table */
{
    struct SymTab           *Link;                  /* link to next entry in hash chain */
    char                    *Name;                  /* pointer to symbol */
    double                  Value;                  /* value */
};

/****************************************************************/
/*  Internal Functions                                          */
/****************************************************************/

static double GetNum(EXPR *expr, const char *operand,int loc);
static char *GetString(EXPR *expr, char *buf, const char *s, int loc);
static void InitOpPrec(EXPR *expr);
static void DoMath(EXPR *expr, int newprec, char commaoperator);
static struct SymTab *AddSymTab(EXPR *expr, const char *label,double value);
static char *AddName(EXPR *expr, const char *name);
static void quit_cleanup(EXPR *expr, const char *s);
static void Error(EXPR *expr, int pos,int errornum);

static double EXPR_max(int count, const double *data);
static double EXPR_min(int count, const double *data);
static double EXPR_hypot(int count, const double *data);
static double EXPR_log2(double value);
static double EXPR_bswap(double value);
static double EXPR_floorpow2(double value);
static double EXPR_ceilpow2(double value);
static double EXPR_rand(double value);
static double EXPR_round(double value);
static double EXPR_int(double value);

/****************************************************************/
/*  Local Variables                                             */
/****************************************************************/

static void InitOpPrec(EXPR *expr)
{
    char       *OpPtr;
    char       *FOpPtr;

    OpPtr = EXPR::OpPrec;
    FOpPtr = EXPR::FOpPrec;

    if (!OpPtr[BOPARY])
    {
    //    memset(FOpPtr,0L,256);
    //    memset(OpPtr,0L,256);

        /* binary operators     unary operators */
        /* ----------------     --------------- */

        OpPtr[BOPARY] = 16;

                                FOpPtr[UOPNEG] = 15;
                                FOpPtr[UOPCOM] = 15;
                                FOpPtr[UOPNOT] = 15;
                                FOpPtr[UOPHIW] = 15;
                                FOpPtr[UOPLOW] = 15;
                                FOpPtr[UOPASL] = 15;
                                FOpPtr[UOPASR] = 15;

        OpPtr['*'] =    14;     FOpPtr['*'] =    15;
        OpPtr['/'] =    14;
        OpPtr['%'] =    14;
        OpPtr[BOPMOD] = 14;
        OpPtr['+'] =    13;     FOpPtr['+'] =    15;
        OpPtr['-'] =    13;     FOpPtr['-'] =    15;
        OpPtr[BOPASL] = 12;
        OpPtr[BOPASR] = 12;
        OpPtr['<'] =    11;     FOpPtr['<'] =    15;
        OpPtr[BOPLE] =  11;
        OpPtr['>'] =    11;     FOpPtr['>'] =    15;
        OpPtr[BOPGE] =  11;
        OpPtr[BOPPOW] = 11;
        OpPtr[BOPEQ] =  10;
        OpPtr[BOPNE] =  10;
        OpPtr['#'] =    10;
        OpPtr['&'] =    9;
        OpPtr['^'] =    8;
        OpPtr['~'] =    8;      FOpPtr['~'] =    15;
        OpPtr['|'] =    7;
        OpPtr['!'] =    7;      FOpPtr['!'] =    15;
        OpPtr[BOPMIN] = 6; // C++ GNU extension.. precidence not defined formally
        OpPtr[BOPMAX] = 6;
        OpPtr[BOPAND] = 6;
        OpPtr[BOPOR] =  5;
        OpPtr[BOPXOR] = 5;
        OpPtr['?'] =    4;
        OpPtr[':'] =    4;
        OpPtr[BOPIF] =  4;
        OpPtr[BOPNIF] = 4;


        OpPtr['='] =    3;  // assignment

        OpPtr[')'] =    2;      FOpPtr[')'] =    2;
        OpPtr[']'] =    2;      FOpPtr[']'] =    2;
        OpPtr[','] =    2;      FOpPtr[','] =    2;
        OpPtr['('] =    1;      FOpPtr['('] =    1;
        OpPtr['['] =    1;      FOpPtr['['] =    1;
        OpPtr[';'] =    1;      FOpPtr[';'] =    1;
        OpPtr['\0'] =   1;      FOpPtr['\0'] =   1;
    }
}

/* Set up the operator precedence tables.
   Note that for EXPR, the OpPrec[] tbl is modified in EXPR.c and EXPROPER.c
   in order to change handling of the following operators:

   ?   which can be used as part of temporary labels
   #   provided to allow users to use # as part of a label
   .   which can be used in normal labels (ReSource)

 */



static double EXPR_max(int count, const double *data)
{
    int i;
    double r=-DOUBLE_INFINITY_VALUE;

    for (i=0; i<count; ++i)
        if (data[i]>r)
            r = data[i];
    return r;
}

static double EXPR_min(int count, const double *data)
{
    int i;
    double r=DOUBLE_INFINITY_VALUE;

    for (i=0; i<count; ++i)
        if (data[i]<r)
            r = data[i];
    return r;
}



static double EXPR_rand(double value)
{
#if (defined(_MSC_VER) || defined(__WATCOMC__)) && !defined(_XBOX)
    static int seed=0;
    if (!seed)
    {
        __asm
        {
            _emit   0x0f
            _emit   0x31
            mov     seed,eax
        }
        srand(seed);
    }
#endif
    if (!value)
        value = 4294967295.0; // 2^32-1
    return ((double) rand()) * value / (double) 0x7fff;
}

static double EXPR_round(double value)
{
    if (value<0)
        return -floor(-value+0.5);
    return floor(value+0.5);
}

static double EXPR_int(double value)
{
    return (double)(INT64)value;
}

static double EXPR_hypot(int count, const double *data)
{
    int i;
    double r=0;

    for (i=0; i<count; ++i)
        r = sqrt(data[i]*data[i]+r*r);
    return r;
}

static double EXPR_log2(double value)
{
    return log(value)/log(2.0f);
}

static double EXPR_ceilpow2(double value)
{
    return pow(2.0,ceil(log(value)/log(2.0f)));
}

static double EXPR_floorpow2(double value)
{
    return pow(2.0,floor(log(value)/log(2.0f)));
}

static double EXPR_bswap(double value)
{
    unsigned int ivalue = (unsigned int) value;

    return (double) (((ivalue&0xff000000)>>24)
                    |((ivalue&0x00ff0000)>>8)
                    |((ivalue&0x0000ff00)<<8)
                    |((ivalue&0x000000ff)<<24));

}

static double EXPR_tolower(double value)
{
    return (double) tolower((int) value);
}

static double EXPR_toupper(double value)
{
    return (double) toupper((int) value);
}

static char *ufnstr[]=
{
    "defined",  // Preprocessor style defined(symbol)
    "abs",
    (char *) 0 ,
};



// DJB2 hash
// http://www.cs.yorku.ca/~oz/hash.html
// NOTE require int to be 32 bits

#if defined(_MSC_VER) && _MSC_VER>=1310

// does 2 at a time for efficiency

static inline unsigned int Hash(const char *string,unsigned int hash=5381)
{
    __asm
    {
        mov     ebx,string
        mov     eax,hash
        jmp     hstart

        align 4

htop:   add     eax,edx
        add     ebx,2
hstart: movzx   edx,byte ptr [ebx]
        add     eax,edx
        shl     edx,5
        je      hend

        add     eax,edx
        movzx   edx,byte ptr [ebx+1]
        add     eax,edx
        shl     edx,5
        jne     htop
hend:
    }
}

#else

static inline unsigned int Hash(const char *string,unsigned int hash=5381)
{
    const unsigned char *s = (const unsigned char *) string;
    int c;

    while ((c = *s++))
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}
#endif

#if 0
// http://www.isthe.com/chongo/src/fnv/hash_32.c

/*
 * fnv_32_str - perform a 32 bit Fowler/Noll/Vo hash on a string
 *
 * input:
 * str - string to hash
 * hval - previous hash value or 0 if first call
 *
 * returns:
 * 32 bit hash as a static hash type
 *
 * NOTE: To use the 32 bit FNV-0 historic hash, use FNV0_32_INIT as the hval
 *  argument on the first call to either fnv_32_buf() or fnv_32_str().
 *
 * NOTE: To use the recommended 32 bit FNV-1 hash, use FNV1_32_INIT as the hval
 *  argument on the first call to either fnv_32_buf() or fnv_32_str().
 */

typedef unsigned int Fnv32_t;

 /*
 * 32 bit magic FNV-0 and FNV-1 prime
 */
#define FNV_32_PRIME ((Fnv32_t)0x01000193)
#define FNV1_32_INIT ((Fnv32_t)0x811c9dc5)

static inline unsigned int Hash(const char *string,unsigned int hash=0x811c9dc5)
{
    const unsigned char *s = (const unsigned char *)string; /* unsigned string */

    /*
     * FNV-1 hash each octet in the buffer
     */
    while (*s)
    {
        /* multiply by the 32 bit FNV magic prime mod 2^64 */
        hash *= FNV_32_PRIME;

        /* xor the bottom with the current octet */
        hash ^= (Fnv32_t)*s++;
    }

    /* return our new hash value */
    return hash;
}

static inline unsigned int Hash(const char *string,unsigned int hash=5381)
{
    __asm
    {
        mov     esi,string
        mov     eax,hash
        jmp     hstart

        align 4

htop:   add     eax,edx
        add     esi,1
hstart: movzx   edx,byte ptr [esi]
        add     eax,edx
        shl     edx,5
        jne     htop
    }
}

static inline unsigned int Hash(const char *string,unsigned int hash=5381)
{
    __asm
    {
        mov     esi,string
        mov     eax,hash
        jmp     hstart

        align 4
htop:
        add     esi,2

        add     edx,eax
        shl     eax,5
        add     eax,edx

hstart:
        movzx   edx,byte ptr [esi]
        cmp     edx,0
        je      hdone

        add     edx,eax
        shl     eax,5
        add     eax,edx

        movzx   edx,byte ptr [esi+1]
        cmp     edx,0
        jne     htop
hdone:
    }
}

static inline unsigned int Hash(const char *string,unsigned int hash=5381)
{
    __asm
    {
        mov     ebx,string
        mov     eax,hash
        jmp     hstart

        align 4

htop:   add     eax,edx
        add     ebx,1
hstart: movzx   edx,byte ptr [ebx]      *string
        add     eax,edx
        shl     edx,5                   <<5
        jne     htop
    }
}
#endif


static inline struct SymTab **HashIt(EXPR *expr, const char *str)
{
    unsigned int hash = Hash(str);
    return expr->Hash + (hash & (HASHSIZE-1));
}

/* Searches the symbol table for the given label.
   If found, returns Sym pointing to the proper table entry.
   If not found, returns NULL
*/

static inline struct SymTab * ReadSymTab(EXPR *expr, const char *label)
{
    struct SymTab **hashindex = HashIt(expr, label);
    struct SymTab *NSym = *hashindex;                          /* make sure we got something! */

    /* Make sure that we don't get a NULL back from HashIt */
    if (NSym)
    {
        if (expr->Fast && !NSym->Link)
            return NSym;

        do
        {
            if (!strcmp(NSym->Name, label)) // NOTE this is slow
            {
                return NSym;
            }
            NSym = NSym->Link;
        } while (NSym);
    }
    return NULL;
}



/****************************************************************/
/*  Modify Symbol Table Entry                                   */
/*  - calls ReadSymTab to find the symbol                       */
/*  - sets symbol table values same as AddSymTab                */
/*  - used for internal reserved symbols                        */
/****************************************************************/


/* Adds the name in "name" to the name heap.
    "macflag" can take any of the following values:
    0 - normal name
    1 - first line of macro text - there must be room on the
        name heap for at least one character following "name".
    2 - additional lines of macro text - make sure there's room
        for an addition character (as in 1 above).  Also,
        if it is necessary to allocate a new chunk of memory,
        first place a newline character after the last entry
        in the old chunk.  This acts as a flag when retrieving
        lines of macro text during an expansion.
    3 - equs string
    This function returns a pointer to "name" on the name heap. */

static char *AddName(EXPR *expr, const char *name)
{
    char                *s;
    const char          *t;
    struct NameChunk    *n;

    s = expr->NameLim + strlen(name) + 1;             /* The new entry ends here. */
    if ((s - (char *) expr->NameCurr) > CHUNKSIZE)    /* If this chunk is full */
    {
        n = (struct NameChunk *) galloc(CHUNKSIZE);
        if (!n)
            quit_cleanup (expr, "Out of memory!\n");

        expr->NameCurr->Link = n;                     /* Link from previous chunk */
        expr->NameCurr = n;                           /* Make the new chunk current. */
        expr->NameCurr->Link = NULL;                  /* Clear forward pointer. */
        s = (char *) expr->NameCurr;
        expr->NameLim = s + sizeof(void *);                  /* Skip over pointer entry. */
    }
    s = expr->NameLim;
    t = name;
    while ((*s++ = *t++) != '\0')               /* Store name. */
        ;
    t = expr->NameLim;
    expr->NameLim = s;                                /* Update table limit. */
    return (char *)t;
}


/********************************************************************/
/* Inserts a new entry to the symbol table                       .  */
/* "Sym" will be set to point to the new entry.                     */
/********************************************************************/

static struct SymTab *AddSymTab(EXPR *expr, const char *label,double value)
{
    struct SymTab       *chainsym;
    struct SymTab       **hashindex;
    char                wlab[EXPRMAXITEM];

    strcpy(wlab, label);

    expr->Sym = expr->SymLim;                 /* Pointer to new entry */
    ++expr->SymLim;                           /* Bump limit pointer. */
    if (((char *) expr->SymLim - (char *) expr->SymCurr) > CHUNKSIZE)
    {
        expr->Sym = (struct SymTab *) galloc(CHUNKSIZE);
        if (!expr->Sym)
            quit_cleanup (expr, "Out of memory!\n");
        expr->SymCurr->Link = expr->Sym;                        /* Link from previous chunk */
        expr->SymCurr = expr->Sym;                              /* Make the new chunk current. */
        expr->SymCurr->Link = NULL;                             /* Clear forward pointer. */
        ++expr->Sym;                                            /* Skip over pointer entry. */
        expr->SymLim = expr->Sym;                               /* New table limit */
        ++expr->SymLim;                                         /* Bump it. */
    }
    expr->Sym->Link =     NULL;                       /* Clear hash chain link. */
    expr->Sym->Name =      AddName(expr, wlab);      /* Pointer to symbol */
    expr->Sym->Value =      value;                      /* Value */

    hashindex = HashIt(expr, wlab);                   /* Get hash index. */
    if (!*hashindex)
    {
        *hashindex = expr->Sym;                 /* First entry in this hash chain */
        return expr->Sym;
    }
    chainsym = *hashindex;
    while (chainsym->Link)
        chainsym = chainsym->Link;              /* Scan for end of hash chain. */
    chainsym->Link = expr->Sym;                 /* Insert new entry at the end. */
    return expr->Sym;
}


static inline double SymbolToValue(const struct SymTab *NSym)
{
    return (double) (intptr_t) NSym;
}

static inline struct SymTab *ValueToSymbol(double value)
{
    return (struct SymTab *) (int) value;
}


static inline bool iswhitespace(char c)
{
    return c==' ' || c=='\t' || c=='\r' || c=='\n';
}

/****************************************************************/
/*  Get a Value (Expression)                                    */
/****************************************************************/


/* the comma is an operator if the paramdepth is 0 (not in a function), or the parncount is greater than
the parncount when we opened the function, e.g. f(a,(b,c)) where the second comma is an operator */
#define CommaOperator   (char)(!expr->paramdepth || ((*(expr->plist))[expr->paramdepth-1].funcparncount<parncnt-1))

EXPR_DECAL double EXPR_get(EXPR *expr, const char *expression)
{
    const char *s;
    const char *o;
    char *d;
    char *OpPrecREG;
    int  nextoper;
    int  parncnt;
    int  nextprec;
    int  instring;
    int  i;
    int  hashloc;
    struct TermStack *origterm;
    char tempop[EXPRMAXITEM];
    ParamList * plist;

    expr->paramdepth=0;                          /* clear parameter lists */
    expr->Error = 0;                             /* reset Error on new expressions */

    /* Scan for operators */

    s = expression;
    OpPrecREG = EXPR::OpPrec;

    if (!EXPR::FOpPrec[*s++])
    {
        do
        {
            ;
        } while (!OpPrecREG[*s++]);
    }

    if (!*(s-1))
    {
        return GetNum(expr, expression,0);              /* Short cut for single term */
    }

    parncnt = 0;
    o = (char *) (((intptr_t) expr->NextFNS + 3L) & ~3L);
    origterm = expr->Term = (struct TermStack *) o;     /* Term stack */
    expr->Ops = (struct OpStack *) expr->InF;           /* Operator stack */
    --expr->Ops;
    expr->Ops->chr = ' ';                                /* Prime the operator stack. */
    expr->Ops->prec = -1;

    o = expression;                                /* Current position in expression */

    for (;;)
    {
        while (iswhitespace(*o))                           /* allow spaces */
            ++o;

        if (*o=='(' || *o=='[')                 /* Left parenthesis */
        {
            --expr->Ops;
            expr->Ops->chr  = *o;
            expr->Ops->prec = EXPR::OpPrec[expr->Ops->chr];
            ++parncnt;
            ++o;
            continue;
        }

        /* Handle unary operators */

        if ((*o=='+') || (*o=='-') || (*o=='~') || (*o=='!') || (*o=='<') || (*o=='>'))
        {
            nextoper = (int) *o++;
            switch (nextoper)
            {
                case '+':
                    continue;

                case '-':
                    nextoper = UOPNEG;
                    break;

                case '~':
                    nextoper = UOPCOM;
                    break;

                case '!':
                    nextoper = UOPNOT;
                    break;

                case '<':
                    nextoper = UOPHIW;
                    if (*(o+1)=='<')
                    {
                        nextoper = UOPASL;
                        ++o;
                    }
                    break;

                case '>':
                    nextoper = UOPLOW;
                    if (*(o+1)=='>')
                    {
                        nextoper = UOPASR;
                        ++o;
                    }
                    break;
            }

            nextprec = EXPR::FOpPrec[nextoper];
            --expr->Ops;
            expr->Ops->chr  = (char) nextoper;
            expr->Ops->prec = nextprec;

            continue;
        }

        /* Strip off the expression (stop stripping at operator) */

        d = tempop;                                 /* Get a term. */
        instring = 0;
        if (*o)
        {
            while (iswhitespace(*o))                         /* allow spaces */
                ++o;
            if (*o=='\'')
                instring=!instring;                 /* String delimiter */
            if (instring || !EXPR::FOpPrec[*o])
            {
                *d++ = *o++;                            /* Get a character. */
                while (*o)
                {
                    if (*o=='\'')
                        instring=!instring;         /* String delimiter */
                    if (!instring && EXPR::OpPrec[*o])
                        break;                      /* Found an operator - stop. */
                    if (iswhitespace(*o))
                        break;
                    if ((d-tempop)<((int)sizeof(tempop)-1))  // avoid buffer overflow
                        *d++ = *o;                    /* Get a character. */
                    ++o;
                }
            }
        }
        *d = '\0';

        /* Handle builtin functions */

        nextoper = 0;
        if (*o=='(')
        {
            i = 0;
            while (ufnstr[i])
            {
                if (!stricmp(tempop, ufnstr[i]))
                {
                    nextoper = UFNS+i;
                    break;
                }
                ++i;
            }
            if (!nextoper)
            {
                nextoper=FNOTHER;                           /* implied OTHER FUNCTION */
                plist=&(*expr->plist)[expr->paramdepth];
                plist->numparams=0;

                hashloc=HASH_check(expr->hasher,tempop);
                if (hashloc==-1)
                {
                    Error(expr, 0, Undef);
                    return 0.0;
                }
                else
                {   /* use preloaded function */
                    plist->funcparncount=parncnt;
                    plist->function=&(*expr->Functions)[hashloc];
                }
                ++expr->paramdepth;
            }
            nextprec = MAXPREC;
            --expr->Ops;
            expr->Ops->chr  = (char) nextoper;
            expr->Ops->prec = nextprec;
            continue;
        }

        /* handle arrays */

        if (*o=='[')
        {
            nextoper = BOPARY;                          /* array index add */
        }

        /* Calculate the value of the term */

        if (expr->Ops->chr=='(' && (int)(unsigned char)(expr->Ops[1].chr)==UFNDEFINED)  // special check for defined(macro)
        {
            struct SymTab *NSym = ReadSymTab(expr,tempop);   /* Look up symbol. */
            if (NSym)
                expr->Term->value = 1.0;
            else
                expr->Term->value = 0.0;
        }
        else
            expr->Term->value   = GetNum(expr, tempop, 0);


        ++expr->Term;

        while (iswhitespace(*o))                           /* allow spaces */
            ++o;

        /* Check for Right parenthesis */

        while (*o==')' || *o==']')
        {
            if (parncnt==0)
            {
                Error(expr, (int) (0 + (o - expression)), OperErr);
                return 0.0;
            }
            DoMath(expr, (int)EXPR::OpPrec[*o],CommaOperator);            /* Unstack what we can. */
            if (expr->Ops->chr=='(' || expr->Ops->chr=='[')
                ++expr->Ops;                          /* Drop paired parentheses. */
            else
            {
                --expr->Ops;
                expr->Ops->chr  = *o;                 /* Stack parenthesis for now. */
                expr->Ops->prec = EXPR::OpPrec[*o];
            }
            --parncnt;
            ++o;
        }

        /* Check operator precidence of next operator */
        /* Evaluate if lower precidence than what we have stacked */
        /* heres where we parse binary operators into tokens */

        if (*o)
        {
            if (!nextoper)
            {
                nextoper = (int) *o++;
                if ((unsigned char)nextoper<=(unsigned char)' ')
                {
                    while ((unsigned char)nextoper<=(unsigned char)' ' && nextoper)
                        nextoper = (int) *o++;
                }

                switch (nextoper)
                {
                    case '/':
                        if (*o=='/')
                        {
                            nextoper = '%';
                            ++o;
                        }
                        break;

                    case '%':
                        if (*o=='%')
                        {
                            nextoper = BOPMOD;
                            ++o;
                        }
                        break;


                    case '<':
                        if (*o=='<')
                        {
                            nextoper = BOPASL;
                            ++o;
                        }
                        else if (*o=='=')
                        {
                            nextoper = BOPLE;
                            ++o;
                        }
                        else if (*o=='>')
                        {
                            nextoper = BOPNE;
                            ++o;
                        }
                        else if (*o=='?')
                        {
                            nextoper = BOPMIN;
                            ++o;
                        }
                        break;


                        break;

                    case '>':
                        if (*o=='>')
                        {
                            nextoper = BOPASR;
                            ++o;
                        }
                        else if (*o=='=')
                        {
                            nextoper = BOPGE;
                            ++o;
                        }
                        else if (*o=='?')
                        {
                            nextoper = BOPMAX;
                            ++o;
                        }
                        break;

                    case '=':
                        if (*o=='=')
                        {
                            nextoper = BOPEQ;
                            ++o;
                        }
                        break;

                    case '!':
                        if (*o=='=')
                        {
                            nextoper = BOPNE;
                            ++o;
                        }
                        else if (*o==':')
                        {
                            nextoper = BOPIF;
                            ++o;
                        }
                        else if (*o=='?')
                        {
                            nextoper = BOPNIF;
                            ++o;
                        }
                        break;

                    case '&':
                        if (*o=='&')
                        {
                            nextoper = BOPAND;
                            ++o;
                        }
                        break;

                    case '|':
                        if (*o=='|')
                        {
                            nextoper = BOPOR;
                            ++o;
                        }
                        break;

                    case '*':
                        if (*o=='*')
                        {
                            nextoper = BOPPOW;
                            ++o;
                        }
                        break;

                    case '+':
                        if (*o=='+')
                        {
                            nextoper = BOPOR;
                            ++o;
                        }
                        break;

                    case '-':
                        if (*o=='-')
                        {
                            nextoper = BOPXOR;
                            ++o;
                        }
                        break;

                    default:
                        break;
                }
            }

            nextprec = EXPR::OpPrec[nextoper];
            if (nextprec<=1)
            {
                Error(expr, (int) (0 + (o - expression)), OperErr);
                return 0.0;                     /* We expected an operator. */
            }


            /* Assignment convert term to lvalue */

            if (nextoper=='=')  // special check for defined(macro)
            {
                --expr->Term;

                struct SymTab *NSym = ReadSymTab(expr,tempop);   /* Look up symbol. */
                if (!NSym)
                    NSym = AddSymTab(expr, tempop, 0.0);

                expr->Term->value = SymbolToValue(NSym);

                ++expr->Term;
            }

            DoMath(expr, nextprec,CommaOperator);                   /* Unstack what we can. */
            --expr->Ops;
            expr->Ops->chr  = (char) nextoper;                      /* Stack the next operator. */
            expr->Ops->prec = nextprec;
        }

        /* End of expression  */
        /* Evaluate what we have stacked */

        else
        {
            if (parncnt)
            {
                Error(expr, (int) (0 + (o - expression)), OperErr);
                return 0.0;                     /* Too many left parentheses */
            }
            DoMath(expr, 0,CommaOperator);      /* Unstack what's left. */
            if (--expr->Term != origterm)       /* Should be only one term left */
                Error(expr, 0, OperErr);        /* Parser bug? */

            return expr->Term->value;           /* Final value */
        }
    }
}


/* As long as the top operator on the operator stack has a precedence
    greater than or equal to the contents of "newprec", this routine
    will pop the two top terms from the term stack, combine them
    according to the operator on the top of the operator stack (which
    is also popped), and push the result back onto the term stack. */

static void DoMath(EXPR *expr, int newprec, char commaoperator)
{
    struct ParamList * plistptr;
    bool isRightToLeft = newprec==PRECUNARY || expr->Ops->chr==(int)'=';  // unary operators and assignment are right to left

    /* Unstack an operator. */

    while (expr->Ops->prec>newprec
       || (expr->Ops->prec==newprec && !isRightToLeft))
    {
        int OpsChr = (int) ((unsigned char) expr->Ops->chr);

        if (OpsChr>=UOPS)
        {
            expr->Term -= 1;
        }
        else
        {
            expr->Term -= 2;
        }

        /* Perform the operation. */

        switch (OpsChr)
        {
            case UFNDEFINED:    // has already been evaluated
                break;

            case UFNABS:
                expr->Term->value = fabs(expr->Term->value);
                break;

            case UOPNEG:    expr->Term->value = -expr->Term->value;                     break;
            case UOPCOM:    expr->Term->value = -expr->Term->value-1;                   break;
            case UOPNOT:    expr->Term->value = !FloatToBool(expr->Term->value);                     break;
            case UOPASL:    expr->Term->value = (double)((INT64)expr->Term->value << 1);        break;
            case UOPASR:    expr->Term->value = (double)((INT64)expr->Term->value >> 1);        break;
            case UOPHIW:    expr->Term->value = (double)(((unsigned int)expr->Term->value>>16) & 0x0000ffffL);  break;
            case UOPLOW:    expr->Term->value = (double)((unsigned short)expr->Term->value);  break;

            case BOPPOW:    expr->Term->value = pow(expr->Term->value,(expr->Term+1)->value);   break;
            case BOPMIN:    expr->Term->value = min(expr->Term->value,(expr->Term+1)->value);   break;
            case BOPMAX:    expr->Term->value = max(expr->Term->value,(expr->Term+1)->value);   break;
            case '*':       expr->Term->value = expr->Term->value *  (expr->Term+1)->value;   break;
            case '/':       if (FloatToBool((expr->Term+1)->value))
                                expr->Term->value = expr->Term->value /  (expr->Term+1)->value;   break;
            case '%':       if (FloatToBool((expr->Term+1)->value))
                                expr->Term->value = FloatMod(expr->Term->value,(expr->Term+1)->value);   break;
            case BOPARY:    expr->Term->value = expr->Term->value +  (expr->Term+1)->value*1; break;
            case '+':       expr->Term->value = expr->Term->value +  (expr->Term+1)->value;   break;
            case '-':       expr->Term->value = expr->Term->value -  (expr->Term+1)->value;   break;
            case BOPASL:    expr->Term->value = expr->Term->value * pow(2.0,(expr->Term+1)->value); break;
            case BOPASR:    expr->Term->value = (double)((INT64)expr->Term->value >> (int)(expr->Term+1)->value);   break;
            case '<':       expr->Term->value = expr->Term->value <  (expr->Term+1)->value;   break;
            case BOPLE:     expr->Term->value = expr->Term->value <= (expr->Term+1)->value;   break;
            case '>':       expr->Term->value = expr->Term->value >  (expr->Term+1)->value;   break;
            case BOPGE:     expr->Term->value = expr->Term->value >= (expr->Term+1)->value;   break;
            case BOPEQ:     expr->Term->value = expr->Term->value == (expr->Term+1)->value;   break;
            case '#':
            case BOPNE:     expr->Term->value = expr->Term->value != (expr->Term+1)->value;   break;
            case '&':       expr->Term->value = (double)((INT64)expr->Term->value & (INT64)(expr->Term+1)->value);   break;
            case '~':
            case '^':       expr->Term->value = (double)((INT64)expr->Term->value ^  (INT64)(expr->Term+1)->value);   break;
            case '!':
            case '|':       expr->Term->value = (double)((INT64)expr->Term->value | (INT64)(expr->Term+1)->value);   break;
            case BOPAND:    expr->Term->value = FloatToBool(expr->Term->value) && FloatToBool((expr->Term+1)->value);   break;
            case BOPOR:     expr->Term->value = FloatToBool(expr->Term->value) || FloatToBool((expr->Term+1)->value);   break;
            case BOPXOR:    expr->Term->value = FloatToBool((expr->Term->value) || FloatToBool((expr->Term+1)->value))
                            && !(FloatToBool(expr->Term->value) && FloatToBool((expr->Term+1)->value));           break;
            case BOPIF:
            case '?':       expr->Term->question = (unsigned char)(FloatToBool(expr->Term->value));
                            expr->Term->value=(expr->Term+1)->value;
                            break;
            case BOPNIF:
            case ':':       expr->Term->value = (expr->Term->question)
                                ? expr->Term->value : (expr->Term+1)->value;            break;

            // lvalue points to ValueToPointer
            case '=':
            {               struct SymTab *NSym = ValueToSymbol(expr->Term->value); // lvalue to assign to
                            NSym->Value = (expr->Term+1)->value;
                            expr->Term->value=(expr->Term+1)->value;
                            break;
            }

            case ';':       expr->Term->value=(expr->Term+1)->value;
                            break;
            case ',':       if (expr->paramdepth && !commaoperator)
                            {   /* if we are processing a function's parameters (as opposed to a ',' list) */
                                plistptr = &(*expr->plist)[expr->paramdepth-1];
                                if (plistptr->numparams<EXPRMAXPARAMS)
                                { /* ignore extra parameters */
                                    plistptr->params[plistptr->numparams++]=expr->Term->value;
                                }
                            }
                            expr->Term->value=(expr->Term+1)->value;
                            break;
            case FNOTHER:   if (expr->paramdepth)
                            {
                                plistptr = &(*expr->plist)[expr->paramdepth-1];
                                if (plistptr->numparams<EXPRMAXPARAMS)
                                {   /* ignore extra parameters */
                                    plistptr->params[plistptr->numparams++]=expr->Term->value;
                                }
                                switch(plistptr->function->functype)
                                {
                                case 0:
                                    expr->Term->value=(*(EXPR_func0*)(plistptr->function->funcptr))
                                        (expr,plistptr->numparams,plistptr->params);
                                        break;
                                case 1:
                                    expr->Term->value=(*(EXPR_func1*)(plistptr->function->funcptr))
                                        (plistptr->numparams,plistptr->params);
                                        break;
                                case 2:
                                expr->Term->value=(*(EXPR_func2*)(plistptr->function->funcptr))
                                    (plistptr->params[0], plistptr->params[1], plistptr->params[2], plistptr->params[3],
                                    plistptr->params[4], plistptr->params[5], plistptr->params[6], plistptr->params[7],
                                    plistptr->params[8], plistptr->params[9], plistptr->params[10],plistptr->params[11],
                                    plistptr->params[12],plistptr->params[13],plistptr->params[14],plistptr->params[15]);
                                        break;
                                }
                                --(expr->paramdepth);
                            }
                            else
                            {
                                Error(expr, 0, OperErr);    /* parser bug as well */
                            }
                            break;
            /* Parser bug? */
            default:
                Error(expr, 0, OperErr);
                break;
        }
        ++expr->Term;
        ++expr->Ops;
    }
}

/****************************************************************/
/*  Get a Single Number                                         */
// symbols from symbol table (start with alpha)
// constants in the following forms
// 'four'  a fourcc code
// "strn"  same as fourcc
// $123    hex
// .123    decimal
// 1010b   binary (intel asm style)
// 123d    decimal (intel asm style)
// 123h    hex (intel asm style)
// 123o    octal (intel asm style)
// 123q    octal (intel asm style)
// 0x123   hex C++ style
// 0b1010  binary (google style)
// 0o377   octal (google style)
/****************************************************************/

static double GetNum(EXPR *expr, const char *s,int loc)
{
    char c;
    const char *opstart;
    const char *numstart;
    int  base;
    double nextval,decimalplace,invbase;
    char buf[EXPRMAXITEM];

    if (*s<='@' || expr->Radix>10)
    {
        c = *s++;
        if (*s)                                 /* more than 1 char? */
        {
            opstart = s;
            if (c<'0')
            {
                base = 16;
                if (c!='$')                       /* hex  */
                {
                    switch (c)
                    {
                        case '"':               /* start a string */
                        case '\'':
                        {
                            double tempval=0.0;

                            s = GetString(expr, buf, s-1, loc);
                            if (*(s-1)!='"' && *(s-1)!='\'')    /* no closing delimiter */
                            {
                                Error(expr, loc, NoStrEnd);
                            }
                            else
                            {
                                if (expr->GetStringLen>8)   /* 56bit mantissa->maximum string length=7 */
                                {
                                    Error(expr, loc, FBSizeErr);
                                }
                                else
                                {
                                    s = buf;
                                    while (expr->GetStringLen--)
                                    {
                                        tempval = tempval*256 + (double)((unsigned char) *s++);
                                    }
                                }
                            }
                            return tempval;
                        }

                        case '.':
                            --s;
                            base = 10;
                            break;

                        case 0:                 /* end of term */
                            Error(expr, loc, OperErr);
                            return 0.0;

                        default:
                            base = 0;
                    }
                }
            }
            else
            {
                base = expr->Radix;
                --s;

                if (c=='0')                       /* hex 0x */
                {
                    char nc = tolower(*(s+1));
                    if (nc=='x' && expr->Radix<('x'-'a'+11))
                    {
                        base = 16;
                        s += 2;
                    }
                    else if (nc=='b' && expr->Radix<('b'-'a'+11))
                    {
                        base = 2;
                        s += 2;
                    }
                    else if (nc=='o' && expr->Radix<('o'-'a'+11))
                    {
                        base = 8;
                        s += 2;
                    }
                }
            }

            nextval = 0;
            numstart = s;
            while (*s)
            {
                c = *s++;
                if (c=='.')
                {
                    decimalplace=1;
                    invbase=1.0F/base;
                    while (*s)
                    {
                        c = *s++;
                        CharToValue(c);
                        if ((unsigned char) c >= (unsigned char) base)
                        {
                            if (*(s-1)=='_')
                            {
                                ++numstart;             /* adjust start for len calc */
                                continue;
                            }
                            s--;
                            break;
                        }
                        decimalplace*=invbase;
                        nextval = nextval + (unsigned char)c*decimalplace;
                    }
                }
                else
                {
                    CharToValue(c);
                    if ((unsigned char) c >= (unsigned char) base)
                    {
                        if (*(s-1)=='_')
                        {
                            ++numstart;             /* adjust start for len calc */
                            continue;
                        }
                        s--;
                        break;
                    }
                    nextval = nextval * base + (unsigned char) c;
                }
            }
            if (!*s)                    /* did we finish the number? */
            {
                return nextval;
            }

            s = opstart-1;
            if (isdigit(*s))                    /* was first char a digit */
            {
                while (*s)                      /* skip to the end */
                {
                    ++s;
                }

                c = *(s-1);
                *(char *)(s-1) = '\0';
                switch (c|' ')
                {
                    case 'b':
                        base = expr->Radix;
                        expr->Radix = 2;
                        nextval = GetNum(expr, opstart-1, loc);
                        expr->Radix = base;
                        *(char *)(s-1) = c;
                        return nextval;

                    case 'd':
                        base = expr->Radix;
                        expr->Radix = 10;
                        nextval = GetNum(expr, opstart-1, loc);
                        expr->Radix = base;
                        *(char *)(s-1) = c;
                        return nextval;

                    case 'h':
                        base = expr->Radix;
                        expr->Radix = 16;
                        nextval = GetNum(expr, opstart-1, loc);
                        expr->Radix = base;
                        *(char *)(s-1) = c;
                        return nextval;

                    case 'o':
                    case 'q':
                        base = expr->Radix;
                        expr->Radix = 8;
                        nextval = GetNum(expr, opstart-1, loc);
                        expr->Radix = base;
                        *(char *)(s-1) = c;
                        return nextval;
                }
                *(char *)(s-1) = c;
                s = opstart-1;
            }
        }
        else                                            /* one character */
        {
            CharToValue(c);

            if ((unsigned char) c < (unsigned char) expr->Radix)
            {
                return (double) c ;
            }
            c = *--s;
        }
    }
    struct SymTab *NSym =  ReadSymTab(expr,s);
    if (NSym)                      /* Look up symbol. */
    {
        return NSym->Value;
    }
    Error(expr, loc, Undef);              /* Undefined */
    return 0.0;
}

static char *GetString(EXPR *expr, char *buf, const char *s,int loc)
{
    char       c;
    char       n;
    char       *d;
    char       firstc;
    const char *olds;

    d = buf;
    firstc = *s;
    olds = s;
    if (firstc=='"')
    {
        do
        {
            ++s;
            do
            {
                c = *s++;
                if (c=='\\')
                {
                    c = *s++;
                    switch(c)
                    {
                        case 'n':   *d++ = (char)10;    break;
                        case 't':   *d++ = '\t';  break;
                        case 'b':   *d++ = '\b';  break;
                        case 'r':   *d++ = (char)13;    break;
                        case 'f':   *d++ = '\f';  break;
                        case '\\':  *d++ = '\\';  break;
                        case '<':   *d++ = '<';   break;
                        case '\'':  *d++ = '\'';   break;
                        case '"':   *d++ = '"'; break;
                        default:
                            c -= '0';
                            if ((unsigned char) c < (unsigned char) 8)
                            {
                                n = c;
                                c = (char)(*s++-'0');
                                if ((unsigned char) c < (unsigned char) 8)
                                {
                                    n = (char)(n*8+c);
                                    c = (char)(*s++-'0');
                                    if ((unsigned char) c < (unsigned char) 8)
                                    {
                                        if ((unsigned char) n >= (unsigned char) 32)
                                            Error(expr, loc+(int)(s-olds), FBSizeErr);
                                        n = (char)(n*8+c);
                                        ++s;
                                    }
                                }
                                *d++ = n;
                            }
                            if (c==(char)8 || c==(char)9)
                                Error(expr, loc+(int)(s-olds), BadDigit);
                            --s;
                            c = '0';
                    }
                }
                else
                {
                    *d++ = c;
                }
            } while (c!='\0' && c!='"');
        } while (*s=='"');

        if (c=='"')
            *--d = '\0';                        /* remove close quote or slash */
        else
            --s;
    }
    else
    {
        if (firstc=='\'')
        {
            do
            {
                ++s;
                do
                {
                    c = *s++;
                    *d++ = c;
                } while (c!='\0' && c!='\'');
            } while (*s=='\'');

            if (c=='\'')
                *--d = '\0';                    /* remove close quote or slash */
            else
                --s;
        }
        else
        {
            if (firstc=='\\')
            {
                ++s;
            }

            do
            {
                c = *s++;
                *d++ = c;
            } while ((unsigned char)c>(unsigned char)' ' && c!=';' && c!=',');

            if (c)
            {
                *--d = '\0';                    /* remove ; or , or white space */
                if (*(d-1)=='\\')
                    *--d = '\0';                /* remove slash */
            }
            else
                --s;

            struct SymTab *NSym =  ReadSymTab(expr,buf);
            if (NSym)                      /* see if its an equs */
            {
                if (firstc=='\\')
                    Error(expr, loc+(int)(s-olds), OperErr);
            }
            else
            {
                if (firstc=='\\')
                    Error(expr, loc+(int)(s-olds), Undef);
            }

        }
    }

    expr->GetStringLen = d-buf;

    return (char *)s;
}

static void quit_cleanup( EXPR *expr, const char *s)
{
    EXPR_close(expr);       /* free all allocated memory ** redundant code removed */
    puts(s);
    exit(10);
}

static void Error(EXPR *expr, int pos,int errornum)
{
    switch (errornum)
    {
        case Undef:   printf("expression - ERROR UNDEFINED SYMBOL AT POSITION %d\n",pos); break;
        case OperErr: printf("expression - ERROR OPERAND SYNTAX ERROR AT POSITION %d\n",pos); break;
        default:      printf("expression - ERROR # %d AT POSITION %d\n",errornum,pos); break;
    }
    expr->Error=-1;
    return;
}

/****************************************************************/
/*  Expression API entry points                                 */
/****************************************************************/

EXPR_DECAL EXPR* EXPR_open(void)
{
    int                         i;
    int                        templong;
    int                        maxheap2;
    struct SymTab               **hashptr;
    EXPR *expr = NULL;

    expr = (EXPR *) galloc(sizeof(EXPR));
    if (expr)
    {
        memset(expr, 0, sizeof(EXPR));

        InitOpPrec(expr);

        expr->Radix = 10;

        expr->Error = 0;        /* clear expression Error flag */
        expr->Hash = NULL;                                /* Clear all memory pointers - */
        expr->SymStart = NULL;                            /*  we haven't allocated anything yet. */
        expr->NameStart = NULL;
        expr->Heap2 = NULL;

        maxheap2 = DEFHEAP2;                              /* Secondary heap size default */

        /* Allocate initial symbol table chunks. */

        templong = sizeof(void *) * HASHSIZE;
        expr->Hash = (struct SymTab **) galloc(templong);
        if (!expr->Hash)
            quit_cleanup(expr, "Out of memory!\n");
        for (hashptr=expr->Hash, i=0; i<HASHSIZE; hashptr++, i++)
            *hashptr = NULL;                        /* Clear the hash table. */

        expr->SymStart = (struct SymTab *) galloc(CHUNKSIZE);
        if (!expr->SymStart)
            quit_cleanup(expr, "Out of memory!\n");
        expr->SymCurr = expr->SymStart;                   /* Make the first chunk current. */
        expr->SymCurr->Link = NULL;                       /* Clear forward pointer. */
        expr->SymLim = expr->SymCurr;
        expr->SymLim++;                                   /* Start of names */

        expr->NameStart = (struct NameChunk *) galloc(CHUNKSIZE);
        if (!expr->NameStart)
            quit_cleanup(expr, "Out of memory!\n");
        expr->NameCurr = expr->NameStart;                     /* Make the first chunk current. */
        expr->NameCurr->Link = NULL;                          /* Clear forward pointer. */
        expr->NameLim = (char *) expr->NameCurr + sizeof(void *);    /* Start of names */

        /* Allocate the secondary heap (input files and parser stack). */

        expr->Heap2 = (char *) galloc(maxheap2);
        if (!expr->Heap2)
            quit_cleanup(expr, "Out of memory!\n");
        expr->NextFNS = expr->Heap2;
        expr->InF = (struct InFCtl *) (expr->Heap2 + maxheap2);
        expr->InF--;

        expr->plist = (struct ParamList (*)[EXPRMAXDEPTH])galloc(sizeof(struct ParamList [EXPRMAXDEPTH]));

        expr->hasher = HASH_init(65536);

        expr->Functions = (struct ExternalFunc (*)[EXPRMAXFUNCTIONS]) galloc(sizeof(struct ExternalFunc [EXPRMAXFUNCTIONS]));

        expr->ExFuncs=0;


        EXPR_addfunc(expr,"max",   (double (*)(...)) &EXPR_max,1);
        EXPR_addfunc(expr,"min",   (double (*)(...)) &EXPR_min,1);
        EXPR_addfunc(expr,"hypot", (double (*)(...)) &EXPR_hypot,1);
        EXPR_addfunc(expr,"int",   (double (*)(...)) &EXPR_int,2);
        EXPR_addfunc(expr,"round", (double (*)(...)) &EXPR_round,2);
        EXPR_addfunc(expr,"log2",  (double (*)(...)) &EXPR_log2,2);
        EXPR_addfunc(expr,"ceilpow2", (double (*)(...)) &EXPR_ceilpow2,2);
        EXPR_addfunc(expr,"floorpow2", (double (*)(...)) &EXPR_floorpow2,2);
        EXPR_addfunc(expr,"rand",  (double (*)(...)) &EXPR_rand,2);
        EXPR_addfunc(expr,"bswap", (double (*)(...)) &EXPR_bswap,2);

        EXPR_addfunc(expr,"sqrt",  (double (*)(...)) (double (*)(double))sqrt,2);
        EXPR_addfunc(expr,"floor", (double (*)(...)) (double (*)(double))floor,2);
        EXPR_addfunc(expr,"ceil",  (double (*)(...)) (double (*)(double))ceil,2);
        EXPR_addfunc(expr,"sin",   (double (*)(...)) (double (*)(double))sin,2);
        EXPR_addfunc(expr,"cos",   (double (*)(...)) (double (*)(double))cos,2);
        EXPR_addfunc(expr,"tan",   (double (*)(...)) (double (*)(double))tan,2);
        EXPR_addfunc(expr,"asin",  (double (*)(...)) (double (*)(double))asin,2);
        EXPR_addfunc(expr,"acos",  (double (*)(...)) (double (*)(double))acos,2);
        EXPR_addfunc(expr,"atan",  (double (*)(...)) (double (*)(double))atan,2);
        EXPR_addfunc(expr,"exp",   (double (*)(...)) (double (*)(double))exp,2);
        EXPR_addfunc(expr,"log",   (double (*)(...)) (double (*)(double))log,2);
        EXPR_addfunc(expr,"log10", (double (*)(...)) (double (*)(double))log10,2);
        EXPR_addfunc(expr,"pow",   (double (*)(...)) (double (*)(double,double))pow,2);

        EXPR_addfunc(expr,"tolower",(double (*)(...)) &EXPR_tolower,2);
        EXPR_addfunc(expr,"toupper",(double (*)(...)) &EXPR_toupper,2);

    }
    return expr;
}

EXPR_DECAL void EXPR_close(EXPR *expr)
{
    if (expr)
    {
        if (expr->Heap2)     gfree(expr->Heap2);
        if (expr->NameStart) gfree(expr->NameStart);
        while (expr->SymStart)
        {
            struct SymTab *Link=expr->SymStart->Link;

            gfree(expr->SymStart);
            expr->SymStart = Link;
        }
        if (expr->Hash)      gfree(expr->Hash);
        if (expr->plist)     gfree(expr->plist);
        while (expr->ExFuncs)
        {   /* free all allocated libraries */
            (*expr->Functions)[0]=(*expr->Functions)[--expr->ExFuncs];
        }
        if (expr->Functions) gfree(expr->Functions);
        HASH_purge(expr->hasher);
        gfree(expr);
    }
}

EXPR_DECAL void EXPR_set(EXPR *expr, const char *symbol,double value)
{
    struct SymTab *NSym = ReadSymTab(expr,symbol);
    if (NSym)                      /* Look up symbol. */
    {
        NSym->Value =      value;                 /* Value */
    }
    else
    {
        AddSymTab(expr, symbol, value);
    }
}


EXPR_DECAL void EXPR_addfunc(EXPR *expr, const char *fname, double (*fptr)(...), int ftype)
{
    if (expr->ExFuncs<EXPRMAXFUNCTIONS)
    {
        (*expr->Functions)[expr->ExFuncs].functype=ftype;
        (*expr->Functions)[expr->ExFuncs].funcptr=fptr;
        HASH_add(expr->hasher,fname,expr->ExFuncs++);
    }
}

EXPR_DECAL int EXPR_error(EXPR *expr)
{
    return expr->Error;
}

EXPR_DECAL void EXPR_setmode(EXPR *expr, int mode)
{
    expr->Fast = mode;
}

EXPR_DECAL void EXPR_setbase(EXPR *expr, int base)
{
    expr->Radix = base;
}

EXPR_DECAL int EXPR_getbase(EXPR *expr)
{
    return expr->Radix;
}
