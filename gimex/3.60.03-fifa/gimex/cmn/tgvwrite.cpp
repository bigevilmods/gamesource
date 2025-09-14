// (c) 1994-2007 Electronic Arts Inc.


#include <string.h>
#include "gimex.h"
#include "tgvgimex.h"
#include "codex.h"
#include "refcodex.h"

/***********************************************************************************/
/*                           Move a int on all machine                            */
/***********************************************************************************/

#ifdef sgi
#define MOVLONG(d,s) *(d+0)=*(s+0);*(d+1)=*(s+1);*(d+2)=*(s+2);*(d+3)=*(s+3)
#else
#ifdef PSX
#define MOVLONG(d,s) *(d+0)=*(s+0);*(d+1)=*(s+1);*(d+2)=*(s+2);*(d+3)=*(s+3)
#else
#ifdef MSDOS
#define MOVLONG(d,s) *(int*)(d) = *(int*)(s)
#else
#ifdef applec
#define MOVLONG(d,s) *(int*)(d) = *(int*)(s)
#else
#define MOVLONG(d,s) *(d+0)=*(s+0);*(d+1)=*(s+1);*(d+2)=*(s+2);*(d+3)=*(s+3)
#endif
#endif
#endif
#endif

/***********************************************************************************/
/*           Calcul all the error in advance to speed up the scane                 */
/***********************************************************************************/

#define CDelta(a,b) (((int)a)-((int)b))

#define CalcEChanel(a,b) ((unsigned int) (CDelta(a,b)*CDelta(a,b)))

#define CalcDeltaColour(c1,c2) CalcEChanel(c1.r,c2.r)+CalcEChanel(c1.g,c2.g)+CalcEChanel(c1.b,c2.b)

static void CalculAllError(TgvGimexData* tgv)
{
    ARGB*   Palette;
    ARGB    SourceColour,PrevColour;
    int    Index;

    Palette=tgv->fColourTable;
    for (Index=0; Index<kMaxNumError; Index++)
    {
        SourceColour=Palette[Index&0x000000FF];                         /* Take the low index */
        PrevColour=Palette[(Index>>8)&0x000000FF];                      /* Take the hight index */
        tgv->fError[Index]=CalcDeltaColour(SourceColour,PrevColour);    /* Calcul the error for this Colours */
    }
}

/***********************************************************************************/
/*           Calcul a delta using a vector and the previous picture                */
/***********************************************************************************/

#define GetCell(p,n) (*(unsigned char*) (((unsigned char*)(p))+n))

#define GetHightIndex(s,n) (((unsigned int) GetCell(s,n))<<8)

#define GetLowIndex(p,n) ((unsigned int) GetCell(p,n))

#define GetFastError(e,i,p,n) e[i|GetLowIndex(p,n)]

#define GetError(e,s,p,n) e[GetHightIndex(s,n)|GetLowIndex(p,n)]

#define GetFastErrorLine(e,i0,i1,i2,i3,p) GetFastError(e,i0,p,0)+GetFastError(e,i1,p,1)+GetFastError(e,i2,p,2)+GetFastError(e,i3,p,3)

#define GetFastErrorLiteral(e,i0,i1,i2,i3,p,o0,o1,o2,o3) GetFastError(e,i0,p,o0)+GetFastError(e,i1,p,o1)+GetFastError(e,i2,p,o2)+GetFastError(e,i3,p,o3)

#define GetErrorLine(e,s,p,r) r+=GetError(e,s,p,0)+GetError(e,s,p,1)+GetError(e,s,p,2)+GetError(e,s,p,3)

#define CalcDelta8(ListError,ScaneSource,RowByteS,ScanePrev,RowByteP,Delta) \
{\
    Delta=0;\
    GetErrorLine(ListError,ScaneSource,ScanePrev,Delta);\
    ScaneSource+=RowByteS;\
    ScanePrev+=RowByteP;\
    GetErrorLine(ListError,ScaneSource,ScanePrev,Delta);\
    ScaneSource+=RowByteS;\
    ScanePrev+=RowByteP;\
    GetErrorLine(ListError,ScaneSource,ScanePrev,Delta);\
    ScaneSource+=RowByteS;\
    ScanePrev+=RowByteP;\
    GetErrorLine(ListError,ScaneSource,ScanePrev,Delta);\
}


/***********************************************************************************/
/*              Search the best delta in the whole previous picture                */
/***********************************************************************************/

static void GetBestDelta8(unsigned int* ListError,char* Source,int SRowbytes,char* Prev,int PRowbytes,int w,int h,unsigned int* BestMatch,MVector* MotionVector)
{
    int             x,y,fx=0,fy=0;
    unsigned int   Match,CurrentMatch=kLowestMatch;
    unsigned int   i00,i01,i02,i03;
    unsigned int   i10,i11,i12,i13;
    unsigned int   i20,i21,i22,i23;
    unsigned int   i30,i31,i32,i33;
    char*           DestTable0;
    char*           DestTable1;
    char*           DestTable2;
    char*           DestTable3;

    i00=GetHightIndex(Source,0);
    i01=GetHightIndex(Source,1);
    i02=GetHightIndex(Source,2);
    i03=GetHightIndex(Source,3);
    Source+=SRowbytes;
    i10=GetHightIndex(Source,0);
    i11=GetHightIndex(Source,1);
    i12=GetHightIndex(Source,2);
    i13=GetHightIndex(Source,3);
    Source+=SRowbytes;
    i20=GetHightIndex(Source,0);
    i21=GetHightIndex(Source,1);
    i22=GetHightIndex(Source,2);
    i23=GetHightIndex(Source,3);
    Source+=SRowbytes;
    i30=GetHightIndex(Source,0);
    i31=GetHightIndex(Source,1);
    i32=GetHightIndex(Source,2);
    i33=GetHightIndex(Source,3);
    for (y=0; y<h; y++)                                                                             /* For each line */
    {
        DestTable0=Prev+(y*PRowbytes);
        DestTable1=DestTable0+PRowbytes;
        DestTable2=DestTable1+PRowbytes;
        DestTable3=DestTable2+PRowbytes;
        for (x=0; x<w; x++)                                                                         /* For each colone */
        {
            Match=GetFastError(ListError,i00,DestTable0,0);                                         /* Pixel 1 */
            if (Match<CurrentMatch)
            {
                Match+=GetFastError(ListError,i03,DestTable0,3);                                    /* Pixel 2 */
                if (Match<CurrentMatch)
                {
                    Match+=GetFastError(ListError,i02,DestTable0,2);                                /* Pixel 3 */
                    if (Match<CurrentMatch)
                    {
                        Match+=GetFastError(ListError,i01,DestTable0,1);                            /* Pixel 4 */
                        if (Match<CurrentMatch)
                        {
                            Match+=GetFastErrorLine(ListError,i10,i11,i12,i13,DestTable1);
                            if (Match<CurrentMatch)                                                 /* New better match */
                            {
                                Match+=GetFastErrorLine(ListError,i20,i21,i22,i23,DestTable2);
                                if (Match<CurrentMatch)                                             /* New better match */
                                {
                                    Match+=GetFastErrorLine(ListError,i30,i31,i32,i33,DestTable3);
                                    if (Match<CurrentMatch)                                         /* New better match */
                                    {
                                        fx=x;                                                       /* The vector is done in absolute coordinate */
                                        fy=y;                                                       /* from the topLeft of the previous picture */
                                        CurrentMatch=Match;
                                    }
                                }
                            }
                        }
                    }
                }
            }
            DestTable0++;
            DestTable1++;
            DestTable2++;
            DestTable3++;
        }
    }
    *BestMatch=CurrentMatch;                                                                        /* Update the values */
    MotionVector->x=fx;
    MotionVector->y=fy;
}


/***********************************************************************************/
/*                           Draw and load literal and vectors                     */
/***********************************************************************************/

static void LoadLiteral(char* TheLiteral,int x,int y,char* Source,int SRowbytes)
{
    Source+=(y*SRowbytes)+x;
    MOVLONG(TheLiteral,Source);
    TheLiteral+=sizeof(int);Source+=SRowbytes;
    MOVLONG(TheLiteral,Source);
    TheLiteral+=sizeof(int);Source+=SRowbytes;
    MOVLONG(TheLiteral,Source);
    TheLiteral+=sizeof(int);Source+=SRowbytes;
    MOVLONG(TheLiteral,Source);
}


static void DrawLiteral(char* TheLiteral,int x,int y,char* Source,int SRowbytes)
{
    Source+=(y*SRowbytes)+x;
    MOVLONG(Source,TheLiteral);
    TheLiteral+=sizeof(int);Source+=SRowbytes;
    MOVLONG(Source,TheLiteral);
    TheLiteral+=sizeof(int);Source+=SRowbytes;
    MOVLONG(Source,TheLiteral);
    TheLiteral+=sizeof(int);Source+=SRowbytes;
    MOVLONG(Source,TheLiteral);
}


