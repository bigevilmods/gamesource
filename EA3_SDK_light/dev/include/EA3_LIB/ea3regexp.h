// (c) 1998-2010 Electronic Arts Inc.

#ifndef REGEXP_H
#define REGEXP_H
#ifndef _ANSI_ARGS_
#define _ANSI_ARGS_(x) x
#endif
#define NSUBEXP 20
typedef struct regexp {
	char *startp[NSUBEXP];
	char *endp[NSUBEXP];
	char regstart;		/* Internal use only. */
	char reganch;		/* Internal use only. */
	char *regmust;		/* Internal use only. */
	int regmlen;		/* Internal use only. */
	char program[1];	/* Unwarranted chumminess with compiler. */
} regexp;
#if !defined(EA3_EXPORT) 
  #if defined(EA_PLATFORM_WINDOWS) && defined(EA_DLL) 
    #ifdef EA3_EXPORTS
		#define EA3_EXPORT __declspec(dllexport)
	#else
		#define EA3_EXPORT __declspec(dllimport)
	#endif
  #else
    #define EA3_EXPORT
  #endif
#endif
EA3_EXPORT regexp *regComp(char *exp);
EA3_EXPORT int regExec(regexp *prog,char *string,char *start);
EA3_EXPORT char *regsub(const char *inPattern,const char *inString,const char *inSubString,int *numMatches,bool all=true,bool noCase=false);

/*
void TclRegSub(regexp *prog, char *source, char *dest);
*/
EA3_EXPORT void regError(const char *msg);
EA3_EXPORT int stringMatch(const char *string,const char *pattern);
EA3_EXPORT const char *getRegError(void);
#endif
