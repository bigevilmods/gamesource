// (c) 1994-2007 Electronic Arts Inc.

#include <string.h>
#include "gimex.h"
#include "t16type.h"
#include "t16gimex.h"
#include "codex.h"

#include "refwrite.cpp"

/***********************************************************************************/
/*                           Move a int on all machine                            */
/***********************************************************************************/
#ifdef sgi
#define MOVLONG(d,s) *(d+0)=*(s+0);*(d+1)=*(s+1);*(d+2)=*(s+2);*(d+3)=*(s+3)
#define MOVLONGIN(d,s) MOVLONG(d,s);d=((char*)d)+sizeof(int);s=((char*)s)+sizeof(int)
#else
#ifdef PSX
#define MOVLONG(d,s) *(d+0)=*(s+0);*(d+1)=*(s+1);*(d+2)=*(s+2);*(d+3)=*(s+3)
#define MOVLONGIN(d,s) MOVLONG(d,s);d=((char*)d)+sizeof(int);s=((char*)s)+sizeof(int)
#else
#ifdef MSDOS
#define MOVLONG(d,s) *(int*)(d) = *(int*)(s)
#define MOVLONGIN(d,s) *((int*)(d))++ = *((int*)(s))++
#else
#ifdef applec
#define MOVLONG(d,s) *(int*)(d) = *(int*)(s)
#define MOVLONGIN(d,s) *((int*)(d))++ = *((int*)(s))++
#else
#define MOVLONG(d,s) *(d+0)=*(s+0);*(d+1)=*(s+1);*(d+2)=*(s+2);*(d+3)=*(s+3)
#define MOVLONGIN(d,s) MOVLONG(d,s);d=((char*)d)+sizeof(int);s=((char*)s)+sizeof(int)
#endif
#endif
#endif
#endif

#define TGVRowBytes(width) (width*sizeof(int))

/***********************************************************************************/
/*           Calcul all the error in advance to speed up the scane                 */
/***********************************************************************************/

#define CDelta(a,b) (((int)a)-((int)b))

#define CalcEChanel(a,b) ((unsigned int) (CDelta(a,b)*CDelta(a,b)))

#define Calc16DeltaColour(c1,c2) CalcEChanel(c1.r,c2.r)+CalcEChanel(c1.g,c2.g)+CalcEChanel(c1.b,c2.b)

static void CalculAllError(T16GimexData* tgv)
{
    unsigned char   Chanel1;
    unsigned char   Chanel2;
    int            Index;

    for (Index=0; Index<kMaxNumError; Index++)
    {
        Chanel1=(unsigned char) (Index&0x000000FF);         /* All the combinaison of values for 2 chanel */
        Chanel2=(unsigned char) ((Index>>8)&0x000000FF);
        tgv->fError[Index]=CalcEChanel(Chanel1,Chanel2);    /* Store the result in the list of error */
    }
}


/***********************************************************************************/
/*           Calcul a delta using a vector and the previous picture                */
/***********************************************************************************/

#define GetCell(p,n) (*(unsigned char*) (((unsigned char*)(p))+n))

#define GetHightIndex(s,n) (((unsigned int) GetCell(s,n))<<8)

#define GetLowIndex(p,n) ((unsigned int) GetCell(p,n))

#define GetErrorRGB(e,s,p,n) e[GetHightIndex(s,n)|GetLowIndex(p,n)]

#define GetError(e,s,p,n) GetErrorRGB(e,s,p,n+1)+GetErrorRGB(e,s,p,n+2)+GetErrorRGB(e,s,p,n+3)

#define GetErrorLine(e,s,p) GetError(e,s,p,0)+GetError(e,s,p,4)+GetError(e,s,p,8)+GetError(e,s,p,12)

#define CalcDelta8(ListError,ScaneSource,RowByteS,ScanePrev,RowByteP,Delta) \
{\
    Delta=GetErrorLine(ListError,ScaneSource,ScanePrev);\
    ScaneSource+=RowByteS;\
    ScanePrev+=RowByteP;\
    Delta+=GetErrorLine(ListError,ScaneSource,ScanePrev);\
    ScaneSource+=RowByteS;\
    ScanePrev+=RowByteP;\
    Delta+=GetErrorLine(ListError,ScaneSource,ScanePrev);\
    ScaneSource+=RowByteS;\
    ScanePrev+=RowByteP;\
    Delta+=GetErrorLine(ListError,ScaneSource,ScanePrev);\
}

/***********************************************************************************/
/*              Search the best delta in the whole previous picture                */
/***********************************************************************************/

static void GetBestDelta8(unsigned int* ListError,char* Source,int SRowbytes,char* Prev,int PRowbytes,int w,int h,unsigned int* BestMatch,MVector* MotionVector)
{
    int             x,y,fx,fy;
    unsigned int   fBestMatch;
    unsigned int   Match;
    char*           ScanePrev;
    char*           ScaneSource0;
    char*           ScaneSource1;
    char*           ScaneSource2;
    char*           ScaneSource3;

    ScaneSource0=Source;
    ScaneSource1=ScaneSource0+SRowbytes;
    ScaneSource2=ScaneSource1+SRowbytes;
    ScaneSource3=ScaneSource2+SRowbytes;
    fx=0;
    fy=0;
    fBestMatch=kLowestMatch;                        /* To be sure to match on the first iteration */
    for (y=0; y<h; y++)
    {
        for (x=0; x<w; x++)                         /* For each point x,y of the previous picture */
        {
            ScanePrev=Prev+TGVRowBytes(x);
            Match=GetErrorLine(ListError,ScaneSource0,ScanePrev);
            if (Match<fBestMatch)
            {
                ScanePrev+=PRowbytes;
                Match+=GetErrorLine(ListError,ScaneSource1,ScanePrev);
                if (Match<fBestMatch)
                {
                    ScanePrev+=PRowbytes;
                    Match+=GetErrorLine(ListError,ScaneSource2,ScanePrev);
                    if (Match<fBestMatch)
                    {
                        ScanePrev+=PRowbytes;
                        Match+=GetErrorLine(ListError,ScaneSource3,ScanePrev);
                        if (Match<fBestMatch)       /* New better match */
                        {
                            fx=x;                   /* The vector is done in absolute coordinate */
                            fy=y;                   /* from the topLeft of the previous picture */
                            fBestMatch=Match;
                        }
                    }
                }
            }
        }
        Prev+=PRowbytes;
        if (!fBestMatch) y=h;                       /* Exact match, so abord the loop (MinError can be used here) */
    }
    *BestMatch=fBestMatch;                          /* Save the value */
    MotionVector->x=fx;
    MotionVector->y=fy;
}


/***********************************************************************************/
/*                           Draw and load literal and vectors                     */
/***********************************************************************************/

static void LoadLiteral(char* TheLiteral,int x,int y,char* Source,int Rowbytes)
{
    char*   Scr;

    Source+=y*Rowbytes+TGVRowBytes(x);
    Scr=Source;
    MOVLONGIN(TheLiteral,Scr);
    MOVLONGIN(TheLiteral,Scr);
    MOVLONGIN(TheLiteral,Scr);
    MOVLONGIN(TheLiteral,Scr);
    Scr=Source+Rowbytes;
    MOVLONGIN(TheLiteral,Scr);
    MOVLONGIN(TheLiteral,Scr);
    MOVLONGIN(TheLiteral,Scr);
    MOVLONGIN(TheLiteral,Scr);
    Scr=Source+Rowbytes*2;
    MOVLONGIN(TheLiteral,Scr);
    MOVLONGIN(TheLiteral,Scr);
    MOVLONGIN(TheLiteral,Scr);
    MOVLONGIN(TheLiteral,Scr);
    Scr=Source+Rowbytes*3;
    MOVLONGIN(TheLiteral,Scr);
    MOVLONGIN(TheLiteral,Scr);
    MOVLONGIN(TheLiteral,Scr);
    MOVLONGIN(TheLiteral,Scr);
}


static void DrawLiteral(char* TheLiteral,int x,int y,char* Destination,int Rowbytes)
{
    char*   Dest;

    Destination+=y*Rowbytes+TGVRowBytes(x);
    Dest=Destination;
    MOVLONGIN(Dest,TheLiteral);
    MOVLONGIN(Dest,TheLiteral);
    MOVLONGIN(Dest,TheLiteral);
    MOVLONGIN(Dest,TheLiteral);
    Dest=Destination+Rowbytes;
    MOVLONGIN(Dest,TheLiteral);
    MOVLONGIN(Dest,TheLiteral);
    MOVLONGIN(Dest,TheLiteral);
    MOVLONGIN(Dest,TheLiteral);
    Dest=Destination+Rowbytes*2;
    MOVLONGIN(Dest,TheLiteral);
    MOVLONGIN(Dest,TheLiteral);
    MOVLONGIN(Dest,TheLiteral);
    MOVLONGIN(Dest,TheLiteral);
    Dest=Destination+Rowbytes*3;
    MOVLONGIN(Dest,TheLiteral);
    MOVLONGIN(Dest,TheLiteral);
    MOVLONGIN(Dest,TheLiteral);
    MOVLONGIN(Dest,TheLiteral);
}


static void DrawMatrix8(char* Destination,int DRowbytes,char* Source,int SRowbytes)
{
    char*   Dest;
    char*   Scr;

    Dest=Destination;
    Scr=Source;
    MOVLONGIN(Dest,Scr);
    MOVLONGIN(Dest,Scr);
    MOVLONGIN(Dest,Scr);
    MOVLONGIN(Dest,Scr);
    Dest=Destination+DRowbytes;
    Scr=Source+SRowbytes;
    MOVLONGIN(Dest,Scr);
    MOVLONGIN(Dest,Scr);
    MOVLONGIN(Dest,Scr);
    MOVLONGIN(Dest,Scr);
    Dest=Destination+DRowbytes*2;
    Scr=Source+SRowbytes*2;
    MOVLONGIN(Dest,Scr);
    MOVLONGIN(Dest,Scr);
    MOVLONGIN(Dest,Scr);
    MOVLONGIN(Dest,Scr);
    Dest=Destination+DRowbytes*3;
    Scr=Source+SRowbytes*3;
    MOVLONGIN(Dest,Scr);
    MOVLONGIN(Dest,Scr);
    MOVLONGIN(Dest,Scr);
    MOVLONGIN(Dest,Scr);
}


/***********************************************************************************/
/*                             Vector list management                              */
/***********************************************************************************/

static void AddMotionVector(T16Enviro* Bvm,MVector MotionVector)
{
    int     i,InvalVector;

    if (Bvm->TheVector->NbMotionVector==Bvm->TheVector->MaxMotionVector)                        /* The stack is full */
    {
        InvalVector=Bvm->TheVector->NbMotionVector-1;                                           /* The bottom vector is invalid by default */
        for (i=Bvm->TheVector->NbMotionVector-2; i>=kReservVector; i--)                         /* Scane the whole list of vector (Bottom to the top of the list) */
        {
            if (Bvm->MVFrequency->Frequency[i]<=Bvm->MVFrequency->Frequency[InvalVector])
            {
                InvalVector=i;                                                                  /* Search the lowest "Frequence", closed to the top of the list */
            }
        }
        for (i=InvalVector; i<Bvm->TheVector->NbMotionVector-1; i++)
        {
            Bvm->TheVector->ListMV[i]=Bvm->TheVector->ListMV[i+1];                              /* Move the stack to the bottom */
            Bvm->MVFrequency->Frequency[i]=Bvm->MVFrequency->Frequency[i+1];                    /* Move the stack to the bottom */
        }
        Bvm->TheVector->NbMotionVector--;                                                       /* Delete the "InvalVector" */
    }
    Bvm->MVFrequency->Frequency[Bvm->TheVector->NbMotionVector]=1;                              /* Used one time */
    Bvm->TheVector->ListMV[Bvm->TheVector->NbMotionVector]=MotionVector;                        /* memorized the value of the vector */
    (Bvm->TheVector->NbMotionVector)++;
}