#define DrawMatrix8(ScanSource,NextLineS,ScanPrev,NextLineP) \
{\
    MOVLONG((ScanSource),(ScanPrev));\
    MOVLONG((ScanSource+NextLineS),(ScanPrev+NextLineP));\
    MOVLONG((ScanSource+NextLineS*2),(ScanPrev+NextLineP*2));\
    MOVLONG((ScanSource+NextLineS*3),(ScanPrev+NextLineP*3));\
}


/***********************************************************************************/
/*                             Vector list management                              */
/***********************************************************************************/

static void AddMotionVector(TgvEnviro* Bvm,MVector MotionVector)
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


static void SearchMotionVector(unsigned int* ListError,TgvEnviro* Bvm,char* Source,int SRowbytes,char* Previous,int PRowbytes,int x,int y,unsigned int* BestMatch,int* BVIndex)
{
    unsigned int   i00,i01,i02,i03;
    unsigned int   i10,i11,i12,i13;
    unsigned int   i20,i21,i22,i23;
    unsigned int   i30,i31,i32,i33;
    unsigned int   Match,CurrentMatch=kLowestMatch;
    int             CurrentIndex=kInvalidIndex;
    int             i,w,h,xx,yy,NbMotionVector;
    char*           ScaneMV;
    DMVector*       TheVector;

    i00=GetHightIndex(Source,0);
    i01=GetHightIndex(Source,1);
    i02=GetHightIndex(Source,2);
    i03=GetHightIndex(Source,3);
    Source+=SRowbytes;
    i10=GetHightIndex(Source,0);
    i11=GetHightIndex(Source,1);
    i12=GetHightIndex(Source,2);
    i13=GetHightIndex(Source,3);
    Source+=SRowbytes;
    i20=GetHightIndex(Source,0);
    i21=GetHightIndex(Source,1);
    i22=GetHightIndex(Source,2);
    i23=GetHightIndex(Source,3);
    Source+=SRowbytes;
    i30=GetHightIndex(Source,0);
    i31=GetHightIndex(Source,1);
    i32=GetHightIndex(Source,2);
    i33=GetHightIndex(Source,3);
    w=(Bvm->TheMatrix->xMatrix-1)*kXRes;                                                                    /* Valid bounds of the picture */
    h=(Bvm->TheMatrix->yMatrix-1)*kYRes;
    TheVector=Bvm->TheVector;
    NbMotionVector=TheVector->NbMotionVector;
    for (i=0; i<NbMotionVector; i++)                                                                        /* Scane the whole list of vector (Top to bottom) */
    {
        xx=x+TheVector->ListMV[i].x;                                                                        /* Calculation the real position using the vector */
        yy=y+TheVector->ListMV[i].y;
        if (xx<=w)
        {
            if (yy<=h)
            {
                if (xx>=0)
                {
                    if (yy>=0)                                                                              /* The bloc is inside the previous picture */
                    {
                        ScaneMV=Previous+xx+(yy*PRowbytes);
                        Match=GetFastError(ListError,i00,ScaneMV,0);                                        /* Pixel 1 */
                        if (Match<CurrentMatch)
                        {
                            Match+=GetFastError(ListError,i03,ScaneMV,3);                                   /* Pixel 2 */
                            if (Match<CurrentMatch)
                            {
                                Match+=GetFastError(ListError,i02,ScaneMV,2);                               /* Pixel 3 */
                                if (Match<CurrentMatch)
                                {
                                    Match+=GetFastError(ListError,i01,ScaneMV,1);                           /* Pixel 4 */
                                    if (Match<CurrentMatch)
                                    {
                                        ScaneMV+=PRowbytes;
                                        Match+=GetFastErrorLine(ListError,i10,i11,i12,i13,ScaneMV);
                                        if (Match<CurrentMatch)                                             /* New better match */
                                        {
                                            ScaneMV+=PRowbytes;
                                            Match+=GetFastErrorLine(ListError,i20,i21,i22,i23,ScaneMV);
                                            if (Match<CurrentMatch)                                         /* New better match */
                                            {
                                                ScaneMV+=PRowbytes;
                                                Match+=GetFastErrorLine(ListError,i30,i31,i32,i33,ScaneMV);
                                                if (Match<CurrentMatch)                                     /* New better match */
                                                {
                                                    CurrentIndex=i;
                                                    CurrentMatch=Match;
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
        }
    }
    *BestMatch=CurrentMatch;
    *BVIndex=CurrentIndex;
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
    unsigned int   i00,i01,i02,i03;
    unsigned int   i10,i11,i12,i13;
    unsigned int   i20,i21,i22,i23;
    unsigned int   i30,i31,i32,i33;
    unsigned int   Match,CurrentMatch;
    int             i,NbLiteral,CurrentIndex;
    char*           ScaneLit;

    NbLiteral=TheDictionary->NbLiteral;
    if (!NbLiteral)
    {
        *BLIndex=kInvalidIndex;
        *BestMatch=kLowestMatch;
    }
    else
    {
        i00=GetHightIndex(((char*)TheLiteral),0);
        i01=GetHightIndex(((char*)TheLiteral),1);
        i02=GetHightIndex(((char*)TheLiteral),2);
        i03=GetHightIndex(((char*)TheLiteral),3);
        i10=GetHightIndex(((char*)TheLiteral),4);
        i11=GetHightIndex(((char*)TheLiteral),5);
        i12=GetHightIndex(((char*)TheLiteral),6);
        i13=GetHightIndex(((char*)TheLiteral),7);
        i20=GetHightIndex(((char*)TheLiteral),8);
        i21=GetHightIndex(((char*)TheLiteral),9);
        i22=GetHightIndex(((char*)TheLiteral),10);
        i23=GetHightIndex(((char*)TheLiteral),11);
        i30=GetHightIndex(((char*)TheLiteral),12);
        i31=GetHightIndex(((char*)TheLiteral),13);
        i32=GetHightIndex(((char*)TheLiteral),14);
        i33=GetHightIndex(((char*)TheLiteral),15);
        ScaneLit=(char*)&(TheDictionary->ListLiteral[0]);
        CurrentMatch=GetFastErrorLiteral(ListError,i00,i01,i02,i03,ScaneLit,0,1,2,3);
        CurrentMatch+=GetFastErrorLiteral(ListError,i10,i11,i12,i13,ScaneLit,4,5,6,7);
        CurrentMatch+=GetFastErrorLiteral(ListError,i20,i21,i22,i23,ScaneLit,8,9,10,11);
        CurrentMatch+=GetFastErrorLiteral(ListError,i30,i31,i32,i33,ScaneLit,12,13,14,15);
        CurrentIndex=0;
        for (i=1; i<NbLiteral; i++)                                                                 /* Scanne the dictionary */
        {
            ScaneLit=(char*)&(TheDictionary->ListLiteral[i]);
            Match=GetFastError(ListError,i00,ScaneLit,0);                                           /* Pixel 1 */
            if (Match<CurrentMatch)
            {
                Match+=GetFastError(ListError,i03,ScaneLit,3);                                      /* Pixel 2 */
                if (Match<CurrentMatch)
                {
                    Match+=GetFastError(ListError,i02,ScaneLit,2);                                  /* Pixel 3 */
                    if (Match<CurrentMatch)
                    {
                        Match+=GetFastError(ListError,i01,ScaneLit,1);                              /* Pixel 4 */
                        if (Match<CurrentMatch)
                        {
                            Match+=GetFastErrorLiteral(ListError,i10,i11,i12,i13,ScaneLit,4,5,6,7);
                            if (Match<CurrentMatch)                                                 /* New better match */
                            {
                                Match+=GetFastErrorLiteral(ListError,i20,i21,i22,i23,ScaneLit,8,9,10,11);
                                if (Match<CurrentMatch)                                             /* New better match */
                                {
                                    Match+=GetFastErrorLiteral(ListError,i30,i31,i32,i33,ScaneLit,12,13,14,15);
                                    if (Match<CurrentMatch)                                         /* New better match */
                                    {
                                        CurrentMatch=Match;
                                        CurrentIndex=i;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        *BestMatch=CurrentMatch;                                                                    /* Update the values */
        *BLIndex=CurrentIndex;
    }
}


/****************************************************************/
/*                    Low level Functions                       */
/****************************************************************/

#ifdef MSDOS

#define ggetintellong(p) *(unsigned int*) (p)

#define gsetintellong(p,v) *(unsigned int*)(p)=v

#else

#define ggetintellong(p) \
   (((unsigned int)(*(unsigned char *) (((char*)(p))+0))<<0) \
  | ((unsigned int)(*(unsigned char *) (((char*)(p))+1))<<8) \
  | ((unsigned int)(*(unsigned char *) (((char*)(p))+2))<<16) \
  | ((unsigned int)(*(unsigned char *) (((char*)(p))+3))<<24))

#define gsetintellong(p,v) \
    *(unsigned char*)(((char*)(p))+0)=(unsigned char)(((v>>0)&0x000000FF)); \
    *(unsigned char*)(((char*)(p))+1)=(unsigned char)(((v>>8)&0x000000FF)); \
    *(unsigned char*)(((char*)(p))+2)=(unsigned char)(((v>>16)&0x000000FF)); \
    *(unsigned char*)(((char*)(p))+3)=(unsigned char)(((v>>24)&0x000000FF))

#endif

#define getfieldTGV(ptr,bitoff,n) ((ggetintellong((ptr)+((bitoff)>>3)) >> ((bitoff)&7)) & ((1<<n)-1))

static void setfieldTGV(char *ptr,unsigned int bitoff,unsigned int n,unsigned int v)
{
    unsigned int   m;

    ptr+=(bitoff>>3);
    m=(1<<n)-1;                             /* n bits of 1s */
    v<<=(bitoff&7);                         /* position correctly in bit stream */
    m<<=(bitoff&7);                         /* position correctly in bit stream */
    v=(ggetintellong(ptr) & ~m) | (v & m);  /* read and combine bits together */
    gsetintellong(ptr,v);                   /* write it out */
}


/****************************************************************/
/*                Compress and Decompress the data              */
/****************************************************************/

static int CompressDataDepth(char* CPtr,int NbCell,int CDepth)
{
    int                i;
    unsigned int*       ShortPtr;

    ShortPtr=(unsigned int*)CPtr;
    for (i=0; i<NbCell; i++) setfieldTGV(CPtr,i*CDepth,CDepth,(unsigned int)ShortPtr[i]);
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
            Value = getfieldTGV(CPtr,bitoff,CDepth);
            bitoff -= CDepth;
            if (Value&signbit) Value|=NegVal; /* Complement to 32 */
            *--d = (int) Value;
        }
    }
    else
    {
        for (i=0; i<NbCell; i++)
        {
            Value = getfieldTGV(CPtr,bitoff,CDepth);
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

static int OptimalDepthMatrix(int NbMV,int NbL,int NbL4,int Extra)
{
    int         NewDepth,MaxIndex;

    MaxIndex=NbMV+NbL+NbL4+Extra;           /* Total number of elements references by an index in the matrix */
    NewDepth=1;                             /* The minimum depth */
    while (MaxIndex>(1<<NewDepth)) NewDepth++;
    return(NewDepth);
}


/***********************************************************************************/
/*                Calcul the exact size of each element in the file                */
/***********************************************************************************/

static int GetSizeHeader()
{
    return(sizeof(FrameStruct));
}


static int GetSizeVector(TgvEnviro* Bvm)
{
    int    TheSize;

    TheSize=CalcC16Cell(Bvm->TheVector->NbMotionVector*2,kCompressVector)*sizeof(int);
    return(TheSize);
}


static int GetSizeMatrix(TgvEnviro* Bvm,int Extra)
{
    int    TheSize,NbCellMatrix,MatrixDepth;

    NbCellMatrix=Bvm->TheMatrix->xMatrix*Bvm->TheMatrix->yMatrix;
    MatrixDepth=OptimalDepthMatrix(Bvm->TheVector->NbMotionVector,Bvm->TheLiteral->NbLiteral,Bvm->TheLiteral4->NbLiteral,Extra);
    TheSize=(CalcC16Cell(NbCellMatrix,MatrixDepth)*sizeof(int));
    return(TheSize);
}


static int GetSizeLiterale(TgvEnviro* Bvm)
{
    int    TheSize;

    TheSize=(((int)Bvm->TheLiteral->NbLiteral)*sizeof(LiteralMatrix));             /* 16 byte * NbLiteral */
    TheSize+=(((int)Bvm->TheLiteral4->NbLiteral)*sizeof(Literal4Matrix));          /* 8 byte * NbLiteral */
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
        if (TheSize>(int)SizeRate) NbMotionVector-=xMatrix;
    } while (TheSize>(int)SizeRate);
    if (NbMotionVector<(kReservVector+1)) NbMotionVector=kReservVector+1;           /* There is a minimum of 1 vector and the kReservVector */
    return(NbMotionVector);
}


static int MoreLiteral(TgvEnviro* Bvm,unsigned int SizeRate)
{
    int    TheSize;

    TheSize=GetSizeHeader();            /* Header */
    TheSize+=GetSizeVector(Bvm);        /* Vector */
    TheSize+=GetSizeMatrix(Bvm,1);      /* Matrix */
    TheSize+=GetSizeLiterale(Bvm);      /* Literale */
    TheSize+=sizeof(LiteralMatrix);     /* The size of the next literal */
    if (Bvm->TheLiteral->NbLiteral<Bvm->TheLiteral->MaxLiteral)
    {
        if (Bvm->TheLiteral4->NbLiteral<Bvm->TheLiteral4->MaxLiteral)
        {
            if (TheSize<=(int)SizeRate) return(1);
        }
    }
    return(0);
}


/***********************************************************************************/
/*                                Matrix management                                */
/***********************************************************************************/

static void IndexMatrixToXY(TgvEnviro* Bvm,int Index,int* xScan,int* yScan)
{
    *yScan=Index/Bvm->TheMatrix->xMatrix;
    *xScan=Index-((*yScan)*Bvm->TheMatrix->xMatrix);
}


/***********************************************************************************/
/*                               Dictionary management                             */
/***********************************************************************************/

static int CheckAllDictionary(TgvEnviro* Bvm)
{
    int     i,CheckTotal;

    if (TGVDEBUG)
    {
        CheckTotal=0;
        for (i=0; i<Bvm->TheVector->NbMotionVector; i++)
        {
            CheckTotal+=Bvm->MVFrequency->Frequency[i];                     /* The total of elements is all the matrix */
        }
        for (i=0; i<Bvm->TheLiteral->NbLiteral; i++)
        {
            CheckTotal+=Bvm->LFrequency->Frequency[i];                      /* The total of elements is all the matrix */
        }
        for (i=0; i<Bvm->TheLiteral4->NbLiteral; i++)
        {
            CheckTotal+=Bvm->LFrequency4->Frequency[i];                     /* The total of elements is all the matrix */
        }
        if (CheckTotal!=(Bvm->TheMatrix->xMatrix*Bvm->TheMatrix->yMatrix))
        {
            return(0);
        }
    }
    return(1);
}


static int CompressDictionary(TgvEnviro* Bvm,int TypeDictionary)
{
    int             SizeRemap = 0,MaxElem = 0;
    FrequencyRec*   TheFrequency          = 0;
    int*            RemapIndex            = 0;
    int             DoRemap               = 0;
    int             MaxCell,i,NewIndex;
    WkMatrix*       WorkMatrix;

    switch (TypeDictionary)                                                                         /* Dispatch on the right type of dictionary */
    {
        case kMotionVector:
            MaxElem=Bvm->TheVector->NbMotionVector;
            Bvm->TheVector->NbMotionVector=0;                                                       /* Reset the counter */
            TheFrequency=Bvm->MVFrequency;
            break;
        case kFullLiteral:
            MaxElem=Bvm->TheLiteral->NbLiteral;
            Bvm->TheLiteral->NbLiteral=0;                                                           /* Reset the counter */
            TheFrequency=Bvm->LFrequency;
            break;
        case kLiteral4:
            MaxElem=Bvm->TheLiteral4->NbLiteral;
            Bvm->TheLiteral4->NbLiteral=0;                                                          /* Reset the counter */
            TheFrequency=Bvm->LFrequency4;
            break;
    }
    SizeRemap=sizeof(int)*MaxElem;
    RemapIndex=(int*)galloc(SizeRemap);                                                             /* Allocate the remap structure */
    if (RemapIndex)
    {
        NewIndex=0;
        for (i=0; i<MaxElem; i++)                                                                   /* Build a list of remapping values */
        {
            if (TheFrequency->Frequency[i])                                                         /* Keep this element */
            {
                switch (TypeDictionary)                                                             /* Dispatch on the right type of dictionary */
                {
                    case kMotionVector:
                        Bvm->TheVector->ListMV[NewIndex]=Bvm->TheVector->ListMV[i];                 /* Compress the list of vector */
                        (Bvm->TheVector->NbMotionVector)++;                                         /* Increment the counter */
                        break;
                    case kFullLiteral:
                        Bvm->TheLiteral->ListLiteral[NewIndex]=Bvm->TheLiteral->ListLiteral[i];     /* Compress the list of literal */
                        (Bvm->TheLiteral->NbLiteral)++;                                             /* Increment the counter */
                        break;
                    case kLiteral4:
                        Bvm->TheLiteral4->ListLiteral[NewIndex]=Bvm->TheLiteral4->ListLiteral[i];   /* Compress the list of literal */
                        (Bvm->TheLiteral4->NbLiteral)++;                                            /* Increment the counter */
                        break;
                }
                TheFrequency->Frequency[NewIndex]=TheFrequency->Frequency[i];                       /* Compress the list of frequence */
                RemapIndex[i]=NewIndex;                                                             /* Save the valid index */
                NewIndex++;
            }
            else                                                                                    /* Delete it */
            {
                RemapIndex[i]=kInvalidIndex;                                                        /* This value should not be used */
                DoRemap=1;                                                                          /* There is at least one remap to do */
            }
        }
        if (DoRemap)
        {
            MaxCell=Bvm->TheMatrix->yMatrix*Bvm->TheMatrix->xMatrix;
            WorkMatrix=Bvm->WorkMatrix;
            for (i=0; i<MaxCell; i++)
            {
                if ((int)(WorkMatrix[i].TypeDictionary)==TypeDictionary)                                   /* It is the right type of element */
                {
                    NewIndex=RemapIndex[Bvm->TheMatrix->MXIndex[i]];
                    Bvm->TheMatrix->MXIndex[i]=NewIndex;                                            /* Remap the value */
                }
            }
        }
        gfree((char*)RemapIndex);
        return(1);
    }
    return(0);
}


static void ClearInDictionary(TgvEnviro* Bvm,int TypeDictionary,int IndexElem)
{
    int             LastIndex    = 0, MaxCell = 0,i = 0;
    FrequencyRec*   TheFrequency = 0;
    WkMatrix*       WorkMatrix   = 0;

    switch (TypeDictionary)                                                                         /* Dispatch on the right type of dictionary */
    {
        case kMotionVector:TheFrequency=Bvm->MVFrequency;break;
        case kFullLiteral:TheFrequency=Bvm->LFrequency;break;
        case kLiteral4:TheFrequency=Bvm->LFrequency4;break;
    }
    (TheFrequency->Frequency[IndexElem])--;                                                         /* Dicrease the frequency */
    if (TheFrequency->Frequency[IndexElem]==0)                                                      /* This element is not used anymore */
    {
        switch (TypeDictionary)                                                                     /* Dispatch on the right type of dictionary */
        {
            case kMotionVector:
                (Bvm->TheVector->NbMotionVector)--;                                                 /* Decrement the counter */
                LastIndex=Bvm->TheVector->NbMotionVector;
                Bvm->TheVector->ListMV[IndexElem]=Bvm->TheVector->ListMV[LastIndex];                /* Compress the list of vector */
                break;
            case kFullLiteral:
                (Bvm->TheLiteral->NbLiteral)--;                                                     /* Decrement the counter */
                LastIndex=Bvm->TheLiteral->NbLiteral;
                Bvm->TheLiteral->ListLiteral[IndexElem]=Bvm->TheLiteral->ListLiteral[LastIndex];    /* Compress the list of literal */
                break;
            case kLiteral4:
                (Bvm->TheLiteral4->NbLiteral)--;                                                    /* Decrement the counter */
                LastIndex=Bvm->TheLiteral4->NbLiteral;
                Bvm->TheLiteral4->ListLiteral[IndexElem]=Bvm->TheLiteral4->ListLiteral[LastIndex];  /* Compress the list of literal */
                break;
        }
        TheFrequency->Frequency[IndexElem]=TheFrequency->Frequency[LastIndex];                      /* Compress the list of frequence */
        MaxCell=Bvm->TheMatrix->yMatrix*Bvm->TheMatrix->xMatrix;
        WorkMatrix=Bvm->WorkMatrix;
        for (i=0; i<MaxCell; i++)
        {
            if ((int)(WorkMatrix[i].TypeDictionary)==TypeDictionary)                                       /* It is the right type of element */
            {
                if (Bvm->TheMatrix->MXIndex[i]==LastIndex)                                          /* Swap the last element with the invalid one */
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

static void BuildBaseVector(TgvEnviro* Bvm)
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


static int BuildVectorDictionary(TgvGimexData* tgv,TgvEnviro* Bvm,char* Source,int Rowbytes)
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
            Scr=Source+(y*Rowbytes)+x;                                          /* Same source for the whole search */
            SearchMotionVector( tgv->fError,Bvm,
                                Scr,Rowbytes,tgv->fPrevSource,tgv->fWidth,
                                x,y,&BestMatch,&BVIndex);
            if ((BestMatch<=MinError) && (BVIndex!=kInvalidIndex))
            {
                (Bvm->MVFrequency->Frequency[BVIndex])++;                       /* This vector is used one more time */
            }
            else
            {
                GetBestDelta8(  tgv->fError,Scr,Rowbytes,
                                tgv->fPrevSource,tgv->fWidth,w,h,
                                &FindBestMatch,&MotionVector);
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

static int DelDupplicateVectors(TgvGimexData* tgv,MVector* BLocation,TgvEnviro* Bvm)
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
            if (SizeMultiVector >= (int)(sizeof(LiteralMatrix)))                                         /* The vectors using the same bloc are bigger that the bloc itself */
            {
                LoadLiteral((char*)&TheLiteral,BlRef.x,BlRef.y,tgv->fPrevSource,tgv->fWidth);   /* Load this literal bloc */
                TypeDictionary=kFullLiteral;                                                    /* Use a full literal */
                AddLiteral(Bvm->TheLiteral,Bvm->LFrequency,&TheLiteral,&BLIndex);               /* Add a new literal in the list */
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
                                    (Bvm->LFrequency->Frequency[BLIndex])++;                    /* This literal is used one more time */
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


static int RemapAllVectors(TgvGimexData* tgv,TgvEnviro* Bvm,char* Source,int Rowbytes)
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
            Scr=Source+(y*Rowbytes)+x;                                                  /* Same source for the whole search */
            SearchMotionVector( tgv->fError,Bvm,Scr,Rowbytes,
                                tgv->fPrevSource,tgv->fWidth,
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
            if (CheckAllDictionary(Bvm)) CheckError=DelDupplicateVectors(tgv,BLocation,Bvm);
        }
        gfree((char*)BLocation);
    }
    return(CheckError);
}


/***********************************************************************************/
/*                                Calcul of literals                               */
/***********************************************************************************/

static void MergeColour(ARGB* COrg,ARGB* CAux,int StatOrigin,int StatMerge)
{
    int    Total;

    Total=StatMerge+StatOrigin;
    COrg->r=(unsigned char) (((int)COrg->r)+(((((int)CAux->r)-((int)COrg->r))*StatMerge)/Total));
    COrg->g=(unsigned char) (((int)COrg->g)+(((((int)CAux->g)-((int)COrg->g))*StatMerge)/Total));
    COrg->b=(unsigned char) (((int)COrg->b)+(((((int)CAux->b)-((int)COrg->b))*StatMerge)/Total));
}


static int SearchBestColour(ARGB SearchColour,ARGB* ThePalette,int NbEntries)
{
    int             i,BestIndex=0;
    unsigned int   BestDelta,TempDelta;

    for (i=0; i<NbEntries; i++)
    {
        if (ThePalette[NbEntries-1-i].a) BestIndex=NbEntries-1-i;   /* Search a valid color */
    }
    BestDelta=CalcDeltaColour(SearchColour,ThePalette[BestIndex]);  /* BestIndex is the default value */
    for (i=0; i<NbEntries; i++)
    {
        if (ThePalette[NbEntries-1-i].a)                            /* its a valid color */
        {
            TempDelta=CalcDeltaColour(SearchColour,ThePalette[i]);
            if (TempDelta<BestDelta)
            {
                BestDelta=TempDelta;
                BestIndex=i;
            }
        }
    }
    return(BestIndex);
}


static int CalcHistogram(LiteralMatrix* TheLiteral,LiteralMatrix* Histogram,LiteralMatrix* ThePalette)
{
    int     i,j,NbColour;

    memset(Histogram,1,sizeof(LiteralMatrix));                              /* Each pixel is used one time */
    NbColour=0;
    for (i=0; i<kXResYRes; i++)                                             /* Scane the literal */
    {
        if (Histogram->PMatrix[i])                                          /* Its not handle for now */
        {
            ThePalette->PMatrix[NbColour]=TheLiteral->PMatrix[i];           /* Set this indexed Colour */
            Histogram->PMatrix[NbColour]=0;                                 /* Reset the histogram */
            for (j=i; j<kXResYRes; j++)                                     /* Scane until the end of the literal */
            {
                if (TheLiteral->PMatrix[j]==ThePalette->PMatrix[NbColour])  /* This is the same Colour */
                {
                    Histogram->PMatrix[j]=0;                                /* Reset this pixel */
                    (Histogram->PMatrix[NbColour])++;                       /* Add this pixel */
                }
            }
            NbColour++;                                                     /* One more unic Colour */
        }
    }
    return(NbColour);
}


static void SortPalette(LiteralMatrix* Histogram,LiteralMatrix* ThePalette,int NbColour)
{
    int             i,SwitchColour=1;
    unsigned char   TempVal;

    while (SwitchColour)                                                    /* Sort the palette by popularity */
    {
        SwitchColour=0;
        for (i=1; i<NbColour; i++)
        {
            if (Histogram->PMatrix[i]>Histogram->PMatrix[i-1])
            {
                TempVal=Histogram->PMatrix[i-1];
                Histogram->PMatrix[i-1]=Histogram->PMatrix[i];
                Histogram->PMatrix[i]=TempVal;
                TempVal=ThePalette->PMatrix[i-1];
                ThePalette->PMatrix[i-1]=ThePalette->PMatrix[i];
                ThePalette->PMatrix[i]=TempVal;
                SwitchColour=1;
            }
        }

    }
}


static void ReduceListRGB(LiteralMatrix* Histogram,ARGB* ThePalette,int NbColour,int NbSearchColour)
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
                    TempDelta=CalcDeltaColour(ThePalette[i],ThePalette[j]);
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
        MergeColour(&ThePalette[aIndex],&(ThePalette[bIndex]),Histogram->PMatrix[aIndex],Histogram->PMatrix[bIndex]);
        Histogram->PMatrix[aIndex]=(unsigned char)(Histogram->PMatrix[aIndex]+Histogram->PMatrix[bIndex]);
        NbColour--;
        ThePalette[bIndex]=ThePalette[NbColour];                                    /* Update the last color */
        Histogram->PMatrix[bIndex]=Histogram->PMatrix[NbColour];                    /* Update the last frequency */
    }
}


static void RemapLiteral(unsigned int* ListError,LiteralMatrix* BestPalette,int NbColour,LiteralMatrix* ResultLiteral)
{
    unsigned char   RemapTable[256];
    int             i,j,BestIndex;
    unsigned int   BestDelta;
    unsigned int    Index;

    memset(RemapTable,0,256);                                                   /* Initialized the remap table */
    for (i=0; i<kXResYRes; i++)                                                 /* Calcul the remap table */
    {
        BestDelta=kLowestMatch;
        BestIndex=0;
        for (j=0; j<NbColour; j++)                                              /* Search the best match in the best palette */
        {
            Index=(unsigned char)ResultLiteral->PMatrix[i];
            Index<<=8;
            Index|=(unsigned char)BestPalette->PMatrix[j];
            if (ListError[Index]<BestDelta)
            {
                BestDelta=ListError[Index];
                BestIndex=BestPalette->PMatrix[j];                              /* Save the value */
            }
        }
        RemapTable[ResultLiteral->PMatrix[i]]=(unsigned char)(BestIndex);                        /* Set the best match in the remap table */
    }
    for (i=0; i<kXResYRes; i++)
    {
        ResultLiteral->PMatrix[i]=RemapTable[ResultLiteral->PMatrix[i]];        /* Remap the literal */
    }
}


static void ColorReduceLiteral(ARGB* ColourPalette,int NbEntries,unsigned int* ListError,int NbSearchColour,LiteralMatrix* TheLiteral,LiteralMatrix* ResultLiteral)
{
    int             i,NbColour;
    LiteralMatrix   Histogram,ThePalette;
    LiteralMatrix   BestPalette,PopPalette;
    LiteralMatrix   PopLiteral,ListColorLiteral;
    unsigned int   PopPixDelta,ListColorDelta;
    ARGB            RGBPalette[kXResYRes];
    char*           Prev;
    char*           Source;

    *ResultLiteral=*TheLiteral;                                                 /* Perfect quality */
    NbColour=CalcHistogram(TheLiteral,&Histogram,&ThePalette);
    if (NbColour>NbSearchColour)                                                /* Need to reduced the number of colours */
    {
        SortPalette(&Histogram,&ThePalette,NbColour);                           /* Sort the palette by popularity */
        PopPalette=ThePalette;                                                  /* Save the popular palette */
        for (i=0; i<NbColour; i++)                                              /* Build the basic RGB palette */
        {
            RGBPalette[i]=ColourPalette[ThePalette.PMatrix[i]];
        }
        ReduceListRGB(&Histogram,RGBPalette,NbColour,NbSearchColour);           /* Color reduce */
        for (i=0; i<NbSearchColour; i++)                                        /* Remap the best palette */
        {
            BestPalette.PMatrix[i]=(unsigned char)(SearchBestColour(RGBPalette[i],ColourPalette,NbEntries));
        }
        PopLiteral=*TheLiteral;
        ListColorLiteral=*TheLiteral;
        RemapLiteral(ListError,&PopPalette,NbSearchColour,&PopLiteral);         /* Remap the popular literal */
        RemapLiteral(ListError,&BestPalette,NbSearchColour,&ListColorLiteral);  /* Remap the transform literal */
        Source=(char*)TheLiteral;
        Prev=(char*)&PopLiteral;
        CalcDelta8(ListError,Source,kXRes,Prev,kXRes,PopPixDelta);
        Source=(char*)TheLiteral;
        Prev=(char*)&ListColorLiteral;
        CalcDelta8(ListError,Source,kXRes,Prev,kXRes,ListColorDelta);
        if (ListColorDelta<PopPixDelta) *ResultLiteral=ListColorLiteral;        /* Wich one has the best error */
        else *ResultLiteral=PopLiteral;
    }
}


static void CalculLiteral(TgvGimexData* tgv,int NbColour,LiteralMatrix* TheLiteral,LiteralMatrix* ResultLiteral,unsigned int* BestMatch)
{
    char*           Prev;
    char*           Source;
    unsigned int    ListColorDelta;
    LiteralMatrix   ListColorLiteral;

    ColorReduceLiteral(tgv->fColourTable,tgv->fNbColour,tgv->fError,NbColour,TheLiteral,&ListColorLiteral);
    Source=(char*)TheLiteral;
    Prev=(char*)&ListColorLiteral;
    CalcDelta8(tgv->fError,Source,kXRes,Prev,kXRes,ListColorDelta);
    *BestMatch=ListColorDelta;
    *ResultLiteral=ListColorLiteral;
}


/***********************************************************************************/
/*                         decompression of the literal                            */
/***********************************************************************************/

static void DecompressLiteral4(unsigned char* Scr,LiteralMatrix* DestLiteral)
{
    unsigned char   c[4];
    unsigned int   matrix=0;

    c[0]=Scr[0];
    c[1]=Scr[1];
    c[2]=Scr[2];
    c[3]=Scr[3];
    matrix=ggetintellong(Scr+4);
    DestLiteral->PMatrix[0]= c[(matrix>>30)&3];
    DestLiteral->PMatrix[1]= c[(matrix>>28)&3];
    DestLiteral->PMatrix[2]= c[(matrix>>26)&3];
    DestLiteral->PMatrix[3]= c[(matrix>>24)&3];
    DestLiteral->PMatrix[4]= c[(matrix>>22)&3];
    DestLiteral->PMatrix[5]= c[(matrix>>20)&3];
    DestLiteral->PMatrix[6]= c[(matrix>>18)&3];
    DestLiteral->PMatrix[7]= c[(matrix>>16)&3];
    DestLiteral->PMatrix[8]= c[(matrix>>14)&3];
    DestLiteral->PMatrix[9]= c[(matrix>>12)&3];
    DestLiteral->PMatrix[10]= c[(matrix>>10)&3];
    DestLiteral->PMatrix[11]= c[(matrix>>8)&3];
    DestLiteral->PMatrix[12]= c[(matrix>>6)&3];
    DestLiteral->PMatrix[13]= c[(matrix>>4)&3];
    DestLiteral->PMatrix[14]= c[(matrix>>2)&3];
    DestLiteral->PMatrix[15]= c[(matrix>>0)&3];
}


static void DecompressLiteral(LiteralMatrix* ListLiteral,int NbLiteral,int TypeDictionary)
{
    unsigned char*  LiteralPtr = 0;
    int             i          = 0;

    if (NbLiteral)
    {
        switch (TypeDictionary)                     /* Dispatch on the right type of dictionary */
        {
            case kLiteral4:
                LiteralPtr=(unsigned char*)ListLiteral;
                LiteralPtr+=((NbLiteral-1)*sizeof(Literal4Matrix));
                break;
        }
        for (i=NbLiteral; i>0; i--)                 /* Scanne the dictionary */
        {
            switch (TypeDictionary)                 /* Dispatch on the right type of dictionary */
            {
                case kLiteral4:
                    DecompressLiteral4(LiteralPtr,&ListLiteral[i-1]);
                    LiteralPtr-=sizeof(Literal4Matrix);
                    break;
            }
        }
    }
}


/***********************************************************************************/
/*                           Compression of the literal                            */
/***********************************************************************************/

static void CompressLiteral4(unsigned char *Dest,LiteralMatrix* ScrLiteral)
{
    unsigned char c[4];
    unsigned char colour;
    int i;
    int j;
    int nextc=0;
    unsigned int cmatrix=0;

    for (j=0; j<4; ++j)
        c[j] = 0;

    for (i=0; i<kXResYRes; i++)
    {
        colour=ScrLiteral->PMatrix[i];

        for (j=0; j<nextc; ++j)
            if (colour==c[j])
                break;

        if (j>=nextc)
            c[nextc++] = colour;

        cmatrix = (cmatrix<<2)|j;
    }
    Dest[0]=c[0];
    Dest[1]=c[1];
    Dest[2]=c[2];
    Dest[3]=c[3];
    TGVSETADRESS(Dest+4,cmatrix,4);
}


static void CompressLiteral(LiteralMatrix* ListLiteral,int NbLiteral,int TypeDictionary)
{
    unsigned char*  LiteralPtr;
    int             i;

    LiteralPtr=(unsigned char*)ListLiteral;
    for (i=0; i<NbLiteral; i++)                 /* Scanne the dictionary */
    {
        switch (TypeDictionary)                 /* Dispatch on the right type of dictionary */
        {
            case kLiteral4:
                CompressLiteral4(LiteralPtr,&ListLiteral[i]);
                LiteralPtr+=sizeof(Literal4Matrix);
                break;
        }
    }
}


/***********************************************************************************/
/*            Calcul the dictionary and replace the right cell in the matrix       */
/***********************************************************************************/

static int FindNextLiteral(TgvEnviro* Bvm,int* xIndex,int* yIndex)
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


static void SearchBestLiteral(TgvGimexData* tgv,TgvEnviro* Bvm,LiteralMatrix* TheLiteral,int *TypeDictionary,unsigned int *BestMatch,int *BLIndex)
{
    unsigned int   TempMatch;
    int             TempIndex;

    *TypeDictionary=kFullLiteral;
    SearchLiteral(tgv->fError,Bvm->TheLiteral,TheLiteral,BestMatch,BLIndex);        /* The default dictionary is the full literal dictionary */
    SearchLiteral(tgv->fError,Bvm->TheLiteral4,TheLiteral,&TempMatch,&TempIndex);   /* Try literal 4 dictionary */
    if (TempMatch<(*BestMatch))                                                     /* It is better */
    {
        *TypeDictionary=kLiteral4;
        *BestMatch=TempMatch;
        *BLIndex=TempIndex;
    }
}


static int BuildLiteralDictionary(unsigned int SizeRate,TgvGimexData* tgv,TgvEnviro* Bvm,char* Source,int Rowbytes)
{
    int             IndexMatrix,TypeDictionary;
    int             SaveBLIndex,SaveTypeDictionary;
    LiteralMatrix   TheLiteral,CalcLiteral;
    int             xIndex,yIndex;
    unsigned int   BestMatch;
    int             BLIndex;

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
            CalculLiteral(tgv,4,&TheLiteral,&CalcLiteral,&BestMatch);                   /* Build a literal 4 */
            if (BestMatch<Bvm->WorkMatrix[IndexMatrix].BestMatch)                       /* It is better that the previous error */
            {
                TypeDictionary=kLiteral4;
                AddLiteral(Bvm->TheLiteral4,Bvm->LFrequency4,&CalcLiteral,&BLIndex);    /* Add a new literal in the literal 4 dictionary */
            }
            else
            {
                TypeDictionary=kFullLiteral;
                AddLiteral(Bvm->TheLiteral,Bvm->LFrequency,&TheLiteral,&BLIndex);       /* Add a new literal in the list */
                BestMatch=0;                                                            /* The best part of this function */
            }
        }
        switch (TypeDictionary)                                                         /* Dispatch on the right type of dictionary */
        {
            case kFullLiteral:
                (Bvm->LFrequency->Frequency[BLIndex])++;                                /* This literal is used one more time */
                break;
            case kLiteral4:
                (Bvm->LFrequency4->Frequency[BLIndex])++;                               /* This literal is used one more time */
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

static int RemapAllLiteral(TgvGimexData* tgv,TgvEnviro* Bvm,char* Source,int Rowbytes)
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
                case kFullLiteral:
                    (Bvm->LFrequency->Frequency[BLIndex])++;                        /* This literal is used one more time */
                    break;
                case kLiteral4:
                    (Bvm->LFrequency4->Frequency[BLIndex])++;                       /* This literal is used one more time */
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


static int CalculBestLiteral(TgvGimexData* tgv,TgvEnviro* Bvm,char* Source,int Rowbytes,int NbBloc,int TypeDictionary,int SearchIndex,LiteralMatrix* ResultLiteral)
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
                if ((int)(WorkMatrix[i].TypeDictionary)==TypeDictionary)                   /* It is the right type of element */
                {
                    if (Bvm->TheMatrix->MXIndex[i]==SearchIndex)                    /* Search the blocs with the right Index */
                    {
                        IndexMatrixToXY(Bvm,i,&xScan,&yScan);
                        LoadLiteral(    (char*)&TheLiteral,
                                        xScan*kXRes,yScan*kYRes,
                                        Source,Rowbytes);                           /* Load the bloc */
                        if (TypeDictionary==kLiteral4)                              /* Color reduce in 4 colors */
                        {
                            ColorReduceLiteral( tgv->fColourTable,
                                                tgv->fNbColour,
                                                tgv->fError,
                                                4,
                                                &TheLiteral,
                                                &ListBloc[Count]);
                        }
                        else ListBloc[Count]=TheLiteral;                            /* Save the bloc in the list */
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


static int CalculBestDictionary(TgvGimexData* tgv,TgvEnviro* Bvm,char* Source,int Rowbytes,FrequencyRec* TheFrequency,DLiteral* TheDictionary,int TypeDictionary)
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


static int CalculBestDictionaries(TgvGimexData* tgv,TgvEnviro* Bvm,char* Source,int Rowbytes)
{
    if (CalculBestDictionary(tgv,Bvm,Source,Rowbytes,Bvm->LFrequency,Bvm->TheLiteral,kFullLiteral))
    {
        if (CalculBestDictionary(tgv,Bvm,Source,Rowbytes,Bvm->LFrequency4,Bvm->TheLiteral4,kLiteral4))
        {
            return(1);
        }
    }
    return(0);
}


/***********************************************************************************/
/*                         Compression of the final matrix                         */
/***********************************************************************************/


static int CompressMatrix(TgvEnviro* Bvm)
{
    DIMatrix*       TheMatrix;
    WkMatrix*       WorkMatrix;
    int             MaxCell,i,TypeDictionary;
    int             NbMotionVector,NbLiteral,NbLiteral4;

    TheMatrix=Bvm->TheMatrix;
    WorkMatrix=Bvm->WorkMatrix;
    MaxCell=TheMatrix->yMatrix*TheMatrix->xMatrix;
    NbMotionVector=Bvm->TheVector->NbMotionVector;
    NbLiteral=Bvm->TheLiteral->NbLiteral;
    NbLiteral4=Bvm->TheLiteral4->NbLiteral;
    if (TGVDEBUG)
    {
        for (i=0; i<NbMotionVector; i++) Bvm->MVFrequency->Frequency[i]=0;  /* Reset the frequence */
        for (i=0; i<NbLiteral; i++) Bvm->LFrequency->Frequency[i]=0;        /* Reset the frequence */
        for (i=0; i<NbLiteral4; i++) Bvm->LFrequency4->Frequency[i]=0;      /* Reset the frequence */
    }
    for (i=0; i<MaxCell; i++)
    {
        TypeDictionary=WorkMatrix[i].TypeDictionary;
        if (TGVDEBUG)
        {
            switch (TypeDictionary)
            {
                case kMotionVector:(Bvm->MVFrequency->Frequency[TheMatrix->MXIndex[i]])++;break;
                case kFullLiteral:(Bvm->LFrequency->Frequency[TheMatrix->MXIndex[i]])++;break;
                case kLiteral4:(Bvm->LFrequency4->Frequency[TheMatrix->MXIndex[i]])++;break;
            }
        }
        if (TypeDictionary!=kMotionVector)                                  /* Calcul the real index of each elements */
        {
            TheMatrix->MXIndex[i]+=NbMotionVector;
            if (TypeDictionary!=kFullLiteral)
            {
                TheMatrix->MXIndex[i]+=NbLiteral;
                if (TypeDictionary!=kLiteral4) TheMatrix->MXIndex[i]+=NbLiteral4;
            }
        }
    }
    if (TGVDEBUG)
    {
        for (i=0; i<NbMotionVector; i++) if (!Bvm->MVFrequency->Frequency[i]) return(0);                                                        /* Check unused element */
        for (i=0; i<NbLiteral; i++) if (!Bvm->LFrequency->Frequency[i]) return(0);                                                      /* Check unused element */
        for (i=0; i<NbLiteral4; i++) if (!Bvm->LFrequency4->Frequency[i]) return(0);                                                        /* Check unused element */
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
    TheSize=(TheSize+3)&~3; /* The matrix as to be align on 32 bits */
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


static void FreeEnviro(TgvEnviro* TheEnviro)
{
    if (TheEnviro->TheMatrix) gfree((char*)TheEnviro->TheMatrix);
    if (TheEnviro->WorkMatrix) gfree((char*)TheEnviro->WorkMatrix);
    if (TheEnviro->TheVector) gfree((char*)TheEnviro->TheVector);
    if (TheEnviro->MVFrequency) gfree((char*)TheEnviro->MVFrequency);
    if (TheEnviro->TheLiteral) gfree((char*)TheEnviro->TheLiteral);
    if (TheEnviro->LFrequency) gfree((char*)TheEnviro->LFrequency);
    if (TheEnviro->TheLiteral4) gfree((char*)TheEnviro->TheLiteral4);
    if (TheEnviro->LFrequency4) gfree((char*)TheEnviro->LFrequency4);
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


static TgvEnviro* AllocateEnviro(int Width,int Height,unsigned int SizeRate)
{
    TgvEnviro*  ResultEnviro;
    int         xMatrix,yMatrix;
    int        TheSize;
    int         MaxMotionVector;

    xMatrix=Width/kXRes;
    yMatrix=Height/kYRes;
    ResultEnviro=(TgvEnviro*)galloc(sizeof(TgvEnviro));
    if (ResultEnviro)
    {
        ResultEnviro->TheMatrix=0L;             /* xMatrix*yMatrix of index */
        ResultEnviro->WorkMatrix=0L;            /* xMatrix*yMatrix of working elements */
        ResultEnviro->TheVector=0L;             /* The dictionnary of vectors */
        ResultEnviro->MVFrequency=0L;           /* The frequency record for the vectors */
        ResultEnviro->TheLiteral=0L;            /* The dictionnary of literals of 16 Colours */
        ResultEnviro->LFrequency=0L;            /* The frequency record for the literals of 16 Colours */
        ResultEnviro->TheLiteral4=0L;           /* The dictionnary of literals using 4 Colours */
        ResultEnviro->LFrequency4=0L;           /* The frequency record for the literals using 4 Colours */
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
                        ResultEnviro->TheLiteral=AllocateLiteral(0,kMaxFullLiteral);
                        if (ResultEnviro->TheLiteral)
                        {
                            ResultEnviro->LFrequency=AllocateFrequencyRec(kMaxFullLiteral);
                            if (ResultEnviro->LFrequency)
                            {
                                ResultEnviro->TheLiteral4=AllocateLiteral(0,kMaxLiteral4);
                                if (ResultEnviro->TheLiteral4)
                                {
                                    ResultEnviro->LFrequency4=AllocateFrequencyRec(kMaxLiteral4);
                                    if (ResultEnviro->LFrequency4) return(ResultEnviro);
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


/***********************************************************************************/
/*                       Change and save the current palette                       */
/***********************************************************************************/

static int ChangePalette(TgvGimexData* tgv,GINFO* ginfo)
{
    int     NbColours,i,DiffColour=0;

    NbColours=ginfo->numcolours;
    if (NbColours!=tgv->fNbColour) DiffColour=1;
    else
    {
        for (i=0; i<NbColours; i++)
        {
            if (!DiffColour)
            {
                if (ginfo->colourtbl[i].r!=tgv->fColourTable[i].r) DiffColour=1;
                if (ginfo->colourtbl[i].g!=tgv->fColourTable[i].g) DiffColour=1;
                if (ginfo->colourtbl[i].b!=tgv->fColourTable[i].b) DiffColour=1;
            }
        }
    }
    if (DiffColour)
    {
        memcpy(tgv->fColourTable,ginfo->colourtbl,NbColours*sizeof(ARGB));
        tgv->fStartColour=0;
        tgv->fNbColour=NbColours;
        CalculAllError(tgv);
    }
    return(DiffColour);
}


/***********************************************************************************/
/*                                    TGV write                                    */
/***********************************************************************************/

static int tgvwrite(GSTREAM *g, void *buf, int size)
{
    if (size) return(gwrite(g,buf,size));
    else return(1);
}


/***********************************************************************************/
/*                                   Write in file                                 */
/***********************************************************************************/

static char* CompressKeyFrame(GINFO* ginfo,char* source,int rowbytes,int nbcolour,int* PackSize)
{
    int    OrgSize,DeltaStructure;
    int    i;
    char*   OrgBuffer;
    char*   DestBuffer;
    char*   PackBuffer=0L;

    *PackSize=0;
    OrgSize=ginfo->height*ginfo->width;
    OrgBuffer=(char*)galloc(OrgSize);
    if (OrgBuffer)
    {
        DestBuffer=OrgBuffer;
        for (i=0; i<ginfo->height; i++)                                 /* Copy all the ligne in the buffer */
        {
            memcpy(DestBuffer,source,ginfo->width);
            DestBuffer+=ginfo->width;
            source+=rowbytes;
        }
        DeltaStructure=sizeof(KeyFrameStruct);                          /* Structure to store the information of the key frame */
        DeltaStructure+=nbcolour*kSizeColour;                           /* Add the size of the colour palette */
        PackBuffer=(char*)galloc(DeltaStructure+(OrgSize*2)+8192);      /* Plus the double of the original buffer and the a constant */
        if (PackBuffer)
        {
            if (ginfo->subtype&2)
            {
                char *d=PackBuffer+DeltaStructure;
                memcpy(d,OrgBuffer,OrgSize);
                *PackSize=OrgSize;
            }
            else
                *PackSize=REF_encode(PackBuffer+DeltaStructure,OrgBuffer,OrgSize,0);

            *PackSize=(*PackSize)+DeltaStructure;                       /* Add the size of the structure */
            *PackSize=(*PackSize+3)&~3;                                 /* To align on 32 bits */
        }
        gfree(OrgBuffer);
    }
    return(PackBuffer);
}


static int SaveKeyFrame(GSTREAM *g,GINFO* ginfo,KeyFrameStruct* TheKeyFrame,int PackSize)
{
    char*   ColourAdr;
    int    i,SizePixel;

    TGVPUTFIELD(TheKeyFrame->chunkid,TGVK);
    TGVPUTFIELD(TheKeyFrame->chunksize,PackSize);
    TGVPUTFIELD(TheKeyFrame->w,ginfo->width);
    TGVPUTFIELD(TheKeyFrame->h,ginfo->height);
    TGVPUTFIELD(TheKeyFrame->startcolour,0);
    TGVPUTFIELD(TheKeyFrame->nbcolour,ginfo->numcolours);
    TGVPUTFIELD(TheKeyFrame->flags,(ginfo->subtype&2)?1:0);

    ColourAdr=(char*)TheKeyFrame+sizeof(KeyFrameStruct);
    for (i=0; i<ginfo->numcolours; i++)
    {
        *ColourAdr++=ginfo->colourtbl[i].r;
        *ColourAdr++=ginfo->colourtbl[i].g;
        *ColourAdr++=ginfo->colourtbl[i].b;
    }
    SizePixel=PackSize-(ginfo->numcolours*kSizeColour)-sizeof(KeyFrameStruct);
    if (tgvwrite(g,(char*)TheKeyFrame,PackSize)) return(1);
    return(0);
}


static int SaveFrame(GSTREAM *g,TgvEnviro* Bvm)
{
    FrameStruct     TheFrame;
    int            TotalSize,MatrixDepth;
    int            TheSize,NbLong;
    int             NbCellMatrix;
    int             ok=0;

    TotalSize=GetSizeHeader();
    TotalSize+=GetSizeVector(Bvm);
    TotalSize+=GetSizeMatrix(Bvm,0);
    TotalSize+=GetSizeLiterale(Bvm);
    MatrixDepth=OptimalDepthMatrix(Bvm->TheVector->NbMotionVector,Bvm->TheLiteral->NbLiteral,Bvm->TheLiteral4->NbLiteral,0);
    TGVPUTFIELD(TheFrame.chunkid,TGVF);
    TGVPUTFIELD(TheFrame.chunksize,TotalSize);
    TGVPUTFIELD(TheFrame.NbMotionVector,Bvm->TheVector->NbMotionVector);
    TGVPUTFIELD(TheFrame.NbLiteral16,Bvm->TheLiteral->NbLiteral);
    TGVPUTFIELD(TheFrame.NbLiteral4,Bvm->TheLiteral4->NbLiteral);
    TGVPUTFIELD(TheFrame.MatrixDepth,MatrixDepth);
    TGVPUTFIELD(TheFrame.reserved,0);
    TheSize=sizeof(FrameStruct);
    if (tgvwrite(g,(char*)&TheFrame,TheSize))
    {
        if (CompressDataDepth((char*)Bvm->TheVector->ListMV,Bvm->TheVector->NbMotionVector*2,kCompressVector))
        {
            NbLong=CalcC16Cell(Bvm->TheVector->NbMotionVector*2,kCompressVector);
            if (tgvwrite(g,(char*)Bvm->TheVector->ListMV,sizeof(int)*NbLong))
            {
                NbLong=(Bvm->TheLiteral->NbLiteral*sizeof(LiteralMatrix))/sizeof(int);
                if (tgvwrite(g,(char*)Bvm->TheLiteral->ListLiteral,sizeof(int)*NbLong))
                {
                    CompressLiteral(Bvm->TheLiteral4->ListLiteral,Bvm->TheLiteral4->NbLiteral,kLiteral4);
                    NbLong=(Bvm->TheLiteral4->NbLiteral*sizeof(Literal4Matrix))/sizeof(int);
                    if (tgvwrite(g,(char*)Bvm->TheLiteral4->ListLiteral,sizeof(int)*NbLong))
                    {
                        NbCellMatrix=Bvm->TheMatrix->xMatrix*Bvm->TheMatrix->yMatrix;


/* refpack here */


                        if (CompressDataDepth((char*)Bvm->TheMatrix->MXIndex,NbCellMatrix,MatrixDepth))
                        {
                            NbLong=CalcC16Cell(NbCellMatrix,MatrixDepth);
                            if (tgvwrite(g,(char*)Bvm->TheMatrix->MXIndex,sizeof(int)*NbLong)) ok=1;
                            DecompressDataDepth((char*)Bvm->TheMatrix->MXIndex,NbCellMatrix,MatrixDepth,0);
                        }
                    }
                    DecompressLiteral(Bvm->TheLiteral4->ListLiteral,Bvm->TheLiteral4->NbLiteral,kLiteral4);
                }
            }
            DecompressDataDepth((char*)Bvm->TheVector->ListMV,Bvm->TheVector->NbMotionVector*2,kCompressVector,1);
        }
    }
    return(ok);
}


/****************************************************************/
/*       Copy in the current frame in the previous buffer       */
/****************************************************************/

static int CheckBuffers(struct TgvGimexData *tgv,int width,int height,int IsKeyFrame)
{
    int    bitmapsize;

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
        if (!tgv->fPrevSource)
        {
            bitmapsize=width*height;
            tgv->fPrevSource=(char*)galloc(bitmapsize);
            if (!tgv->fPrevSource) return(0);
        }
    }
    return(1);
}


static void CopyBuffer(char *source,int srowbytes,char *dest,int drowbytes,int height)
{
    int         y,bpl;

    if (srowbytes<drowbytes) bpl=srowbytes;
    else bpl=drowbytes;
    for (y=0; y<height; ++y)
    {
        memcpy(dest,source,bpl);
        dest+=drowbytes;
        source+=srowbytes;
    }
}


/****************************************************************/
/*             Decompress in the source buffer                  */
/****************************************************************/

static void DrawVector(char* Source,int SRowbytes,char* Prev,int PRowbytes,int x,int y,MVector* MotionVector)
{
    Source+=x+(y*SRowbytes);
    x+=MotionVector->x;
    y+=MotionVector->y;
    Prev+=x+(y*PRowbytes);
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
    NbMotionVector = (short)(TheVector->NbMotionVector);
    NbLiteral      = (short)(TheLiteral->NbLiteral);
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

int GCALL TGV_wopen(GINSTANCE **gx, GSTREAM *g, const char *pathname, int numframes)
{
    GINSTANCE*      Result=0L;
    TgvGimexData*   tgv;

    tgv=(TgvGimexData*)galloc(sizeof(TgvGimexData));
    if (tgv)
    {
        memset(tgv,0,sizeof(TgvGimexData));
        tgv->fPrevSource=0L;
        tgv->fWidth=0;
        tgv->fHeight=0;
        tgv->fStartColour=0;
        tgv->fNbColour=0;
        tgv->fError=0L;
        Result=(GINSTANCE*)galloc(sizeof(GINSTANCE));
        if (Result)
        {
            tgv->fError=(unsigned int*)galloc(sizeof(unsigned int)*kMaxNumError);
            if (tgv->fError)
            {
                memset(Result,0,sizeof(GINSTANCE));
                Result->frames=0;
                Result->gstream=g;
                Result->gref=(char*)tgv;
                *gx=Result;
                return(1);
            }
            gfree((char*)Result);
        }
        gfree((char*)tgv);
    }
    return(0);
}


int GCALL TGV_write(GINSTANCE *gx, const GINFO *ginfo, char *source, int rowbytes)
{
    TgvGimexData*       tgv;
    TgvEnviro*          Bvm;
    int                 ok=0;
    unsigned int       SizeRate;
    int                 IsKeyFrame=0;
    KeyFrameStruct*     TheKeyFrame=0L;
    int                PackSize;
    int                 NbLiteral,i;
    DLiteral*           AllLiteral;
    char*               TheSource;
    GINFO*              TheGINFO;

    if (ginfo->bpp<=8)
    {
        TheGINFO=(GINFO*)galloc(sizeof(GINFO));
        if (TheGINFO)
        {
            memcpy(TheGINFO,ginfo,sizeof(GINFO));
            TheGINFO->width=((ginfo->width+7)>>3)<<3;       /* Aligned on 8*4 */
            TheGINFO->height=((ginfo->height+3)>>2)<<2;
            TheSource=(char*)galloc(TheGINFO->width*TheGINFO->height);
            if (TheSource)
            {
                memset(TheSource,0,TheGINFO->width*TheGINFO->height);
                CopyBuffer(source,rowbytes,TheSource,TheGINFO->width,ginfo->height);
                SizeRate=((unsigned int)TheGINFO->height)*((unsigned int)TheGINFO->width);
                SizeRate=(SizeRate*((unsigned int)TheGINFO->quality))/100;
                tgv=(TgvGimexData*)gx->gref;
                if (tgv)
                {
                    if (TheGINFO->bpp==8)
                    {
                        TheKeyFrame=(KeyFrameStruct*)CompressKeyFrame(TheGINFO,TheSource,TheGINFO->width,TheGINFO->numcolours,&PackSize);

                        if (ginfo->subtype&1)
                            if ((PackSize<(int)SizeRate) && (TheKeyFrame)) IsKeyFrame=1;     /* This is a key frame */

                        if (TheGINFO->quality==100) IsKeyFrame=1;                   /* This is a key frame */
                        if (ChangePalette(tgv,TheGINFO)) IsKeyFrame=1;              /* This is a key frame */
                        if (IsKeyFrame)                                             /* Make it a key frame */
                        {
                            if (!TheKeyFrame) ok=0;
                            else
                            {
                                if (SaveKeyFrame(gx->gstream,TheGINFO,TheKeyFrame,PackSize)) ok=1;
                            }
                            if (TheKeyFrame) gfree(TheKeyFrame);
                            ok=CheckBuffers(tgv,TheGINFO->width,TheGINFO->height,1);
                        }
                        else
                        {
                            if (TheKeyFrame) gfree(TheKeyFrame);
                            if (CheckBuffers(tgv,TheGINFO->width,TheGINFO->height,0))
                            {
                                Bvm=AllocateEnviro(TheGINFO->width,TheGINFO->height,SizeRate);
                                if (Bvm)
                                {
                                    if (BuildVectorDictionary(tgv,Bvm,TheSource,TheGINFO->width))
                                    {
                                        if (RemapAllVectors(tgv,Bvm,TheSource,TheGINFO->width))
                                        {
                                            if (BuildLiteralDictionary(SizeRate,tgv,Bvm,TheSource,TheGINFO->width))
                                            {
                                                if (RemapAllLiteral(tgv,Bvm,TheSource,TheGINFO->width))
                                                {
                                                    if (CalculBestDictionaries(tgv,Bvm,TheSource,TheGINFO->width))
                                                    {
                                                        if (CompressMatrix(Bvm))
                                                        {
                                                            if (SaveFrame(gx->gstream,Bvm))
                                                            {
                                                                NbLiteral=0;
                                                                NbLiteral+=Bvm->TheLiteral->NbLiteral;
                                                                NbLiteral+=Bvm->TheLiteral4->NbLiteral;
                                                                AllLiteral=AllocateLiteral(NbLiteral,NbLiteral);
                                                                if (AllLiteral)
                                                                {
                                                                    NbLiteral=0;
                                                                    for (i=0; i<Bvm->TheLiteral->NbLiteral; i++)
                                                                    {
                                                                        AllLiteral->ListLiteral[NbLiteral++]=Bvm->TheLiteral->ListLiteral[i];
                                                                    }
                                                                    for (i=0; i<Bvm->TheLiteral4->NbLiteral; i++)
                                                                    {
                                                                        AllLiteral->ListLiteral[NbLiteral++]=Bvm->TheLiteral4->ListLiteral[i];
                                                                    }
                                                                    ok=DecompressPicture(Bvm->TheMatrix,Bvm->TheVector,AllLiteral,TheSource,TheGINFO->width,tgv->fPrevSource,tgv->fWidth);
                                                                    gfree((char*)AllLiteral);
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
                    }
                }
                CopyBuffer(TheSource,TheGINFO->width,source,rowbytes,ginfo->height);
                if (tgv->fPrevSource) gfree(tgv->fPrevSource);
                tgv->fPrevSource=TheSource;
            }
            gfree((char*)TheGINFO);
        }
    }
    return(ok);
}


int GCALL TGV_wclose(GINSTANCE *gx)
{
    TgvGimexData*   tgv;

    if (gx)
    {
        tgv=(TgvGimexData*)gx->gref;
        if (tgv)
        {
            if (tgv->fPrevSource) gfree((char*)tgv->fPrevSource);
            if (tgv->fError) gfree((char*)tgv->fError);
            gfree((char*)tgv);
        }
        gfree((void*)gx);
    }
    return(1);
}


