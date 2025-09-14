// (c) Electronic Arts. All Rights Reserved.

#ifndef WILDMATCH_H
#define WILDMATCH_H

#ifdef __cplusplus
extern "C" {
#endif

#define WILDMATCH_VERSION 104

extern int WILDcharstar;
extern int WILDcharquestion;
extern int WILDcharnumber;
extern int WILDcharexclamation;
extern int WILDcharleftbracket;
extern int WILDcharrightbracket;
extern int WILDtoupper;

int WILD_imatchpath(char *dest, const char *src, const char *wildin, char *wildout);

int WILD_match(char *dest, const char *src, const char *wildin, char *wildout);
int WILD_nmatch(char *dest, const char *src, const char *wildin, char *wildout, int *index);
int WILD_imatch(char *dest, const char *src, const char *wildin, char *wildout);
int WILD_nimatch(char *dest, const char *src, const char *wildin, char *wildout, int *index);
int WILD_icmatch(char *dest, const char *src, const char *wildin, char *wildout);
int WILD_nicmatch(char *dest, const char *src, const char *wildin, char *wildout, int *index);

int WILD_applymathrules(char **numbers, int curnum, int numnumbers);
void WILD_addmathrule(char *strin);
void WILD_deletemathrules(void);

#ifdef __cplusplus
}
#endif

#endif // WILDMATCH_H