static void SearchMotionVector(unsigned int* ListError,T16Enviro* Bvm,char* Source,int SRowbytes,char* Prev,int PRowbytes,int x,int y,unsigned int* BestMatch,int* BVIndex)
{
    MVector         MotionVector;
    unsigned int   Match,fBestMatch;
    int             fBVIndex,i,xMax,yMax,xx,yy;
    char*           MovePrev;
    char*           MoveSource;

    fBestMatch=kLowestMatch;                                                /* To be sure to match on the first iteration */
    fBVIndex=kInvalidIndex;
    xMax=(Bvm->TheMatrix->xMatrix-1)*kXRes;                                 /* Bounds of the picture minus one matrix of kXRes*kYRes */
    yMax=(Bvm->TheMatrix->yMatrix-1)*kYRes;
    for (i=0; i<Bvm->TheVector->NbMotionVector; i++)                        /* Scane the whole list of vector (Top to bottom) */
    {
        MotionVector=Bvm->TheVector->ListMV[i];
        xx=x+MotionVector.x;
        yy=y+MotionVector.y;
        if (xx<=xMax)                                                       /* Valid vector */
        {
            if (yy<=yMax)                                                   /* Valid vector */
            {
                if (xx>=0)                                                  /* Valid vector */
                {
                    if (yy>=0)                                              /* Valid vector */
                    {
                        MovePrev=Prev+yy*PRowbytes+TGVRowBytes(xx);
                        MoveSource=Source;
                        Match=GetErrorLine(ListError,MoveSource,MovePrev);
                        if (Match<fBestMatch)
                        {
                            MoveSource+=SRowbytes;
                            MovePrev+=PRowbytes;
                            Match+=GetErrorLine(ListError,MoveSource,MovePrev);
                            if (Match<fBestMatch)
                            {
                                MoveSource+=SRowbytes;
                                MovePrev+=PRowbytes;
                                Match+=GetErrorLine(ListError,MoveSource,MovePrev);
                                if (Match<fBestMatch)
                                {
                                    MoveSource+=SRowbytes;
                                    MovePrev+=PRowbytes;
                                    Match+=GetErrorLine(ListError,MoveSource,MovePrev);
                                    if (Match<fBestMatch)                   /* New better match or initialization */
                                    {
                                        fBestMatch=Match;
                                        fBVIndex=i;
                                        if (!Match)                         /* Exact match, so abord the loop */
                                        {
                                            i=Bvm->TheVector->NbMotionVector;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    *BestMatch=fBestMatch;                                                  /* Save the value */
    *BVIndex=fBVIndex;
}


/***********************************************************************************/
/*                            Literal list management                              */
/***********************************************************************************/

static void AddLiteral(DLiteral* TheDictionary,FrequencyRec* TheFrequency,LiteralMatrix* TheLiteral,int* BLIndex)
{
    *BLIndex=TheDictionary->NbLiteral;
    TheDictionary->ListLiteral[TheDictionary->NbLiteral]=*TheLiteral;   /* memorized the value of the literal */
    TheFrequency->Frequency[TheDictionary->NbLiteral]=0;                /* It is not used for now */
    (TheDictionary->NbLiteral)++;
}


static void SearchLiteral(unsigned int* ListError,DLiteral* TheDictionary,LiteralMatrix* TheLiteral,unsigned int* BestMatch,int* BLIndex)
{
    int             i,fBLIndex;
    unsigned int   Match,fBestMatch;
    char*           Prev;
    char*           Source;

    fBestMatch=kLowestMatch;
    fBLIndex=kInvalidIndex;
    Source=(char*)TheLiteral;
    for (i=0; i<TheDictionary->NbLiteral; i++)                          /* Scanne the dictionary */
    {
        Prev=(char*)&(TheDictionary->ListLiteral[i]);
        Match=GetErrorLine(ListError,Source,Prev);
        if (Match<fBestMatch)
        {
            Match+=GetErrorLine(ListError,(Source+16),(Prev+16));
            if (Match<fBestMatch)
            {
                Match+=GetErrorLine(ListError,(Source+32),(Prev+32));
                if (Match<fBestMatch)
                {
                    Match+=GetErrorLine(ListError,(Source+48),(Prev+48));
                    if (Match<fBestMatch)                               /* New better match or initialization */
                    {
                        fBestMatch=Match;
                        fBLIndex=i;
                        if (!Match) i=TheDictionary->NbLiteral;         /* Exact match, so abord the loop */
                    }
                }
            }
        }
    }
    *BLIndex=fBLIndex;
    *BestMatch=fBestMatch;
}


/****************************************************************/
/*                    Low level Functions                       */
/****************************************************************/

#ifdef MSDOS

#define ggetintellongT16(p) *(unsigned int*) (p)

#define gsetintellongT16(p,v) *(unsigned int*)(p)=v

#else

#define ggetintellongT16(p) \
   (((unsigned int)(*(unsigned char *) (((char*)(p))+0))<<0) \
  | ((unsigned int)(*(unsigned char *) (((char*)(p))+1))<<8) \
  | ((unsigned int)(*(unsigned char *) (((char*)(p))+2))<<16) \
  | ((unsigned int)(*(unsigned char *) (((char*)(p))+3))<<24))

#define gsetintellongT16(p,v) \
    *(unsigned char*)(((char*)(p))+0)=((v>>0)&0x000000FF); \
    *(unsigned char*)(((char*)(p))+1)=((v>>8)&0x000000FF); \
    *(unsigned char*)(((char*)(p))+2)=((v>>16)&0x000000FF); \
    *(unsigned char*)(((char*)(p))+3)=((v>>24)&0x000000FF)

#endif

#define getfieldT16(ptr,bitoff,n) ((ggetintellongT16((ptr)+((bitoff)>>3)) >> ((bitoff)&7)) & ((1<<n)-1))

static void setfieldT16(char *ptr,unsigned int bitoff,unsigned int n,unsigned int v)
{
    unsigned int   m;

    ptr+=(bitoff>>3);
    m=(1<<n)-1;                                 /* n bits of 1s */
    v<<=(bitoff&7);                             /* position correctly in bit stream */
    m<<=(bitoff&7);                             /* position correctly in bit stream */
    v=(ggetintellongT16(ptr) & ~m) | (v & m);   /* read and combine bits together */
    gsetintellongT16(ptr,v);                    /* write it out */
}


/****************************************************************/
/*                Compress and Decompress the data              */
/****************************************************************/

static int CompressDataDepth(char* CPtr,int NbCell,int CDepth)
{
    int                i;
    unsigned int*       ShortPtr;

    ShortPtr=(unsigned int*)CPtr;
    for (i=0; i<NbCell; i++) setfieldT16(CPtr,i*CDepth,CDepth,(unsigned int)ShortPtr[i]);
    return(1);
}


static void DecompressDataDepth(char* CPtr,int NbCell,int CDepth,int SignCode)
{
    int     i;
    int     bitoff;
    int    signbit;
    int    Value;
    int    NegVal;
    int*    d;

    bitoff = (NbCell-1)*CDepth;
    d = (int *)CPtr + (NbCell);
    if (SignCode)
    {
        if (SignCode) signbit = 1<<(CDepth-1);
        else signbit = 0;
        NegVal = (0xFFFFFFFF<<CDepth);
        for (i=0; i<NbCell; i++)
        {
            Value = getfieldT16(CPtr,bitoff,CDepth);
            bitoff -= CDepth;
            if (Value&signbit) Value|=NegVal; /* Complement to 32 */
            *--d = (int) Value;
        }
    }
    else
    {
        for (i=0; i<NbCell; i++)
        {
            Value = getfieldT16(CPtr,bitoff,CDepth);
            bitoff -= CDepth;
            *--d = (int) Value;
        }
    }
}


static int CalcC16Cell(int NbCell,int CDepth)
{
    int    Result;

    Result=((NbCell*CDepth)+7)/8;   /* Size in byte */
    Result=(Result+3)/4;            /* Nb int align on 32 */
    return(Result);
}


/***********************************************************************************/
/*                      Calcul the optimal depth of the Matrix                     */
/***********************************************************************************/

static int OptimalDepthMatrix(int NbMV,int NbL,int NbLL,int Extra)
{
    int         NewDepth,MaxIndex;

    MaxIndex=NbMV+NbL+NbLL+Extra;               /* Total number of elements references by an index in the matrix */
    NewDepth=1;                                 /* The minimum depth */
    while (MaxIndex>(1<<NewDepth)) NewDepth++;
    return(NewDepth);
}


/***********************************************************************************/
/*                Calcul the exact size of each element in the file                */
/***********************************************************************************/

static int GetSizeHeader()
{
    return(sizeof(T16FrameStruct));
}

static int GetSizeVector(T16Enviro* Bvm)
{
    int    TheSize;

    TheSize=CalcC16Cell(Bvm->TheVector->NbMotionVector*2,kCompressVector)*sizeof(int);
    return(TheSize);
}


static int GetSizeMatrix(T16Enviro* Bvm,int Extra)
{
    int    TheSize,NbCellMatrix,MatrixDepth;

    NbCellMatrix=Bvm->TheMatrix->xMatrix*Bvm->TheMatrix->yMatrix;
    MatrixDepth=OptimalDepthMatrix( Bvm->TheVector->NbMotionVector,
                                    Bvm->TheHighLiteral->NbLiteral,
                                    Bvm->TheLowLiteral->NbLiteral,
                                    Extra);
    TheSize=(CalcC16Cell(NbCellMatrix,MatrixDepth)*sizeof(int));
    return(TheSize);
}


static int GetSizeLiterale(T16Enviro* Bvm)
{
    int    TheSize;

    TheSize=(((int)Bvm->TheHighLiteral->NbLiteral)*sizeof(HighLiteralMatrix));     /* 16 short * NbLiteral */
    TheSize+=(((int)Bvm->TheLowLiteral->NbLiteral)*sizeof(LowLiteralMatrix));      /* Low res literal * NbLiteral */
    return(TheSize);
}


static int GetInitSizeVector(int xMatrix,int yMatrix,unsigned int SizeRate)
{
    int    TheSize,NbCellMatrix;
    int    MatrixDepth,NbMotionVector;

    SizeRate=(SizeRate*kInitSizeVector)/100;                                        /* This value allow to save some space for the literal */
    NbCellMatrix=xMatrix*yMatrix;
    NbMotionVector=NbCellMatrix;                                                    /* This is the maximum number of vector */
    do
    {
        TheSize=GetSizeHeader();                                                    /* Size of the header */
        MatrixDepth=OptimalDepthMatrix(NbMotionVector,0,0,0);
        TheSize+=(CalcC16Cell(NbCellMatrix,MatrixDepth)*sizeof(int));              /* Size of the matrix */
        TheSize+=CalcC16Cell(NbMotionVector*2,kCompressVector)*sizeof(int);        /* Size of the vectors */
        if (TheSize>SizeRate) NbMotionVector-=xMatrix;
    } while (TheSize>SizeRate);
    if (NbMotionVector<(kReservVector+1)) NbMotionVector=kReservVector+1;           /* There is a minimum of 1 vector and the kReservVector */
    return(NbMotionVector);
}


static int MoreLiteral(T16Enviro* Bvm,unsigned int SizeRate)
{
    int    TheSize;

    TheSize=GetSizeHeader();            /* Header */
    TheSize+=GetSizeVector(Bvm);        /* Vector */
    TheSize+=GetSizeLiterale(Bvm);      /* Literale */
    TheSize+=GetSizeMatrix(Bvm,1);      /* Matrix */
    TheSize+=sizeof(HighLiteralMatrix); /* The size of the next literal */
    if (Bvm->TheHighLiteral->NbLiteral<Bvm->TheHighLiteral->MaxLiteral)
    {
        if (Bvm->TheLowLiteral->NbLiteral<Bvm->TheLowLiteral->MaxLiteral)
        {
            if (TheSize<=SizeRate) return(1);
        }
    }
    return(0);
}


/***********************************************************************************/
/*                                Matrix management                                */
/***********************************************************************************/

static void IndexMatrixToXY(T16Enviro* Bvm,int Index,int* xScan,int* yScan)
{
    *yScan=Index/Bvm->TheMatrix->xMatrix;
    *xScan=Index-((*yScan)*Bvm->TheMatrix->xMatrix);
}


/***********************************************************************************/
/*                               Dictionary management                             */
/***********************************************************************************/

static int CheckAllDictionary(T16Enviro* Bvm)
{
    int     i,CheckTotal;

    if (TGVDEBUG)
    {
        CheckTotal=0;
        for (i=0; i<Bvm->TheVector->NbMotionVector; i++)
        {
            CheckTotal+=Bvm->MVFrequency->Frequency[i];                     /* The total of elements is all the matrix */
        }
        for (i=0; i<Bvm->TheHighLiteral->NbLiteral; i++)
        {
            CheckTotal+=Bvm->HLFrequency->Frequency[i];                     /* The total of elements is all the matrix */
        }
        for (i=0; i<Bvm->TheLowLiteral->NbLiteral; i++)
        {
            CheckTotal+=Bvm->LLFrequency->Frequency[i];                     /* The total of elements is all the matrix */
        }
        if (CheckTotal!=(Bvm->TheMatrix->xMatrix*Bvm->TheMatrix->yMatrix))
        {
            return(0);
        }
    }
    return(1);
}


static int CompressDictionary(T16Enviro* Bvm,int TypeDictionary)
{
    int            SizeRemap,MaxElem;
    FrequencyRec*   TheFrequency;
    int*            RemapIndex;
    int             DoRemap=0;
    int             MaxCell,i,NewIndex;
    WkMatrix*       WorkMatrix;

    switch (TypeDictionary)                                                                             /* Dispatch on the right type of dictionary */
    {
        case kMotionVector:
            MaxElem=Bvm->TheVector->NbMotionVector;
            Bvm->TheVector->NbMotionVector=0;                                                           /* Reset the counter */
            TheFrequency=Bvm->MVFrequency;
            break;
        case kHighLiteral:
            MaxElem=Bvm->TheHighLiteral->NbLiteral;
            Bvm->TheHighLiteral->NbLiteral=0;                                                           /* Reset the counter */
            TheFrequency=Bvm->HLFrequency;
            break;
        case kLowLiteral:
            MaxElem=Bvm->TheLowLiteral->NbLiteral;
            Bvm->TheLowLiteral->NbLiteral=0;                                                            /* Reset the counter */
            TheFrequency=Bvm->LLFrequency;
            break;
    }
    SizeRemap=sizeof(int)*MaxElem;
    RemapIndex=(int*)galloc(SizeRemap);                                                                 /* Allocate the remap structure */
    if (RemapIndex)
    {
        NewIndex=0;
        for (i=0; i<MaxElem; i++)                                                                       /* Build a list of remapping values */
        {
            if (TheFrequency->Frequency[i])                                                             /* Keep this element */
            {
                switch (TypeDictionary)                                                                 /* Dispatch on the right type of dictionary */
                {
                    case kMotionVector:
                        Bvm->TheVector->ListMV[NewIndex]=Bvm->TheVector->ListMV[i];                     /* Compress the list of vector */
                        (Bvm->TheVector->NbMotionVector)++;                                             /* Increment the counter */
                        break;
                    case kHighLiteral:
                        Bvm->TheHighLiteral->ListLiteral[NewIndex]=Bvm->TheHighLiteral->ListLiteral[i]; /* Compress the list of literal */
                        (Bvm->TheHighLiteral->NbLiteral)++;                                             /* Increment the counter */
                        break;
                    case kLowLiteral:
                        Bvm->TheLowLiteral->ListLiteral[NewIndex]=Bvm->TheLowLiteral->ListLiteral[i];   /* Compress the list of literal */
                        (Bvm->TheLowLiteral->NbLiteral)++;                                              /* Increment the counter */
                        break;
                }
                TheFrequency->Frequency[NewIndex]=TheFrequency->Frequency[i];                           /* Compress the list of frequence */
                RemapIndex[i]=NewIndex;                                                                 /* Save the valid index */
                NewIndex++;
            }
            else                                                                                        /* Delete it */
            {
                RemapIndex[i]=kInvalidIndex;                                                            /* This value should not be used */
                DoRemap=1;                                                                              /* There is at least one remap to do */
            }
        }
        if (DoRemap)
        {
            MaxCell=Bvm->TheMatrix->yMatrix*Bvm->TheMatrix->xMatrix;
            WorkMatrix=Bvm->WorkMatrix;
            for (i=0; i<MaxCell; i++)
            {
                if (WorkMatrix[i].TypeDictionary==TypeDictionary)                                       /* It is the right type of element */
                {
                    NewIndex=RemapIndex[Bvm->TheMatrix->MXIndex[i]];
                    Bvm->TheMatrix->MXIndex[i]=NewIndex;                                                /* Remap the value */
                }
            }
        }
        gfree((char*)RemapIndex);
        return(1);
    }
    return(0);
}


static void ClearInDictionary(T16Enviro* Bvm,int TypeDictionary,int IndexElem)
{
    int             LastIndex,MaxCell,i;
    FrequencyRec*   TheFrequency;
    WkMatrix*       WorkMatrix;

    switch (TypeDictionary)                                                                             /* Dispatch on the right type of dictionary */
    {
        case kMotionVector:TheFrequency=Bvm->MVFrequency;break;
        case kHighLiteral:TheFrequency=Bvm->HLFrequency;break;
        case kLowLiteral:TheFrequency=Bvm->LLFrequency;break;
    }
    (TheFrequency->Frequency[IndexElem])--;                                                             /* Dicrease the frequency */
    if (TheFrequency->Frequency[IndexElem]==0)                                                          /* This element is not used anymore */
    {
        switch (TypeDictionary)                                                                         /* Dispatch on the right type of dictionary */
        {
            case kMotionVector:
                (Bvm->TheVector->NbMotionVector)--;                                                     /* Decrement the counter */
                LastIndex=Bvm->TheVector->NbMotionVector;
                Bvm->TheVector->ListMV[IndexElem]=Bvm->TheVector->ListMV[LastIndex];                    /* Compress the list of vector */
                break;
            case kHighLiteral:
                (Bvm->TheHighLiteral->NbLiteral)--;                                                     /* Decrement the counter */
                LastIndex=Bvm->TheHighLiteral->NbLiteral;
                Bvm->TheHighLiteral->ListLiteral[IndexElem]=Bvm->TheHighLiteral->ListLiteral[LastIndex];/* Compress the list of literal */
                break;
            case kLowLiteral:
                (Bvm->TheLowLiteral->NbLiteral)--;                                                      /* Decrement the counter */
                LastIndex=Bvm->TheLowLiteral->NbLiteral;
                Bvm->TheLowLiteral->ListLiteral[IndexElem]=Bvm->TheLowLiteral->ListLiteral[LastIndex];  /* Compress the list of literal */
                break;
        }
        TheFrequency->Frequency[IndexElem]=TheFrequency->Frequency[LastIndex];                          /* Compress the list of frequence */
        MaxCell=Bvm->TheMatrix->yMatrix*Bvm->TheMatrix->xMatrix;
        WorkMatrix=Bvm->WorkMatrix;
        for (i=0; i<MaxCell; i++)
        {
            if (WorkMatrix[i].TypeDictionary==TypeDictionary)                                           /* It is the right type of element */
            {
                if (Bvm->TheMatrix->MXIndex[i]==LastIndex)                                              /* Swap the last element with the invalid one */
                {
                    Bvm->TheMatrix->MXIndex[i]=IndexElem;
                }
            }
        }
    }
}


/***********************************************************************************/
/*      Search the best set of vector for this picture, using the previous one     */
/***********************************************************************************/

static void BuildBaseVector(T16Enviro* Bvm)
{
    MVector         MotionVector;
    int             x,y,Max,NbMotionVector;

    MotionVector.x=0;                                                       /* The reserved vector is 0,0 */
    MotionVector.y=0;                                                       /* To be sure to match at least one time */
    NbMotionVector=Bvm->TheVector->NbMotionVector;
    Bvm->MVFrequency->Frequency[NbMotionVector]=0;                          /* Used 0 time */
    Bvm->TheVector->ListMV[NbMotionVector]=MotionVector;                    /* memorized the value of this vector */
    NbMotionVector++;
    Max=(kDefFocus*2)+1;                                                    /* Build the vector in focus */
    Max*=Max;                                                               /* Square of this line */
    if (Max<Bvm->TheVector->MaxMotionVector)                                /* Enought vector to build the base */
    {
        for (x=(-1*kDefFocus); x<=kDefFocus; x++)                           /* Build all the vectors of the focus area */
        {
            for (y=(-1*kDefFocus); y<=kDefFocus; y++)
            {
                if (x&y)                                                    /* The nul vector already exist */
                {
                    MotionVector.x=x;
                    MotionVector.y=y;
                    Bvm->MVFrequency->Frequency[NbMotionVector]=0;
                    Bvm->TheVector->ListMV[NbMotionVector]=MotionVector;
                    NbMotionVector++;                                       /* Add this vector in the default dictionary */
                }
            }
        }
    }
    Bvm->TheVector->NbMotionVector=NbMotionVector;
}


static int BuildVectorDictionary(T16GimexData* tgv,T16Enviro* Bvm,char* Source,int Rowbytes)
{
    int             xScan,yScan,w,h,x,y,BVIndex;
    unsigned int   MinError,BestMatch,FindBestMatch;
    MVector         MotionVector;
    char*           Scr;

    MinError=CalcEChanel(kTVisual,0)*3;
    MinError=MinError*kXResYRes;
    BuildBaseVector(Bvm);
    w=(Bvm->TheMatrix->xMatrix-1)*kXRes;                                        /* Bounds of the picture minus one matrix of kXRes*kYRes */
    h=(Bvm->TheMatrix->yMatrix-1)*kYRes;
    for (yScan=0; yScan<Bvm->TheMatrix->yMatrix; yScan++)                       /* Scanne the array of matrix in the array resolution */
    {
        GIMEXIDLE;
        for (xScan=0; xScan<Bvm->TheMatrix->xMatrix; xScan++)
        {
            x=xScan*kXRes;
            y=yScan*kYRes;
            Scr=Source+y*Rowbytes+TGVRowBytes(x);                               /* Same source for the whole search */
            SearchMotionVector( tgv->fError,Bvm,
                                Scr,Rowbytes,
                                tgv->fPrevSource,TGVRowBytes(tgv->fWidth),
                                x,y,&BestMatch,&BVIndex);
            if ((BestMatch<=MinError) && (BVIndex!=kInvalidIndex))
            {
                (Bvm->MVFrequency->Frequency[BVIndex])++;                       /* This vector is used one more time */
            }
            else
            {
                GetBestDelta8(  tgv->fError,
                                Scr,Rowbytes,
                                tgv->fPrevSource,TGVRowBytes(tgv->fWidth),
                                w,h,&FindBestMatch,&MotionVector);
                if (FindBestMatch<BestMatch)                                    /* Better match that in the dictionnary (avoid the dupplicate) */
                {
                    MotionVector.x-=x;                                          /* The vector is convert in relative coordinate */
                    MotionVector.y-=y;
                    AddMotionVector(Bvm,MotionVector);                          /* Let try a new UNIC entry */
                }
                else (Bvm->MVFrequency->Frequency[BVIndex])++;                  /* This vector is used one more time */
            }
        }
    }
    return(1);
}


/***********************************************************************************/
/*                                 Remap the vector                                */
/***********************************************************************************/

static int DelDupplicateVectors(T16GimexData* tgv,T16Enviro* Bvm,MVector* BLocation)
{
    int                MaxElem,i,j;
    int                SizeMultiVector;
    int                 BLIndex,TypeDictionary;
    int                 SaveBLIndex,SaveTypeDictionary;
    MVector             BlRef;
    LiteralMatrix       TheLiteral;

    MaxElem=Bvm->TheMatrix->xMatrix*Bvm->TheMatrix->yMatrix;                                    /* The maximum number of cell in the matrix */
    for (i=0; i<MaxElem; i++)                                                                   /* Scane the matrix */
    {
        SizeMultiVector=0;                                                                      /* Reset the counter */
        BlRef=BLocation[i];                                                                     /* The reference location is the first bloc */
        if ((BlRef.x!=-1) && (BlRef.y!=-1))                                                     /* Already done */
        {
            for (j=i; j<MaxElem; j++)                                                           /* Scane all the cell from this index */
            {
                if (BLocation[j].x==BlRef.x)
                {
                    if (BLocation[j].y==BlRef.y)                                                /* The location match */
                    {
                        if (Bvm->WorkMatrix[j].TypeDictionary==kMotionVector)                   /* The type dictionary match */
                        {
                            if (Bvm->MVFrequency->Frequency[Bvm->TheMatrix->MXIndex[j]]==1)     /* The vector is used 1 time */
                            {
                                SizeMultiVector++;                                              /* One more location that match */
                            }
                        }
                    }
                }
            }
            SizeMultiVector=(SizeMultiVector*kCompressVector*2)/8;                              /* The position uses 2 vectors of 10 bits */
            if (SizeMultiVector>=sizeof(HighLiteralMatrix))                                     /* The vectors using the same bloc are bigger that the bloc itself */
            {
                LoadLiteral(    (char*)&TheLiteral,BlRef.x,BlRef.y,
                                tgv->fPrevSource,TGVRowBytes(tgv->fWidth));                     /* Load this literal bloc */
                TypeDictionary=kHighLiteral;                                                    /* Use a full literal */
                AddLiteral(Bvm->TheHighLiteral,Bvm->HLFrequency,&TheLiteral,&BLIndex);          /* Add a new literal in the list */
                for (j=i; j<MaxElem; j++)                                                       /* Scane all the cell from this index */
                {
                    if (BLocation[j].x==BlRef.x)
                    {
                        if (BLocation[j].y==BlRef.y)
                        {
                            if (Bvm->WorkMatrix[j].TypeDictionary==kMotionVector)               /* The type dictionary match */
                            {
                                if (Bvm->MVFrequency->Frequency[Bvm->TheMatrix->MXIndex[j]]==1) /* The vector is used 1 time */
                                {
                                    SaveBLIndex=Bvm->TheMatrix->MXIndex[j];                     /* Save the index */
                                    SaveTypeDictionary=Bvm->WorkMatrix[j].TypeDictionary;       /* Save type dictionary */
                                    (Bvm->HLFrequency->Frequency[BLIndex])++;                   /* This literal is used one more time */
                                    Bvm->TheMatrix->MXIndex[j]=BLIndex;                         /* Update the index */
                                    Bvm->WorkMatrix[j].TypeDictionary=TypeDictionary;           /* Update type dictionary */
                                    ClearInDictionary(Bvm,SaveTypeDictionary,SaveBLIndex);      /* Clear the unsused element in the right dictionary */
                                    BLocation[j].x=-1;
                                    BLocation[j].y=-1;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return(CheckAllDictionary(Bvm));
}


static int RemapAllVectors(T16GimexData* tgv,T16Enviro* Bvm,char* Source,int Rowbytes)
{
    int             xScan,yScan,x,y;
    int             BVIndex,MaxCell,i;
    unsigned int   BestMatch;
    char*           Scr;
    MVector*        BLocation;
    MVector         BlRef;
    int             CheckError=0;
    WkMatrix*       WorkMatrix;

    MaxCell=Bvm->TheMatrix->yMatrix*Bvm->TheMatrix->xMatrix;
    WorkMatrix=Bvm->WorkMatrix;
    for (i=0; i<Bvm->TheVector->NbMotionVector; i++) Bvm->MVFrequency->Frequency[i]=0;  /* Reset the frequence */
    BLocation=(MVector*)galloc(MaxCell*sizeof(MVector));                                /* Allocate the bloc used matrix */
    if (BLocation)
    {
        for (i=0; i<MaxCell; i++)
        {
            IndexMatrixToXY(Bvm,i,&xScan,&yScan);
            x=xScan*kXRes;
            y=yScan*kYRes;
            Scr=Source+y*Rowbytes+TGVRowBytes(x);                                       /* Same source for the whole search */
            SearchMotionVector( tgv->fError,Bvm,
                                Scr,Rowbytes,
                                tgv->fPrevSource,TGVRowBytes(tgv->fWidth),
                                x,y,&BestMatch,&BVIndex);
            (Bvm->MVFrequency->Frequency[BVIndex])++;                                   /* This vector is used one more time */
            Bvm->TheMatrix->MXIndex[i]=BVIndex;
            WorkMatrix[i].BestMatch=BestMatch;                                          /* BestMatch is the best value for this matrix of vector */
            WorkMatrix[i].TypeDictionary=kMotionVector;                                 /* The dictionary used is the motion vector dictionary */
            BlRef=Bvm->TheVector->ListMV[BVIndex];
            BlRef.x+=x;                                                                 /* The vector is convert in local coordinate */
            BlRef.y+=y;
            BLocation[i]=BlRef;                                                         /* Store the coordinate in the matrix */
        }
        if (CompressDictionary(Bvm,kMotionVector))
        {
            if (CheckAllDictionary(Bvm)) CheckError=DelDupplicateVectors(tgv,Bvm,BLocation);
        }
        gfree((char*)BLocation);
    }
    return(CheckError);
}


/***********************************************************************************/
/*          compression and decompression of the literals in high quality          */
/***********************************************************************************/

static void CompressHighLiteral(unsigned char* Dest,LiteralMatrix* ScrLiteral)
{
    int                 i;
    HighLiteralMatrix*  Dest16;
    unsigned short      Rgb5;
    unsigned int       Rgb8;

    Dest16=(HighLiteralMatrix*)Dest;
    for (i=0; i<kXResYRes; i++)
    {
        Rgb8=ScrLiteral->PMatrix[i];
        C8to5(Rgb8,Rgb5);
        T16PUTFIELD(Dest16->PMatrix[i],Rgb5);
    }
}


/***********************************************************************************/
/*          compression and decompression of the literals in low quality           */
/***********************************************************************************/

static void MergeColour(ARGB* COrg,ARGB* CAux,int StatOrigin,int StatMerge)
{
    int Total;

    Total=StatMerge+StatOrigin;
    COrg->r=(unsigned char) (((int)COrg->r)+(((((int)CAux->r)-((int)COrg->r))*StatMerge)/Total));
    COrg->g=(unsigned char) (((int)COrg->g)+(((((int)CAux->g)-((int)COrg->g))*StatMerge)/Total));
    COrg->b=(unsigned char) (((int)COrg->b)+(((((int)CAux->b)-((int)COrg->b))*StatMerge)/Total));
}


static int CalcHistogram(ARGB* TheLiteral,int* Histogram,ARGB* ThePalette)
{
    int     i,j,NbColour=0;

    for (i=0; i<kXResYRes; i++) Histogram[i]=1;                                 /* Each pixel is used one time */
    for (i=0; i<kXResYRes; i++)                                                 /* Scane the literal */
    {
        if (Histogram[i])                                                       /* Its not handle for now */
        {
            ThePalette[NbColour]=TheLiteral[i];                                 /* Set this Colour */
            Histogram[NbColour]=0;                                              /* Reset the histogram */
            for (j=i; j<kXResYRes; j++)                                         /* Scane until the end of the literal */
            {
                if (TheLiteral[j].r==ThePalette[NbColour].r)
                {
                    if (TheLiteral[j].g==ThePalette[NbColour].g)
                    {
                        if (TheLiteral[j].b==ThePalette[NbColour].b)            /* This is the same Colour */
                        {
                            Histogram[j]=0;                                     /* Reset this pixel */
                            (Histogram[NbColour])++;                            /* Add this pixel */
                        }
                    }
                }
            }
            NbColour++;                                                         /* One more unic Colour */
        }
    }
    return(NbColour);
}


static void SortPalette(int* Histogram,ARGB* ThePalette,int NbColour)
{
    int     i,SwitchColour=1;
    int     TempVal;
    ARGB    TempColor;

    while (SwitchColour)        /* Sort the palette by popularity */
    {
        SwitchColour=0;
        for (i=1; i<NbColour; i++)
        {
            if (Histogram[i]>Histogram[i-1])
            {
                TempVal=Histogram[i-1];
                Histogram[i-1]=Histogram[i];
                Histogram[i]=TempVal;
                TempColor=ThePalette[i-1];
                ThePalette[i-1]=ThePalette[i];
                ThePalette[i]=TempColor;
                SwitchColour=1;
            }
        }

    }
}


static void ReduceListRGB(int* Histogram,ARGB* ThePalette,int NbColour,int NbSearchColour)
{
    int             i,j,aIndex,bIndex,TempIndex;
    unsigned int   BestDelta,TempDelta;

    while (NbColour>NbSearchColour)                                                 /* While there is too much colors */
    {
        BestDelta=kLowestMatch;                                                     /* The best delta is initialized with the worst case */
        aIndex=0;                                                                   /* The index 'a' is initialized with the first color */
        bIndex=1;                                                                   /* The index 'b' is initialized with the second color */
        for (i=0; i<NbColour; i++)                                                  /* For all the combinaison of color */
        {
            for (j=0; j<NbColour; j++)
            {
                if (i!=j)                                                           /* It is non sense */
                {
                    TempDelta=Calc16DeltaColour(ThePalette[i],ThePalette[j]);
                    if (TempDelta<BestDelta)                                        /* It is a better match */
                    {
                        aIndex=i;
                        bIndex=j;
                        BestDelta=TempDelta;
                    }
                }
            }
        }
        if (aIndex>bIndex)                                                          /* Merge the hightest index into the lowest */
        {
            TempIndex=aIndex;
            aIndex=bIndex;
            bIndex=TempIndex;
        }
        MergeColour(&ThePalette[aIndex],&ThePalette[bIndex],Histogram[aIndex],Histogram[bIndex]);
        Histogram[aIndex]=Histogram[aIndex]+Histogram[bIndex];
        NbColour--;
        ThePalette[bIndex]=ThePalette[NbColour];                                    /* Update the last color */
        Histogram[bIndex]=Histogram[NbColour];                                      /* Update the last frequency */
    }
}


static void ColorReduceLiteral(ARGB* TheLiteral,ARGB* ResultLiteral,int NbSearchColour)
{
    int             Histogram[kXResYRes];
    ARGB            ThePalette[kXResYRes];
    int             NbColour,i,j;
    ARGB            BestColor;
    unsigned int   TempDelta,BestDelta;

    for (i=0; i<kXResYRes; i++) ResultLiteral[i]=TheLiteral[i];                     /* Perfect quality */
    NbColour=CalcHistogram(TheLiteral,Histogram,ThePalette);
    if (NbColour>NbSearchColour)                                                    /* Need to reduced the number of colours */
    {
        SortPalette(Histogram,ThePalette,NbColour);                                 /* Sort the palette by popularity */
        ReduceListRGB(Histogram,ThePalette,NbColour,NbSearchColour);                /* Color reduce */
        for (i=0; i<kXResYRes; i++)                                                 /* Remap the literal */
        {
            BestDelta=kLowestMatch;
            for (j=0; j<NbSearchColour; j++)                                        /* Search the best color in the palette */
            {
                TempDelta=Calc16DeltaColour(ResultLiteral[i],ThePalette[j]);
                if (TempDelta<BestDelta)                                            /* It is a better match */
                {
                    BestColor=ThePalette[j];
                    BestDelta=TempDelta;
                }
            }
            ResultLiteral[i]=BestColor;                                             /* set the new value for this pixel */
        }
    }
}


static void CompressLowLiteral(unsigned char* Dest,LiteralMatrix* ScrLiteral)
{
    unsigned int       c[4];
    unsigned int       Colour;
    unsigned int       i,matrix=0;
    unsigned short      Rgb5;
    LowLiteralMatrix*   Dest16;

    c[0]=ScrLiteral->PMatrix[0];
    c[1]=c[0];
    c[2]=c[0];
    c[3]=c[0];
    for (i=0; i<kXResYRes; i++)
    {
        matrix<<=2;
        Colour=ScrLiteral->PMatrix[i];
        if (Colour!=c[0])
        {
            if (c[1]==c[0]) c[1]=Colour;
            if (Colour!=c[1])
            {
                if (c[2]==c[0]) c[2]=Colour;
                if (Colour!=c[2])
                {
                    if (c[3]==c[0]) c[3]=Colour;
                }
            }
        }
        if (Colour==c[0]) matrix|=0;
        else
        {
            if (Colour==c[1]) matrix|=1;
            else
            {
                if (Colour==c[2]) matrix|=2;
                else
                {
                    if (Colour==c[3]) matrix|=3;
                }
            }
        }
    }
    Dest16=(LowLiteralMatrix*)Dest;
    C8to5(c[0],Rgb5);
    T16PUTFIELD(Dest16->CPalette[0],Rgb5);
    C8to5(c[1],Rgb5);
    T16PUTFIELD(Dest16->CPalette[1],Rgb5);
    C8to5(c[2],Rgb5);
    T16PUTFIELD(Dest16->CPalette[2],Rgb5);
    C8to5(c[3],Rgb5);
    T16PUTFIELD(Dest16->CPalette[3],Rgb5);
    T16PUTFIELD(Dest16->PMatrix,matrix);
}


static void CalculLowLiteral(LiteralMatrix* TheLiteral,LiteralMatrix* ResultLiteral)
{
    ColorReduceLiteral((ARGB*)TheLiteral,(ARGB*)ResultLiteral,4);
}


/***********************************************************************************/
/*                 compression and decompression of the literals                   */
/***********************************************************************************/

static void CompressLiteral(LiteralMatrix* ListLiteral,int NbLiteral,int TypeDictionary)
{
    unsigned char*  LiteralPtr;
    int             i;

    LiteralPtr=(unsigned char*)ListLiteral;
    for (i=0; i<NbLiteral; i++)                 /* Scanne the dictionary */
    {
        switch (TypeDictionary)                 /* Dispatch on the right type of dictionary */
        {
            case kHighLiteral:
                CompressHighLiteral(LiteralPtr,&ListLiteral[i]);
                LiteralPtr+=sizeof(HighLiteralMatrix);
                break;
            case kLowLiteral:
                CompressLowLiteral(LiteralPtr,&ListLiteral[i]);
                LiteralPtr+=sizeof(LowLiteralMatrix);
                break;
        }
    }
}


/***********************************************************************************/
/*            Calcul the dictionary and replace the right cell in the matrix       */
/***********************************************************************************/

static int FindNextLiteral(T16Enviro* Bvm,int* xIndex,int* yIndex)
{
    int            MaxCell,i;
    int            FindIndex=-1;
    WkMatrix*       WorkMatrix;
    unsigned int   BestMatch=0;

    *xIndex=-1;
    *yIndex=-1;
    MaxCell=Bvm->TheMatrix->yMatrix*Bvm->TheMatrix->xMatrix;
    WorkMatrix=Bvm->WorkMatrix;
    for (i=0; i<MaxCell; i++)
    {
        if (WorkMatrix[i].BestMatch>BestMatch)          /* Lowest match */
        {
            FindIndex=i;                                /* Update with the new values */
            BestMatch=WorkMatrix[i].BestMatch;
        }
    }
    if (FindIndex!=-1) IndexMatrixToXY(Bvm,FindIndex,xIndex,yIndex);
    return(FindIndex);
}


static void SearchBestLiteral(T16GimexData* tgv,T16Enviro* Bvm,LiteralMatrix* TheLiteral,int *TypeDictionary,unsigned int *BestMatch,int *BLIndex)
{
    unsigned int   TempMatch;
    int             TempIndex;

    *TypeDictionary=kHighLiteral;
    SearchLiteral(tgv->fError,Bvm->TheHighLiteral,TheLiteral,BestMatch,BLIndex);    /* The default dictionary is the full literal dictionary */
    SearchLiteral(tgv->fError,Bvm->TheLowLiteral,TheLiteral,&TempMatch,&TempIndex); /* Try the low res literal dictionary */
    if (TempMatch<(*BestMatch))                                                     /* It is better */
    {
        *TypeDictionary=kLowLiteral;
        *BestMatch=TempMatch;
        *BLIndex=TempIndex;
    }
}


static int BuildLiteralDictionary(unsigned int SizeRate,T16GimexData* tgv,T16Enviro* Bvm,char* Source,int Rowbytes)
{
    int             IndexMatrix,TypeDictionary;
    int             SaveBLIndex,SaveTypeDictionary;
    LiteralMatrix   TheLiteral,CalcLiteral;
    int             xIndex,yIndex;
    unsigned int   BestMatch;
    int             BLIndex;
    char*           Original;
    char*           LowRes;

    IndexMatrix=FindNextLiteral(Bvm,&xIndex,&yIndex);                                   /* Search a literal and its position */
    while ((IndexMatrix!=-1)&&(MoreLiteral(Bvm,SizeRate)))                              /* Enought free space to add a literal */
    {
        GIMEXIDLE;
        SaveBLIndex=Bvm->TheMatrix->MXIndex[IndexMatrix];                               /* Save the index */
        SaveTypeDictionary=Bvm->WorkMatrix[IndexMatrix].TypeDictionary;                 /* Save type dictionary */
        LoadLiteral((char*)&TheLiteral,xIndex*kXRes,yIndex*kYRes,Source,Rowbytes);      /* Load a new bloc */
        SearchBestLiteral(tgv,Bvm,&TheLiteral,&TypeDictionary,&BestMatch,&BLIndex);     /* Search the best literal in the existing dictionary */
        if (BestMatch>=Bvm->WorkMatrix[IndexMatrix].BestMatch) BLIndex=kInvalidIndex;   /* The previous error was better */
        if (BLIndex==kInvalidIndex)                                                     /* Need to build a literal */
        {
            CalculLowLiteral(&TheLiteral,&CalcLiteral);                                 /* Build a low res literal */
            Original=(char*)&TheLiteral;
            LowRes=(char*)&CalcLiteral;
            CalcDelta8(tgv->fError,Original,kXRes,LowRes,kXRes,BestMatch);
            if (BestMatch<Bvm->WorkMatrix[IndexMatrix].BestMatch)                       /* It is better that the previous error */
            {
                TypeDictionary=kLowLiteral;
                AddLiteral(Bvm->TheLowLiteral,Bvm->LLFrequency,&CalcLiteral,&BLIndex);  /* Add a new literal in the low res literal dictionary */
            }
            else
            {
                TypeDictionary=kHighLiteral;
                AddLiteral(Bvm->TheHighLiteral,Bvm->HLFrequency,&TheLiteral,&BLIndex);  /* Add a new literal in the list */
                BestMatch=0;                                                            /* The best part of this function */
            }
        }
        switch (TypeDictionary)                                                         /* Dispatch on the right type of dictionary */
        {
            case kHighLiteral:
                (Bvm->HLFrequency->Frequency[BLIndex])++;                               /* This literal is used one more time */
                break;
            case kLowLiteral:
                (Bvm->LLFrequency->Frequency[BLIndex])++;                               /* This literal is used one more time */
                break;
        }
        Bvm->TheMatrix->MXIndex[IndexMatrix]=BLIndex;                                   /* Update the index */
        Bvm->WorkMatrix[IndexMatrix].BestMatch=BestMatch;                               /* Update quality */
        Bvm->WorkMatrix[IndexMatrix].TypeDictionary=TypeDictionary;                     /* Update type dictionary */
        ClearInDictionary(Bvm,SaveTypeDictionary,SaveBLIndex);                          /* Clear the unsused element in the right dictionary */
        IndexMatrix=FindNextLiteral(Bvm,&xIndex,&yIndex);                               /* Search the next literal */
    }
    return(CheckAllDictionary(Bvm));
}


/***********************************************************************************/
/*                            Remap using all the Literal                          */
/***********************************************************************************/

static int RemapAllLiteral(T16GimexData* tgv,T16Enviro* Bvm,char* Source,int Rowbytes)
{
    WkMatrix*       WorkMatrix;
    int             xScan,yScan;
    int             BLIndex,i,TypeDictionary,MaxCell;
    int             SaveBLIndex,SaveTypeDictionary;
    unsigned int   BestMatch;
    LiteralMatrix   TheLiteral;

    MaxCell=Bvm->TheMatrix->yMatrix*Bvm->TheMatrix->xMatrix;
    WorkMatrix=Bvm->WorkMatrix;
    for (i=0; i<MaxCell; i++)
    {
        GIMEXIDLE;
        IndexMatrixToXY(Bvm,i,&xScan,&yScan);
        SaveBLIndex=Bvm->TheMatrix->MXIndex[i];                                     /* Save the index */
        SaveTypeDictionary=WorkMatrix[i].TypeDictionary;                            /* Save type dictionary */
        LoadLiteral((char*)&TheLiteral,xScan*kXRes,yScan*kYRes,Source,Rowbytes);    /* Fetch the literal */
        SearchBestLiteral(tgv,Bvm,&TheLiteral,&TypeDictionary,&BestMatch,&BLIndex); /* Search the best literal in the existing dictionary */
        if (BestMatch<WorkMatrix[i].BestMatch)                                      /* It is better */
        {
            switch (TypeDictionary)                                                 /* Dispatch on the right type of dictionary */
            {
                case kHighLiteral:
                    (Bvm->HLFrequency->Frequency[BLIndex])++;                       /* This literal is used one more time */
                    break;
                case kLowLiteral:
                    (Bvm->LLFrequency->Frequency[BLIndex])++;                       /* This literal is used one more time */
                    break;
            }
            Bvm->TheMatrix->MXIndex[i]=BLIndex;                                     /* Update the index */
            WorkMatrix[i].BestMatch=BestMatch;                                      /* Update quality */
            WorkMatrix[i].TypeDictionary=TypeDictionary;                            /* Update type dictionary */
            ClearInDictionary(Bvm,SaveTypeDictionary,SaveBLIndex);                  /* Clear the unsused element in the right dictionary */
        }
    }
    return(CheckAllDictionary(Bvm));
}


/***********************************************************************************/
/*                  Calcul the best literal using the original bloc                */
/***********************************************************************************/

static int SortListBloc(unsigned int* ListError,int NbBloc,LiteralMatrix* ListBloc,LiteralMatrix* ResultLiteral)
{
    unsigned int   BestMatch,TempMatch,Match;
    int             BLIndex,i,j;
    char*           Scr;
    char*           Prev;
    int             ok=1;

    BestMatch=kLowestMatch;                                                 /* Reset the match value */
    BLIndex=-1;
    for (i=0; i<NbBloc; i++)
    {
        TempMatch=0;                                                        /* Reset the total of match value */
        for (j=0; j<NbBloc; j++)
        {
            if (i!=j)                                                       /* It is a valid test */
            {
                Scr=(char*)&ListBloc[i];                                    /* The bloc to test */
                Prev=(char*)&ListBloc[j];
                CalcDelta8(ListError,Scr,kXRes,Prev,kXRes,Match);
                TempMatch+=Match;
            }
        }
        if (TempMatch<BestMatch)                                            /* New better match */
        {
            BestMatch=TempMatch;
            BLIndex=i;
        }
    }
    if (BLIndex==-1) ok=0;
    else *ResultLiteral=ListBloc[BLIndex];
    return(ok);
}


static int CalculBestLiteral(T16GimexData* tgv,T16Enviro* Bvm,char* Source,int Rowbytes,int NbBloc,int TypeDictionary,int SearchIndex,LiteralMatrix* ResultLiteral)
{
    WkMatrix*       WorkMatrix;
    LiteralMatrix*  ListBloc=0L;
    LiteralMatrix   TheLiteral;
    int             i,MaxCell;
    int             xScan,yScan;
    int             Count=0;
    int             ok=1;

    ListBloc=(LiteralMatrix*)galloc(sizeof(LiteralMatrix)*NbBloc);                  /* Allocate the list */
    if (ListBloc)
    {
        MaxCell=Bvm->TheMatrix->yMatrix*Bvm->TheMatrix->xMatrix;
        WorkMatrix=Bvm->WorkMatrix;
        for (i=0; i<MaxCell; i++)
        {
            if (ok)
            {
                if (WorkMatrix[i].TypeDictionary==TypeDictionary)                   /* It is the right type of element */
                {
                    if (Bvm->TheMatrix->MXIndex[i]==SearchIndex)                    /* Search the blocs with the right Index */
                    {
                        IndexMatrixToXY(Bvm,i,&xScan,&yScan);
                        LoadLiteral(    (char*)&TheLiteral,
                                        xScan*kXRes,yScan*kYRes,
                                        Source,Rowbytes);                           /* Load the bloc */
                        if (TypeDictionary==kLowLiteral)                            /* Calcul the low res version of this literal */
                        {
                            CalculLowLiteral(&TheLiteral,&ListBloc[Count]);
                        }
                        else                                                        /* Save the bloc in the list */
                        {
                            ListBloc[Count]=TheLiteral;
                        }
                        Count++;
                    }
                }
            }
        }
        if (ok) ok=SortListBloc(tgv->fError,NbBloc,ListBloc,ResultLiteral);         /* Search the best element in the list */
        gfree((char*)ListBloc);
    }
    else ok=0;
    return(ok);
}


static int CalculBestDictionary(T16GimexData* tgv,T16Enviro* Bvm,char* Source,int Rowbytes,FrequencyRec* TheFrequency,DLiteral* TheDictionary,int TypeDictionary)
{
    int                 i,NbBloc;
    LiteralMatrix       ResultLiteral;
    int                 ok=1;

    for (i=0; i<TheDictionary->NbLiteral; i++)                                                          /* Scanne the dictionary */
    {
        if (ok)
        {
            NbBloc=TheFrequency->Frequency[i];                                                          /* Nb blocs that use this literal */
            if (NbBloc>1)                                                                               /* If there is more than one bloc */
            {
                ResultLiteral=TheDictionary->ListLiteral[i];
                ok=CalculBestLiteral(tgv,Bvm,Source,Rowbytes,NbBloc,TypeDictionary,i,&ResultLiteral);   /* Calcul the best literal */
                TheDictionary->ListLiteral[i]=ResultLiteral;                                            /* Save the result in the dictionary */
            }
        }

    }
    return(ok);
}


static int CalculBestDictionaries(T16GimexData* tgv,T16Enviro* Bvm,char* Source,int Rowbytes)
{
    if (CalculBestDictionary(tgv,Bvm,Source,Rowbytes,Bvm->HLFrequency,Bvm->TheHighLiteral,kHighLiteral))
    {
        if (CalculBestDictionary(tgv,Bvm,Source,Rowbytes,Bvm->LLFrequency,Bvm->TheLowLiteral,kLowLiteral))
        {
            return(1);
        }
    }
    return(0);
}


/***********************************************************************************/
/*                         Compression of the final matrix                         */
/***********************************************************************************/

static int CompressMatrix(T16Enviro* Bvm)
{
    DIMatrix*       TheMatrix;
    WkMatrix*       WorkMatrix;
    int             MaxCell,i,TypeDictionary;
    int             NbMotionVector,NbHighLiteral,NbLowLiteral;

    TheMatrix=Bvm->TheMatrix;
    WorkMatrix=Bvm->WorkMatrix;
    MaxCell=TheMatrix->yMatrix*TheMatrix->xMatrix;
    NbMotionVector=Bvm->TheVector->NbMotionVector;
    NbHighLiteral=Bvm->TheHighLiteral->NbLiteral;
    NbLowLiteral=Bvm->TheLowLiteral->NbLiteral;
    if (TGVDEBUG)
    {
        for (i=0; i<NbMotionVector; i++) Bvm->MVFrequency->Frequency[i]=0;                  /* Reset the frequence */
        for (i=0; i<NbHighLiteral; i++) Bvm->HLFrequency->Frequency[i]=0;                   /* Reset the frequence */
        for (i=0; i<NbLowLiteral; i++) Bvm->LLFrequency->Frequency[i]=0;                    /* Reset the frequence */
    }
    for (i=0; i<MaxCell; i++)
    {
        TypeDictionary=WorkMatrix[i].TypeDictionary;
        if (TGVDEBUG)
        {
            switch (TypeDictionary)
            {
                case kMotionVector:(Bvm->MVFrequency->Frequency[TheMatrix->MXIndex[i]])++;break;
                case kHighLiteral:(Bvm->HLFrequency->Frequency[TheMatrix->MXIndex[i]])++;break;
                case kLowLiteral:(Bvm->LLFrequency->Frequency[TheMatrix->MXIndex[i]])++;break;
            }
        }
        if (TypeDictionary!=kMotionVector)                                                  /* Calcul the real index of each elements */
        {
            TheMatrix->MXIndex[i]+=NbMotionVector;
            if (TypeDictionary!=kHighLiteral)
            {
                TheMatrix->MXIndex[i]+=NbHighLiteral;
                if (TypeDictionary!=kLowLiteral)
                {
                    TheMatrix->MXIndex[i]+=NbLowLiteral;
                }
            }
        }
    }
    if (TGVDEBUG)
    {
        for (i=0; i<NbMotionVector; i++) if (!Bvm->MVFrequency->Frequency[i]) return(0);    /* Check unused element */
        for (i=0; i<NbHighLiteral; i++) if (!Bvm->HLFrequency->Frequency[i]) return(0);     /* Check unused element */
        for (i=0; i<NbLowLiteral; i++) if (!Bvm->LLFrequency->Frequency[i]) return(0);      /* Check unused element */
    }
    return(1);
}


/***********************************************************************************/
/*        Allocate the master array that store all the best vector of an image     */
/***********************************************************************************/

static DIMatrix* AllocateMatrix(int xMatrix,int yMatrix)
{
    DIMatrix*   ResultMatrix;
    int        TheSize;

    TheSize=((int)xMatrix)*((int)yMatrix)*sizeof(int);
    TheSize=(TheSize+3)&~3;                                 /* The matrix as to be align on 32 bits */
    TheSize+=sizeof(DIMatrix)-sizeof(char);
    ResultMatrix=(DIMatrix*)galloc(TheSize);
    if (ResultMatrix)
    {
        ResultMatrix->xMatrix=xMatrix;
        ResultMatrix->yMatrix=yMatrix;
    }
    return(ResultMatrix);
}


static DMVector* AllocateMotionVector(int NbMotionVector,int MaxVector)
{
    DMVector*   ResultMotionVector;
    int        TheSize;

    TheSize=sizeof(DMVector)-sizeof(MVector);
    TheSize+=(((int)MaxVector)*sizeof(MVector));
    ResultMotionVector=(DMVector*)galloc(TheSize);
    if (ResultMotionVector)
    {
        ResultMotionVector->NbMotionVector=NbMotionVector;
        ResultMotionVector->MaxMotionVector=MaxVector;
    }
    return(ResultMotionVector);
}


static DLiteral* AllocateLiteral(int NbLiteral,int MaxLiteral)
{
    DLiteral*   ResultLiteral;
    int        TheSize;

    TheSize=sizeof(DLiteral)-sizeof(LiteralMatrix);
    TheSize+=(((int)MaxLiteral)*sizeof(LiteralMatrix));
    ResultLiteral=(DLiteral*)galloc(TheSize);
    if (ResultLiteral)
    {
        ResultLiteral->NbLiteral=NbLiteral;
        ResultLiteral->MaxLiteral=MaxLiteral;
    }
    return(ResultLiteral);
}


static void FreeEnviro(T16Enviro* TheEnviro)
{
    if (TheEnviro->TheMatrix) gfree((char*)TheEnviro->TheMatrix);
    if (TheEnviro->WorkMatrix) gfree((char*)TheEnviro->WorkMatrix);
    if (TheEnviro->TheVector) gfree((char*)TheEnviro->TheVector);
    if (TheEnviro->MVFrequency) gfree((char*)TheEnviro->MVFrequency);
    if (TheEnviro->TheHighLiteral) gfree((char*)TheEnviro->TheHighLiteral);
    if (TheEnviro->HLFrequency) gfree((char*)TheEnviro->HLFrequency);
    if (TheEnviro->TheLowLiteral) gfree((char*)TheEnviro->TheLowLiteral);
    if (TheEnviro->LLFrequency) gfree((char*)TheEnviro->LLFrequency);
    gfree((char*)TheEnviro);
}


static FrequencyRec* AllocateFrequencyRec(int NbFrequency)
{
    int                TheSize;
    FrequencyRec*       TheFrequencyRec;

    TheSize=sizeof(FrequencyRec)-sizeof(unsigned int);
    TheSize+=(((int)NbFrequency)*sizeof(unsigned int));
    TheFrequencyRec=(FrequencyRec*)galloc(TheSize);
    return(TheFrequencyRec);
}


static T16Enviro* AllocateEnviro(int Width,int Height,unsigned int SizeRate)
{
    T16Enviro*  ResultEnviro;
    int         xMatrix,yMatrix;
    int        TheSize;
    int         MaxMotionVector;

    xMatrix=Width/kXRes;
    yMatrix=Height/kYRes;
    ResultEnviro=(T16Enviro*)galloc(sizeof(T16Enviro));
    if (ResultEnviro)
    {
        ResultEnviro->TheMatrix=0L;             /* xMatrix*yMatrix of index */
        ResultEnviro->WorkMatrix=0L;            /* xMatrix*yMatrix of working elements */
        ResultEnviro->TheVector=0L;             /* The dictionnary of vectors */
        ResultEnviro->MVFrequency=0L;           /* The frequency record for the vectors */
        ResultEnviro->TheHighLiteral=0L;        /* The dictionnary of literals of 16 Colours */
        ResultEnviro->HLFrequency=0L;           /* The frequency record for the literals of 16 Colours */
        ResultEnviro->TheLowLiteral=0L;         /* The dictionnary of low res literals */
        ResultEnviro->LLFrequency=0L;           /* The frequency record for the low res literals */
        ResultEnviro->TheMatrix=AllocateMatrix(xMatrix,yMatrix);
        if (ResultEnviro->TheMatrix)
        {
            TheSize=((int)xMatrix)*((int)yMatrix)*sizeof(WkMatrix);
            ResultEnviro->WorkMatrix=(WkMatrix*)galloc(TheSize);
            if (ResultEnviro->WorkMatrix)
            {
                MaxMotionVector=GetInitSizeVector(xMatrix,yMatrix,SizeRate);
                ResultEnviro->TheVector=AllocateMotionVector(0,MaxMotionVector);
                if (ResultEnviro->TheVector)
                {
                    ResultEnviro->MVFrequency=AllocateFrequencyRec(MaxMotionVector);
                    if (ResultEnviro->MVFrequency)
                    {
                        ResultEnviro->TheHighLiteral=AllocateLiteral(0,kMaxLiteral);
                        if (ResultEnviro->TheHighLiteral)
                        {
                            ResultEnviro->HLFrequency=AllocateFrequencyRec(kMaxLiteral);
                            if (ResultEnviro->HLFrequency)
                            {
                                ResultEnviro->TheLowLiteral=AllocateLiteral(0,kMaxLiteral);
                                if (ResultEnviro->TheLowLiteral)
                                {
                                    ResultEnviro->LLFrequency=AllocateFrequencyRec(kMaxLiteral);
                                    if (ResultEnviro->LLFrequency)
                                    {
                                        return(ResultEnviro);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    if (ResultEnviro) FreeEnviro(ResultEnviro);
    return(0L);
}


/****************************************************************/
/*              Automatic detection of the key frames           */
/****************************************************************/

#define kResHistogram   (1<<9)                                                      /* Resolution of the histogram 333 */
#define kSizeHistogram  (sizeof(unsigned int)*kResHistogram)

static int InitDetectKeyFrame(T16GimexData *tgv)
{
    tgv->fRealSizeRate=0;                                                           /* Real size of the movie */
    tgv->fLogicSizeRate=0;                                                          /* Size calculated with the quality */
    tgv->fSizeKeyFrame=0;                                                           /* Sum of the size of the key frame */
    tgv->fNumKeyFrame=0;                                                            /* Number of key frame in this movie */
    tgv->fDKF=(unsigned int*)galloc(kSizeHistogram);                               /* Contains the previous histogram */
    if (tgv->fDKF) tgv->fTDKF=(unsigned int*)galloc(kSizeHistogram);               /* Contains the current histogram */
    if ((tgv->fDKF) && (tgv->fTDKF))
    {
        memset(tgv->fDKF,0,kSizeHistogram);
        return(1);
    }
    return(0);
}


static int StartSizeRate(T16GimexData *tgv,GINFO *ginfo)
{
    int    SizeRate,SizeKeyFrame;

    SizeRate=ginfo->width*sizeof(unsigned short)*ginfo->height*ginfo->quality/100;  /* Regular size rate for this frame */
    if (ginfo->subtype&kUseDKF)
    {
        if (tgv->fNumKeyFrame)                                                      /* Is not the first frame */
        {
            SizeKeyFrame=tgv->fSizeKeyFrame/tgv->fNumKeyFrame;                      /* Average size for a key frame */
            if (tgv->fRealSizeRate>tgv->fLogicSizeRate)
            {
                SizeRate=(SizeRate*kFastBackup)/100;                                /* Fast backup */
            }
            else
            {
                if ((tgv->fLogicSizeRate-tgv->fRealSizeRate)<SizeKeyFrame)          /* Space for a key frame */
                {
                    SizeRate=(SizeRate*kSlowBackup)/100;                            /* Slow backup */
                }
            }
        }
    }
    return(SizeRate);
}


static void StopSizeRate(T16GimexData *tgv,GINFO *ginfo,int SizeRate,int IsKeyFrame,char* Scr,int Rowbytes)
{
    if (IsKeyFrame)                                                                 /* The frame has been save as a key frame */
    {
        tgv->fSizeKeyFrame+=SizeRate;                                               /* Increase the sum of the key frame */
        tgv->fNumKeyFrame++;                                                        /* Increase the number of key frame */
    }
    tgv->fRealSizeRate+=SizeRate;                                                   /* The total size of the file increase */
    SizeRate=ginfo->width*sizeof(unsigned short)*ginfo->height*ginfo->quality/100;  /* Logical size of the frame */
    tgv->fLogicSizeRate+=SizeRate;                                                  /* The Logical size of the file increase */
}


static int DetectLogicalKeyFrame(T16GimexData *tgv,GINFO *ginfo,char* Scr,int Rowbytes)
{
    int             x,y,InxHist;
    int*           ScanScr;
    unsigned int*  TDKF;
    unsigned int*  DKF;
    unsigned int   Rgb8,Delta=0;
    int             IsKeyFrame=0;

    TDKF=tgv->fTDKF;                                                                /* Histogram of the previous frame */
    DKF=tgv->fDKF;                                                                  /* Histogram of the current frame */
    memset((char*)TDKF,0,kSizeHistogram);                                           /* Calculation of the current frame histogram */
    for (y=0; y<ginfo->height; y++)
    {
        ScanScr=(int*)(Scr+y*Rowbytes);
        for (x=0; x<ginfo->width; x++)
        {
            Rgb8=*ScanScr++;
            (TDKF[((Rgb8&0x00E00000)>>15)|((Rgb8&0x0000E000)>>10)|((Rgb8&0x000000E0)>>5)])++;
        }
        Scr+=Rowbytes;
    }
    for (InxHist=0; InxHist<kResHistogram; InxHist++)
    {
        if (DKF[InxHist]>TDKF[InxHist]) Delta+=DKF[InxHist]-TDKF[InxHist];          /* Calculation of the delta between the two histogram */
        else Delta+=TDKF[InxHist]-DKF[InxHist];
    }
    if (ginfo->subtype&kUseDKF)
    {
        if (Delta>((ginfo->height*ginfo->width)/kTresLDKFl))                        /* The delta is more important that width * height * Threshold value */
        {
            if (tgv->fRealSizeRate<tgv->fLogicSizeRate) IsKeyFrame=1;               /* There is enought space to generate a key frame */
        }
    }
    for (InxHist=0; InxHist<kResHistogram; InxHist++) DKF[InxHist]=TDKF[InxHist];   /* Copy the current histogram in the previous */
    return(IsKeyFrame);
}


static int DetectQualityKeyFrame(T16GimexData *tgv,GINFO *ginfo,char* CompScr,int CompScrRowbytes,char* Scr,int ScrRowbytes)
{
    int             x,y;
    char*           ScanCompScr;
    char*           ScanScr;
    unsigned int   RefError,TotalErr=0;
    int             IsKeyFrame=0;

    if (ginfo->subtype&kUseDKF)
    {
        for (y=0; y<ginfo->height; y++)                                             /* Sum of all errors between the current frame and the previous */
        {
            ScanCompScr=CompScr+y*CompScrRowbytes;
            ScanScr=Scr+y*ScrRowbytes;
            for (x=0; x<ginfo->width; x++)
            {
                TotalErr+=GetError(tgv->fError,ScanCompScr,ScanScr,0);
                ScanCompScr+=sizeof(int);
                ScanScr+=sizeof(int);
            }
        }
        if (TotalErr>((ginfo->height*ginfo->width)*kTresLDKFq))                     /* The delta is more important that width * height * Threshold value */
        {
            if (tgv->fRealSizeRate<tgv->fLogicSizeRate) IsKeyFrame=1;               /* There is enought space to generate a key frame */
        }
    }
    return(IsKeyFrame);
}


/***********************************************************************************/
/*                                    T16 write                                    */
/***********************************************************************************/

static int t16write(GSTREAM *g, void *buf, int size)
{
    if (size) return(gwrite(g,buf,size));
    else return(1);
}


/***********************************************************************************/
/*                                   Write in file                                 */
/***********************************************************************************/

static T16KeyStruct* CompressKeyFrame(GINFO* ginfo,char* source,int rowbytes,int* PackSize)
{
    int                 BufferSize,x,y;
    char*               UnPackBuffer;
    char*               PackBuffer;
    unsigned short*     TempBuffer16;
    unsigned int*      TempBuffer32;
    unsigned short      Rgb5;
    unsigned int       Rgb8;

    *PackSize=0;
    BufferSize=ginfo->height*ginfo->width*sizeof(unsigned short);       /* x*y in 16 bits */
    UnPackBuffer=(char*)galloc(BufferSize);                             /* The buffer to translate the pixmap in 16 bits */
    if (!UnPackBuffer) return(0);
    TempBuffer16=(unsigned short*)UnPackBuffer;
    for (y=0; y<ginfo->height; ++y)
    {
        TempBuffer32=(unsigned int*)source;
        for (x=0; x<ginfo->width; ++x)                                  /* We may increase the quality here! */
        {
            Rgb8=*TempBuffer32++;
            C8to5(Rgb8,Rgb5);
            T16PUTFIELD(*TempBuffer16,Rgb5);
            TempBuffer16++;
        }
        source+=rowbytes;
    }
    PackBuffer=(char*)galloc(sizeof(T16KeyStruct)+(BufferSize*2)+8192); /* Plus the double of the original buffer and a constant */
    if (PackBuffer)
    {
        *PackSize=REF_encode(UnPackBuffer,BufferSize,PackBuffer+sizeof(T16KeyStruct),0);
        *PackSize=(((*PackSize)+3)/4)*4;                                /* To align on 32 bits */
        *PackSize=(*PackSize)+sizeof(T16KeyStruct);                     /* Add the size of the structure */
    }
    gfree((char*)UnPackBuffer);
    return((T16KeyStruct*)PackBuffer);
}


static int SaveKeyFrame(GSTREAM *g,T16GimexData* tgv,GINFO* ginfo,T16KeyStruct* TheKeyFrame,int PackSize,char* Scr,int Rowbytes)
{
    StopSizeRate(tgv,ginfo,PackSize,1,Scr,Rowbytes);
    T16PUTFIELD(TheKeyFrame->chunkid,T16K);
    T16PUTFIELD(TheKeyFrame->chunksize,PackSize);
    T16PUTFIELD(TheKeyFrame->frameWidth,ginfo->width);
    T16PUTFIELD(TheKeyFrame->frameHeight,ginfo->height);
    T16PUTFIELD(TheKeyFrame->reserved,0);
    if (gwrite(g,(char*)TheKeyFrame,PackSize)) return(1);
    return(0);
}


static int SaveFrame(GSTREAM *g,T16GimexData* tgv,GINFO* ginfo,T16Enviro* Bvm,char* Scr,int Rowbytes)
{
    T16FrameStruct  TheFrame;
    int            TotalSize,MatrixDepth;
    int            TheSize,NbLong;
    int             NbCellMatrix,ok=0;

    TotalSize=GetSizeHeader();
    TotalSize+=GetSizeVector(Bvm);
    TotalSize+=GetSizeMatrix(Bvm,0);
    TotalSize+=GetSizeLiterale(Bvm);
    MatrixDepth=OptimalDepthMatrix( Bvm->TheVector->NbMotionVector,
                                    Bvm->TheHighLiteral->NbLiteral,
                                    Bvm->TheLowLiteral->NbLiteral,
                                    0);
    StopSizeRate(tgv,ginfo,TotalSize,0,Scr,Rowbytes);
    T16PUTFIELD(TheFrame.chunkid,T16F);
    T16PUTFIELD(TheFrame.chunksize,TotalSize);
    T16PUTFIELD(TheFrame.indexBits,MatrixDepth);
    T16PUTFIELD(TheFrame.numMotion,Bvm->TheVector->NbMotionVector);
    T16PUTFIELD(TheFrame.numHighLiteral,Bvm->TheHighLiteral->NbLiteral);
    T16PUTFIELD(TheFrame.numLowLiteral,Bvm->TheLowLiteral->NbLiteral);
    T16PUTFIELD(TheFrame.pad,0);
    TheSize=sizeof(T16FrameStruct);
    if (t16write(g,(char*)&TheFrame,TheSize))
    {
        if (CompressDataDepth((char*)Bvm->TheVector->ListMV,Bvm->TheVector->NbMotionVector*2,kCompressVector))
        {
            NbLong=CalcC16Cell(Bvm->TheVector->NbMotionVector*2,kCompressVector);
            if (t16write(g,(char*)Bvm->TheVector->ListMV,sizeof(int)*NbLong))
            {
                CompressLiteral(Bvm->TheHighLiteral->ListLiteral,Bvm->TheHighLiteral->NbLiteral,kHighLiteral);
                NbLong=(Bvm->TheHighLiteral->NbLiteral*sizeof(HighLiteralMatrix))/sizeof(int);
                if (t16write(g,(char*)Bvm->TheHighLiteral->ListLiteral,sizeof(int)*NbLong))
                {
                    CompressLiteral(Bvm->TheLowLiteral->ListLiteral,Bvm->TheLowLiteral->NbLiteral,kLowLiteral);
                    NbLong=(Bvm->TheLowLiteral->NbLiteral*sizeof(LowLiteralMatrix))/sizeof(int);
                    if (t16write(g,(char*)Bvm->TheLowLiteral->ListLiteral,sizeof(int)*NbLong))
                    {
                        NbCellMatrix=Bvm->TheMatrix->xMatrix*Bvm->TheMatrix->yMatrix;
                        if (CompressDataDepth((char*)Bvm->TheMatrix->MXIndex,NbCellMatrix,MatrixDepth))
                        {
                            NbLong=CalcC16Cell(NbCellMatrix,MatrixDepth);
                            if (t16write(g,(char*)Bvm->TheMatrix->MXIndex,sizeof(int)*NbLong)) ok=1;
                        }
                    }
                }
            }
        }
    }
    return(ok);
}


/****************************************************************/
/*       Copy in the current frame in the previous buffer       */
/****************************************************************/

static int CheckBuffers(T16GimexData *tgv,int width,int height,int IsKeyFrame)
{
    int    TheSize;

    if (!IsKeyFrame)                                    /* Everything should be already allocated */
    {
        if (tgv->fWidth!=width) return(0);              /* The size is different of the previous picture */
        if (tgv->fHeight!=height) return(0);            /* The size is different of the previous picture */
        if (!tgv->fPrevSource) return(0);               /* The buffer is not already allocated */
    }
    else
    {
        tgv->fWidth=width;
        tgv->fHeight=height;
    }
    return(1);
}


static void CopyBuffer(char *source,int srowbytes,char *dest,int drowbytes,int height)
{
    int     y,bpl;

    if (srowbytes<drowbytes) bpl=srowbytes;
    else bpl=drowbytes;
    for (y=0; y<height; ++y)
    {
        memcpy(dest,source,bpl);
        dest+=drowbytes;
        source+=srowbytes;
    }
}


static void Convert32To16(char *source,int srowbytes,char *dest,int drowbytes,int height)
{
    int             x,y,bpl;
    unsigned int*  ScanSource;
    unsigned int*  ScanDest;
    unsigned int   Rgb8;
    unsigned short  Rgb5;

    if (srowbytes<drowbytes) bpl=srowbytes;
    else bpl=drowbytes;
    for (y=0; y<height; y++)
    {
        ScanSource=(unsigned int*)(source+y*srowbytes);
        ScanDest=(unsigned int*)(dest+y*drowbytes);
        for (x=0; x<bpl; x+=sizeof(int))
        {
            Rgb8=*ScanSource++;
            C8to5(Rgb8,Rgb5);
            C5to8(Rgb5,Rgb8);
            *ScanDest++=Rgb8;
        }
    }
}


/****************************************************************/
/*             Decompress in the source buffer                  */
/****************************************************************/

static void DrawVector(char* Source,int SRowbytes,char* Prev,int PRowbytes,int x,int y,MVector* MotionVector)
{
    Source+=y*SRowbytes+TGVRowBytes(x);
    x+=MotionVector->x;
    y+=MotionVector->y;
    Prev+=y*PRowbytes+TGVRowBytes(x);
    DrawMatrix8(Source,SRowbytes,Prev,PRowbytes);
}


static int DecompressPicture(DIMatrix* TheMatrix,DMVector* TheVector,DLiteral* TheLiteral,char* Source,int SRowbytes,char* Prev,int PRowbytes)
{
    int        xMatrix,yMatrix;
    int         IndexMatrixOffset=0;
    int        IndexMatrix;
    int        xScan,yScan;
    MVector     MotionVector;
    short       NbMotionVector,NbLiteral;

    xMatrix=TheMatrix->xMatrix;
    yMatrix=TheMatrix->yMatrix;
    NbMotionVector=TheVector->NbMotionVector;
    NbLiteral=TheLiteral->NbLiteral;
    for (yScan=0; yScan<yMatrix; yScan++)           /* Scanne the array of matrix in the array resolution */
    {
        for (xScan=0; xScan<xMatrix; xScan++)
        {
            IndexMatrix=TheMatrix->MXIndex[IndexMatrixOffset++];
            if (IndexMatrix>=NbMotionVector)
            {
                if (TGVDEBUG)
                {
                    if (IndexMatrix>(NbMotionVector+NbLiteral-1)) return(0);
                }
                IndexMatrix-=NbMotionVector;
                DrawLiteral((char*)&(TheLiteral->ListLiteral[IndexMatrix]),xScan*kXRes,yScan*kYRes,Source,SRowbytes);
            }
            else
            {
                if (TGVDEBUG)
                {
                    if (IndexMatrix<0) return(0);
                }
                MotionVector=TheVector->ListMV[IndexMatrix];
                DrawVector(Source,SRowbytes,Prev,PRowbytes,xScan*kXRes,yScan*kYRes,&MotionVector);
            }
        }
    }
    return(1);
}


/***********************************************************************************/
/*                                Export Functions                                 */
/***********************************************************************************/

int GCALL T16_wopen(GINSTANCE **gx, GSTREAM *g, const char *pathname, int numframes)
{
    GINSTANCE*      Result=0L;
    T16GimexData*   tgv;

    tgv=(T16GimexData*)galloc(sizeof(T16GimexData));
    if (tgv)
    {
        memset(tgv,0,sizeof(T16GimexData));
        tgv->fPrevSource=0L;
        tgv->fWidth=0;
        tgv->fHeight=0;
        tgv->fError=0L;
        tgv->fDKF=0L;
        tgv->fTDKF=0L;
        Result=(GINSTANCE*)galloc(sizeof(GINSTANCE));
        if (Result)
        {
            tgv->fError=(unsigned int*)galloc(sizeof(unsigned int)*kMaxNumError);
            if (tgv->fError)
            {
                CalculAllError(tgv);
                if (InitDetectKeyFrame(tgv))
                {
                    memset(Result,0,sizeof(GINSTANCE));
                    Result->frames=0;
                    Result->gstream=g;
                    Result->gref=(char*)tgv;
                    *gx=Result;
                    return(1);
                }
            }
            gfree((char*)Result);
        }
        gfree((char*)tgv);
    }
    return(0);
}


int GCALL T16_write(GINSTANCE *gx, const GINFO *ginfo, char *source, int rowbytes)
{
    int                 TheRowbytes,ok=0;
    GINFO*              TheGINFO;
    char*               TheSource;
    T16KeyStruct*       TheKeyFrame;
    T16GimexData*       tgv;
    int                PackSize,SizeRate;
    T16Enviro*          Bvm;
    int                 IsKeyFrame=0;
    int                 NbLiteral,i;
    DLiteral*           AllLiteral;

    if (ginfo->bpp==32)
    {
        if (ginfo->originalbpp!=16) Convert32To16(source,rowbytes,source,rowbytes,ginfo->height);
        tgv=(T16GimexData*)gx->gref;
        if (tgv)
        {
            TheGINFO=(GINFO*)galloc(sizeof(GINFO));                                         /* Dupplicate GInfo */
            if (TheGINFO)
            {
                memcpy(TheGINFO,ginfo,sizeof(GINFO));
                TheGINFO->width=((ginfo->width+7)>>3)<<3;                                   /* Aligned on 8*4 */
                TheGINFO->height=((ginfo->height+3)>>2)<<2;
                TheRowbytes=TGVRowBytes(TheGINFO->width);
                TheSource=(char*)galloc(TheRowbytes*TheGINFO->height);                      /* Allocate a buffer of 32 bits */
                if (TheSource)
                {
                    IsKeyFrame=DetectLogicalKeyFrame(tgv,ginfo,source,rowbytes);
                    memset(TheSource,0,TheRowbytes*TheGINFO->height);
                    CopyBuffer(source,rowbytes,TheSource,TheRowbytes,ginfo->height);        /* Copy the picture in the buffer */
                    SizeRate=StartSizeRate(tgv,TheGINFO);
                    TheKeyFrame=CompressKeyFrame(TheGINFO,TheSource,TheRowbytes,&PackSize);
                    if (TheKeyFrame)
                    {
                        if (!tgv->fPrevSource) IsKeyFrame=1;                                /* The first frame is key frame */
                        if (PackSize<SizeRate) IsKeyFrame=1;                                /* This is a key frame */
                        if (TheGINFO->quality==100) IsKeyFrame=1;                           /* This is a key frame */
                        if (IsKeyFrame)                                                     /* Make it a key frame */
                        {
                            if (SaveKeyFrame(gx->gstream,tgv,TheGINFO,TheKeyFrame,PackSize,TheSource,TheRowbytes))
                            {
                                ok=CheckBuffers(tgv,TheGINFO->width,TheGINFO->height,1);
                            }
                        }
                        else
                        {
                            if (CheckBuffers(tgv,TheGINFO->width,TheGINFO->height,0))
                            {
                                Bvm=AllocateEnviro(TheGINFO->width,TheGINFO->height,SizeRate);
                                if (Bvm)
                                {
                                    if (BuildVectorDictionary(tgv,Bvm,TheSource,TheRowbytes))
                                    {
                                        if (RemapAllVectors(tgv,Bvm,TheSource,TheRowbytes))
                                        {
                                            if (BuildLiteralDictionary(SizeRate,tgv,Bvm,TheSource,TheRowbytes))
                                            {
                                                if (RemapAllLiteral(tgv,Bvm,TheSource,TheRowbytes))
                                                {
                                                    if (CalculBestDictionaries(tgv,Bvm,TheSource,TheRowbytes))
                                                    {
                                                        if (CompressMatrix(Bvm))
                                                        {
                                                            NbLiteral=0;
                                                            NbLiteral+=Bvm->TheHighLiteral->NbLiteral;
                                                            NbLiteral+=Bvm->TheLowLiteral->NbLiteral;
                                                            AllLiteral=AllocateLiteral(NbLiteral,NbLiteral);
                                                            if (AllLiteral)
                                                            {
                                                                NbLiteral=0;
                                                                for (i=0; i<Bvm->TheHighLiteral->NbLiteral; i++)
                                                                {
                                                                    AllLiteral->ListLiteral[NbLiteral++]=Bvm->TheHighLiteral->ListLiteral[i];
                                                                }
                                                                for (i=0; i<Bvm->TheLowLiteral->NbLiteral; i++)
                                                                {
                                                                    AllLiteral->ListLiteral[NbLiteral++]=Bvm->TheLowLiteral->ListLiteral[i];
                                                                }
                                                                ok=DecompressPicture(Bvm->TheMatrix,Bvm->TheVector,AllLiteral,TheSource,TheRowbytes,tgv->fPrevSource,TGVRowBytes(tgv->fWidth));
                                                                gfree((char*)AllLiteral);
                                                                if (ok)
                                                                {

                                                                    IsKeyFrame=DetectQualityKeyFrame(tgv,ginfo,source,rowbytes,TheSource,TheRowbytes);
                                                                    if (IsKeyFrame)
                                                                    {
                                                                        if (SaveKeyFrame(gx->gstream,tgv,TheGINFO,TheKeyFrame,PackSize,TheSource,TheRowbytes))
                                                                        {
                                                                            memset(TheSource,0,TheRowbytes*TheGINFO->height);
                                                                            CopyBuffer(source,rowbytes,TheSource,TheRowbytes,ginfo->height);
                                                                            ok=CheckBuffers(tgv,TheGINFO->width,TheGINFO->height,1);
                                                                        }
                                                                    }
                                                                    else ok=SaveFrame(gx->gstream,tgv,TheGINFO,Bvm,TheSource,TheRowbytes);
                                                                }
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                    FreeEnviro(Bvm);
                                }
                            }
                        }
                        gfree(TheKeyFrame);
                    }
                    CopyBuffer(TheSource,TheRowbytes,source,rowbytes,ginfo->height);        /* Restore the picture in GInfo */
                    if (tgv->fPrevSource) gfree(tgv->fPrevSource);
                    tgv->fPrevSource=TheSource;
                }
                gfree((char*)TheGINFO);
            }
        }
    }
    return(ok);
}


int GCALL T16_wclose(GINSTANCE *gx)
{
    T16GimexData*   tgv;

    if (gx)
    {
        tgv=(T16GimexData*)gx->gref;
        if (tgv)
        {
            if (tgv->fPrevSource) gfree((char*)tgv->fPrevSource);
            if (tgv->fError) gfree((char*)tgv->fError);
            if (tgv->fDKF) gfree((char*)tgv->fDKF);
            if (tgv->fTDKF) gfree((char*)tgv->fTDKF);
            gfree((char*)tgv);
        }
        gfree((void*)gx);
    }
    return(1);
}
