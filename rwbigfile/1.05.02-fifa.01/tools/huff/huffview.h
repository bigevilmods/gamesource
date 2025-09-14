// (c) Electronic Arts. All Rights Reserved.

/*------------------------------------------------------------------*/
/*																	*/
/*					   Lumpy File Support v6.10 					*/
/*																	*/
/*				  by Pete Veys and FrANK G. Barchard				*/
/*																	*/
/*					  Header File - Aug 12, 1993					*/
/*																	*/
/*------------------------------------------------------------------*/

/* simple pack types */

#if defined(_MSC_VER)
#pragma once
#endif


#define NOPAC			0

#define BTREEPAC		1
#define HUFFPAC 		2
#define REFPAC			3
#define BZ2PAC			4
#define ZIPPAC			5

#define PACKTYPES		5

#define DELTAWPAC		20
#define LOHIPAC 		21

#define NEWPACKTYPES	22

#define BESTPAC 		35
#define LUMPUNIT		36
#define LUMPGROUP		37
#define AEXE			38


int 	TypeOFile(const unsigned char *s);
size_t 	GetULen(char *ptr, size_t len);
int 	ViewFile(char *ufname, char *name, struct SPITMEM *infile, int n, int offset);
void	StartViewFile(void);
void	StopViewFile(void);
char	*StartOFile(const char *s);

char	*LUMPYpercentstr(char *buf, size_t olen, size_t nlen);
