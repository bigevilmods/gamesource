// (c) 1994-2007 Electronic Arts Inc.

#include "shpgimex.h"
#include "shpcmn.h"


#ifdef DEBUG
#include <stdio.h>
#endif

//**************************************************************************************************************************************************************
// Data conversion routines. Used in both Shapes and Fonts.
//**************************************************************************************************************************************************************

//**************************************************************************************************************************************************************
// READ FUNCTIONS
//**************************************************************************************************************************************************************

//--------------------------------------------------------------------------------------------------------
// convert FSH 16 bit entry to 32 bit ARGB entry 
int FSHRpixel5To8(unsigned int aRgb5)
{
    ARGB    aRgb8;

    aRgb8.a = (unsigned char)((((aRgb5>>15)&1)*255));
	aRgb8.r = (unsigned char)((((aRgb5>>10)&31)*255+16)/31);
	aRgb8.g = (unsigned char)((((aRgb5>>5 )&31)*255+16)/31);
	aRgb8.b = (unsigned char)((((aRgb5>>0 )&31)*255+16)/31);
    return(*(int *)&aRgb8);
}

//--------------------------------------------------------------------------------------------------------
// convert FSH 16 bit entry to 32 bit ARGB entry
int FSHRpixel565To8(unsigned int aRgb5)
{
    ARGB    aRgb8;

	aRgb8.a = (unsigned char)(255);
	aRgb8.r = (unsigned char)((((aRgb5>>11)&31)*255+16)/31);
	aRgb8.g = (unsigned char)((((aRgb5>>5 )&63)*255+32)/63);
	aRgb8.b = (unsigned char)((((aRgb5>>0 )&31)*255+16)/31);

    return(*(int *)&aRgb8);
}

//--------------------------------------------------------------------------------------------------------
// convert FSH 655 entry to 32 bit ARGB entry
int FSHRpixel655To8(unsigned int aRgb5)
{
    ARGB    aRgb8;

	aRgb8.a = (unsigned char)(255);
	aRgb8.r = (unsigned char)((((aRgb5>>10)&63)*255+32)/63);
	aRgb8.g = (unsigned char)((((aRgb5>>5 )&31)*255+16)/31);
	aRgb8.b = (unsigned char)((((aRgb5>>0 )&31)*255+16)/31);

    return(*(int *)&aRgb8);
}


//--------------------------------------------------------------------------------------------------------
// convert PSX 16 bit entry to 32 bit ARGB entry
// with transparency and tranluciency
int PSHRpixel5To8(unsigned int aRgb5)
{
    ARGB    aRgb8;

    if (aRgb5&0x8000)   /* transluciency on */
    {
        aRgb8.b = (unsigned char)((((aRgb5>>10)&31)*255+16)/31);
        aRgb8.g = (unsigned char)((((aRgb5>>5 )&31)*255+16)/31);
        aRgb8.r = (unsigned char)((((aRgb5>>0 )&31)*255+16)/31);
        aRgb8.a = (unsigned char)(128);
    }
    else
    {
        if (aRgb5==0)   /* transparent */
        {
            aRgb8.r = aRgb8.g = aRgb8.b = aRgb8.a = 0;
        }
        else            /* opaque */
        {
            aRgb8.b = (unsigned char)((((aRgb5>>10)&31)*255+16)/31);
            aRgb8.g = (unsigned char)((((aRgb5>>5 )&31)*255+16)/31);
            aRgb8.r = (unsigned char)((((aRgb5>>0 )&31)*255+16)/31);
            aRgb8.a = (unsigned char)(255);
        }
    }
    return(*(int *)&aRgb8);
}

//--------------------------------------------------------------------------------------------------------
// convert SSH 1555 bit entry to 32 bit ARGB entry
int SSHRpixel5To8(unsigned int aRgb5)
{
    ARGB    aRgb8;

	aRgb8.a = (unsigned char)((((aRgb5>>15)&1)*255));
	aRgb8.b = (unsigned char)((((aRgb5>>10)&31)*255+16)/31);
	aRgb8.g = (unsigned char)((((aRgb5>>5 )&31)*255+16)/31);
	aRgb8.r = (unsigned char)((((aRgb5>>0 )&31)*255+16)/31);

    return(*(int *)&aRgb8);
}

//--------------------------------------------------------------------------------------------------------
// convert GameCube 16 bit entry to 32 bit ARGB entry 
int GSHRpixel5To8(unsigned int aRgb5)
{
    ARGB    aRgb8;

    if (aRgb5&0x8000)  // rgb555
    {
        aRgb8.a = (unsigned char)(255);
        aRgb8.r = (unsigned char)((((aRgb5>>10)&31)*255+16)/31);
        aRgb8.g = (unsigned char)((((aRgb5>>5 )&31)*255+16)/31);
        aRgb8.b = (unsigned char)((((aRgb5>>0 )&31)*255+16)/31);
    }
    else // argb3444
    {
        aRgb8.a = (unsigned char)((((aRgb5>>12)&7) *255+4)/7);
        aRgb8.r = (unsigned char)((((aRgb5>>8 )&15)*255+8)/15);
        aRgb8.g = (unsigned char)((((aRgb5>>4 )&15)*255+8)/15);
        aRgb8.b = (unsigned char)((((aRgb5>>0 )&15)*255+8)/15);
    }
    return(*(int *)&aRgb8);
}

//--------------------------------------------------------------------------------------------------------
// convert FSH 16 bit 4444 entry to 32 bit ARGB entry
int FSHRpixel4444To8(unsigned int aRgb4444)
{
    ARGB    aRgb8;

    aRgb8.a = (unsigned char)((((aRgb4444>>12)&15)*255+8)/15);
    aRgb8.r = (unsigned char)((((aRgb4444>>8)&15)*255+8)/15);
    aRgb8.g = (unsigned char)((((aRgb4444>>4 )&15)*255+8)/15);
    aRgb8.b = (unsigned char)((((aRgb4444>>0 )&15)*255+8)/15);

    return(*(int *)&aRgb8);
}

//--------------------------------------------------------------------------------------------------------
// convert FSH 24 bit 6666 entry to 32 bit ARGB entry
int FSHRpixel6666To8(unsigned int aRgb6666)
{
    ARGB    aRgb8;

    aRgb8.a = (unsigned char)((((aRgb6666>>18)&63)*255+32)/63);
    aRgb8.r = (unsigned char)((((aRgb6666>>12)&63)*255+32)/63);
    aRgb8.g = (unsigned char)((((aRgb6666>>6 )&63)*255+32)/63);
    aRgb8.b = (unsigned char)((((aRgb6666>>0 )&63)*255+32)/63);

    return(*(int *)&aRgb8);
}

//--------------------------------------------------------------------------------------------------------
// convert FSH 32 bit 2101010 entry to 32 bit ARGB entry
int FSHRpixel2101010To8(unsigned int aRgb2101010)
{
    ARGB    aRgb8;

    aRgb8.a = (unsigned char)((((aRgb2101010>>30)&3)*255+2)/3);
    aRgb8.r = (unsigned char)((((aRgb2101010>>20)&1023)*255+512)/1023);
    aRgb8.g = (unsigned char)((((aRgb2101010>>10)&1023)*255+512)/1023);
    aRgb8.b = (unsigned char)((((aRgb2101010>>0 )&1023)*255+512)/1023);

    return(*(int *)&aRgb8);
}



//--------------------------------------------------------------------------------------------------------
// convert NSH 16 bit RGBA entry to 32 bit ARGB entry
int NSHRpixel5To8(unsigned int aRgb5)
{
    ARGB    aRgb8;

    aRgb8.r = (unsigned char)((((aRgb5>>11)&31)*255+16)/31);
    aRgb8.g = (unsigned char)((((aRgb5>> 6)&31)*255+16)/31);
    aRgb8.b = (unsigned char)((((aRgb5>> 1)&31)*255+16)/31);
    if (aRgb5 & 0x0001)
        aRgb8.a = 255;
    else
        aRgb8.a = 0;

    return(*(int *)&aRgb8);
}

//--------------------------------------------------------------------------------------------------------
// convert NSH 16 bit IA entry to 32 bit ARGB entry 
int NSHRpixelIATo8(unsigned int ia88)
{
    ARGB    aRgb8;

    aRgb8.r = aRgb8.g = aRgb8.b = (unsigned char)((ia88 >> 8));
    aRgb8.a = (unsigned char)((ia88 & 0x00ff));

    return(*(int *)&aRgb8);
}

//--------------------------------------------------------------------------------------------------------
int GSHRpixelIATo8(unsigned int ia88)
{
    ARGB    aRgb8;

    aRgb8.r = aRgb8.g = aRgb8.b = (unsigned char)((ia88 & 0x00ff));
    aRgb8.a = (unsigned char)((ia88 >> 8));
    return(*(int *)&aRgb8);
}




//--------------------------------------------------------------------------------------------------------
// swizzled PS2 cluts.
int SHPRcsmsize(int colours)
{
    if ((colours&31)>=8 && (colours&31)<=15)
        colours += 8;
    else if ((colours&31)>=16 && (colours&31)<=23)
        colours = (colours&~31)+24;
    return(colours);
}


//--------------------------------------------------------------------------------------------------------
// convert MSH 16 bit 4444 entry to 32 bit ARGB entry
int MSHRpixel4444To8(unsigned int aRgb4444)
{
    ARGB    aRgb8;

    aRgb8.a = (unsigned char)((((aRgb4444>>12)&15)*255+8)/15);
    aRgb8.b = (unsigned char)((((aRgb4444>>8)&15)*255+8)/15);
    aRgb8.g = (unsigned char)((((aRgb4444>>4 )&15)*255+8)/15);
    aRgb8.r = (unsigned char)((((aRgb4444>>0 )&15)*255+8)/15);

    return(*(int *)&aRgb8);
}

//--------------------------------------------------------------------------------------------------------
// convert MSH 16 bit entry to 32 bit ARGB entry
int MSHRpixel565To8(unsigned int aRgb5)
{
    ARGB    aRgb8;

	aRgb8.a = (unsigned char)(255);
	aRgb8.b = (unsigned char)((((aRgb5>>11)&31)*255+16)/31);
	aRgb8.g = (unsigned char)((((aRgb5>>5 )&63)*255+32)/63);
	aRgb8.r = (unsigned char)((((aRgb5>>0 )&31)*255+16)/31);

    return(*(int *)&aRgb8);
}


//**************************************************************************************************************************************************************
// WRITE FUNCTIONS
//**************************************************************************************************************************************************************



//--------------------------------------------------------------------------------------------------------------------------------------------------------------
// convert 32 bit ARGB entry to FSH 15 bit entry 
// with transparency 
unsigned short FSHWpixel8To5(const ARGB *aRgb8)
{
    int a = (int)aRgb8->a>>7;
    int r = (int)aRgb8->r>>(8-5);
    int g = (int)aRgb8->g>>(8-5);
    int b = (int)aRgb8->b>>(8-5);
    unsigned int aRgb5 = (a<<15)|(r<<10)|(g<<5)|(b);
    return((unsigned short) aRgb5);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------
// convert 32 bit ARGB entry to FSH 16 bit entry
// with transparency
unsigned short FSHWpixel8To565(const ARGB *aRgb8)
{
    int r = (int)aRgb8->r>>(8-5);
    int g = (int)aRgb8->g>>(8-6);
    int b = (int)aRgb8->b>>(8-5);
    unsigned int aRgb5 = (r<<11)|(g<<5)|(b);
    return((unsigned short) aRgb5);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------
// convert 32 bit ARGB entry to FSH 655 entry
// with transparency
unsigned short FSHWpixel8To655(const ARGB *aRgb8)
{
    int r = (int)aRgb8->r>>(8-6);
    int g = (int)aRgb8->g>>(8-5);
    int b = (int)aRgb8->b>>(8-5);
    unsigned int aRgb5 = (r<<10)|(g<<5)|(b);
    return((unsigned short) aRgb5);
}



//--------------------------------------------------------------------------------------------------------------------------------------------------------------
// convert 32 bit ARGB entry to PSX 16 bit entry
// with transparency and tranluciency
unsigned short PSHWpixel8To5(const ARGB *aRgb8)
{
    unsigned int a;
    unsigned int r;
    unsigned int g;
    unsigned int b;
    unsigned int aRgb5;

    a = aRgb8->a;
    r = (int)aRgb8->r>>(8-5);
    g = (int)aRgb8->g>>(8-5);
    b = (int)aRgb8->b>>(8-5);

    aRgb5 = (b<<10)|(g<<5)|r;

    if (a<25*255/100)           /* transparent */
    {
        aRgb5 = 0;
    }
    else if (a>=75*255/100)     /* opaque */
    {
        if (!aRgb5)
            aRgb5 = (1<<10);    /* black remap to blue */
    }
    else                        /* translucient */
    {
        aRgb5 |= 0x8000;
    }
    return((unsigned short) aRgb5);
}


#define QuickRound(v) ((v - (int)v) >= 0.5)?((int)v+1):((int)v)
//--------------------------------------------------------------------------------------------------------------------------------------------------------------
// convert 32 bit ARGB entry to NSH 16 bit IA (GREY8 macro borrowed from fgrey.c) 
#define GREY8(c) ((double)((double)((c)->r)*0.29900) + (double)((double)((c)->g)*0.58700) +  (double)((double)((c)->b)*0.11400))
unsigned short NSHWpixel8ToIA(const ARGB *aRgb8)
{
    unsigned int    ia88;

    ia88 = ((unsigned int)(QuickRound(GREY8(aRgb8)))) << 8;
    ia88 |= aRgb8->a;
    return((unsigned short) ia88);
}


//--------------------------------------------------------------------------------------------------------------------------------------------------------------
// convert 8bit ARGB data to 16bit Intensity alpha information for GSH.
unsigned short GSHWpixel8ToIA(const ARGB *aRgb8)
{
    unsigned int ia88;

    ia88 =  ((unsigned int)(QuickRound(GREY8(aRgb8))));
	ia88 |= ((unsigned int) (aRgb8->a) << 8);
    return((unsigned short) ia88);
}


//--------------------------------------------------------------------------------------------------------------------------------------------------------------
// convert 8bit data to 8bit Intensity alpha  information for GSH.
unsigned char GSHWpixel4ToIA(const ARGB *aRgb8)
{
    unsigned char grey = (unsigned char)(QuickRound(GREY8(aRgb8)));
    return( (aRgb8->a)&0xF0 | (grey>>4)&0x0F);
}


//--------------------------------------------------------------------------------------------------------------------------------------------------------------
// convert 8bit ARGB data to 8bit Intensity information for GSH.
unsigned char GSHWpixel8ToI(const ARGB *aRgb8)
{
    unsigned char ia88;

    ia88 = (unsigned char)(QuickRound(GREY8(aRgb8)));
    return((unsigned char)ia88);
}



#undef GREY8


//--------------------------------------------------------------------------------------------------------------------------------------------------------------
// convert 32 bit ARGB entry to NSH 16 bit RGBA 
// with transparency 
unsigned short NSHWpixel8To5(const ARGB *aRgb8)
{
    unsigned int    aRgb5;
    int a;
    int r;
    int g;
    int b;

    a = aRgb8->a;
    r = ((int)aRgb8->r>>(8-5))&31;
    g = ((int)aRgb8->g>>(8-5))&31;
    b = ((int)aRgb8->b>>(8-5))&31;
    if (a<25*255/100)           /* transparent */
    {
        aRgb5 = 0;
    }
    else
    {
        aRgb5 = (r<<11)|(g<<6)|(b<<1)|0x0001;
    }
    return((unsigned short) aRgb5);
}


//--------------------------------------------------------------------------------------------------------------------------------------------------------------
// convert 32 bit ARGB entry to SSH 15 bit entry 
// with transparency 
unsigned short SSHWpixel8To5(const ARGB *aRgb8)
{
    int a = ((int)aRgb8->a*1+128)/255;
    int r = (int)aRgb8->r>>(8-5);
    int g = (int)aRgb8->g>>(8-5);
    int b = (int)aRgb8->b>>(8-5);
    unsigned int aRgb5 = (a<<15)|(b<<10)|(g<<5)|(r);
    return((unsigned short) aRgb5);
}


//--------------------------------------------------------------------------------------------------------------------------------------------------------------
// convert 32 bit ARGB entry to FSH 16 bit ARGB 4444 entry
// with transparency
unsigned short FSHWpixel8To4444(const ARGB *aRgb8)
{
    unsigned int argb16;
    int a;
    int r;
    int g;
    int b;

    a = ((int)aRgb8->a*15+128)/255;
    r = ((int)aRgb8->r*15+128)/255;
    g = ((int)aRgb8->g*15+128)/255;
    b = ((int)aRgb8->b*15+128)/255;
    if(aRgb8->a > 0 && a == 0)
        a = 1;
    argb16 = (a<<12)|(r<<8)|(g<<4)|(b);
    return((unsigned short) argb16);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------
// convert 32 bit ARGB entry to FSH 24 bit ARGB 6666 entry
// with transparency
unsigned int FSHWpixel8To6666(const ARGB *aRgb8)
{
    unsigned int argb16;
    int a;
    int r;
    int g;
    int b;

    a = ((int)aRgb8->a*63+128)/255;
    r = ((int)aRgb8->r*63+128)/255;
    g = ((int)aRgb8->g*63+128)/255;
    b = ((int)aRgb8->b*63+128)/255;
    if(aRgb8->a > 0 && a == 0)
        a = 1;
    argb16 = (a<<18)|(r<<12)|(g<<6)|(b);
    return((unsigned int) argb16);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------
// convert 32 bit ARGB entry to FSH 32 bit ARGB 2101010 entry
// with transparency
unsigned int FSHWpixel8To2101010(const ARGB *aRgb8)
{
    unsigned int argb16;
    int a;
    int r;
    int g;
    int b;

    a = ((int)aRgb8->a*3+128)/255;
    r = ((int)aRgb8->r*1023+128)/255;
    g = ((int)aRgb8->g*1023+128)/255;
    b = ((int)aRgb8->b*1023+128)/255;
    argb16 = (a<<30)|(r<<20)|(g<<10)|(b);
    return((unsigned int) argb16);
}



//--------------------------------------------------------------------------------------------------------------------------------------------------------------
// bit depth conversion function for GSH.
unsigned short GSHWpixel8To5(const ARGB *aRgb8)
{
    unsigned int a;
    unsigned int r;
    unsigned int g;
    unsigned int b;
    unsigned int argb;

    if (aRgb8->a<=0xec) // 3444 - only 3 bits of alpha significant
    {
        a = ((int)aRgb8->a*7+128)/255;
        r = ((int)aRgb8->r*15+128)/255;
        g = ((int)aRgb8->g*15+128)/255;
        b = ((int)aRgb8->b*15+128)/255;
        argb = (0<<15)|(a<<12)|(r<<8)|(g<<4)|b;
    }
    else    // 555
    {
        r = (int)aRgb8->r>>(8-5);
        g = (int)aRgb8->g>>(8-5);
        b = (int)aRgb8->b>>(8-5);

        argb = (1<<15)|(r<<10)|(g<<5)|b;
    }
    return((unsigned short) argb);
}


//--------------------------------------------------------------------------------------------------------------------------------------------------------------
// convert 32 bit ARGB entry to MSH 16 bit ARGB 4444 entry
// with transparency
unsigned short MSHWpixel8To4444(const ARGB *aRgb8)
{
    unsigned int argb16;
    int a;
    int r;
    int g;
    int b;

    a = ((int)aRgb8->a*15+128)/255;
    r = ((int)aRgb8->r*15+128)/255;
    g = ((int)aRgb8->g*15+128)/255;
    b = ((int)aRgb8->b*15+128)/255;
    if(aRgb8->a > 0 && a == 0)
        a = 1;
    argb16 = (a<<12)|(b<<8)|(g<<4)|(r);
    return((unsigned short) argb16);
}


//--------------------------------------------------------------------------------------------------------------------------------------------------------------
// convert 32 bit ARGB entry to MSH 16 bit entry
// with transparency
unsigned short MSHWpixel8To565(const ARGB *aRgb8)
{
    int r = (int)aRgb8->r>>(8-5);
    int g = (int)aRgb8->g>>(8-6);
    int b = (int)aRgb8->b>>(8-5);
    unsigned int aRgb5 = (b<<11)|(g<<5)|(r);
    return((unsigned short) aRgb5);
}


//--------------------------------------------------------------------------------------------------------------------------------------------------------------
bool SHPismipmap(char * shape_ID,int len)
{
    // if the name is the old one.
    if (shape_ID[0] == 'm'
	 && shape_ID[1] == 'm' 
	 && shape_ID[2] >= '0' && shape_ID[2] <= '9'
     && ((shape_ID[3] >= '1' && shape_ID[3] <= '9') || shape_ID[2] >= '1'))
        return true;

    // if the name is the new one.
    if (len == 0 || len > 4)
    {
        int    i = 0;
        char * s = shape_ID;

        if (len == 0)
            len = strlen(shape_ID) - 3;

        for (i = 0; i < len; i++)
        {
            if ((s[0] == '_') &&
                (s[1] == 'm') &&
                (s[2] == 'm') &&
                (s[3] >= '0') && (s[3] <= '9'))
                    return true;
            s++;
        }

    }
    
    // otherwise not a mipmap.
    return false;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------
// check if it's A8 or greyscale.
int NSHWIsGreyPalette(const ARGB *colourtbl, int NbColours, int nopal)
{
    int grey = 1;
    int i;
    int lastgreyvalue = -1;

/*    \* If palette was stripped then assume it was a regular colour palette. *\ */
/*    if (nopal) */
/*        return(0); */

    /* If no colour table then assume grey scale (e.g. intensity image). */
    if (!colourtbl)
        return(1);

    /* Otherwise check that each colour is greyscale and ascending. */
    for (i = 0; i < NbColours; ++i)
    {
        if ((colourtbl[i].r != colourtbl[i].g) ||
            (colourtbl[i].r != colourtbl[i].b) ||
            (colourtbl[i].g != colourtbl[i].b) ||
            (colourtbl[i].r <= lastgreyvalue))
        {
            grey = 0;
            break;
        }

        lastgreyvalue = colourtbl[i].r;
    }

    return grey;
}


//--------------------------------------------------------------------------------------------------------------------------------------------------------------
// CLUT for PS2
int SHPWcsmsize(int colours)
{
    if ((colours&31)>=8 && (colours&31)<=15)
        colours += 8;
    else if ((colours&31)>=16 && (colours&31)<=23)
        colours = (colours&~31)+24;
    return(colours);
}


//**************************************************************************************************************************************************************
// CLUT routines. Used in both shapes and fonts.
//**************************************************************************************************************************************************************


//--------------------------------------------------------------------------------------------------------------------------------------------------------------
// figure out the clut type based on channel depth.

int fshcluttype(int a, int r, int g, int b)
{
    int type=0;

         if (a<=1 && r<=5 && g<=5 && b<=5) type = 0x2d; /* 15 bit palette (555) */
    else if (a<=1 && r<=5 && g<=6 && b<=5) type = 0x29; /* 16 bit palette (565) */
    else                                   type = 0x2a; /* 32 bit palette (8888) */

    return(type);
}

int fshadvancedcluttype(int a, int r, int g, int b)
{
    int type=0;

         if (a<=1 && r<=5 && g<=5 && b<=5) type = 0x2d; /* 15 bit palette (555) */
    else if (a<=1 && r<=5 && g<=6 && b<=5) type = 0x29; /* 16 bit palette (565) */
    else if (a<=1 && r<=6 && g<=6 && b<=6) type = 0x22; /* 24 bit palette (666) */
    else if (a<=1 && r<=8 && g<=8 && b<=8) type = 0x2f; /* 24 bit palette (888) */
    else if (        r<=5 && g<=5 && b<=5) type = 0x2e; /* 32 bit palette (8555) */
    else if (        r<=5 && g<=6 && b<=5) type = 0x2c; /* 32 bit palette (8565) */
    else                                   type = 0x2a; /* 32 bit palette (8888) */

    return(type);
}


//--------------------------------------------------------------------------------------------------------------------------------------------------------------
// figure out clut type based on channel depth.
int xshcluttype(int a, int r, int g, int b)
{
    int type=0;

    type = 0x2a; /* 32 bit palette (8888) */

    return(type);
}


//--------------------------------------------------------------------------------------------------------------------------------------------------------------
// figure out clut type based on channel depth.
int gshcluttype(int a, int r, int g, int b)
{
    int type=0;

         if (g==0 && b==0)                 type = 0x30; /* IA8 16 bit palette (8800) */
    else if (a==0 && r<=5 && g<=6 && b<=5) type = 0x31; /* 565 bit palette (0565)    */
    else                                   type = 0x32; /* 3555 bit palette (3555)   */

    return(type);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------
// figure out clut type based on channel depth.
int gshadvancedcluttype(int a, int r, int g, int b)
{
    int type=0;

         if (g==0 && b==0)                 type = 0x30; /* IA8 16 bit palette (8800) */
    else if (a==0 && r<=5 && g<=6 && b<=5) type = 0x31; /* 565 bit palette (0565)    */
    else if (a<=3 && r<=5 && g<=5 && b<=5) type = 0x32; /* 3555 bit palette (3555)   */
    else                                   type = 0x33; /* 8888 bit palette (2*IA8)  */

    return(type);
}


//--------------------------------------------------------------------------------------------------------------------------------------------------------------
// figure out clut type based on channel depth.
int sshcluttype(int a, int r, int g, int b)
{
    int type=0;

         if (a<=1 && r<=5 && g<=5 && b<=5) type = 0x20; /* 15 bit palette (1555) */
    else                                   type = 0x21; /* 32 bit palette (8888) */

    return(type);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------
// figure out clut type based on channel depth.
int mshcluttype(int a, int r, int g, int b)
{
    int type=0;

         if (a<=2 && r<=5 && g<=5 && b<=5) type = 0x39; /* 15 bit palette (555) */
    else if (a<=2 && r<=5 && g<=6 && b<=5) type = 0x38; /* 16 bit palette (565) */
    else if (a<=4 && r<=4 && g<=4 && b<=4) type = 0x3a; /* 16 bit palette (4444) */
    else                                   type = 0x3b; /* 32 bit palette (8888) */
   
    return(type);
}

//**************************************************************************************************************************************************************
// SWIZZLE routines. Used in both shapes and fonts.
//**************************************************************************************************************************************************************

//--------------------------------------------------------------------------------------------------------------------------------------------------------------
// support function for SHAPE_swizzle8() swizzles a single block withing a single page
// swizzles first as 8bpp then unswizzles as 16bpp.
int WBlockConv8to32(unsigned char *p_input, unsigned char *p_output) 
{

    static int lut[] = {
        // even column
        0, 36, 8,  44,
        1, 37, 9,  45,
        2, 38, 10, 46,
        3, 39, 11, 47,
        4, 32, 12, 40,
        5, 33, 13, 41,
        6, 34, 14, 42,
        7, 35, 15, 43,

        16, 52, 24, 60,
        17, 53, 25, 61,
        18, 54, 26, 62,
        19, 55, 27, 63, 
        20, 48, 28, 56,
        21, 49, 29, 57,
        22, 50, 30, 58,
        23, 51, 31, 59,

        // odd column
        4, 32, 12, 40,
        5, 33, 13, 41,
        6, 34, 14, 42,
        7, 35, 15, 43,
        0, 36, 8,  44,
        1, 37, 9,  45,
        2, 38, 10, 46,
        3, 39, 11, 47,

        20, 48, 28, 56,
        21, 49, 29, 57,
        22, 50, 30, 58,
        23, 51, 31, 59,
        16, 52, 24, 60,
        17, 53, 25, 61,
        18, 54, 26, 62,
        19, 55, 27, 63
        };

    unsigned int i, j, k, i0;
    unsigned int index0, index1;
    unsigned char *pIn;

    pIn = p_input;

    // for first step, we only think for a single block. (4bits, 32x16)
    index1 = 0;

    for(k = 0; k < 4; k++) {

        index0 = (k % 2) * 64;

        for(i = 0; i < 16; i++) {
            for(j = 0; j < 4; j++) {
                i0 = lut[index0++];
                p_output[index1++] = pIn[i0];
            }
        }
        pIn += 64;
    }

    return 0;
}


//--------------------------------------------------------------------------------------------------------------------------------------------------------------
// support function for SHAPE_swizzle8 PS2 swizzling function. Swizzles a single page within
// the texture space.
#define PSMT8_BLOCK_WIDTH  16
#define PSMT8_BLOCK_HEIGHT 16
#define PSMCT32_BLOCK_WIDTH 8
#define PSMCT32_BLOCK_HEIGHT 8
int WPageConv8to32(int width, int height, unsigned char *p_input, unsigned char *p_output)
{

    static unsigned int block_table8[] = {
         0,  1,  4,  5, 16, 17, 20, 21,
         2,  3,  6,  7, 18, 19, 22, 23,
         8,  9, 12, 13, 24, 25, 28, 29,
        10, 11, 14, 15, 26, 27, 30, 31
    };

    static unsigned int block_table32[] = {
         0,  1,  4,  5, 16, 17, 20, 21,
         2,  3,  6,  7, 18, 19, 22, 23,
         8,  9, 12, 13, 24, 25, 28, 29,
        10, 11, 14, 15, 26, 27, 30, 31
    };

    unsigned int *index32_h, *index32_v, in_block_nb;

    unsigned char input_block[16 * 16], output_block[16 * 16];
    unsigned char *pi0, *pi1, *po0, *po1;
    int index0, index1, i, j, k;
    int n_width, n_height, input_page_line_size;
    int output_page_line_size;



    // --- create table for output 32bit buffer ---
    index32_h = (unsigned int*) galloc(8 * 4 * sizeof(unsigned int));
    index32_v = (unsigned int*) galloc(8 * 4 * sizeof(unsigned int));
    index0 = 0;
    for(i = 0; i < 4; i++) {
        for(j = 0; j < 8; j++) {
            index1 = block_table32[index0];
            index32_h[index1] = j;
            index32_v[index1] = i;
            index0++;
        }
    }


    // how many blocks we should calc (width/height)
    n_width = width / PSMT8_BLOCK_WIDTH;
    n_height = height / PSMT8_BLOCK_HEIGHT;

    memset(input_block, 0, 16 *16);
    memset(output_block, 0, 16 * 16);
    
    input_page_line_size  = 128;    // PSMT8 page width (byte)
    output_page_line_size = 256;    // PSMCT32 page width (byte)

    // now assume copying from page top. 
    for(i = 0; i < n_height; i++) {

        for(j = 0; j < n_width; j++) {
            
            pi0 = input_block;
            pi1 = p_input + PSMT8_BLOCK_HEIGHT * i * input_page_line_size + j * PSMT8_BLOCK_WIDTH; // byte

            in_block_nb = block_table8[i * n_width + j]; 

            for(k = 0; k < PSMT8_BLOCK_HEIGHT; k++) {
                memcpy(pi0, pi1, PSMT8_BLOCK_WIDTH); // copy full 1 line of 1 block.
                pi0 += PSMT8_BLOCK_WIDTH;  
                pi1 += input_page_line_size;
            }
            
            WBlockConv8to32(input_block, output_block);

            po0 = output_block;
            po1 = p_output + 8 * index32_v[in_block_nb] * output_page_line_size + index32_h[in_block_nb] * 32;
            for(k = 0; k < PSMCT32_BLOCK_HEIGHT; k++) {
                memcpy(po1, po0, PSMCT32_BLOCK_WIDTH * 4);
                po0 += PSMCT32_BLOCK_WIDTH * 4;
                po1 += output_page_line_size;   
            }

        }
    }
    
    gfree(index32_h);
    gfree(index32_v);

    return 0;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------
// PS2 swizzle function, swizzles as 8 then unswizzles as 32, for faster upload times. See 
// comment on SHAPE_swizzle4 for more information.
#define PSMT8_PAGE_WIDTH    128
#define PSMT8_PAGE_HEIGHT   64
#define PSMCT32_PAGE_WIDTH  64
#define PSMCT32_PAGE_HEIGHT 32
int SHAPE_swizzle8(unsigned char *p_input, unsigned char *p_output,int width,int height)
{

    int i, j, k;
    int n_page_h, n_page_w, n_page8_width_byte, n_page32_width_byte;
    int n_input_width_byte, n_output_width_byte, n_input_height, n_output_height;
    unsigned char *pi0, *pi1, *po0, *po1;
    unsigned char input_page[PSMT8_PAGE_WIDTH * PSMT8_PAGE_HEIGHT];
    unsigned char output_page[PSMCT32_PAGE_WIDTH * 4 * PSMCT32_PAGE_HEIGHT];

    // ----- check width -----
 

    memset(input_page, 0, PSMT8_PAGE_WIDTH * PSMT8_PAGE_HEIGHT);
    memset(output_page, 0, PSMCT32_PAGE_WIDTH * 4 * PSMCT32_PAGE_HEIGHT);

    n_page_w = (width - 1) / PSMT8_PAGE_WIDTH + 1;
    n_page_h = (height - 1) / PSMT8_PAGE_HEIGHT + 1;

    n_page8_width_byte = PSMT8_PAGE_WIDTH;
    n_page32_width_byte = PSMCT32_PAGE_WIDTH * 4;

    // --- set in/out buffer size (for image smaller than one page) ---
    if(n_page_w == 1) {
        n_input_width_byte = width;
        n_output_width_byte = width * 2;
    } else {
        n_input_width_byte = n_page8_width_byte;
        n_output_width_byte = n_page32_width_byte;
    }

    if(n_page_h == 1) {
        n_input_height = height;
        n_output_height = height / 2;
    } else {
        n_input_height = PSMT8_PAGE_HEIGHT;
        n_output_height = PSMCT32_PAGE_HEIGHT;
    }

    // --- conversion ---
    for(i = 0; i < n_page_h; i++) {
		int mpageheight = i*PSMT8_PAGE_HEIGHT; 
        for(j = 0; j < n_page_w; j++) {
            pi0 = p_input + (n_input_width_byte * n_input_height) * n_page_w * i 
                          + n_input_width_byte * j;
//            pi0 = p_input + (n_input_width_byte * PSMT8_PAGE_HEIGHT) * n_page_w * i 
//                          + n_input_width_byte * j;
            pi1 = input_page;

            for(k = 0; k < n_input_height && (mpageheight+k) < height; k++) {
                memcpy(pi1, pi0, n_input_width_byte);
                pi0 += n_input_width_byte * n_page_w;
                pi1 += n_page8_width_byte;
            }

            // --- convert a page ---
            WPageConv8to32(PSMT8_PAGE_WIDTH, PSMT8_PAGE_HEIGHT, input_page, output_page);

            po0 = p_output + (n_output_width_byte * n_output_height) * n_page_w * i
                           + n_output_width_byte * j;
            po1 = output_page;
            for(k = 0; k < n_output_height && (mpageheight+(k*2)) < height; k++) {
                memcpy(po0, po1, n_output_width_byte);
                po0 += n_output_width_byte * n_page_w;
                po1 += n_page32_width_byte;
            }          
        }
    }
    return 0;
}


//--------------------------------------------------------------------------------------------------------------------------------------------------------------
// support function for SHAPE_swizzle4() swizzles a block inside a page.
// swizzles first as 4 then unswizzles as 32.
int WBlockConv4to32(unsigned char *p_input, unsigned char *p_output)
{

    static int lut[] = {
        // even column
        0, 68, 8,  76, 16, 84, 24, 92,
        1, 69, 9,  77, 17, 85, 25, 93,
        2, 70, 10, 78, 18, 86, 26, 94,
        3, 71, 11, 79, 19, 87, 27, 95,
        4, 64, 12, 72, 20, 80, 28, 88,
        5, 65, 13, 73, 21, 81, 29, 89,
        6, 66, 14, 74, 22, 82, 30, 90,
        7, 67, 15, 75, 23, 83, 31, 91,

        32, 100, 40, 108, 48, 116, 56, 124,
        33, 101, 41, 109, 49, 117, 57, 125,
        34, 102, 42, 110, 50, 118, 58, 126,
        35, 103, 43, 111, 51, 119, 59, 127,
        36, 96,  44, 104, 52, 112, 60, 120,
        37, 97,  45, 105, 53, 113, 61, 121,
        38, 98,  46, 106, 54, 114, 62, 122,
        39, 99,  47, 107, 55, 115, 63, 123,
    

        // odd column
        4, 64, 12, 72, 20, 80, 28, 88,
        5, 65, 13, 73, 21, 81, 29, 89,
        6, 66, 14, 74, 22, 82, 30, 90,
        7, 67, 15, 75, 23, 83, 31, 91,
        0, 68, 8,  76, 16, 84, 24, 92,
        1, 69, 9,  77, 17, 85, 25, 93,
        2, 70, 10, 78, 18, 86, 26, 94,
        3, 71, 11, 79, 19, 87, 27, 95,
        
        36, 96,  44, 104, 52, 112, 60, 120,
        37, 97,  45, 105, 53, 113, 61, 121,
        38, 98,  46, 106, 54, 114, 62, 122,
        39, 99,  47, 107, 55, 115, 63, 123,
        32, 100, 40, 108, 48, 116, 56, 124,
        33, 101, 41, 109, 49, 117, 57, 125,
        34, 102, 42, 110, 50, 118, 58, 126,
        35, 103, 43, 111, 51, 119, 59, 127
        };

    unsigned int i, j, k, i0, i1, i2;
    unsigned int index0, index1;
    unsigned char c_in, c_out, *pIn;

    pIn = p_input;

    // for first step, we only think for a single block. (4bits, 32x16)
    index1 = 0;

    for(k = 0; k < 4; k++) {
        index0 = (k % 2) * 128;

        for(i = 0; i < 16; i++) {

            for(j = 0; j < 4; j++) {

                c_out = 0x00;

                // lower 4bit.
                i0 = lut[index0++];
                i1 = i0 / 2;
                i2 = (i0 & 0x1) * 4;
                c_in = (pIn[i1] & (0x0f << i2)) >> i2;
                c_out = c_out | c_in;
                
                // uppper 4bit
                i0 = lut[index0++];
                i1 = i0 / 2;
                i2 = (i0 & 0x1) * 4;
                c_in = (pIn[i1] & (0x0f << i2)) >> i2;
                c_out = c_out | ((c_in << 4) & 0xf0);

                p_output[index1++] = c_out;
            }
        }
        pIn += 64;
    }

    return 0;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------
// support function for SHAPE_swizzle4 PS2 swizzle function. swizzles the pages inside texture
// memory.
#define PSMT4_BLOCK_WIDTH 32
#define PSMT4_BLOCK_HEIGHT 16
int WPageConv4to32(int width, int height, unsigned char *p_input, unsigned char *p_output)
{

    static unsigned int block_table4[] = {
         0,  2,  8, 10,
         1,  3,  9, 11,
         4,  6, 12, 14,
         5,  7, 13, 15,
        16, 18, 24, 26,
        17, 19, 25, 27,
        20, 22, 28, 30,
        21, 23, 29, 31
    };

    static unsigned int block_table32[] = {
         0,  1,  4,  5, 16, 17, 20, 21,
         2,  3,  6,  7, 18, 19, 22, 23,
         8,  9, 12, 13, 24, 25, 28, 29,
        10, 11, 14, 15, 26, 27, 30, 31
    };

    unsigned int *index32_h, *index32_v, in_block_nb;

    unsigned char input_block[16 * 16], output_block[16 * 16];
    unsigned char *pi0, *pi1, *po0, *po1;
    int index0, index1, i, j, k;
    int n_width, n_height, input_page_line_size;
    int output_page_line_size;

   



    // --- create table for output 32bit buffer ---
    index32_h = (unsigned int*) galloc(8 * 4 * sizeof(unsigned int));
    index32_v = (unsigned int*) galloc(8 * 4 * sizeof(unsigned int));
    index0 = 0;
    for(i = 0; i < 4; i++) {
        for(j = 0; j < 8; j++) {
            index1 = block_table32[index0];
            index32_h[index1] = j;
            index32_v[index1] = i;
            index0++;
        }
    }




    n_width = width / 32;
    n_height = height / 16;

    memset(input_block, 0, 16 *16);
    memset(output_block, 0, 16 * 16);
    
    input_page_line_size = 128 / 2;    // PSMT4 page width (byte)
    output_page_line_size = 256;       // PSMCT32 page width (byte)

    // now assume copying from page top. 
    for(i = 0; i < n_height; i++) {

        for(j = 0; j < n_width; j++) {
            
            pi0 = input_block;
            pi1 = p_input + 16 * i * input_page_line_size + j * 16;

            in_block_nb = block_table4[i * n_width + j]; 

            for(k = 0; k < PSMT4_BLOCK_HEIGHT; k++) {
                memcpy(pi0, pi1, PSMT4_BLOCK_WIDTH / 2); // copy full 1 line of 1 block.
                pi0 += PSMT4_BLOCK_WIDTH / 2;
                pi1 += input_page_line_size;
            }
            
            WBlockConv4to32(input_block, output_block);

            po0 = output_block;
            po1 = p_output + 8 * index32_v[in_block_nb] * output_page_line_size + index32_h[in_block_nb] * 32;
            for(k = 0; k < PSMCT32_BLOCK_HEIGHT; k++) {
                memcpy(po1, po0, PSMCT32_BLOCK_WIDTH * 4);
                po0 += PSMCT32_BLOCK_WIDTH * 4;
                po1 += output_page_line_size;   
            }

        }
    }
    
    gfree(index32_h);
    gfree(index32_v);

    return 0;
}


//--------------------------------------------------------------------------------------------------------------------------------------------------------------
// PS2 Swizzleing code for 4bpp images, swizzle 4 and then unswizzle 32 so then when I upload as
// 32 and ask for a swizzle I get swizzle 4. Faster than uploading as 4bpp.
#define PSMT4_PAGE_WIDTH    128
#define PSMT4_PAGE_HEIGHT   128
int SHAPE_swizzle4(unsigned char *p_input, unsigned char *p_output,int width, int height)
{


    int i, j, k;
    int n_page_h, n_page_w, n_page4_width_byte, n_page32_width_byte;
    unsigned char *pi0, *pi1, *po0, *po1;
    int n_input_width_byte, n_output_width_byte, n_input_height, n_output_height;
    unsigned char input_page[PSMT4_PAGE_WIDTH / 2 * PSMT4_PAGE_HEIGHT];
    unsigned char output_page[PSMCT32_PAGE_WIDTH * 4 * PSMCT32_PAGE_HEIGHT];

    memset(input_page, 0, PSMT4_PAGE_WIDTH / 2 * PSMT4_PAGE_HEIGHT);
    memset(output_page, 0, PSMCT32_PAGE_WIDTH * 4 * PSMCT32_PAGE_HEIGHT);

    n_page_w = (width - 1) / PSMT4_PAGE_WIDTH + 1;
    n_page_h = (height - 1) / PSMT4_PAGE_HEIGHT + 1;

    n_page4_width_byte = PSMT4_PAGE_WIDTH / 2;
    n_page32_width_byte = PSMCT32_PAGE_WIDTH * 4;

    // --- set in/out buffer size (for image smaller than one page) ---
    if(n_page_w == 1) {
        n_input_width_byte = width / 2;
        n_output_height = width / 4;
    } else {
        n_input_width_byte = n_page4_width_byte;
        n_output_height = PSMCT32_PAGE_HEIGHT;
    }

    if(n_page_h == 1) {
        n_input_height = height;
        n_output_width_byte = height * 2;
    } else {
        n_input_height = PSMT4_PAGE_HEIGHT;
        n_output_width_byte = n_page32_width_byte;
    }


    for(i = 0; i < n_page_h; i++) {
        for(j = 0; j < n_page_w; j++) {
            pi0 = p_input + (n_input_width_byte * n_input_height) * n_page_w * i 
                          + n_input_width_byte * j;
            pi1 = input_page;

            for(k = 0; k < n_input_height; k++) {
                memcpy(pi1, pi0, n_input_width_byte);
                pi0 += n_input_width_byte * n_page_w;
                pi1 += n_page4_width_byte;
            }

            WPageConv4to32(PSMT4_PAGE_WIDTH, PSMT4_PAGE_HEIGHT, input_page, output_page);

            po0 = p_output + (n_output_width_byte * n_output_height) * n_page_w * i
                          + n_output_width_byte * j;
            po1 = output_page;
            for(k = 0; k < n_output_height; k++) {
                memcpy(po0, po1, n_output_width_byte);
                po0 += n_output_width_byte * n_page_w;
                po1 += n_page32_width_byte;
            }          
        }
    }
    return 0;
}



int inline Pixel16Offset(int x, int y, int width)
{
    int pagex,pagey;
    int newx,newy;
    int bit = 0;

    if(width >= 16)
    {
        pagex = x >> 6;
        pagey = y >> 6;
        newx = (y&0x38) + (x&0x07);       // y5+y4+y3+x2+x1+x0
        newy = ((x&0x30)>>1) + (y&0x07);  // x5+x4+y2+y1+y0
        bit = ((x&0x08)<<1);              // x3+0000
        x = (pagex << 6) + newx;
        y = (pagey << 5) + newy;
    }
    return(32*(y*width+x)+bit);
}

int SHAPE_swizzle16(unsigned char *src, unsigned char *dest, int width, int height)
{
    int x = 0;
	int y = 0;
	int ywidth = 0;
	int loc = 0;
    for(y = 0; y < height; y++)
	{
		ywidth = y*width<<1;
		for(x = 0; x < width; x++)
		{	
			loc = (Pixel16Offset(x,y,width)>>3);
            *((short *)(dest + loc)) = *((short *)(src + ((x<<1)+ywidth)));
		}
	}    
	return 1;
}



int SHAPE_unswizzle16(unsigned char *src, unsigned char *dest, int width, int height)
{
    int x = 0;
	int y = 0;
	int ywidth = 0;
	int loc = 0;
    for(y = 0; y < height; y++)
	{
		ywidth = y*width<<1;
		for(x = 0; x < width; x++)
		{	
			loc = (Pixel16Offset(x,y,width)>>3);
            *((short *)(dest + ((x<<1)+ywidth))) = *((short *)(src + loc));
        }
	}
	return 1;
}


#undef PSMT8_BLOCK_WIDTH  
#undef PSMT8_BLOCK_HEIGHT 
#undef PSMCT32_BLOCK_WIDTH
#undef PSMCT32_BLOCK_HEIGHT
#undef PSMT8_PAGE_WIDTH
#undef PSMT8_PAGE_HEIGHT
#undef PSMCT32_PAGE_WIDTH
#undef PSMCT32_PAGE_HEIGHT
#undef PSMT4_BLOCK_WIDTH
#undef PSMT4_BLOCK_HEIGHT
#undef PSMT4_PAGE_WIDTH
#undef PSMT4_PAGE_HEIGHT



//--------------------------------------------------------------------------------------------------------------------------------------------------------------
// Look up the pixel offset for a XBox AND PC Swizzled shape, 
// the XSH should really be XSH and FSH.
int SHPWswizzleXSH(int x,int y,int width,int height)
{
    int off;
    int nx=0;
    int ny=0;
    int xpos=0;
    int ypos=0;
    int tpix=1;
    off = y*width+x;

    while (tpix<width || tpix<height)
    {
        if (tpix<width)
        {
            nx |= ((off&1)<<xpos);
            ++xpos;
            off >>= 1;
        }
        if (tpix<height)
        {
            ny |= ((off&1)<<ypos);
            ++ypos;
            off >>= 1;
        }
        tpix <<= 1;
    }
    off = ny*width+nx;
    return(off);
}


//--------------------------------------------------------------------------------------------------------------------------------------------------------------
// Look up the pixel offset for a PSP Swizzled shape, 
int SHPWswizzleMSH(int x,int y,int width,int height,int bpp)
{
	int blockNumber = 0; // number of blocks that occur before this pixel
	int blockOffset = 0; // offset within the block of the specified pixel
	int blockSize = 0; // blockSize in pixels

	switch(bpp)
	{
		case 4:
			blockNumber = ((width>>5) * (y>>3)) + (x>>5);
			blockOffset = x & 0x1f | (y & 0x07) << 5;
			blockSize = 32 * 8;
			break;
		case 8:
			blockNumber = ((width>>4) * (y>>3)) + (x>>4);
			blockOffset = x & 0x0f | ((y & 0x07) << 4);
			blockSize = 16 * 8;
			break;
		case 15:
		case 16:
			blockNumber = ((width>>3) * (y>>3)) + (x>>3);
			blockOffset = x & 0x07 | ((y & 0x07) << 3);
			blockSize = 8 * 8;
			break;
		case 32:
			blockNumber = ((width>>2) * (y>>3)) + (x>>2);
			blockOffset = x & 0x03 | ((y & 0x07) << 2);
			blockSize = 4 * 8;
			break;
	}

	return (blockNumber * blockSize) + blockOffset;
}


//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// calculate the correct width and height for any swizzled GSH.
void RetGSHSwizzleWidthHeight(int *rw,int *rh,int w,int h,int bpp)
{
    int realwidth  = 0;
	int realheight = 0;
    switch (bpp)
    {
        case 4:// size need to be div by 8x8 for 4 bit else can not swizzle
        {
            realwidth  = (w + 7)&~7;
			realheight = (h + 7)&~7;
            break;
        }
        case 8:// size needs to be div by 8x4 for 8 bit else can not swizzle
        {
            realwidth  = (w + 7)&~7;
			realheight = (h + 3)&~3;
      	    break;
        }
        case 15:
        case 16:
        case 32://size needs to be div by 4x4 for higher bit else can not swizzle
        {
            realwidth  = (w + 3)&~3;
	 		realheight = (h + 3)&~3;
            break;
        }
    }
	*rw = realwidth;
	*rh = realheight;
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// calculate the correct width and height for any swizzled OLDSSH.
// caveat 4 & 16 are likely more constrained than this.  8 bit is correct
void RetSSHSwizzleWidthHeight(int *rw,int *rh,int w,int h,int bpp)
{
    int realwidth  = w;
	int realheight = h;
    switch (bpp)
    {
        case 4:// size need to be div by 8x8 for 4 bit else can not swizzle
        {
            realwidth  = (w + 31)&~31; // actually 32x32, 64x64, 128x128 or multiple of 256 (rectangular okay)
			realheight = (h + 31)&~31;

            break;
        }
        case 8:// size needs to be div by 8x4 for 8 bit else can not swizzle
        {
            realwidth  = (w + 15)&~15;
			realheight = (h + 15)&~15;
      	    break;
        }
        case 15:
        case 16:// size needs to be div by 16x16 for 15 bit else can not swizzle
        {
            realwidth  = (w + 15)&~15;
	 		realheight = (h + 15)&~15;
            break;
        }
    }
	*rw = realwidth;
	*rh = realheight;
}


//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// calculate the correct width and height for any swizzled MSH.
// PSP shapes must fit into blocks that are 16-bytes wide & 8 pixels in height.
void RetMSHSwizzleWidthHeight(int *rw,int *rh,int w,int h,int bpp)
{
    int realwidth  = w;
	int realheight = h;
    switch (bpp)
    {
        case 4:
        {
            realwidth  = (w + 31)&~31;
			realheight = (h + 7)&~7;
            break;
        }
        case 8:
        {
            realwidth  = (w + 15)&~15;
			realheight = (h + 7)&~7;
      	    break;
        }
        case 15:
        case 16:
        {
            realwidth  = (w + 7)&~7;
	 		realheight = (h + 7)&~7;
            break;
        }
		case 32:
		{
            realwidth  = (w + 3)&~3;
	 		realheight = (h + 7)&~7;
            break;
		}
    }
	*rw = realwidth;
	*rh = realheight;
}


//--------------------------------------------------------------------------------------------------------------------------------------------------------------
// look up the pixel offset for 32bpp swizzled shape.
int GC_SHPCMN_SwizzleGetPixelOffset32( int x, int y, int width)
{
    int NumberOfBlocksX = (3+width)>>2;
    int XBlock = x>>2;
    int YBlock = y>>2;
    int XPix   = x&3;
    int YPix   = y&3;

    return  ((YBlock*NumberOfBlocksX + XBlock)<<6) + // the offset to the current block
            ((YPix<<3) + (XPix<<1)); // within the block the current pixel
}    

//--------------------------------------------------------------------------------------------------------------------------------------------------------------
// loop up the pixel offset for a 16bpp swizzled shape.
int GC_SHPCMN_SwizzleGetPixelOffset16( int x, int y, int width)
{
    int NumberOfBlocksX = (3+width)>>2;
    int XBlock = x>>2;
    int YBlock = y>>2;
    int XPix   = x&3;
    int YPix   = y&3;
    
    return  ((YBlock*NumberOfBlocksX + XBlock)<<5) + // the offset to the current block
            ((YPix<<3) + (XPix<<1)); // within the block the current pixel
}    

//--------------------------------------------------------------------------------------------------------------------------------------------------------------
// Look up the pixel offset for a 8bpp swizzled shape.
int GC_SHPCMN_SwizzleGetPixelOffset8( int x, int y, int width)
{
    int NumberOfBlocksX = (7+width)>>3;
    int XBlock = x>>3;
    int YBlock = y>>2;
    int XPix   = x&7;
    int YPix   = y&3;
    
    return  ((YBlock*NumberOfBlocksX + XBlock)<<5) + // the offset to the current block
            ((YPix<<3) + XPix); // within the block the current pixel
}    

//--------------------------------------------------------------------------------------------------------------------------------------------------------------
// look up the pixel offset for a 4bpp GameCube Swizzled shape.
int GC_SHPCMN_SwizzleGetPixelOffset4( int x, int y, int width)
{
    int NumberOfBlocksX = (7+width)>>3;
    int XBlock = x>>3;
    int YBlock = y>>3;
    int XPix   = x&7;
    int YPix   = y&7;
    
    return  ((YBlock*NumberOfBlocksX + XBlock)<<5) + // the offset to the current block
            ((YPix<<2) + (XPix>>1)); // within the block the current pixel
}    

//--------------------------------------------------------------------------------------------------------------------------------------------------------------
// switching function for GameCube specific pixel offset lookups.
int SHPWswizzleGC(int x,int y,int width,int bpp)
{

    switch(bpp)
    {
        case 32:
            return GC_SHPCMN_SwizzleGetPixelOffset32( x, y, width);
        case 16:
        case 15:
            return GC_SHPCMN_SwizzleGetPixelOffset16( x, y, width);
        case 8:
            return GC_SHPCMN_SwizzleGetPixelOffset8 ( x, y, width);
        case 4:
            return GC_SHPCMN_SwizzleGetPixelOffset4 ( x, y, width);
        default:
            return 0;
    }
    return(0);
}


//**************************************************************************************************************************************************************

#define ISPOWER2(v) (!((v)&((v)-1)))

bool CanSwizzleSsh(int w,int h,int bpp)
{
    int realwidth  = w;
    int realheight = h;
    switch (bpp)
    {
        case 4:// size need to be div by 16x32 for 4 bit else can not swizzle
        {
            realwidth  = (w + 31)&~31; // 32x32, 64x64, 128x128 or multiple of 256 (rectangular okay)
			realheight = (h + 31)&~31;
            break;
        }
        case 8:// size needs to be div by 16x16 for 8 bit else can not swizzle
        {
            realwidth  = (w + 15)&~15;
			realheight = (h + 15)&~15;
      	    break;
        }
        case 15:
        case 16:// size needs to be div by 8x16 for 15 bit else can not swizzle
        {
            realwidth  = (w + 7)&~7;
	 		realheight = (h + 15)&~15;
            break;
        }
    }
    if ((realwidth == w && realheight == h)
	 || (ISPOWER2(w) && ISPOWER2(h)))
        return true;
    return false;
}

//--------------------------------------------------------------------------------------------------------
// helper function for SHAPE_unswizzle8() swizzles 8 then unswizzles 32. A little confusing
// but it works. Look up table was generate from Sony's code.
int RBlockConv8to32(unsigned char *p_input, unsigned char *p_output) 
{

	static int lut[] = {
	0 ,4 ,8 ,12 ,16 ,20 ,24 ,28 
	,2 ,6 ,10 ,14 ,18 ,22 ,26 ,30 
	,32 ,36 ,40 ,44 ,48 ,52 ,56 ,60 
	,34 ,38 ,42 ,46 ,50 ,54 ,58 ,62 
	,17 ,21 ,25 ,29 ,1 ,5 ,9 ,13 
	,19 ,23 ,27 ,31 ,3 ,7 ,11 ,15 
	,49 ,53 ,57 ,61 ,33 ,37 ,41 ,45 
	,51 ,55 ,59 ,63 ,35 ,39 ,43 ,47 
	,80 ,84 ,88 ,92 ,64 ,68 ,72 ,76 
	,82 ,86 ,90 ,94 ,66 ,70 ,74 ,78 
	,112 ,116 ,120 ,124 ,96 ,100 ,104 ,108 
	,114 ,118 ,122 ,126 ,98 ,102 ,106 ,110 
	,65 ,69 ,73 ,77 ,81 ,85 ,89 ,93 
	,67 ,71 ,75 ,79 ,83 ,87 ,91 ,95 
	,97 ,101 ,105 ,109 ,113 ,117 ,121 ,125 
	,99 ,103 ,107 ,111 ,115 ,119 ,123 ,127 
	,128 ,132 ,136 ,140 ,144 ,148 ,152 ,156 
	,130 ,134 ,138 ,142 ,146 ,150 ,154 ,158 
	,160 ,164 ,168 ,172 ,176 ,180 ,184 ,188 
	,162 ,166 ,170 ,174 ,178 ,182 ,186 ,190 
	,145 ,149 ,153 ,157 ,129 ,133 ,137 ,141 
	,147 ,151 ,155 ,159 ,131 ,135 ,139 ,143 
	,177 ,181 ,185 ,189 ,161 ,165 ,169 ,173 
	,179 ,183 ,187 ,191 ,163 ,167 ,171 ,175 
	,208 ,212 ,216 ,220 ,192 ,196 ,200 ,204 
	,210 ,214 ,218 ,222 ,194 ,198 ,202 ,206 
	,240 ,244 ,248 ,252 ,224 ,228 ,232 ,236 
	,242 ,246 ,250 ,254 ,226 ,230 ,234 ,238 
	,193 ,197 ,201 ,205 ,209 ,213 ,217 ,221 
	,195 ,199 ,203 ,207 ,211 ,215 ,219 ,223 
	,225 ,229 ,233 ,237 ,241 ,245 ,249 ,253 
	,227 ,231 ,235 ,239 ,243 ,247 ,251 ,255 };
    unsigned int i;
	for(i = 0; i < 256;i++)
	{
		p_output[i] = p_input[lut[i]];
	}
    return 0;
}


//--------------------------------------------------------------------------------------------------------
// helper function for SHAPE_unswizzle8(), handles the unswizzleing of a single page.
#define PSMT8_BLOCK_WIDTH  16
#define PSMT8_BLOCK_HEIGHT 16
#define PSMCT32_BLOCK_WIDTH 8
#define PSMCT32_BLOCK_HEIGHT 8
int RPageConv8to32(int width, int height, unsigned char *p_input, unsigned char *p_output) 
{

    static unsigned int block_table8[] = {
         0,  1,  4,  5, 16, 17, 20, 21,
         2,  3,  6,  7, 18, 19, 22, 23,
         8,  9, 12, 13, 24, 25, 28, 29,
        10, 11, 14, 15, 26, 27, 30, 31
    };

    static unsigned int block_table32[] = {
         0,  1,  4,  5, 16, 17, 20, 21,
         2,  3,  6,  7, 18, 19, 22, 23,
         8,  9, 12, 13, 24, 25, 28, 29,
        10, 11, 14, 15, 26, 27, 30, 31
    };

    unsigned int *index32_h, *index32_v, in_block_nb;

    unsigned char input_block[16 * 16], output_block[16 * 16];
    unsigned char *pi0, *pi1, *po0, *po1;
    int index0, index1, i, j, k;
    int n_width, n_height, input_page_line_size;
    int output_page_line_size;



    // --- create table for output 32bit buffer ---
    index32_h = (unsigned int*) galloc(8 * 4 * sizeof(unsigned int));
    index32_v = (unsigned int*) galloc(8 * 4 * sizeof(unsigned int));
    index0 = 0;
    for(i = 0; i < 4; i++) {
        for(j = 0; j < 8; j++) {
            index1 = block_table32[index0];
            index32_h[index1] = j;
            index32_v[index1] = i;
            index0++;
        }
    }


    // how many blocks we should calc (width/height)
    n_width = width / PSMT8_BLOCK_WIDTH;
    n_height = height / PSMT8_BLOCK_HEIGHT;

    memset(input_block, 0, 16 *16);
    memset(output_block, 0, 16 * 16);
    
    output_page_line_size  = 128;    // PSMT8 page width (byte)
    input_page_line_size = 256;    // PSMCT32 page width (byte)

    // now assume copying from page top. 
    for(i = 0; i < n_height; i++) {

        for(j = 0; j < n_width; j++) {
            

            in_block_nb = block_table8[i * n_width + j]; 
			po0 = input_block;
            po1 = p_input + 8 * index32_v[in_block_nb] * input_page_line_size + index32_h[in_block_nb] * 32;
            for(k = 0; k < PSMCT32_BLOCK_HEIGHT; k++) {
                memcpy(po0, po1, PSMCT32_BLOCK_WIDTH * 4);
                po0 += PSMCT32_BLOCK_WIDTH * 4;
                po1 += input_page_line_size;   
            }
            
            RBlockConv8to32(input_block, output_block);

            pi0 = output_block;
            pi1 = p_output + PSMT8_BLOCK_HEIGHT * i * output_page_line_size + j * PSMT8_BLOCK_WIDTH; // byte

            

            for(k = 0; k < PSMT8_BLOCK_HEIGHT; k++) {
                memcpy(pi1, pi0, PSMT8_BLOCK_WIDTH); // copy full 1 line of 1 block.
                pi0 += PSMT8_BLOCK_WIDTH;  
                pi1 += output_page_line_size;
            }

        }
    }
    
    gfree(index32_h);
    gfree(index32_v);

    return 0;
}

//--------------------------------------------------------------------------------------------------------
// PS2 swizzle function, swizzles as 8 then unswizzle 32bpp. See 4bpp verions for more info.
#define PSMT8_PAGE_WIDTH    128
#define PSMT8_PAGE_HEIGHT   64
#define PSMCT32_PAGE_WIDTH  64
#define PSMCT32_PAGE_HEIGHT 32
int SHAPE_unswizzle8(unsigned char *p_input, unsigned char *p_output,int width,int height) 
{
    int i, j, k;
    int n_page_h, n_page_w, n_page8_width_byte, n_page32_width_byte;
    int n_input_width_byte, n_output_width_byte, n_input_height, n_output_height;
    unsigned char *pi0, *pi1, *po0, *po1;
    unsigned char input_page[PSMT8_PAGE_WIDTH * PSMT8_PAGE_HEIGHT];
    unsigned char output_page[PSMCT32_PAGE_WIDTH * 4 * PSMCT32_PAGE_HEIGHT];
    memset(input_page, 0, PSMT8_PAGE_WIDTH * PSMT8_PAGE_HEIGHT);
    memset(output_page, 0, PSMCT32_PAGE_WIDTH * 4 * PSMCT32_PAGE_HEIGHT);

    n_page_w = (width - 1) / PSMT8_PAGE_WIDTH + 1;
    n_page_h = (height - 1) / PSMT8_PAGE_HEIGHT + 1;

    n_page8_width_byte = PSMT8_PAGE_WIDTH;
    n_page32_width_byte = PSMCT32_PAGE_WIDTH * 4;

    // --- set in/out buffer size (for image smaller than one page) ---
    // possible bug, I think n_page_w and n_page_h need to be swapped as well.
    // for now I cannot duplicate a scenario where it breaks so left as is.
    if(n_page_w == 1) {
        n_input_width_byte = width * 2;
        n_output_width_byte = width;
    } else {
        n_input_width_byte = n_page32_width_byte;
        n_output_width_byte = n_page8_width_byte;
    }

    if(n_page_h == 1) {
        n_input_height = height / 2;
        n_output_height = height;
    } else {
        n_input_height = PSMCT32_PAGE_HEIGHT;
        n_output_height = PSMT8_PAGE_HEIGHT;
    }

    // --- conversion ---
    for(i = 0; i < n_page_h; i++) {
		int mpageheight = i*PSMT8_PAGE_HEIGHT;
        for(j = 0; j < n_page_w; j++) {
            po0 = p_input + (n_input_width_byte * n_input_height) * n_page_w * i
                           + n_input_width_byte * j;
            po1 = input_page;
            for(k = 0; k < n_input_height && (mpageheight+(k*2)) < height; k++) {
                memcpy(po1, po0, n_input_width_byte);
                po0 += n_input_width_byte * n_page_w;
                po1 += n_page32_width_byte;
			}
            
			

            // --- convert a page ---
            RPageConv8to32(PSMT8_PAGE_WIDTH, PSMT8_PAGE_HEIGHT, input_page, output_page);

//			pi0 = p_output + (n_output_width_byte * PSMT8_PAGE_HEIGHT) * n_page_w * i 
//                          + n_output_width_byte * j;
			pi0 = p_output + (n_output_width_byte * n_output_height) * n_page_w * i 
                          + n_output_width_byte * j;
            pi1 = output_page;

            for(k = 0; k < n_output_height && (mpageheight+k) < height; k++) {
                memcpy(pi0, pi1, n_output_width_byte);
                pi0 += n_output_width_byte * n_page_w;
                pi1 += n_page8_width_byte;
            }
                      
        }
    }
    return 0;
}


//--------------------------------------------------------------------------------------------------------
// helper function for SHAPE_unswizzle4() Unswizzles a single block of data.
int RBlockConv4to32(unsigned char *p_input, unsigned char *p_output) 
{

    static int lut[] = {
	   0 ,  8 , 16 , 24 , 32 , 40 , 48 , 56 
	,  2 , 10 , 18 , 26 , 34 , 42 , 50 , 58 
	,  4 , 12 , 20 , 28 , 36 , 44 , 52 , 60 
	,  6 , 14 , 22 , 30 , 38 , 46 , 54 , 62 
	, 64 , 72 , 80 , 88 , 96 ,104 ,112 ,120 
	, 66 , 74 , 82 , 90 , 98 ,106 ,114 ,122 
	, 68 , 76 , 84 , 92 ,100 ,108 ,116 ,124 
	, 70 , 78 , 86 , 94 ,102 ,110 ,118 ,126 
	, 33 , 41 , 49 , 57 ,  1 ,  9 , 17 , 25 
	, 35 , 43 , 51 , 59 ,  3 , 11 , 19 , 27 
	, 37 , 45 , 53 , 61 ,  5 , 13 , 21 , 29 
	, 39 , 47 , 55 , 63 ,  7 , 15 , 23 , 31 
	, 97 ,105 ,113 ,121 , 65 , 73 , 81 , 89 
	, 99 ,107 ,115 ,123 , 67 , 75 , 83 , 91 
	,101 ,109 ,117 ,125 , 69 , 77 , 85 , 93 
	,103 ,111 ,119 ,127 , 71 , 79 , 87 , 95 
	, 32 , 40 , 48 , 56 ,  0 ,  8 , 16 , 24 
	, 34 , 42 , 50 , 58 ,  2 , 10 , 18 , 26 
	, 36 , 44 , 52 , 60 ,  4 , 12 , 20 , 28 
	, 38 , 46 , 54 , 62 ,  6 , 14 , 22 , 30 
	, 96 ,104 ,112 ,120 , 64 , 72 , 80 , 88 
	, 98 ,106 ,114 ,122 , 66 , 74 , 82 , 90 
	,100 ,108 ,116 ,124 , 68 , 76 , 84 , 92 
	,102 ,110 ,118 ,126 , 70 , 78 , 86 , 94 
	,  1 ,  9 , 17 , 25 , 33 , 41 , 49 , 57 
	,  3 , 11 , 19 , 27 , 35 , 43 , 51 , 59 
	,  5 , 13 , 21 , 29 , 37 , 45 , 53 , 61 
	,  7 , 15 , 23 , 31 , 39 , 47 , 55 , 63 
	, 65 , 73 , 81 , 89 , 97 ,105 ,113 ,121 
	, 67 , 75 , 83 , 91 , 99 ,107 ,115 ,123 
	, 69 , 77 , 85 , 93 ,101 ,109 ,117 ,125 
	, 71 , 79 , 87 , 95 ,103 ,111 ,119 ,127};
	   
    unsigned int i, j, k, i0, i1, i2;
    unsigned int index0, index1;
    unsigned char c_in, c_out, *pIn;

    pIn = p_input;

    // for first step, we only think for a single block. (4bits, 32x16)
    index1 = 0;

    for(k = 0; k < 4; k++) {
        index0 = (k % 2) * 128;

        for(i = 0; i < 16; i++) {

            for(j = 0; j < 4; j++) {

                c_out = 0x00;

                // lower 4bit.
                i0 = lut[index0++];
                i1 = i0 / 2;
                i2 = (i0 & 0x1) * 4;
                c_in = (unsigned char)((pIn[i1] & (0x0f << i2)) >> i2);
                c_out = c_out | c_in;
                
                // uppper 4bit
                i0 = lut[index0++];
                i1 = i0 / 2;
                i2 = (i0 & 0x1) * 4;
                c_in = (unsigned char)((pIn[i1] & (0x0f << i2)) >> i2);
                c_out = (unsigned char)(c_out | ((c_in << 4) & 0xf0));

                p_output[index1++] = c_out;
            }
        }
        pIn += 64;
    }

    return 0;
}

//--------------------------------------------------------------------------------------------------------
// helper function for SHAPE_unswizzle4() unswizzles a single page.
#define PSMT4_BLOCK_WIDTH 32
#define PSMT4_BLOCK_HEIGHT 16
int RPageConv4to32(int width, int height, unsigned char *p_input, unsigned char *p_output) 
{

    static unsigned int block_table4[] = {
         0,  2,  8, 10,
         1,  3,  9, 11,
         4,  6, 12, 14,
         5,  7, 13, 15,
        16, 18, 24, 26,
        17, 19, 25, 27,
        20, 22, 28, 30,
        21, 23, 29, 31
    };

    static unsigned int block_table32[] = {
         0,  1,  4,  5, 16, 17, 20, 21,
         2,  3,  6,  7, 18, 19, 22, 23,
         8,  9, 12, 13, 24, 25, 28, 29,
        10, 11, 14, 15, 26, 27, 30, 31
    };

    unsigned int *index32_h, *index32_v, in_block_nb;
    unsigned char input_block[16 * 16], output_block[16 * 16];
    unsigned char *pi0, *pi1, *po0, *po1;
    int index0, index1, i, j, k;
    int n_width, n_height, input_page_line_size;
    int output_page_line_size;

    // --- create table for output 32bit buffer ---
    index32_h = (unsigned int*) galloc(8 * 4 * sizeof(unsigned int));
    index32_v = (unsigned int*) galloc(8 * 4 * sizeof(unsigned int));
    index0 = 0;
    for(i = 0; i < 4; i++) {
        for(j = 0; j < 8; j++) {
            index1 = block_table32[index0];
            index32_h[index1] = j;
            index32_v[index1] = i;
            index0++;
        }
    }
    n_width = width / 32;
    n_height = height / 16;
    memset(input_block, 0, 16 *16);
    memset(output_block, 0, 16 * 16);
    
    input_page_line_size = 256;       // PSMCT32 page width (byte)
    output_page_line_size = 128 / 2;    // PSMT4 page width (byte)

    // now assume copying from page top. 
    for(i = 0; i < n_height; i++) {

        for(j = 0; j < n_width; j++) {
            
			in_block_nb = block_table4[i * n_width + j]; 

            po0 = input_block;
            po1 = p_input + 8 * index32_v[in_block_nb] * input_page_line_size + index32_h[in_block_nb] * 32;
            for(k = 0; k < PSMCT32_BLOCK_HEIGHT; k++) {
                memcpy(po0, po1, PSMCT32_BLOCK_WIDTH * 4);
                po0 += PSMCT32_BLOCK_WIDTH * 4;
                po1 += input_page_line_size;   
            }

            RBlockConv4to32(input_block, output_block);
			pi0 = output_block;
            pi1 = p_output + 16 * i * output_page_line_size + j * 16;
            for(k = 0; k < PSMT4_BLOCK_HEIGHT; k++) 
			{
                memcpy(pi1, pi0, PSMT4_BLOCK_WIDTH / 2); // copy full 1 line of 1 block.
                pi0 += PSMT4_BLOCK_WIDTH / 2;
                pi1 += output_page_line_size;
            }


        }
    }   
    gfree(index32_h);
    gfree(index32_v);

    return 0;
}


//--------------------------------------------------------------------------------------------------------
// unswizzle function for 4bpp PS2 shapes. swizzles 32 and then unswizzles 4. We usually
// swizzle 4 then unswizzle 32 so when we upload as 32 we get swizzled 4, this is faster
// than uploading 4.
#define PSMT4_PAGE_WIDTH    128
#define PSMT4_PAGE_HEIGHT   128
int SHAPE_unswizzle4(unsigned char *p_input, unsigned char *p_output,int width, int height) 
{
    int i, j, k;
    int n_page_h, n_page_w, n_page4_width_byte, n_page32_width_byte;
    unsigned char *pi0, *pi1, *po0, *po1;
    int n_input_width_byte, n_output_width_byte, n_input_height, n_output_height;
    unsigned char input_page[PSMT4_PAGE_WIDTH / 2 * PSMT4_PAGE_HEIGHT];
    unsigned char output_page[PSMCT32_PAGE_WIDTH * 4 * PSMCT32_PAGE_HEIGHT];

    memset(input_page, 0, PSMT4_PAGE_WIDTH / 2 * PSMT4_PAGE_HEIGHT);
    memset(output_page, 0, PSMCT32_PAGE_WIDTH * 4 * PSMCT32_PAGE_HEIGHT);

    n_page_w = (width - 1) / PSMT4_PAGE_WIDTH + 1;
    n_page_h = (height - 1) / PSMT4_PAGE_HEIGHT + 1;

    n_page4_width_byte = PSMT4_PAGE_WIDTH / 2;
    n_page32_width_byte = PSMCT32_PAGE_WIDTH * 4;

   // --- set in/out buffer size (for image smaller than one page) ---
    if(n_page_h == 1) {
        n_input_width_byte = height * 2; 
        n_output_height = height;
    } else {
        n_input_width_byte = n_page32_width_byte;
        n_output_height = PSMT4_PAGE_HEIGHT;
    }

    if(n_page_w == 1) {
        n_input_height = width / 4;
        n_output_width_byte = width / 2;
    } else {
        n_input_height = PSMCT32_PAGE_HEIGHT;
        n_output_width_byte = n_page4_width_byte;
    }

    for(i = 0; i < n_page_h; i++) {
        for(j = 0; j < n_page_w; j++) {
            po0 = p_input + (n_input_width_byte * n_input_height) * n_page_w * i
                          + n_input_width_byte * j;
            po1 = input_page;
            for(k = 0; k < n_input_height; k++) {
                memcpy(po1, po0, n_input_width_byte);
                po0 += n_input_width_byte * n_page_w;
                po1 += n_page32_width_byte;
            }          
 
            RPageConv4to32(PSMT4_PAGE_WIDTH, PSMT4_PAGE_HEIGHT, input_page, output_page);

			pi0 = p_output + (n_output_width_byte * n_output_height) * n_page_w * i 
                          + n_output_width_byte * j;
            pi1 = output_page;

            for(k = 0; k < n_output_height; k++) {
                memcpy(pi0, pi1, n_output_width_byte);
                pi0 += n_output_width_byte * n_page_w;
                pi1 += n_page4_width_byte;
            }

        }
    }
    return 0;
}



#undef PSMT8_BLOCK_WIDTH  
#undef PSMT8_BLOCK_HEIGHT 
#undef PSMCT32_BLOCK_WIDTH
#undef PSMCT32_BLOCK_HEIGHT
#undef PSMT8_PAGE_WIDTH
#undef PSMT8_PAGE_HEIGHT
#undef PSMCT32_PAGE_WIDTH
#undef PSMCT32_PAGE_HEIGHT
#undef PSMT4_BLOCK_WIDTH
#undef PSMT4_BLOCK_HEIGHT
#undef PSMT4_PAGE_WIDTH
#undef PSMT4_PAGE_HEIGHT


//--------------------------------------------------------------------------------------------------------
// pixel lookup function for BOTH XBox and PC Shapes. The name is a bit misleading,
// it should have both XSH and FSH in the name.
int SHPRswizzleXSH(int x,int y,int width,int height)
{
    int off;
    int nx=0;
    int ny=0;
    int xpos=0;
    int ypos=0;
    int tpix=1;
    off = y*width+x;

    while (tpix<width || tpix<height)
    {
        if (tpix<width)
        {
            nx |= ((off&1)<<xpos);
            ++xpos;
            off >>= 1;
        }
        if (tpix<height)
        {
            ny |= ((off&1)<<ypos);
            ++ypos;
            off >>= 1;
        }
        tpix <<= 1;
    }
    off = ny*width+nx;
    return(off);
}

/*

//--------------------------------------------------------------------------------------------------------
// pixel lookup function for 32bpp GameCube Shapes.
static int GC_SwizzleGetPixelOffset32( int x, int y, int width)
{
    int NumberOfBlocksX = (3+width)>>2;
    int XBlock = x>>2;
    int YBlock = y>>2;
    int XPix   = x&3;
    int YPix   = y&3;

    return  ((YBlock*NumberOfBlocksX + XBlock)<<6) + // the offset to the current block
            ((YPix<<3) + (XPix<<1)); // within the block the current pixel
}

//--------------------------------------------------------------------------------------------------------
// pixel lookup function for 16bpp GameCube Shapes.
static int GC_SwizzleGetPixelOffset16( int x, int y, int width)
{
    int NumberOfBlocksX = (3+width)>>2;
    int XBlock = x>>2;
    int YBlock = y>>2;
    int XPix   = x&3;
    int YPix   = y&3;
    
    return  ((YBlock*NumberOfBlocksX + XBlock)<<5) + // the offset to the current block
            ((YPix<<3) + (XPix<<1)); // within the block the current pixel
}    

//--------------------------------------------------------------------------------------------------------
// pixel lookup function for 8bpp GameCube Shapes.
static int GC_SwizzleGetPixelOffset8( int x, int y, int width)
{
    int NumberOfBlocksX = (7+width)>>3;
    int XBlock = x>>3;
    int YBlock = y>>2;
    int XPix   = x&7;
    int YPix   = y&3;
    
    return  ((YBlock*NumberOfBlocksX + XBlock)<<5) + // the offset to the current block
            ((YPix<<3) + XPix); // within the block the current pixel
}    

//--------------------------------------------------------------------------------------------------------
// pixel lookup function for 4bpp GameCube Shapes.
static int GC_SwizzleGetPixelOffset4( int x, int y, int width)
{
    int NumberOfBlocksX = (7+width)>>3;
    int XBlock = x>>3;
    int YBlock = y>>3;
    int XPix   = x&7;
    int YPix   = y&7;
    
    return  ((YBlock*NumberOfBlocksX + XBlock)<<5) + // the offset to the current block
            ((YPix<<2) + (XPix>>1)); // within the block the current pixel
}    


*/

//--------------------------------------------------------------------------------------------------------
// switching function for GameCube swizzling.
int SHPRswizzleGC(int x,int y,int width,int bpp)
{

    switch(bpp)
    {
        case 32:
            return GC_SHPCMN_SwizzleGetPixelOffset32( x, y, width);
        case 16:
        case 15:
            return GC_SHPCMN_SwizzleGetPixelOffset16( x, y, width);
        case 8:
            return GC_SHPCMN_SwizzleGetPixelOffset8 ( x, y, width);
        case 4:
            return GC_SHPCMN_SwizzleGetPixelOffset4 ( x, y, width);
        default:
            return 0;
    }
    return(0);

}

//--------------------------------------------------------------------------------------------------------
// scan over the entire image and determine if the image is greyscale or not.
// return non zero on greyscale. 
// for 8bpp images data can be NULL.
bool isShapeIntensityTexture(ARGB *data, const GINFO * ginfo)
{
    int i = 0;
    bool allsame = true;


    if(ginfo->bpp == 8)
    {
        for(i = ginfo->startcolour; i < ginfo->numcolours;i++)
        {
            if(ginfo->colourtbl[i].r != ginfo->colourtbl[i].g || ginfo->colourtbl[i].g != ginfo->colourtbl[i].b)    
                allsame = false;
        }
    }
    else
    {
        int j     = 0;
        int index = 0;
        for(i = 0; i < ginfo->height; i++)
        {
            for(j = 0; j < ginfo->width;j++)
            {
                index = i*ginfo->width + j;
                if(data[index].r != data[index].g || data[index].g != data[index].b)
                    allsame = false;
            }
        }
    }
    return allsame;
}


//--------------------------------------------------------------------------------
static bool isbig(int type)
{
    if (type==TYPEGSH || type==TYPEGSS || type==TYPEZSH || type==TYPEZSS || type==TYPEYSH || type==TYPEYSS)
        return true;
    return false;
}

//--------------------------------------------------------------------------------
void gputx(void *memptr, unsigned int val, int numbytes, int type)
{
	if (isbig(type))
        gputm(memptr, val, numbytes);
    else
        gputi(memptr, val, numbytes);
    return;
}

//--------------------------------------------------------------------------------
unsigned int ggetx(const void *memptr, int numbytes, int type)
{
    if (isbig(type))
        return ggetm(memptr, numbytes);

    return ggeti(memptr, numbytes);
}

