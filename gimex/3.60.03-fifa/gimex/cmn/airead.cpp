// (c) 1994-2007 Electronic Arts Inc.

/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*                 AI - Adobe Illustrator Gimex                     */
/*                                                                  */
/*                      by Kaz Okuda EAC                            */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History:                                       */
/* ------- ----   -- --------                                       */
/*    1.00 980105 KO Initial version                                */
/*    1.01 980127 KO Uses DLL, rasterizes 2x the reduces            */
/*    1.02 000906 FB close file before freeing handle               */
/*    1.03 001204 FB ifdef for intelc                               */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* 32 bit                                                           */
/* write not supported                                              */
/* Requires ghostscript installed and in path to work               */
/* Used gsdll32.dll (Which may require other files) to rasterize    */
/*                                                                  */
/* Constructs a temporary layer file which ghostscript rasterizes   */
/* to a png file.  The png file is imported using the png gimex.    */
/*                                                                  */
/* Links with pngread.c does not include it as it defines some      */
/* incompatabilities with windows.h                                 */
/*                                                                  */
/* Files: airead.cpp aiwrite.cpp aigimex.h pnggimex.h                   */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Format Notes:                                                    */
/* -------------                                                    */
/* Modified postscript. Includes layers which are treated as frames */
/*                                                                  */
/*------------------------------------------------------------------*/
/* END ABSTRACT */

#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>

#include "gimex.h"
#include "aigimex.h"
#include "fsize.cpp"
#include "fcrop.cpp"

/*
#define PNG_about AIaboutgimex
#define PNG_is    AIisgimex
#define PNG_open  AIopengimex
#define PNG_info  AIinfogimex
#define PNG_read  AIreadgimex
#define PNG_close AIclosegimex

#include "pngread.cpp"
*/

#define AIaboutgimex PNG_about
#define AIisgimex PNG_is
#define AIopengimex PNG_open
#define AIinfogimex PNG_info
#define AIreadgimex PNG_read
#define AIclosegimex PNG_close

#include "pnggimex.h"


static void *grealloc(void *pointer, size_t oldsize, size_t newsize);

/****************************************************************/
/*  Included from ps.c from ghostscript distribution            */
/****************************************************************/
/*
 * ps.c -- Postscript scanning and copying routines.
 * Copyright (C) 1992, 1994  Timothy O. Theisen.  All rights reserved.
 *
 * This file is part of Ghostview.
 *
 * Ghostview is distributed with NO WARRANTY OF ANY KIND.  No author or
 * distributor accepts any responsibility for the consequences of using
 * it, or for whether it serves any particular purpose or works at all,
 * unless he or she says so in writing.  Refer to the Ghostview Free
 * Public License (the "License") for full details.
 *
 * Every copy of Ghostview must include a copy of the License, normally
 * in a plain ASCII text file named PUBLIC.  The License grants you the
 * right to copy, modify and redistribute Ghostview, but only under
 * certain conditions described in the License.  Among other things, the
 * License requires that the copyright notice and this notice be preserved
 * on all copies.
 *
 *
 *   Author: Tim Theisen           Systems Programmer
 * Internet: tim@cs.wisc.edu       Department of Computer Sciences
 *     UUCP: uwvax!tim             University of Wisconsin-Madison
 *    Phone: (608)262-0438         1210 West Dayton Street
 *      FAX: (608)262-9777         Madison, WI   53706
 */
/* modified by Russell Lang, rjl@aladdin.com to use with GSview
 * 1995-01-11
 * supports DOS EPS files (binary header)
 * supports the following incorrect DSC usage
 *   %%BeginData lines > 254 char                 (Microsoft Windows)
 *   EPS inclusions surrounded by %MSEPS Preamble/%MSEPS Trailer
 *    instead of %%BeginDocument/%%EndDocument    (Microsoft Windows).
 *   %!PS-Adobe instead of %!PS-Adobe-            (OS/2)
 *   atend instead of (atend)
 * rjl 1995-04-01
 *   Commented out two lines to deal with documents that include
 *   code after %%EndSetup and before %%Page:     (Microsoft Windows)
 * rjl 1996-08-08
 *   Modified gettext() to skip over trailing ')' of (text)
 *   Modified psscan() to skip over HP LaserJet PJL prologue
 */

#if defined(_MSC_VER)
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#define strcasecmp(s,t) stricmp(s,t)
extern void pserror(char *str);

#else

#ifndef SEEK_SET
#define SEEK_SET 0
#endif
#ifndef BUFSIZ
#define BUFSIZ 1024
#endif
#include <ctype.h>
#include <X11/Xos.h>        /* #includes the appropriate <string.h> */
#define pserror(str)

#endif


/****************
 * INCLUDE PS.H *
 ****************/
/*
 * ps.h -- Include file for PostScript routines.
 * Copyright (C) 1992, 1994  Timothy O. Theisen.  All rights reserved.
 *
 * This file is part of Ghostview.
 *
 * Ghostview is distributed with NO WARRANTY OF ANY KIND.  No author or
 * distributor accepts any responsibility for the consequences of using
 * it, or for whether it serves any particular purpose or works at all,
 * unless he or she says so in writing.  Refer to the Ghostview Free
 * Public License (the "License") for full details.
 *
 * Every copy of Ghostview must include a copy of the License, normally
 * in a plain ASCII text file named PUBLIC.  The License grants you the
 * right to copy, modify and redistribute Ghostview, but only under
 * certain conditions described in the License.  Among other things, the
 * License requires that the copyright notice and this notice be preserved
 * on all copies.
 *
 *   Author: Tim Theisen           Systems Programmer
 * Internet: tim@cs.wisc.edu       Department of Computer Sciences
 *     UUCP: uwvax!tim             University of Wisconsin-Madison
 *    Phone: (608)262-0438         1210 West Dayton Street
 *      FAX: (608)262-9777         Madison, WI   53706
 */

#ifndef NeedFunctionPrototypes
#if defined(FUNCPROTO) || defined(__STDC__) || defined(__cplusplus) || defined(c_plusplus) || defined(__TURBOC__) || defined(OS2)
#define NeedFunctionPrototypes 1
#else
#define NeedFunctionPrototypes 0
#endif /* __STDC__ */
#endif /* NeedFunctionPrototypes */

    /* Constants used to index into the bounding box array. */

#define LLX 0
#define LLY 1
#define URX 2
#define URY 3

    /* Constants used to store keywords that are scanned. */
    /* NONE is not a keyword, it tells when a field was not set */

enum {ATEND = -1, NONE = 0, PORTRAIT, LANDSCAPE, ASCEND, DESCEND, SPECIAL};

#define PSLINELENGTH 257    /* 255 characters + 1 newline + 1 NULL */

/* rjl: DOS binary EPS header */
#define PS_DWORD unsigned int  /* must be 32 bits unsigned */
#define PS_WORD unsigned short  /* must be 16 bits unsigned */
typedef struct tagDOSEPS {
   unsigned char id[4];
   PS_DWORD ps_begin;
   PS_DWORD ps_length;
   PS_DWORD mf_begin;
   PS_DWORD mf_length;
   PS_DWORD tiff_begin;
   PS_DWORD tiff_length;
   PS_WORD checksum;
} DOSEPS;


typedef struct _GRefMod
{
  AIINFO * ai;
  char * pathname;
}GRefMod;


struct document {
    int  epsf;              /* Encapsulated PostScript flag. */
    char *title;            /* Title of document. */
    char *date;             /* Creation date. */
    int  pageorder;         /* ASCEND, DESCEND, SPECIAL */
    int beginheader, endheader;    /* offsets into file */
    unsigned int lenheader;
    int beginpreview, endpreview;
    unsigned int lenpreview;
    int begindefaults, enddefaults;
    unsigned int lendefaults;
    int beginprolog, endprolog;
    unsigned int lenprolog;
    int beginsetup, endsetup;
    unsigned int lensetup;
    int begintrailer, endtrailer;
    unsigned int lentrailer;
    int  boundingbox[4];
    int  default_page_boundingbox[4];
    int  orientation;           /* PORTRAIT, LANDSCAPE */
    int  default_page_orientation;  /* PORTRAIT, LANDSCAPE */
    unsigned int nummedia;
    struct documentmedia *media;
    struct documentmedia *default_page_media;
    DOSEPS *doseps;
    unsigned int numpages;
    struct page *pages;
    int linecount;      /* rjl: line count when parsing DSC comments */
};

struct page {
    char *label;
    int  boundingbox[4];
    struct documentmedia *media;
    int  orientation;           /* PORTRAIT, LANDSCAPE */
    int begin, end;            /* offsets into file */
    unsigned int len;
};

struct documentmedia {
    char *name;
    int width, height;
};

    /* list of standard paper sizes from Adobe's PPD. */

extern struct documentmedia papersizes[];

    /* scans a PostScript file and return a pointer to the document
       structure.  Returns NULL if file does not Conform to commenting
       conventions . */

#if NeedFunctionPrototypes
struct document *psscan(FILE *file);
#else
struct document *psscan();
#endif

    /* gfree data structure galloc'ed by psscan */

#if NeedFunctionPrototypes
void psgfree(struct document *);
#else
void psgfree();
#endif

    /* Copy a portion of the PostScript file */

#if NeedFunctionPrototypes
void pscopy(FILE *from, FILE *to, int begin, int end);
#else
void pscopy();
#endif

    /* Copy a portion of the PostScript file upto a comment */

#if NeedFunctionPrototypes
char *pscopyuntil(FILE *from, FILE *to, int begin, int end,
          const char *comment);
#else
char *pscopyuntil();
#endif

    /* DOS EPS header reading */

#if NeedFunctionPrototypes
unsigned int ps_read_doseps(FILE *file, DOSEPS *doseps);
#else
unsigned int ps_read_doseps();
#endif

#if NeedFunctionPrototypes
PS_DWORD reorder_dword(PS_DWORD val);
#else
PS_DWORD reorder_dword();
#endif

#if NeedFunctionPrototypes
PS_WORD reorder_word(PS_WORD val);
#else
PS_WORD reorder_word();
#endif

/********************
 * END INCLUDE PS.H *
 ********************/




#ifdef BSD4_2
#define memset(a,b,c) bzero(a,c)
#endif

/* redefine fgets to use a version that handles Unix, PC, or Mac EOL characters */
#define fgets psfgets
#if NeedFunctionPrototypes
char *psfgets(char *s, int n, FILE *stream);
#else
char *psfgets();
#endif

/* length calculates string length at compile time */
/* can only be used with character constants */
#define length(a) (sizeof(a)-1)
#define iscomment(a, b) (strncmp(a, b, length(b)) == 0)
#define DSCcomment(a) (a[0] == '%' && a[1] == '%')

    /* list of standard paper sizes from Adobe's PPD. */

struct documentmedia papersizes[] = {
    {"Letter",       612,  792},
    {"LetterSmall",  612,  792},
    {"Tabloid",      792, 1224},
    {"Ledger",      1224,  792},
    {"Legal",        612, 1008},
    {"Statement",    396,  612},
    {"Executive",    540,  720},
    {"A3",       842, 1190},
    {"A4",       595,  842},
    {"A4Small",      595,  842},
    {"A5",       420,  595},
    {"B4",       729, 1032},
    {"B5",       516,  729},
    {"Folio",        612,  936},
    {"Quarto",       610,  780},
    {"10x14",        720, 1008},
    {NULL,         0,    0}
};


#if NeedFunctionPrototypes
static char *readline(char *line, int size, FILE *fp, unsigned int enddoseps,
  int *position, unsigned int *line_len, unsigned int *line_count);
#else
static char *readline();
#endif

#if NeedFunctionPrototypes
static char *gettextline(char *line);
#else
static char *gettextline();
#endif

#if NeedFunctionPrototypes
static char *gettext(char *line, char **next_char);
#else
static char *gettext();
#endif

#if NeedFunctionPrototypes
static int  blank(char *line);
#else
static int  blank();
#endif

static int isPDF(FILE * fp)
{
    char buf[200];
    fread(buf,1,199,fp);
    buf[199] = '\0';
    if(strstr(buf,"PDF"))
        return 1;
    return 0;
}

static void seekToAdobeTag(FILE * fp)
{
    char * adobe = "%!PS-Adobe";
    int i = 0;
    int pos = 0;
    char c = 0;
    while(fread(&c,1,1,fp))
    {
       if(c == adobe[i])
       {
            pos = ftell(fp);
            i++;
            while(fread(&c,1,1,fp) && c == adobe[i] && i != (int)strlen(adobe)){i++;}
            if(i == (int)strlen(adobe))
            {
                fseek(fp,pos-1,SEEK_SET);
                return;
            }
            else
                i = 0;
       }
       i = 0;
    }
    fseek(fp,0,SEEK_SET);
}

/*
static int retEndDataTagPos(FILE * fp)
{
    char * endtag = "%%EndData";
    int i = 0;
    int pos = 0;
    char c = 0;
    while(fread(&c,1,1,fp))
    {
       if(c == endtag[i])
       {
            pos = ftell(fp);
            i++;
            while(fread(&c,1,1,fp) && c == endtag[i] && i != strlen(endtag)){i++;}
            if(i == strlen(endtag))
            {
                pos += strlen(endtag) + 1;    // add the extra byte for \n.
                //fseek(fp,pos-1,SEEK_SET);
                rewind(fp);
                seekToAdobeTag(fp);
                return pos;
            }
            else
                i = 0;
       }
       i = 0;
    }
    fseek(fp,0,SEEK_SET);
}

*/

/*
 *  psscan -- scan the PostScript file for document structuring comments.
 *
 *  This scanner is designed to retrieve the information necessary for
 *  the ghostview previewer.  It will scan files that conform to any
 *  version (1.0, 2.0, 2.1, or 3.0) of the document structuring conventions.
 *  It does not really care which version of comments the file contains.
 *  (The comments are largely upward compatible.)  It will scan a number
 *  of non-conforming documents.  (You could have part of the document
 *  conform to V2.0 and the rest conform to V3.0.  It would be similar
 *  to the DC-2 1/2+, it would look funny but it can still fly.)
 *
 *  This routine returns a pointer to the document structure.
 *  The structure contains the information relevant to previewing.
 *      These include EPSF flag (to tell if the file is a encapsulated figure),
 *      Page Media (for the Page Size), Bounding Box (to minimize backing
 *      pixmap size or determine window size for encapsulated PostScript),
 *      Orientation of Paper (for default transformation matrix), and
 *      Page Order.  The title and CreationDate are also retrieved to
 *      help identify the document.
 *
 *      The following comments are examined:
 *
 *      Header section:
 *      Must start with %!PS-Adobe.  Version numbers ignored.
 *
 *      %!PS-Adobe* [EPSF-*]
 *      %%BoundingBox: <int> <int> <int> <int>|(atend)
 *      %%CreationDate: <textline>
 *      %%Orientation: Portrait|Landscape|(atend)
 *      %%Pages: <uint> [<int>]|(atend)
 *      %%PageOrder: Ascend|Descend|Special|(atend)
 *      %%Title: <textline>
 *      %%DocumentMedia: <text> <real> <real> <real> <text> <text>
 *      %%DocumentPaperSizes: <text>
 *      %%EndComments
 *
 *      Note: Either the 3.0 or 2.0 syntax for %%Pages is accepted.
 *            Also either the 2.0 %%DocumentPaperSizes or the 3.0
 *            %%DocumentMedia comments are accepted as well.
 *
 *      The header section ends either explicitly with %%EndComments or
 *      implicitly with any line that does not begin with %X where X is
 *      a not whitespace character.
 *
 *      If the file is encapsulated PostScript the optional Preview section
 *      is next:
 *
 *      %%BeginPreview
 *      %%EndPreview
 *
 *      This section explicitly begins and ends with the above comments.
 *
 *      Next the Defaults section for version 3 page defaults:
 *
 *      %%BeginDefaults
 *      %%PageBoundingBox: <int> <int> <int> <int>
 *      %%PageOrientation: Portrait|Landscape
 *      %%PageMedia: <text>
 *      %%EndDefaults
 *
 *      This section explicitly begins and ends with the above comments.
 *
 *      The prolog section either explicitly starts with %%BeginProlog or
 *      implicitly with any nonblank line.
 *
 *      %%BeginProlog
 *      %%EndProlog
 *
 *      The Prolog should end with %%EndProlog, however the proglog implicitly
 *      ends when %%BeginSetup, %%Page, %%Trailer or %%EOF are encountered.
 *
 *      The Setup section is where the version 2 page defaults are found.
 *      This section either explicitly begins with %%BeginSetup or implicitly
 *      with any nonblank line after the Prolog.
 *
 *      %%BeginSetup
 *      %%PageBoundingBox: <int> <int> <int> <int>
 *      %%PageOrientation: Portrait|Landscape
 *      %%PaperSize: <text>
 *      %%EndSetup
 *
 *      The Setup should end with %%EndSetup, however the setup implicitly
 *      ends when %%Page, %%Trailer or %%EOF are encountered.
 *
 *      Next each page starts explicitly with %%Page and ends implicitly with
 *      %%Page or %%Trailer or %%EOF.  The following comments are recognized:
 *
 *      %%Page: <text> <uint>
 *      %%PageBoundingBox: <int> <int> <int> <int>|(atend)
 *      %%PageOrientation: Portrait|Landscape
 *      %%PageMedia: <text>
 *      %%PaperSize: <text>
 *
 *      The tralier section start explicitly with %%Trailer and end with %%EOF.
 *      The following comment are examined with the proper (atend) notation
 *      was used in the header:
 *
 *      %%Trailer
 *      %%BoundingBox: <int> <int> <int> <int>|(atend)
 *      %%Orientation: Portrait|Landscape|(atend)
 *      %%Pages: <uint> [<int>]|(atend)
 *      %%PageOrder: Ascend|Descend|Special|(atend)
 *      %%EOF
 *
 *
 *  + A DC-3 received severe damage to one of its wings.  The wing was a total
 *    loss.  There was no replacement readily available, so the mechanic
 *    installed a wing from a DC-2.
 */

struct document *
psscan(file)
    FILE *file;
{
    struct document *doc;
    int bb_set = NONE;
    int pages_set = NONE;
    int page_order_set = NONE;
    int orientation_set = NONE;
    int page_bb_set = NONE;
    int page_media_set = NONE;
    int preread;        /* flag which tells the readline isn't needed */
    int i;
    unsigned int maxpages = 0;
    unsigned int nextpage = 1;  /* Next expected page */
    unsigned int thispage;
    int ignore = 0;     /* whether to ignore page ordinals */
    char *label;
    char line[PSLINELENGTH];    /* 255 characters + 1 newline + 1 NULL */
    char text[PSLINELENGTH];    /* Temporary storage for text */
    int position;      /* Position of the current line */
    int beginsection;      /* Position of the beginning of the section */
    unsigned int line_count;    /* line count for error messages */
    unsigned int line_len;  /* Length of the current line */
    unsigned int section_len;   /* Place to accumulate the section length */
    char *next_char;        /* 1st char after text returned by gettext() */
    char *cp;
    struct documentmedia *dmp;
    unsigned int enddoseps;    /* zero of not DOS EPS, otherwise position of end of ps section */
    DOSEPS doseps;


    line_count = 0;
    rewind(file);
    if(isPDF(file))
        seekToAdobeTag(file);
    else
        rewind(file);
    /* rjl: check for DOS EPS files and almost DSC files that start with ^D */
    enddoseps = ps_read_doseps(file, &doseps);
    position = ftell(file);
    if (fgetc(file) != '\004')
    fseek(file, position, SEEK_SET);
    if(position == 0)
    {
        if(isPDF(file))
            seekToAdobeTag(file);
        else
            rewind(file);
    }

    if (readline(line, sizeof line, file, enddoseps, &position, &line_len, &line_count) == NULL) {
    pserror("Warning: empty file.\n");
    return(NULL);
    }

    /* rjl: check for HP LaserJet prologue */
    if (iscomment(line, "\033%-12345X")) {
    /* found prolog, read until first DSC comment */
        while (readline(line, sizeof line, file, enddoseps, &position, &line_len, &line_count)) {
        if (line[0] == '%')
        break;
    }
        if (line[0] != '%') {
        pserror("Warning: error skipping PJL prologue.\n");
        return(NULL);
    }
    }

    /* Header comments */

    if (iscomment(line,"%!PS-Adobe") || iscomment(line,"%PDF-")) {  /* rjl: some almost DSC code doesn't include the hyphen or version number */
    doc = (struct document *) galloc(sizeof(struct document));
    if (doc == NULL) {
        pserror("Fatal Error: Dynamic memory exhausted.\n");
        exit(-1);
    }
    memset(doc, 0, sizeof(struct document));
    sscanf(line, "%*s %s", text);
    doc->epsf = iscomment(text, "EPSF-");
    doc->beginheader = position;
    section_len = line_len;
    if (enddoseps) { /* rjl: add doseps header */
        doc->doseps = (DOSEPS *) galloc(sizeof(DOSEPS));
        if (doc == NULL) {
        pserror("Fatal Error: Dynamic memory exhausted.\n");
        exit(-1);
        }
        *(doc->doseps) = doseps;
    }
    } else {
    return(NULL);
    }

    preread = 0;
    while (preread || readline(line, sizeof line, file, enddoseps, &position, &line_len, &line_count)) {
    if (!preread) section_len += line_len;
    preread = 0;
    if (line[0] != '%' ||
        iscomment(line+1, "%EndComments") ||
        line[1] == ' ' || line[1] == '\t' || line[1] == '\n' ||
        !isprint(line[1])) {
        break;
    } else if (line[1] != '%') {
        /* Do nothing */
    } else if (iscomment(line+2, "Begin") || iscomment(line+2, "End") ||
           iscomment(line+2, "Page:") || iscomment(line+2, "Trailer") ||
           iscomment(line+2, "EOF")) {
        break;
    } else if (doc->title == NULL && iscomment(line+2, "Title:")) {
        doc->title = gettextline(line+length("%%Title:"));
    } else if (doc->date == NULL && iscomment(line+2, "CreationDate:")) {
        doc->date = gettextline(line+length("%%CreationDate:"));
    } else if (bb_set == NONE && iscomment(line+2, "BoundingBox:")) {
        sscanf(line+length("%%BoundingBox:"), "%s", text);
        if (strcmp(text, "(atend)") == 0) {
        bb_set = ATEND;
        }
        else if (strcmp(text, "atend") == 0) { /* rjl: some bad DSC omits the () */
        char buf[64];
        sprintf(buf, "Invalid %%%%BoundingBox on line %d\n", line_count);
        pserror(buf);
        bb_set = ATEND;
        } else {
        if (sscanf(line+length("%%BoundingBox:"), "%d %d %d %d",
               &(doc->boundingbox[LLX]),
               &(doc->boundingbox[LLY]),
               &(doc->boundingbox[URX]),
               &(doc->boundingbox[URY])) == 4)
            bb_set = 1;
        else {
            float fllx, flly, furx, fury;
            if (sscanf(line+length("%%BoundingBox:"), "%f %f %f %f",
                   &fllx, &flly, &furx, &fury) == 4) {
            bb_set = 1;
            doc->boundingbox[LLX] = (int)fllx;
            doc->boundingbox[LLY] = (int)flly;
            doc->boundingbox[URX] = (int)furx;
            doc->boundingbox[URY] = (int)fury;
            if (fllx < doc->boundingbox[LLX])
                doc->boundingbox[LLX]--;
            if (flly < doc->boundingbox[LLY])
                doc->boundingbox[LLY]--;
            if (furx > doc->boundingbox[URX])
                doc->boundingbox[URX]++;
            if (fury > doc->boundingbox[URY])
                doc->boundingbox[URY]++;
            }
        }
        }
    } else if (orientation_set == NONE &&
           iscomment(line+2, "Orientation:")) {
        sscanf(line+length("%%Orientation:"), "%s", text);
        if (strcmp(text, "(atend)") == 0) {
        orientation_set = ATEND;
        }
        else if (strcmp(text, "atend") == 0) { /* rjl: some bad DSC omits the () */
        char buf[64];
        sprintf(buf, "Invalid %%%%Orientation on line %d\n", line_count);
        pserror(buf);
        orientation_set = ATEND;
        } else if (strcmp(text, "Portrait") == 0) {
        doc->orientation = PORTRAIT;
        orientation_set = 1;
        } else if (strcmp(text, "Landscape") == 0) {
        doc->orientation = LANDSCAPE;
        orientation_set = 1;
        }
    } else if (page_order_set == NONE && iscomment(line+2, "PageOrder:")) {
        sscanf(line+length("%%PageOrder:"), "%s", text);
        if (strcmp(text, "(atend)") == 0) {
        page_order_set = ATEND;
        }
        else if (strcmp(text, "atend") == 0) { /* rjl: some bad DSC omits the () */
        char buf[64];
        sprintf(buf, "Invalid %%%%PageOrder on line %d\n", line_count);
        pserror(buf);
        page_order_set = ATEND;
        } else if (strcmp(text, "Ascend") == 0) {
        doc->pageorder = ASCEND;
        page_order_set = 1;
        } else if (strcmp(text, "Descend") == 0) {
        doc->pageorder = DESCEND;
        page_order_set = 1;
        } else if (strcmp(text, "Special") == 0) {
        doc->pageorder = SPECIAL;
        page_order_set = 1;
        }
    } else if (pages_set == NONE && iscomment(line+2, "Pages:")) {
        sscanf(line+length("%%Pages:"), "%s", text);
        if (strcmp(text, "(atend)") == 0) {
        pages_set = ATEND;
        }
        else if (strcmp(text, "atend") == 0) { /* rjl: some bad DSC omits the () */
        char buf[64];
        sprintf(buf, "Invalid %%%%Pages on line %d\n", line_count);
        pserror(buf);
        pages_set = ATEND;
        } else {
        switch (sscanf(line+length("%%Pages:"), "%d %d",
                   &maxpages, &i)) {
            case 2:
            if (page_order_set == NONE) {
                if (i == -1) {
                doc->pageorder = DESCEND;
                page_order_set = 1;
                } else if (i == 0) {
                doc->pageorder = SPECIAL;
                page_order_set = 1;
                } else if (i == 1) {
                doc->pageorder = ASCEND;
                page_order_set = 1;
                }
            }
            case 1:
            if (maxpages > 0) {
                doc->pages = (struct page *) galloc(maxpages * sizeof(struct page));
                if (doc->pages == NULL) {
                pserror("Fatal Error: Dynamic memory exhausted.\n");
                exit(-1);
                }
            }
        }
        }
    } else if (doc->nummedia == NONE &&
           iscomment(line+2, "DocumentMedia:")) {
        float w, h;
        doc->media = (struct documentmedia *)
             galloc(sizeof (struct documentmedia));
        if (doc->media == NULL) {
        pserror("Fatal Error: Dynamic memory exhausted.\n");
        exit(-1);
        }
        doc->media[0].name = gettext(line+length("%%DocumentMedia:"),
                     &next_char);
        if (doc->media[0].name != NULL) {
        if (sscanf(next_char, "%f %f", &w, &h) == 2) {
            doc->media[0].width = (int)(w + 0.5);
            doc->media[0].height = (int)(h + 0.5);
        }
        if (doc->media[0].width != 0 && doc->media[0].height != 0)
            doc->nummedia = 1;
        else
            if (doc->media[0].name) gfree(doc->media[0].name);
        }
        preread=1;
        while (readline(line, sizeof line, file, enddoseps, &position, &line_len, &line_count) &&
           DSCcomment(line) && iscomment(line+2, "+")) {
        section_len += line_len;
        doc->media = (struct documentmedia *)
                 grealloc(doc->media,
                         (doc->nummedia)*sizeof (struct documentmedia),
                         (doc->nummedia+1)*sizeof (struct documentmedia));
        if (doc->media == NULL) {
            pserror("Fatal Error: Dynamic memory exhausted.\n");
            exit(-1);
        }
        doc->media[doc->nummedia].name = gettext(line+length("%%+"),
                             &next_char);
        if (doc->media[doc->nummedia].name != NULL) {
            if (sscanf(next_char, "%f %f", &w, &h) == 2) {
            doc->media[doc->nummedia].width  = (int)(w + 0.5);
            doc->media[doc->nummedia].height = (int)(h + 0.5);
            }
            if (doc->media[doc->nummedia].width != 0 &&
            doc->media[doc->nummedia].height != 0) doc->nummedia++;
            else
            if (doc->media[doc->nummedia].name) gfree(doc->media[doc->nummedia].name);
        }
        }
        section_len += line_len;
        if (doc->nummedia != 0) doc->default_page_media = doc->media;
    } else if (doc->nummedia == NONE &&
           iscomment(line+2, "DocumentPaperSizes:")) {

        doc->media = (struct documentmedia *)
             galloc(sizeof (struct documentmedia));
        if (doc->media == NULL) {
        pserror("Fatal Error: Dynamic memory exhausted.\n");
        exit(-1);
        }
        doc->media[0].name = gettext(line+length("%%DocumentPaperSizes:"),
                     &next_char);
        if (doc->media[0].name != NULL) {
        doc->media[0].width = 0;
        doc->media[0].height = 0;
        for (dmp=papersizes; dmp->name != NULL; dmp++) {
            /* Note: Paper size comment uses down cased paper size
             * name.  Case insensitive compares are only used for
             * PaperSize comments.
             */
            if (strcasecmp(doc->media[0].name, dmp->name) == 0) {
            if (doc->media[0].name) gfree(doc->media[0].name);
            doc->media[0].name =
                (char *)galloc(strlen(dmp->name)+1);
            if (doc->media[0].name == NULL) {
                pserror("Fatal Error: Dynamic memory exhausted.\n");
                exit(-1);
            }
            strcpy(doc->media[0].name, dmp->name);
            doc->media[0].width = dmp->width;
            doc->media[0].height = dmp->height;
            break;
            }
        }
        if (doc->media[0].width != 0 && doc->media[0].height != 0)
            doc->nummedia = 1;
        else
            if (doc->media[0].name) gfree(doc->media[0].name);
        }
        while ( (cp = gettext(next_char, &next_char)) != NULL ) {
        doc->media = (struct documentmedia *)
                 grealloc(doc->media,
                         (doc->nummedia)*sizeof (struct documentmedia),
                         (doc->nummedia+1)*sizeof (struct documentmedia));
        if (doc->media == NULL) {
            pserror("Fatal Error: Dynamic memory exhausted.\n");
            exit(-1);
        }
        doc->media[doc->nummedia].name = cp;
        doc->media[doc->nummedia].width = 0;
        doc->media[doc->nummedia].height = 0;
        for (dmp=papersizes; dmp->name != NULL; dmp++) {
            /* Note: Paper size comment uses down cased paper size
             * name.  Case insensitive compares are only used for
             * PaperSize comments.
             */
            if (strcasecmp(doc->media[doc->nummedia].name,
                   dmp->name) == 0) {
            if (doc->media[doc->nummedia].name) gfree(doc->media[doc->nummedia].name);
            doc->media[doc->nummedia].name =
                (char *)galloc(strlen(dmp->name)+1);
            if (doc->media[doc->nummedia].name == NULL) {
                pserror("Fatal Error: Dynamic memory exhausted.\n");
                exit(-1);
            }
            strcpy(doc->media[doc->nummedia].name, dmp->name);
            doc->media[doc->nummedia].name = dmp->name;
            doc->media[doc->nummedia].width = dmp->width;
            doc->media[doc->nummedia].height = dmp->height;
            break;
            }
        }
        if (doc->media[doc->nummedia].width != 0 &&
            doc->media[doc->nummedia].height != 0) doc->nummedia++;
        else
            if (doc->media[doc->nummedia].name) gfree(doc->media[doc->nummedia].name);
        }
        preread=1;
        while (readline(line, sizeof line, file, enddoseps, &position, &line_len, &line_count) &&
           DSCcomment(line) && iscomment(line+2, "+")) {
        section_len += line_len;
        next_char = line + length("%%+");
        while ( (cp = gettext(next_char, &next_char)) != NULL ) {
            doc->media = (struct documentmedia *)
                 grealloc(doc->media,
                         (doc->nummedia)*sizeof (struct documentmedia),
                         (doc->nummedia+1)*sizeof (struct documentmedia));
            if (doc->media == NULL) {
            pserror("Fatal Error: Dynamic memory exhausted.\n");
            exit(-1);
            }
            doc->media[doc->nummedia].name = cp;
            doc->media[doc->nummedia].width = 0;
            doc->media[doc->nummedia].height = 0;
            for (dmp=papersizes; dmp->name != NULL; dmp++) {
            /* Note: Paper size comment uses down cased paper size
             * name.  Case insensitive compares are only used for
             * PaperSize comments.
             */
            if (strcasecmp(doc->media[doc->nummedia].name,
                   dmp->name) == 0) {
                doc->media[doc->nummedia].width = dmp->width;
                doc->media[doc->nummedia].height = dmp->height;
                break;
            }
            }
            if (doc->media[doc->nummedia].width != 0 &&
            doc->media[doc->nummedia].height != 0) doc->nummedia++;
            else
            if (doc->media[doc->nummedia].name) gfree(doc->media[doc->nummedia].name);
        }
        }
        section_len += line_len;
        if (doc->nummedia != 0) doc->default_page_media = doc->media;
    }
    }

    if (DSCcomment(line) && iscomment(line+2, "EndComments")) {
    readline(line, sizeof line, file, enddoseps, &position, &line_len, &line_count);
    section_len += line_len;
    }
    doc->endheader = position;
    doc->lenheader = section_len - line_len;

    /* Optional Preview comments for encapsulated PostScript files */

    beginsection = position;
    section_len = line_len;
    while (blank(line) &&
       readline(line, sizeof line, file, enddoseps, &position, &line_len, &line_count)) {
    section_len += line_len;
    }

    if (doc->epsf && DSCcomment(line) && iscomment(line+2, "BeginPreview")) {
    doc->beginpreview = beginsection;
    beginsection = 0;
    while (readline(line, sizeof line, file, enddoseps, &position, &line_len, &line_count) &&
           !(DSCcomment(line) && iscomment(line+2, "EndPreview"))) {
        section_len += line_len;
    }
    section_len += line_len;
    readline(line, sizeof line, file, enddoseps, &position, &line_len, &line_count);
    section_len += line_len;
    doc->endpreview = position;
    doc->lenpreview = section_len - line_len;
    }

    /* Page Defaults for Version 3.0 files */

    if (beginsection == 0) {
    beginsection = position;
    section_len = line_len;
    }
    while (blank(line) &&
       readline(line, sizeof line, file, enddoseps, &position, &line_len, &line_count)) {
    section_len += line_len;
    }

    if (DSCcomment(line) && iscomment(line+2, "BeginDefaults")) {
    doc->begindefaults = beginsection;
    beginsection = 0;
    while (readline(line, sizeof line, file, enddoseps, &position, &line_len, &line_count) &&
           !(DSCcomment(line) && iscomment(line+2, "EndDefaults"))) {
        section_len += line_len;
        if (!DSCcomment(line)) {
        /* Do nothing */
        } else if (doc->default_page_orientation == NONE &&
        iscomment(line+2, "PageOrientation:")) {
        sscanf(line+length("%%PageOrientation:"), "%s", text);
        if (strcmp(text, "Portrait") == 0) {
            doc->default_page_orientation = PORTRAIT;
        } else if (strcmp(text, "Landscape") == 0) {
            doc->default_page_orientation = LANDSCAPE;
        }
        } else if (page_media_set == NONE &&
               iscomment(line+2, "PageMedia:")) {
        cp = gettext(line+length("%%PageMedia:"), NULL);
        for (dmp = doc->media, i=0; i<(int)(doc->nummedia); i++, dmp++) {
            if (strcmp(cp, dmp->name) == 0) {
            doc->default_page_media = dmp;
            page_media_set = 1;
            break;
            }
        }
        if (cp) gfree(cp);
        } else if (page_bb_set == NONE &&
               iscomment(line+2, "PageBoundingBox:")) {
        if (sscanf(line+length("%%PageBoundingBox:"), "%d %d %d %d",
               &(doc->default_page_boundingbox[LLX]),
               &(doc->default_page_boundingbox[LLY]),
               &(doc->default_page_boundingbox[URX]),
               &(doc->default_page_boundingbox[URY])) == 4)
            page_bb_set = 1;
        else {
            float fllx, flly, furx, fury;
            if (sscanf(line+length("%%PageBoundingBox:"), "%f %f %f %f",
                   &fllx, &flly, &furx, &fury) == 4) {
            page_bb_set = 1;
            doc->default_page_boundingbox[LLX] = (int)fllx;
            doc->default_page_boundingbox[LLY] = (int)flly;
            doc->default_page_boundingbox[URX] = (int)furx;
            doc->default_page_boundingbox[URY] = (int)fury;
            if (fllx < doc->default_page_boundingbox[LLX])
                doc->default_page_boundingbox[LLX]--;
            if (flly < doc->default_page_boundingbox[LLY])
                doc->default_page_boundingbox[LLY]--;
            if (furx > doc->default_page_boundingbox[URX])
                doc->default_page_boundingbox[URX]++;
            if (fury > doc->default_page_boundingbox[URY])
                doc->default_page_boundingbox[URY]++;
            }
        }
        }
    }
    section_len += line_len;
    readline(line, sizeof line, file, enddoseps, &position, &line_len, &line_count);
    section_len += line_len;
    doc->enddefaults = position;
    doc->lendefaults = section_len - line_len;
    }

    /* Document Prolog */

    if (beginsection == 0) {
    beginsection = position;
    section_len = line_len;
    }
    while (blank(line) &&
       readline(line, sizeof line, file, enddoseps, &position, &line_len, &line_count)) {
    section_len += line_len;
    }

    if (!(DSCcomment(line) &&
      (iscomment(line+2, "BeginSetup") ||
       iscomment(line+2, "Page:") ||
       iscomment(line+2, "Trailer") ||
       iscomment(line+2, "EOF")))) {
    doc->beginprolog = beginsection;
    beginsection = 0;
    preread = 1;

    while ((preread ||
        readline(line, sizeof line, file, enddoseps, &position, &line_len, &line_count)) &&
           !(DSCcomment(line) &&
             (iscomment(line+2, "EndProlog") ||
              iscomment(line+2, "BeginSetup") ||
              iscomment(line+2, "Page:") ||
              iscomment(line+2, "Trailer") ||
              iscomment(line+2, "EOF")))) {
        if (!preread) section_len += line_len;
        preread = 0;
    }
    section_len += line_len;
    if (DSCcomment(line) && iscomment(line+2, "EndProlog")) {
        readline(line, sizeof line, file, enddoseps, &position, &line_len, &line_count);
        section_len += line_len;
    }
    doc->endprolog = position;
    doc->lenprolog = section_len - line_len;
    }

    /* Document Setup,  Page Defaults found here for Version 2 files */

    if (beginsection == 0) {
    beginsection = position;
    section_len = line_len;
    }
    while (blank(line) &&
       readline(line, sizeof line, file, enddoseps, &position, &line_len, &line_count)) {
    section_len += line_len;
    }

    if (!(DSCcomment(line) &&
      (iscomment(line+2, "Page:") ||
       iscomment(line+2, "Trailer") ||
       iscomment(line+2, "EOF")))) {
    doc->beginsetup = beginsection;
    beginsection = 0;
    preread = 1;
    while ((preread ||
        readline(line, sizeof line, file, enddoseps, &position, &line_len, &line_count)) &&
           !(DSCcomment(line) &&
             (iscomment(line+2, "EndSetup") ||
              iscomment(line+2, "Page:") ||
              iscomment(line+2, "Trailer") ||
              iscomment(line+2, "EOF")))) {
        if (!preread) section_len += line_len;
        preread = 0;
        if (!DSCcomment(line)) {
        /* Do nothing */
        } else if (doc->default_page_orientation == NONE &&
        iscomment(line+2, "PageOrientation:")) {
        sscanf(line+length("%%PageOrientation:"), "%s", text);
        if (strcmp(text, "Portrait") == 0) {
            doc->default_page_orientation = PORTRAIT;
        } else if (strcmp(text, "Landscape") == 0) {
            doc->default_page_orientation = LANDSCAPE;
        }
        } else if (page_media_set == NONE &&
               iscomment(line+2, "PaperSize:")) {
        cp = gettext(line+length("%%PaperSize:"), NULL);
        for (dmp = doc->media, i=0; i<(int)(doc->nummedia); i++, dmp++) {
            /* Note: Paper size comment uses down cased paper size
             * name.  Case insensitive compares are only used for
             * PaperSize comments.
             */
            if (strcasecmp(cp, dmp->name) == 0) {
            doc->default_page_media = dmp;
            page_media_set = 1;
            break;
            }
        }
        if (cp) gfree(cp);
        } else if (page_bb_set == NONE &&
               iscomment(line+2, "PageBoundingBox:")) {
        if (sscanf(line+length("%%PageBoundingBox:"), "%d %d %d %d",
               &(doc->default_page_boundingbox[LLX]),
               &(doc->default_page_boundingbox[LLY]),
               &(doc->default_page_boundingbox[URX]),
               &(doc->default_page_boundingbox[URY])) == 4)
            page_bb_set = 1;
        else {
            float fllx, flly, furx, fury;
            if (sscanf(line+length("%%PageBoundingBox:"), "%f %f %f %f",
                   &fllx, &flly, &furx, &fury) == 4) {
            page_bb_set = 1;
            doc->default_page_boundingbox[LLX] = (int)fllx;
            doc->default_page_boundingbox[LLY] = (int)flly;
            doc->default_page_boundingbox[URX] = (int)furx;
            doc->default_page_boundingbox[URY] = (int)fury;
            if (fllx < doc->default_page_boundingbox[LLX])
                doc->default_page_boundingbox[LLX]--;
            if (flly < doc->default_page_boundingbox[LLY])
                doc->default_page_boundingbox[LLY]--;
            if (furx > doc->default_page_boundingbox[URX])
                doc->default_page_boundingbox[URX]++;
            if (fury > doc->default_page_boundingbox[URY])
                doc->default_page_boundingbox[URY]++;
            }
        }
        }
    }
    section_len += line_len;
    if (DSCcomment(line) && iscomment(line+2, "EndSetup")) {
        readline(line, sizeof line, file, enddoseps, &position, &line_len, &line_count);
        section_len += line_len;
    }
    doc->endsetup = position;
    doc->lensetup = section_len - line_len;
    }

    /* Individual Pages */

    if (beginsection == 0) {
    beginsection = position;
    section_len = line_len;
    }
    while (blank(line) &&
       readline(line, sizeof line, file, enddoseps, &position, &line_len, &line_count)) {
    section_len += line_len;
    }

newpage:
    while (DSCcomment(line) && iscomment(line+2, "Page:")) {
    if (maxpages == 0) {
        maxpages = 1;
        doc->pages = (struct page *) galloc(maxpages * sizeof(struct page));
        if (doc->pages == NULL) {
        pserror("Fatal Error: Dynamic memory exhausted.\n");
        exit(-1);
        }
    }
    label = gettext(line+length("%%Page:"), &next_char);
    if (sscanf(next_char, "%d", &thispage) != 1) thispage = 0;
    if (nextpage == 1) {
        ignore = thispage != 1;
    }
    if (!ignore && thispage != nextpage) {
        if (label) gfree(label);
        doc->numpages--;
        goto continuepage;
    }
    nextpage++;
    if (doc->numpages == maxpages) {
        maxpages++;
        doc->pages = (struct page *)
             grealloc(doc->pages, (maxpages-1)*sizeof (struct page), maxpages*sizeof (struct page));
        if (doc->pages == NULL) {
        pserror("Fatal Error: Dynamic memory exhausted.\n");
        exit(-1);
        }
    }
    memset(&(doc->pages[doc->numpages]), 0, sizeof(struct page));
    page_bb_set = NONE;
    doc->pages[doc->numpages].label = label;
    if (beginsection) {
        doc->pages[doc->numpages].begin = beginsection;
        beginsection = 0;
    } else {
        doc->pages[doc->numpages].begin = position;
        section_len = line_len;
    }
continuepage:
    while (readline(line, sizeof line, file, enddoseps, &position, &line_len, &line_count) &&
           !(DSCcomment(line) &&
             (iscomment(line+2, "Page:") ||
              iscomment(line+2, "Trailer") ||
              iscomment(line+2, "EOF")))) {
        section_len += line_len;
        if (!DSCcomment(line)) {
        /* Do nothing */
        } else if (doc->pages[doc->numpages].orientation == NONE &&
        iscomment(line+2, "PageOrientation:")) {
        sscanf(line+length("%%PageOrientation:"), "%s", text);
        if (strcmp(text, "Portrait") == 0) {
            doc->pages[doc->numpages].orientation = PORTRAIT;
        } else if (strcmp(text, "Landscape") == 0) {
            doc->pages[doc->numpages].orientation = LANDSCAPE;
        }
        } else if (doc->pages[doc->numpages].media == NULL &&
               iscomment(line+2, "PageMedia:")) {
        cp = gettext(line+length("%%PageMedia:"), NULL);
        for (dmp = doc->media, i=0; i<(int)(doc->nummedia); i++, dmp++) {
            if (strcmp(cp, dmp->name) == 0) {
            doc->pages[doc->numpages].media = dmp;
            break;
            }
        }
        if (cp) gfree(cp);
        } else if (doc->pages[doc->numpages].media == NULL &&
               iscomment(line+2, "PaperSize:")) {
        cp = gettext(line+length("%%PaperSize:"), NULL);
        for (dmp = doc->media, i=0; i<(int)(doc->nummedia); i++, dmp++) {
            /* Note: Paper size comment uses down cased paper size
             * name.  Case insensitive compares are only used for
             * PaperSize comments.
             */
            if (strcasecmp(cp, dmp->name) == 0) {
            doc->pages[doc->numpages].media = dmp;
            break;
            }
        }
        if (cp) gfree(cp);
        } else if ((page_bb_set == NONE || page_bb_set == ATEND) &&
               iscomment(line+2, "PageBoundingBox:")) {
        sscanf(line+length("%%PageBoundingBox:"), "%s", text);
        if (strcmp(text, "(atend)") == 0) {
            page_bb_set = ATEND;
        }
        else if (strcmp(text, "atend") == 0) { /* rjl: some bad DSC omits the () */
            char buf[64];
            sprintf(buf, "Invalid %%%%PageBoundingBox on line %d\n", line_count);
            pserror(buf);
            page_bb_set = ATEND;
        } else {
            if (sscanf(line+length("%%PageBoundingBox:"), "%d %d %d %d",
                &(doc->pages[doc->numpages].boundingbox[LLX]),
                &(doc->pages[doc->numpages].boundingbox[LLY]),
                &(doc->pages[doc->numpages].boundingbox[URX]),
                &(doc->pages[doc->numpages].boundingbox[URY])) == 4)
            if (page_bb_set == NONE) page_bb_set = 1;
            else {
            float fllx, flly, furx, fury;
            if (sscanf(line+length("%%PageBoundingBox:"),
                   "%f %f %f %f",
                   &fllx, &flly, &furx, &fury) == 4) {
                if (page_bb_set == NONE) page_bb_set = 1;
                doc->pages[doc->numpages].boundingbox[LLX] = (int)fllx;
                doc->pages[doc->numpages].boundingbox[LLY] = (int)flly;
                doc->pages[doc->numpages].boundingbox[URX] = (int)furx;
                doc->pages[doc->numpages].boundingbox[URY] = (int)fury;
                if (fllx <
                    doc->pages[doc->numpages].boundingbox[LLX])
                doc->pages[doc->numpages].boundingbox[LLX]--;
                if (flly <
                    doc->pages[doc->numpages].boundingbox[LLY])
                doc->pages[doc->numpages].boundingbox[LLY]--;
                if (furx >
                    doc->pages[doc->numpages].boundingbox[URX])
                doc->pages[doc->numpages].boundingbox[URX]++;
                if (fury >
                    doc->pages[doc->numpages].boundingbox[URY])
                doc->pages[doc->numpages].boundingbox[URY]++;
            }
            }
        }
        }
    }
    section_len += line_len;
    doc->pages[doc->numpages].end = position;
    doc->pages[doc->numpages].len = section_len - line_len;
    doc->numpages++;
    }

    /* Document Trailer */

    if (beginsection) {
    doc->begintrailer = beginsection;
    beginsection = 0;
    } else {
    doc->begintrailer = position;
    section_len = line_len;
    }

    preread = 1;
    while ((preread ||
        readline(line, sizeof line, file, enddoseps, &position, &line_len, &line_count)) &&
       !(DSCcomment(line) && iscomment(line+2, "EOF"))) {
    if (!preread) section_len += line_len;
    preread = 0;
    if (!DSCcomment(line)) {
        /* Do nothing */
    } else if (iscomment(line+2, "Page:")) {
        gfree(gettext(line+length("%%Page:"), &next_char));
        if (sscanf(next_char, "%d", &thispage) != 1) thispage = 0;
        if (!ignore && thispage == nextpage) {
        if (doc->numpages > 0) {
            doc->pages[doc->numpages-1].end = position;
            doc->pages[doc->numpages-1].len += section_len - line_len;
        } else {
            if (doc->endsetup) {
            doc->endsetup = position;
/* following line removed rjl 1995-04-01
            doc->endsetup += section_len - line_len;
*/
            } else if (doc->endprolog) {
            doc->endprolog = position;
/* following line removed rjl 1995-04-01
            doc->endprolog += section_len - line_len;
*/
            }
        }
        goto newpage;
        }
    } else if (bb_set == ATEND && iscomment(line+2, "BoundingBox:")) {
        if (sscanf(line+length("%%BoundingBox:"), "%d %d %d %d",
               &(doc->boundingbox[LLX]),
               &(doc->boundingbox[LLY]),
               &(doc->boundingbox[URX]),
               &(doc->boundingbox[URY])) != 4) {
        float fllx, flly, furx, fury;
        if (sscanf(line+length("%%BoundingBox:"), "%f %f %f %f",
               &fllx, &flly, &furx, &fury) == 4) {
            doc->boundingbox[LLX] = (int)fllx;
            doc->boundingbox[LLY] = (int)flly;
            doc->boundingbox[URX] = (int)furx;
            doc->boundingbox[URY] = (int)fury;
            if (fllx < doc->boundingbox[LLX])
            doc->boundingbox[LLX]--;
            if (flly < doc->boundingbox[LLY])
            doc->boundingbox[LLY]--;
            if (furx > doc->boundingbox[URX])
            doc->boundingbox[URX]++;
            if (fury > doc->boundingbox[URY])
            doc->boundingbox[URY]++;
        }
        }
    } else if (orientation_set == ATEND &&
           iscomment(line+2, "Orientation:")) {
        sscanf(line+length("%%Orientation:"), "%s", text);
        if (strcmp(text, "Portrait") == 0) {
        doc->orientation = PORTRAIT;
        } else if (strcmp(text, "Landscape") == 0) {
        doc->orientation = LANDSCAPE;
        }
    } else if (page_order_set == ATEND && iscomment(line+2, "PageOrder:")) {
        sscanf(line+length("%%PageOrder:"), "%s", text);
        if (strcmp(text, "Ascend") == 0) {
        doc->pageorder = ASCEND;
        } else if (strcmp(text, "Descend") == 0) {
        doc->pageorder = DESCEND;
        } else if (strcmp(text, "Special") == 0) {
        doc->pageorder = SPECIAL;
        }
    } else if (pages_set == ATEND && iscomment(line+2, "Pages:")) {
        if (sscanf(line+length("%%Pages:"), "%*u %d", &i) == 1) {
        if (page_order_set == NONE) {
            if (i == -1) doc->pageorder = DESCEND;
            else if (i == 0) doc->pageorder = SPECIAL;
            else if (i == 1) doc->pageorder = ASCEND;
        }
        }
    }
    }
    section_len += line_len;
    if (DSCcomment(line) && iscomment(line+2, "EOF")) {
    readline(line, sizeof line, file, enddoseps, &position, &line_len, &line_count);
    section_len += line_len;
    }
    doc->endtrailer = position;
    doc->lentrailer = section_len - line_len;

#if 0
    section_len = line_len;
    preread = 1;
    while (preread ||
       readline(line, sizeof line, file, enddoseps, &position, &line_len, &line_count)) {
    if (!preread) section_len += line_len;
    preread = 0;
    if (DSCcomment(line) && iscomment(line+2, "Page:")) {
        gfree(gettext(line+length("%%Page:"), &next_char));
        if (sscanf(next_char, "%d", &thispage) != 1) thispage = 0;
        if (!ignore && thispage == nextpage) {
        if (doc->numpages > 0) {
            doc->pages[doc->numpages-1].end = position;
            doc->pages[doc->numpages-1].len += doc->lentrailer +
                               section_len - line_len;
        } else {
            if (doc->endsetup) {
            doc->endsetup = position;
            doc->endsetup += doc->lentrailer +
                     section_len - line_len;
            } else if (doc->endprolog) {
            doc->endprolog = position;
            doc->endprolog += doc->lentrailer +
                      section_len - line_len;
            }
        }
        goto newpage;
        }
    }
    }
#endif
    return doc;
}

/*
 *  psgfree -- gfree dynamic storage associated with document structure.
 */

void psgfree(doc)
    struct document *doc;
{
    int i;

    if (doc)
    {
        for (i=0; i<(int)(doc->numpages); i++)
        {
            if (doc->pages[i].label) gfree(doc->pages[i].label);
        }
        for (i=0; i<(int)(doc->nummedia); i++)
        {
            if (doc->media[i].name) gfree(doc->media[i].name);
        }
        if (doc->title) gfree(doc->title);
        if (doc->date) gfree(doc->date);
        if (doc->pages) gfree(doc->pages);
        if (doc->media) gfree(doc->media);
        if (doc->doseps) gfree(doc->doseps); /* rjl: */
        gfree(doc);
    }
}

/*
 * gettextine -- skip over white space and return the rest of the line.
 *               If the text begins with '(' return the text string
 *       using gettext().
 */

static char *
gettextline(line)
    char *line;
{
    char *cp;

    while (*line && (*line == ' ' || *line == '\t')) line++;
    if (*line == '(') {
    return gettext(line, NULL);
    } else {
    if (strlen(line) == 0) return NULL;
    cp = (char *) galloc(strlen(line));
    if (cp == NULL) {
        pserror("Fatal Error: Dynamic memory exhausted.\n");
        exit(-1);
    }
    strncpy(cp, line, strlen(line)-1);
    cp[strlen(line)-1] = '\0';
    return cp;
    }
}

/*
 *  gettext -- return the next text string on the line.
 *         return NULL if nothing is present.
 */

static char *
gettext(line, next_char)
    char *line;
    char **next_char;
{
    char text[PSLINELENGTH];    /* Temporary storage for text */
    char *cp;
    int quoted=0;

    while (*line && (*line == ' ' || *line == '\t')) line++;
    cp = text;
    if (*line == '(') {
    int level = 0;
    quoted=1;
    line++;
    while (*line && !(*line == ')' && level == 0 )) {
        if (*line == '\\') {
        if (*(line+1) == 'n') {
            *cp++ = '\n';
            line += 2;
        } else if (*(line+1) == 'r') {
            *cp++ = '\r';
            line += 2;
        } else if (*(line+1) == 't') {
            *cp++ = '\t';
            line += 2;
        } else if (*(line+1) == 'b') {
            *cp++ = '\b';
            line += 2;
        } else if (*(line+1) == 'f') {
            *cp++ = '\f';
            line += 2;
        } else if (*(line+1) == '\\') {
            *cp++ = '\\';
            line += 2;
        } else if (*(line+1) == '(') {
            *cp++ = '(';
            line += 2;
        } else if (*(line+1) == ')') {
            *cp++ = ')';
            line += 2;
        } else if (*(line+1) >= '0' && *(line+1) <= '9') {
            if (*(line+2) >= '0' && *(line+2) <= '9') {
            if (*(line+3) >= '0' && *(line+3) <= '9') {
                *cp++ = (unsigned char)(((*(line+1) - '0')*8 + *(line+2) - '0')*8 +
                    *(line+3) - '0');
                line += 4;
            } else {
                *cp++ = (unsigned char)((*(line+1) - '0')*8 + *(line+2) - '0');
                line += 3;
            }
            } else {
            *cp++ = (unsigned char)(*(line+1) - '0');
            line += 2;
            }
        } else {
            line++;
            *cp++ = *line++;
        }
        } else if (*line == '(') {
        level++;
        *cp++ = *line++;
        } else if (*line == ')') {
        level--;
        *cp++ = *line++;
        } else {
        *cp++ = *line++;
        }
    }
    if (*line == ')')   /* rjl 1996-08-05 */
        line++;     /* rjl 1996-08-05 */
    } else {
    while (*line && !(*line == ' ' || *line == '\t' || *line == '\n'))
        *cp++ = *line++;
    }
    *cp = '\0';
    if (next_char) *next_char = line;
    if (!quoted && strlen(text) == 0) return NULL;
    cp = (char *) galloc(strlen(text)+1);
    if (cp == NULL) {
    pserror("Fatal Error: Dynamic memory exhausted.\n");
    exit(-1);
    }
    strcpy(cp, text);
    return cp;
}

/*
 *  readline -- Read the next line in the postscript file.
 *                  Automatically skip over data (as indicated by
 *                  %%BeginBinary/%%EndBinary or %%BeginData/%%EndData
 *          comments.)
 *          Also, skip over included documents (as indicated by
 *          %%BeginDocument/%%EndDocument comments.)
 */

static char *
readline(line, size, fp, enddoseps, position, line_len, line_count)
    char *line;
    int size;
    FILE *fp;
    unsigned int enddoseps;
    int *position;
    unsigned int *line_len;
    unsigned int *line_count;
{
    char text[PSLINELENGTH];    /* Temporary storage for text */
    char save[PSLINELENGTH];    /* Temporary storage for text */
    char *cp;
    unsigned int num;
    unsigned int nbytes;
    int i;
    char buf[BUFSIZ];

    if (position) *position = ftell(fp);
    if (position && enddoseps) {
    if (*position >= (int)enddoseps)
        return NULL;    /* don't read any more, we have reached end of dos eps section */
    if (size > (int)enddoseps - *position + 1)
        size = (int)(enddoseps - *position + 1);
    }
    cp = fgets(line, size, fp);
    *line_count += 1;
    if (cp == NULL) line[0] = '\0';
    *line_len = strlen(line);
#if defined(__TURBOC__) || defined(OS2)
    /* remove MS-DOS carriage-return */
    if ((i = *line_len) >= 2) {
    if ((line[i-2] == '\r') && (line[i-1] == '\n')) {
        line[i-2] = '\n';
        line[i-1] = '\0';
    }
    }
#endif
    if (!(DSCcomment(line) && iscomment(line+2, "Begin"))) {
        /* rjl: skip over BUGGY EPS includes from Microsoft Word */
        /* Microsoft's EPSIMP.FLT needs to be fixed */
    if ( (line[0] == '%') && iscomment(line+1, "MSEPS Preamble") ) {
        strcpy(save, line);
        while (readline(line, size, fp, enddoseps, NULL, &nbytes, line_count) &&
           !(line[0] == '%' && iscomment(line+1, "MSEPS Trailer"))) {
        *line_len += nbytes;
        }
        *line_len += nbytes;
        strcpy(line, save);
    }
    else {
        /* Do nothing */
    }
    } else if (iscomment(line+7, "Document:")) {
    strcpy(save, line+7);
    while (readline(line, size, fp, enddoseps, NULL, &nbytes, line_count) &&
           !(DSCcomment(line) && iscomment(line+2, "EndDocument"))) {
        *line_len += nbytes;
    }
    *line_len += nbytes;
    strcpy(line, save);
    } else if (iscomment(line+7, "Feature:")) {
    strcpy(save, line+7);
    while (readline(line, size, fp, enddoseps, NULL, &nbytes, line_count) &&
           !(DSCcomment(line) && iscomment(line+2, "EndFeature"))) {
        *line_len += nbytes;
    }
    *line_len += nbytes;
    strcpy(line, save);
    } else if (iscomment(line+7, "File:")) {
    strcpy(save, line+7);
    while (readline(line, size, fp, enddoseps, NULL, &nbytes, line_count) &&
           !(DSCcomment(line) && iscomment(line+2, "EndFile"))) {
        *line_len += nbytes;
    }
    *line_len += nbytes;
    strcpy(line, save);
    } else if (iscomment(line+7, "Font:")) {
    strcpy(save, line+7);
    while (readline(line, size, fp, enddoseps, NULL, &nbytes, line_count) &&
           !(DSCcomment(line) && iscomment(line+2, "EndFont"))) {
        *line_len += nbytes;
    }
    *line_len += nbytes;
    strcpy(line, save);
    } else if (iscomment(line+7, "ProcSet:")) {
    strcpy(save, line+7);
    while (readline(line, size, fp, enddoseps, NULL, &nbytes, line_count) &&
           !(DSCcomment(line) && iscomment(line+2, "EndProcSet"))) {
        *line_len += nbytes;
    }
    *line_len += nbytes;
    strcpy(line, save);
    } else if (iscomment(line+7, "Resource:")) {
    strcpy(save, line+7);
    while (readline(line, size, fp, enddoseps, NULL, &nbytes, line_count) &&
           !(DSCcomment(line) && iscomment(line+2, "EndResource"))) {
        *line_len += nbytes;
    }
    *line_len += nbytes;
    strcpy(line, save);
    } else if (iscomment(line+7, "Data:")) {
    text[0] = '\0';
    strcpy(save, line+7);
    if (sscanf(line+length("%%BeginData:"), "%d %*s %s", &num, text) >= 1) {
        if (strcmp(text, "Lines") == 0) {
        for (i=0; i < (int)num; i++) {
          *line_count += 1;
          do { /* rjl: handle antisocial PostScript with excessively int lines */
            cp = fgets(line, size, fp);
            *line_len += cp ? strlen(line) : 0;
          } while ( ((int)strlen(line) == size-1) && (line[size-2] != '\n') );
        }
        } else {
        while (num > BUFSIZ) {
            fread(buf, sizeof (char), BUFSIZ, fp);
            *line_len += BUFSIZ;
            num -= BUFSIZ;
        }
        fread(buf, sizeof (char), num, fp);
        *line_len += num;
        }
    }
    while (readline(line, size, fp, enddoseps, NULL, &nbytes, line_count) &&
           !(DSCcomment(line) && iscomment(line+2, "EndData"))) {
        *line_len += nbytes;
    }
    *line_len += nbytes;
    strcpy(line, save);
    } else if (iscomment(line+7, "Binary:")) {
    strcpy(save, line+7);
    if(sscanf(line+length("%%BeginBinary:"), "%d", &num) == 1) {
        while (num > BUFSIZ) {
        fread(buf, sizeof (char), BUFSIZ, fp);
        *line_len += BUFSIZ;
        num -= BUFSIZ;
        }
        fread(buf, sizeof (char), num, fp);
        *line_len += num;
    }
    while (readline(line, size, fp, enddoseps, NULL, &nbytes, line_count) &&
           !(DSCcomment(line) && iscomment(line+2, "EndBinary"))) {
        *line_len += nbytes;
    }
    *line_len += nbytes;
    strcpy(line, save);
    }
    return cp;
}

/*
 *  pscopy -- copy lines of Postscript from a section of one file
 *        to another file.
 *                Automatically switch to binary copying whenever
 *                %%BeginBinary/%%EndBinary or %%BeginData/%%EndData
 *        comments are encountered.
 */

void
pscopy(from, to, begin, end)
    FILE *from;
    FILE *to;
    int begin;         /* set negative to avoid initial seek */
    int end;
{
    char line[PSLINELENGTH];    /* 255 characters + 1 newline + 1 NULL */
    char text[PSLINELENGTH];    /* Temporary storage for text */
    unsigned int num;
    int i;
    char buf[BUFSIZ];

    if (begin >= 0) fseek(from, begin, SEEK_SET);
    while (ftell(from) < end) {

    fgets(line, sizeof line, from);
    fputs(line, to);

    if (!(DSCcomment(line) && iscomment(line+2, "Begin"))) {
        /* Do nothing */
    } else if (iscomment(line+7, "Data:")) {
        text[0] = '\0';
        if (sscanf(line+length("%%BeginData:"),
               "%d %*s %s", &num, text) >= 1) {
        if (strcmp(text, "Lines") == 0) {
            for (i=0; i < (int)num; i++) {
              do { /* rjl: handle antisocial PostScript with excessively int lines */
            fgets(line, sizeof line, from);
            fputs(line, to);
              } while ( (strlen(line) == sizeof(line)-1) && (line[sizeof(line)-2] != '\n') );
            }
        } else {
            while (num > BUFSIZ) {
            fread(buf, sizeof (char), BUFSIZ, from);
            fwrite(buf, sizeof (char), BUFSIZ, to);
            num -= BUFSIZ;
            }
            fread(buf, sizeof (char), num, from);
            fwrite(buf, sizeof (char), num, to);
        }
        }
    } else if (iscomment(line+7, "Binary:")) {
        if(sscanf(line+length("%%BeginBinary:"), "%d", &num) == 1) {
        while (num > BUFSIZ) {
            fread(buf, sizeof (char), BUFSIZ, from);
            fwrite(buf, sizeof (char), BUFSIZ, to);
            num -= BUFSIZ;
        }
        fread(buf, sizeof (char), num, from);
        fwrite(buf, sizeof (char), num, to);
        }
    }
    }
}

/*
 *  pscopyuntil -- copy lines of Postscript from a section of one file
 *             to another file until a particular comment is reached.
 *                     Automatically switch to binary copying whenever
 *                     %%BeginBinary/%%EndBinary or %%BeginData/%%EndData
 *             comments are encountered.
 */

char *
pscopyuntil(from, to, begin, end, comment)
    FILE *from;
    FILE *to;
    int begin;         /* set negative to avoid initial seek */
    int end;
#if NeedFunctionPrototypes
    const
#endif
    char *comment;
{
    char line[PSLINELENGTH];    /* 255 characters + 1 newline + 1 NULL */
    char text[PSLINELENGTH];    /* Temporary storage for text */
    unsigned int num;
    int comment_length = 0;
    int i;
    char buf[BUFSIZ];
    char *cp;

    if (comment)    /* rjl: allow NULL comment to degenerate to pscopy() */
        comment_length = strlen(comment);
    if (begin >= 0) fseek(from, begin, SEEK_SET);
    while (ftell(from) < end) {

    fgets(line, sizeof line, from);

    /* iscomment cannot be used here,
     * because comment_length is not known at compile time. */
    if (comment && (strncmp(line, comment, comment_length) == 0)) {
        cp = (char *) galloc(strlen(line)+1);
        if (cp == NULL) {
        pserror("Fatal Error: Dynamic memory exhausted.\n");
        exit(-1);
        }
        strcpy(cp, line);
        return cp;
    }
    fputs(line, to);
    if (!(DSCcomment(line) && iscomment(line+2, "Begin"))) {
        /* Do nothing */
    } else if (iscomment(line+7, "Data:")) {
        text[0] = '\0';
        if (sscanf(line+length("%%BeginData:"),
               "%d %*s %s", &num, text) >= 1) {
        if (strcmp(text, "Lines") == 0) {
            for (i=0; i < (int)num; i++) {
              do { /* rjl: handle antisocial PostScript with excessively int lines */
            fgets(line, sizeof line, from);
            fputs(line, to);
              } while ( (strlen(line) == sizeof(line)-1) && (line[sizeof(line)-2] != '\n') );
            }
        } else {
            while (num > BUFSIZ) {
            fread(buf, sizeof (char), BUFSIZ, from);
            fwrite(buf, sizeof (char), BUFSIZ, to);
            num -= BUFSIZ;
            }
            fread(buf, sizeof (char), num, from);
            fwrite(buf, sizeof (char), num, to);
        }
        }
    } else if (iscomment(line+7, "Binary:")) {
        if(sscanf(line+length("%%BeginBinary:"), "%d", &num) == 1) {
        while (num > BUFSIZ) {
            fread(buf, sizeof (char), BUFSIZ, from);
            fwrite(buf, sizeof (char), BUFSIZ, to);
            num -= BUFSIZ;
        }
        fread(buf, sizeof (char), num, from);
        fwrite(buf, sizeof (char), num, to);
        }
    }
    }
    return NULL;
}

/*
 *  blank -- determine whether the line contains nothing but whitespace.
 */

static int
blank(line)
    char *line;
{
    char *cp = line;

    while (*cp == ' ' || *cp == '\t') cp++;
    return *cp == '\n' || (*cp == '%' && (line[0] != '%' || line[1] != '%'));
}

/* rjl: routines to handle reading DOS EPS files */
static unsigned int dsc_arch = 0x00000001;

/* change byte order if architecture is big-endian */
PS_DWORD
reorder_dword(val)
    PS_DWORD val;
{
    if (*((char *)(&dsc_arch)))
        return val; /* little endian machine */
    else
    return ((val&0xff) << 24) | ((val&0xff00) << 8)
             | ((val&0xff0000L) >> 8) | ((val>>24)&0xff);
}

/* change byte order if architecture is big-endian */
PS_WORD
reorder_word(val)
    PS_WORD val;
{
    if (*((char *)(&dsc_arch)))
        return val; /* little endian machine */
    else
    return (PS_WORD) ((PS_WORD)(val&0xff) << 8) | (PS_WORD)((val&0xff00) >> 8);
}

/* DOS EPS header reading */
unsigned int
ps_read_doseps(file, doseps)
    FILE *file;
    DOSEPS *doseps;
{
    fread(doseps->id, 1, 4, file);
    if (! ((doseps->id[0]==0xc5) && (doseps->id[1]==0xd0)
        && (doseps->id[2]==0xd3) && (doseps->id[3]==0xc6)) ) {
        /* id is "EPSF" with bit 7 set */
        rewind(file);
        return 0;   /* OK */
    }
    fread(&doseps->ps_begin,    4, 1, file);    /* PS offset */
    doseps->ps_begin = (unsigned int)reorder_dword(doseps->ps_begin);
    fread(&doseps->ps_length,   4, 1, file);    /* PS length */
    doseps->ps_length = (unsigned int)reorder_dword(doseps->ps_length);
    fread(&doseps->mf_begin,    4, 1, file);    /* Metafile offset */
    doseps->mf_begin = (unsigned int)reorder_dword(doseps->mf_begin);
    fread(&doseps->mf_length,   4, 1, file);    /* Metafile length */
    doseps->mf_length = (unsigned int)reorder_dword(doseps->mf_length);
    fread(&doseps->tiff_begin,  4, 1, file);    /* TIFF offset */
    doseps->tiff_begin = (unsigned int)reorder_dword(doseps->tiff_begin);
    fread(&doseps->tiff_length, 4, 1, file);    /* TIFF length */
    doseps->tiff_length = (unsigned int)reorder_dword(doseps->tiff_length);
    fread(&doseps->checksum,    2, 1, file);
        doseps->checksum = (unsigned short)reorder_word(doseps->checksum);
    fseek(file, doseps->ps_begin, SEEK_SET);    /* seek to PS section */
    return doseps->ps_begin + doseps->ps_length;
}


#undef fgets
char *
psfgets(s, n, stream)
    char *s;
    int n;
    FILE *stream;
{
/*
    return fgets(s, n, stream);
*/
    int ch = 0;
    char *p;
    p = s;
    while ( (--n > 0)  && ((ch = fgetc(stream)) != EOF) ) {
    *p++ = (char)ch;
    if (ch == '\n')
        break;
    if (ch == '\r') {
        /* cope with MS-DOS \r\n or Mac \r */
        if (--n > 0) {
            ch = fgetc(stream);
            if (ch == EOF)
            break;
            if (ch == '\n')
                *p++ = (char)ch;
        else
            ungetc(ch, stream);
        break;
        }
    }
    }
    if ((ch == EOF) && (p == s))
    return NULL;
    *p = '\0';
    return (ferror(stream)) ? NULL : s;
}


/****************************************************************/
/*  Local Functions                                             */
/****************************************************************/


#if defined(_MSC_VER)

#undef MSDOS
#if !defined(WIN32_LEAN_AND_MEAN)
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#define SetWindowOrg(hdc, x, y) SetWindowOrgEx(hdc, x, y, (LPPOINT)NULL)
#define SetWindowExt(hdc, x, y) SetWindowExtEx(hdc, x, y, (LPSIZE)NULL)
#define GSCOMMAND "gsdll32"
#define GVFAR
#define GVHUGE
#define BYTE unsigned char
#define WORD  unsigned short
#define UINT  unsigned int
#define DWORD unsigned int
#define LONG  int
#define BOOL  int
#define MAXSTR 256
#define READBIN  "rb"
#define WRITEBIN "wb"
#define DIRSEP '\\'
#define EOLSTR "\r\n"
#include <io.h>
#include <fcntl.h>

#else /* SGI specific */

#define GSCOMMAND "gs"
#define GVFAR
#define GVHUGE
#define BYTE unsigned char
#define WORD  unsigned short
#define UINT  unsigned int
#define DWORD unsigned int
#define LONG  int
#define BOOL  int
#define MAXSTR 256
#define READBIN  "r"
#define WRITEBIN "w"
#define DIRSEP '/'
#define EOLSTR "\n"
#define stricmp(s1, s2) strcasecmp(s1, s2)
#ifndef SEEK_SET
#define SEEK_SET 0
#define SEEK_END 2
#endif

#endif

#define TRUE 1
#define FALSE 0

static char gsname[MAXSTR] = GSCOMMAND;

static void pserror(char *str);
static void psfile_extract_header(PSDOC *doc, FILE *infile, FILE *f);
static void psfile_extract_layer(AIINFO *ai, FILE *infile, FILE *f, int layer);
static AIINFO *openfile(FILE *file);
static int GCALL closefile(AIINFO *ai);
static AIINFO *aiscan(FILE *file);


/*
 * Required by ps.c
 */
static void pserror(char *str)
{
    fputs(str, stderr);
}



/*
 *     TAKEN from ps.c and modified to only find %AI5_NumLayers and %AI5_BeginLayer
 *
 *      Removed all references to 'doc'
 */

/*
 *  psscan -- scan the PostScript file for document structuring comments.
 *
 *      ...
 *
 *      The following comments are examined:
 *
 *      Header section:
 *      Must start with %!PS-Adobe.  Version numbers ignored.
 *
 *      %!PS-Adobe* [EPSF-*]
 *      ...
 *      %AI5_NumLayers: <int>
 *      ...
 *      %%EndComments
 *
 *      The header section ends either explicitly with %%EndComments or
 *      implicitly with any line that does not begin with %X where X is
 *      a not whitespace character.
 *
 *      ...
 *
 *      Each layer starts explicitly with %AI5_BeginLayer and ends implicitly with
 *      %AI5_EndLayer.  The following comments are recognized:
 *
 *      %AI5_BeginLayer
 *      (Layer <int>)
 *      %AI5_EndLayer--
 *
 *      The tralier section start explicitly with %%Trailer and end with %%EOF.
 *      %%Trailer
 *      %%EOF
 *
 */

static AIINFO *aiscan(FILE *file)
{
    AIINFO *ai;
    int preread;        /* flag which tells the readline isn't needed */
    char line[PSLINELENGTH];    /* 255 characters + 1 newline + 1 NULL */
    char text[PSLINELENGTH];    /* Temporary storage for text */
    int position;      /* Position of the current line */
    int beginsection;      /* Position of the beginning of the section */
    unsigned int line_count;    /* line count for error messages */
    unsigned int line_len;  /* Length of the current line */
    unsigned int section_len;   /* Place to accumulate the section length */
    unsigned int enddoseps = 0;

    line_count = 0;
    rewind(file);
    if(isPDF(file))
        seekToAdobeTag(file);
    else
      rewind(file);
    if (readline(line, sizeof line, file, enddoseps, &position, &line_len, &line_count) == NULL)
    {
        pserror("Warning: empty file.\n");
        return(NULL);
    }

    /* Header comments */

    if (iscomment(line,"%!PS-Adobe")|| iscomment(line,"%PDF-"))
    {  /* rjl: some almost DSC code doesn't include the hyphen or version number */
        ai = (AIINFO *) galloc(sizeof(AIINFO));
        if (ai == NULL)
        {
            pserror("Fatal Error: Dynamic memory exhausted.\n");
            exit(-1);
        }
        memset(ai, 0, sizeof(AIINFO));
        sscanf(line, "%*s %s", text);

        section_len = line_len;

    } else {
        printf("didn't like line: %s\n",line);
        return(NULL);
    }
    preread = 0;
    while (preread || readline(line, sizeof line, file, enddoseps, &position, &line_len, &line_count))
    {
        if (!preread) section_len += line_len;
        preread = 0;
        if (line[0] != '%' ||
            iscomment(line+1, "%EndComments") ||
            line[1] == ' ' || line[1] == '\t' || line[1] == '\n' ||
            !isprint(line[1]))
        {
            break;
        }
        else if (line[1] != '%')
        {
            if (ai->numlayers == 0 && iscomment(line+1, "AI5_NumLayers:"))
            {
                if (sscanf(line+length("%AI5_NumLayers:"), "%d", &(ai->numlayers)) != 1)
                    ai->numlayers = -1;
            }
        }
        else if (iscomment(line+2, "Begin") || iscomment(line+2, "End") ||
                    iscomment(line+2, "Page:") || iscomment(line+2, "Trailer") ||
                    iscomment(line+2, "EOF"))
        {
            break;
        }
    }

    if (DSCcomment(line) && iscomment(line+2, "EndComments"))
    {
        readline(line, sizeof line, file, enddoseps, &position, &line_len, &line_count);
        section_len += line_len;
    }

    /* Optional Preview comments for encapsulated PostScript files */

    beginsection = position;
    section_len = line_len;
    while (blank(line) &&
       readline(line, sizeof line, file, enddoseps, &position, &line_len, &line_count)) {
    section_len += line_len;
    }

    if (DSCcomment(line) && iscomment(line+2, "BeginPreview"))
    {
        beginsection = 0;
        while (readline(line, sizeof line, file, enddoseps, &position, &line_len, &line_count) &&
               !(DSCcomment(line) && iscomment(line+2, "EndPreview")))
        {
            section_len += line_len;
        }
        section_len += line_len;
        readline(line, sizeof line, file, enddoseps, &position, &line_len, &line_count);
        section_len += line_len;
    }

    /* Page Defaults for Version 3.0 files */

    if (beginsection == 0)
    {
        beginsection = position;
        section_len = line_len;
    }

    while (blank(line) &&
            readline(line, sizeof line, file, enddoseps, &position, &line_len, &line_count))
    {
        section_len += line_len;
    }

    if (DSCcomment(line) && iscomment(line+2, "BeginDefaults"))
    {
        beginsection = 0;
        while (readline(line, sizeof line, file, enddoseps, &position, &line_len, &line_count) &&
               !(DSCcomment(line) && iscomment(line+2, "EndDefaults")))
        {
            section_len += line_len;
            if (!DSCcomment(line))
            {
                /* Do nothing */
            }
        }
        section_len += line_len;
        readline(line, sizeof line, file, enddoseps, &position, &line_len, &line_count);
        section_len += line_len;
    }

    /* Document Prolog */

    if (beginsection == 0)
    {
        beginsection = position;
        section_len = line_len;
    }

    while (blank(line) &&
            readline(line, sizeof line, file, enddoseps, &position, &line_len, &line_count))
    {
       section_len += line_len;
    }

    if (!(DSCcomment(line) &&
      (iscomment(line+2, "BeginSetup") ||
       iscomment(line+2, "Page:") ||
       iscomment(line+2, "Trailer") ||
       iscomment(line+2, "EOF"))))
    {
        beginsection = 0;
        preread = 1;

        while ((preread ||
                readline(line, sizeof line, file, enddoseps, &position, &line_len, &line_count)) &&
               !(DSCcomment(line) &&
                 (iscomment(line+2, "EndProlog") ||
                  iscomment(line+2, "BeginSetup") ||
                  iscomment(line+2, "Page:") ||
                  iscomment(line+2, "Trailer") ||
                  iscomment(line+2, "EOF"))))
        {
            if (!preread) section_len += line_len;
                preread = 0;
        }

        section_len += line_len;

        if (DSCcomment(line) && iscomment(line+2, "EndProlog"))
        {
            readline(line, sizeof line, file, enddoseps, &position, &line_len, &line_count);
            section_len += line_len;
        }
    }

    /* Document Setup,  Page Defaults found here for Version 2 files */

    if (beginsection == 0)
    {
        beginsection = position;
        section_len = line_len;
    }

    while (blank(line) &&
       readline(line, sizeof line, file, enddoseps, &position, &line_len, &line_count))
    {
        section_len += line_len;
    }

    if (!(DSCcomment(line) &&
         (iscomment(line+2, "Page:") ||
          iscomment(line+1, "AI5_BeginLayer:") ||
          iscomment(line+2, "Trailer") ||
          iscomment(line+2, "EOF"))))
    {
        beginsection = 0;
        preread = 1;
        while ((preread ||
            readline(line, sizeof line, file, enddoseps, &position, &line_len, &line_count)) &&
               !(DSCcomment(line) &&
                 (iscomment(line+2, "EndSetup") ||
                  iscomment(line+2, "Page:") ||
                  iscomment(line+2, "Trailer") ||
                  iscomment(line+2, "EOF"))))
        {
            if (!preread) section_len += line_len;
            preread = 0;
            if (!DSCcomment(line))
            {
                /* Do nothing */
            }
        }

        section_len += line_len;
        if (DSCcomment(line) && iscomment(line+2, "EndSetup"))
        {
            readline(line, sizeof line, file, enddoseps, &position, &line_len, &line_count);
            section_len += line_len;
        }
    }

    /* Individual Pages */

    if (beginsection == 0)
    {
        beginsection = position;
        section_len = line_len;
    }

    /*
     * Parse Each layer
     */
    if (ai->numlayers > 0)
    {
        ai->layerdata = (AILAYER*) galloc(ai->numlayers*sizeof(AILAYER));
        if (ai->layerdata)
        {
            int layer=0;

            memset(ai->layerdata, 0, ai->numlayers*sizeof(AILAYER));
            while (iscomment(line+1, "AI5_BeginLayer"))
            {
                int layerlinecount;

                layerlinecount = 0;

                if (layer >= ai->numlayers)
                {
                    AILAYER *temp;

                    temp = ai->layerdata;
                    ai->layerdata = (AILAYER*)galloc(layer*sizeof(AILAYER));
                    memcpy(ai->layerdata, temp, ai->numlayers*sizeof(AILAYER));
                    gfree(temp);
                }

                if (beginsection)
                {
                    ai->layerdata[layer].begin = beginsection;
                    beginsection = 0;
                }
                else
                {
                    ai->layerdata[layer].begin = position;
                    section_len = line_len;
                }

                /*
                 * Read through the layer looking for (Layer %d)
                 */
                while (readline(line, sizeof line, file, enddoseps, &position, &line_len, &line_count) &&
                       !(iscomment(line+1, "AI5_EndLayer")))
                {
                    section_len += line_len;
                    layerlinecount++;

                    if ( (layerlinecount == 2) && strstr(line, "Ln") )
                    {
                        char *beginname;
                        char *endname;
                        int namelen;

                        beginname = strchr(line, '(') + 1;
                        endname = strchr(line, ')');
                        namelen = (int)(endname - beginname);
                        if (endname && beginname && (namelen > 0))
                        {
                            namelen = namelen>NAMELEN?NAMELEN:namelen;
                            memcpy(ai->layerdata[layer].name, beginname, namelen);
                            ai->layerdata[layer].name[NAMELEN] = '\0';
                        }

                        ai->layerdata[layer].layer = layer;
                    }
                }
                /* Advance past the "EndLayer" */
                readline(line, sizeof line, file, enddoseps, &position, &line_len, &line_count);
                section_len += line_len;

                ai->layerdata[layer].end = position;
                /*printf("layer %d\n", ai->layerdata[layer].layer); */
                /*printf("name  %s\n", ai->layerdata[layer].name); */
                /*printf("begin %d\n", ai->layerdata[layer].begin); */
                /*printf("end   %d\n", ai->layerdata[layer].end); */
                layer++;
            }
        }
    }

    /* Ignore Document Trailer */

    return ai;
}



/* Copy the header to file f */
/* change first line to EPSF if needed */
static void psfile_extract_header(PSDOC *doc, FILE *infile, FILE *f)
{
    char text[PSLINELENGTH];
    char *comment;
    BOOL pages_written = FALSE;
    int position;

    fseek(infile, doc->beginheader, SEEK_SET);
    fgets(text, PSLINELENGTH, infile);
    if (doc->epsf)
        fputs(text,f);
    else
    {
        switch(text[11])
        {
            case 1:
                    fputs("%!PS-Adobe-1.0 EPSF-1.0\r\n",f);
            break;
            case 2:
                    fputs("%!PS-Adobe-2.0 EPSF-2.0\r\n",f);
            break;
            default:
                    fputs("%!PS-Adobe-3.0 EPSF-3.0\r\n",f);
        }
    }

    position = ftell(infile);
    while ( (comment = pscopyuntil(infile, f, position,
               doc->endheader, "%%Pages:")) != (char *)NULL )
    {
        position = ftell(infile);
        if (pages_written)
        {
            gfree(comment);
            continue;
        }
        fprintf(f, "%%%%Pages: 1\r\n");
        pages_written = TRUE;
        gfree(comment);
    }
}


/* Copy the selected page and trailer to file f */
static void psfile_extract_layer(AIINFO *ai, FILE *infile, FILE *f, int layer)
{
    char *comment;
    int i;
    int position;
    PSDOC *doc;

    doc = ai->doc;

    /* don't copy preview because we don't need them */
    /* NOTE: should probably look for AI5_NumLayers and change it to 1 */
    pscopyuntil(infile, f, doc->begindefaults, doc->enddefaults, NULL);
    pscopyuntil(infile, f, doc->beginprolog, doc->endprolog, NULL);
    pscopyuntil(infile, f, doc->beginsetup, doc->endsetup, NULL);

    /* Find the layer in ai */
    for (i=0;i<ai->numlayers;i++)
    {
        if (ai->layerdata[i].layer == layer)
        {
            break;
        }
    }

    /* Default to the first layer if 'layer' does not exist */
    if (i == ai->numlayers) i = 0;

    /*
     * Copy the beginning of the layer until "(Layer"
     * Then rewrite the layer number to 0
     * Then copy the rest of the layer
     */
    comment = pscopyuntil(infile, f, ai->layerdata[i].begin, ai->layerdata[i].end, "(Layer");
    fprintf(f, "(Layer %d) Ln\r\n", 0);
    if (comment) gfree(comment);
    pscopyuntil(infile, f, -1, ai->layerdata[i].end, NULL);

    /*
     * Starting from the end of the last layer
     * copy the rest of the file.
     */
    position = ai->layerdata[ai->numlayers-1].end;
    while ( (comment = pscopyuntil(infile, f, position, doc->endtrailer, "%%Pages:")) != (char *)NULL )
    {
        position = ftell(infile);
        gfree(comment);
    }
}


/*************************************************************************/
/*                      Windows DLL Specific                             */
/*************************************************************************/

#if defined(_MSC_VER)

typedef int GSDLL_CALLBACK(int message, char *str, unsigned int count);

//#if 0
//def __ICL
//
//static int (*gsdll_init) (GSDLL_CALLBACK callback, HWND hwnd, int argc, char *argv[]);
//static int (*gsdll_execute_begin) (void);
//static int (*gsdll_execute_cont) (const char *str, int len);
//static int (*gsdll_execute_end) (void);
//static int (*gsdll_exit) (void);
//
//#else

static int (CALLBACK _export *gsdll_init) (GSDLL_CALLBACK callback, HWND hwnd, int argc, char *argv[]);
static int (CALLBACK _export *gsdll_execute_begin) (void);
static int (CALLBACK _export *gsdll_execute_cont) (const char *str, int len);
static int (CALLBACK _export *gsdll_execute_end) (void);
static int (CALLBACK _export *gsdll_exit) (void);
//#endif

static int AIgscallback(int message, char *str, unsigned int count)
{
//    if(message == 2)
//    {
//        fwrite(str,1,count,stdout);
//        fflush(stdout);
//    }
//    if(message == 1)
//        return count;

    return(0);
}


static void *AIload()
{
    HINSTANCE lib=0;

    lib = LoadLibrary(gsname);
    if (lib)
    {
        gsdll_init = GetProcAddress(lib, "gsdll_init");
        gsdll_execute_begin = GetProcAddress(lib, "gsdll_execute_begin");
        gsdll_execute_cont = GetProcAddress(lib, "gsdll_execute_cont");
        gsdll_execute_end = GetProcAddress(lib, "gsdll_execute_end");
        gsdll_exit = GetProcAddress(lib, "gsdll_exit");

        if (!(gsdll_init && gsdll_execute_begin && gsdll_execute_end && gsdll_exit))
        {
            FreeLibrary(lib);
            lib = 0;
        }
    }

    return((void*)lib);
}


static int AIfree(void *lib)
{
    FreeLibrary((HINSTANCE) lib);
    return(1);
}


static int rasterizefile(AIINFO *ai, char *inputname, char *outputname, char *devname, int width, int height, int xres, int yres)
{

    int ok=0;
    int i;
    int argc = 7;
    char *argv[7];

    for (i=0;i<argc;i++)
    {
        argv[i] = galloc(MAXSTR);
    }

    /* Generate gs command */
    sprintf(argv[0], "-dNOPAUSE");
    sprintf(argv[1], "-dQUIET");
    sprintf(argv[2], "-sDEVICE=%s", devname);
    sprintf(argv[3], "-sOutputFile=%s", outputname);
    sprintf(argv[4], "-r%dx%d", xres, yres);
    sprintf(argv[5], "-g%dx%d", width, height);
    sprintf(argv[6], "%s", inputname);

    if (ai->handle)
    {
        int rc = 0;
        rc = gsdll_init(AIgscallback, NULL, argc, argv);
        if (rc != -1)
        {
            rc = gsdll_execute_begin();
            rc = gsdll_execute_end();
            rc = gsdll_exit();
            ok = 1;
        }
    }

    for (i=0;i<7;i++)
    {
        gfree(argv[i]);
    }

    return(ok);
}

int extract_pdf(AIINFO * ai,char *pathname,char *tempname,char * device,int width, int height, int framenum)
{
    int ok=0;
    int i;
    int argc = 5;
    char *argv[8];
    int rc = 0;
    char *startcode = "systemdict /start get exec\n";
    PSDOC *doc;
    int old_width = 0;
    int old_height = 0;
    int xresolution = 72;
    int yresolution = 72;
    doc = ai->doc;

    if ( ((doc->boundingbox[URX] < doc->boundingbox[LLX]) ||
          (doc->boundingbox[URY] < doc->boundingbox[LLY])) )
    {
        pserror("Bounding Box is inconsistent");
        return 1;
    }

    if ( ((doc->boundingbox[URX] == doc->boundingbox[LLX]) ||
          (doc->boundingbox[URY] == doc->boundingbox[LLY])) )
    {
        pserror("Bounding Box is empty");
        return 1;
    }

    /* calculate page size */
    old_width = (int)((doc->boundingbox[URX] - doc->boundingbox[LLX])*(int)xresolution/72L);
    old_height = (int)((doc->boundingbox[URY] - doc->boundingbox[LLY])*(int)yresolution/72L);
    if(old_width)
        xresolution = (xresolution * width) / old_width;
    if(old_height)
        yresolution = (yresolution * height) / old_height;



    for (i=0;i<argc;i++)
    {
        argv[i] = galloc(MAXSTR);
    }

    /* Generate gs command */
    sprintf(argv[0], "-dNOPAUSE");

    sprintf(argv[1], "-dQUIET");

    sprintf(argv[2], "-sDEVICE=%s", device);
    sprintf(argv[3], "-sOutputFile=%s", tempname);
//    sprintf(argv[4], "-g%dx%d", width, height);
//    sprintf(argv[5], "-r%dx%d", xresolution, yresolution);
    sprintf(argv[4], "%s", pathname);
    argv[6] = NULL;
#ifdef DEBUG
    printf("DEVICE:%s:\n",device);
    printf("OUTPUTFILE:%s:\n",tempname);
    printf("PATHNAME:%s:\n",pathname);
    printf("DIM:%dx%d\n",width,height);
#endif
    rc = gsdll_init(AIgscallback, NULL, argc, argv);
    if(rc != -1)     // 101 is the INIT_QUIT flag, not an error.
    {
        gsdll_execute_begin();
        gsdll_execute_end();
        gsdll_exit();
        ok = 1;
    }
    for (i=0;i<argc;i++)
    {
        gfree(argv[i]);
    }

    return(ok);
}




/*************************************************************************/
/*                      Non DLL Specific                                 */
/*************************************************************************/
#else


static void *AIload()
{
    return(NULL);
}


static int AIfree(void *lib)
{
    return(1);
}


static int AIrunghostscript(AIINFO *ai, char *rspname)
{
    char gscommand[MAXSTR+MAXSTR];

    sprintf(gscommand, "%s @%s", gsname, rspname);
    system(gscommand);
    return(1);
}

static int rasterizefile(AIINFO *ai, char *inputname, char *outputname, char *devname, int width, int height, int xres, int yres)
{
    char rspname[MAXSTR];
    char gscommand[MAXSTR+MAXSTR];
    FILE *rspfile;
    int ok=0;

    /* Get the names of the temporary files */
    tmpnam(rspname);

    rspfile = fopen(rspname, WRITEBIN);
    if (rspfile)
    {
        /* Generate gs command */
        sprintf(gscommand, "-dNOPAUSE\n-dQUIET\n-sDEVICE=%s\n-sOutputFile=\042%s\042\n-r%dx%d\n-g%dx%d\n\042%s\042",
           devname, outputname, xres, yres, width, height, inputname);

        fputs(gscommand, rspfile);
        fclose(rspfile);
        ok = AIrunghostscript(ai, rspname);
        remove(rspname);
    }

    return(ok);
}


#endif


static AIINFO *openfile(FILE *file)
{
    AIINFO *ai;

    ai = aiscan(file);
    ai->doc = psscan(file);
    ai->file = file;
    ai->handle = AIload();
    return(ai);
}


static int GCALL closefile(AIINFO *ai)
{
    int ok=0;

    if (ai)
    {
        if (ai->doc) psgfree(ai->doc);
        if (ai->layerdata) gfree(ai->layerdata);
        if (ai->handle) AIfree(ai->handle);
        ok = gfree(ai);
    }

    return(ok);
}


static int extract_image(FILE *file, AIINFO *ai, char *outname, char *devname, int width, int height, int layer)
{
    char tempname[MAXSTR];
    FILE *tempfile;
    int old_width, old_height;
    int xresolution=72;
    int yresolution=72;
    PSDOC *doc;

    doc = ai->doc;

    if ( ((doc->boundingbox[URX] < doc->boundingbox[LLX]) ||
          (doc->boundingbox[URY] < doc->boundingbox[LLY])) )
    {
        pserror("Bounding Box is inconsistent");
        return 1;
    }

    if ( ((doc->boundingbox[URX] == doc->boundingbox[LLX]) ||
          (doc->boundingbox[URY] == doc->boundingbox[LLY])) )
    {
        pserror("Bounding Box is empty");
        return 1;
    }

    /* Get the names of the temporary files */
    tmpnam(tempname);

    /* Open the temp files */
    tempfile = fopen(tempname, WRITEBIN);
    if (tempfile) {

        /* offset to bottom left corner of bounding box */
        fprintf(tempfile, "%d %d translate\r\n", -doc->boundingbox[LLX], -doc->boundingbox[LLY]);

        /* calculate page size */
        old_width = (int)((doc->boundingbox[URX] - doc->boundingbox[LLX])*(int)xresolution/72L);
        old_height = (int)((doc->boundingbox[URY] - doc->boundingbox[LLY])*(int)yresolution/72L);

        xresolution = (xresolution * width) / old_width;
        yresolution = (yresolution * height) / old_height;

        /* cope with EPS files with and without showpage */
        fprintf(tempfile, "/EPSTOOL_save save def\n/showpage {} def\n");

        /* copy page to temporary file */
        psfile_extract_header(doc, file, tempfile);
        psfile_extract_layer(ai, file, tempfile, layer);

        /* cope with EPS files with and without showpage */
        fprintf(tempfile, "\nEPSTOOL_save restore\nshowpage\n");
        fprintf(tempfile, "\nquit\n");
        fclose(tempfile);

        /* Close the files so we can work with them */
        fclose(tempfile);

        rasterizefile(ai, tempname, outname, devname, width, height, xresolution, yresolution);

        /* Now remove the temp files */
        remove(tempname);
    }

    /* Should do better error checking */
    return 1;
}


static void *grealloc(void *pointer, size_t oldsize, size_t newsize)
{
    char *new;

    new = galloc(newsize);
    memcpy(new, pointer, oldsize);
    free(pointer);
    return(new);
}


/****************************************************************/
/*  Information Functions                                       */
/****************************************************************/

GABOUT * GCALL AI_about(void)
{
    GABOUT *info;

    info = (GABOUT *) galloc(sizeof(GABOUT));
    if (info)
    {
        memset(info, 0, sizeof(GABOUT));

        info->signature       = GMAKEID(0,0,'A','I');
        info->size            = sizeof(GABOUT);
        info->version         = GIMEX_VERSION;
        info->canimport          = 1;    /* supports importing */
        info->canexport          = 0;    /* supports exporting */
        info->importpacked    = 0;    /* supports importing packed */
        info->exportpacked    = 0;    /* supports exporting packed */
        info->import8         = 0;    /* supports importing 8 bit indexed */
        info->export8         = 0;    /* supports exporting 8 bit indexed */
        info->import32        = 1;    /* supports importing 32 bit direct rgb */
        info->export32        = 0;    /* supports exporting 32 bit direct rgb */
        info->multiframe      = 1;    /* supports multiple frames */
        info->multifile       = 0;    /* format requires additional files or resource fork */
        info->multisize       = 0;    /* supports each frame different size */
        info->framebuffer     = 1;    /* module requires memory to buffer entire frame */
        info->external        = 0;    /* module uses external resources */
        info->usesfile        = 1;    /* module is file based */
        info->maxframename    = 0;    /* max frame name character */
        info->resizable       = 1;    /* read respects ginfo width and height */
        info->defaultquality  = 100;  /* default pack quality */
        info->mactype[0]      = GMAKEID('.','A','I',' ');   /* mac file system types used */
        strcpy(info->extensions[0], ".ai");                 /* extensions */
        strcpy(info->authorstr,     "Kaz Okuda");           /* author */
        strcpy(info->versionstr,    "1.03");                /* version # */
        strcpy(info->shorttypestr,  "AI");                  /* type */
        strcpy(info->wordtypestr,   "Illustrator");         /* wordtype */
        strcpy(info->longtypestr,   "Abobe Illustrator");   /* longtype */

#if defined(_MSC_VER) && !defined(_XBOX)
        {
            static int plugintested = 0;

            if (!plugintested)
            {
                plugintested = 1;
                if (!SearchPath(NULL,"gsdll32.dll",NULL,0,NULL,NULL))
                {
                    plugintested = -1;
                }
            }
            if (plugintested<0)
            {
                info->canimport       = 0;    /* supports importing */
                info->importpacked    = 0;    /* supports importing packed */
                info->import8         = 0;    /* supports importing 8 bit indexed */
                info->import32        = 0;    /* supports importing 32 bit direct rgb */
                info->canexport       = 0;    /* supports exporting */
                info->exportpacked    = 0;    /* supports exporting packed */
                info->export8         = 0;    /* supports exporting 8 bit indexed */
                info->export32        = 0;    /* supports exporting 32 bit direct rgb */
            }
        }
#endif
    }
    return(info);
}

/* check for reasonable header: */

#define ISLENGTH 400
int GCALL AI_is(GSTREAM *g)
{
    char aibuf[ISLENGTH];
    int ok=0;

    gseek(g, 0);
    memset(aibuf,0,sizeof(aibuf));
    if (gread(g, aibuf, (int)gmin(ISLENGTH-1,glen(g))))
    {
        aibuf[ISLENGTH-1] = '\0';
        if (strstr(aibuf, "Illustrator"))
            ok = 100;
    }
    return(ok);
}


#undef ISLENGTH
/****************************************************************/
/*  Import Functions                                            */
/****************************************************************/

int GCALL AI_open(GINSTANCE **gxptr, GSTREAM *g, const char *pathname,bool framecountflag)
{
    GINSTANCE *gx;
    int ok=0;
    AIINFO *ai;
    GRefMod * GR;
    FILE *infile;
    GR = (GRefMod *) galloc(sizeof(GRefMod));
    gx = (GINSTANCE *) galloc(sizeof(GINSTANCE));
    if (gx)
    {
        memset(gx, 0, sizeof(GINSTANCE));
        gx->signature=GMAKEID(0,0,'A','I');
        gx->size=sizeof(GINSTANCE);
        gx->gstream = g;

        if (infile = fopen(pathname, READBIN))
        {
            if(isPDF(infile))
                seekToAdobeTag(infile);
            ai = openfile(infile);
            if (ai)
            {
                gx->frames = ai->numlayers;
                GR->ai = ai;
                GR->pathname = (char *)galloc(strlen(pathname)+3);
                strcpy(GR->pathname,pathname);
                gx->gref = GR;//(char *)ai;
                *gxptr = gx;
                ok = gx->frames;
            }
        }
    }
    return(ok);
}

GINFO * GCALL AI_info(GINSTANCE *gx, int framenum)
{
    GINFO *ginfo=0;
    GRefMod * GR;
    AIINFO *ai;
    PSDOC *doc;
    int width;
    int height;
    GR = (GRefMod *)gx->gref;
    ai = GR->ai;
    //ai = (AIINFO*)gx->gref;
    doc = ai->doc;

    width = (int)((doc->boundingbox[URX] - doc->boundingbox[LLX]));
    height = (int)((doc->boundingbox[URY] - doc->boundingbox[LLY]));
    if (framenum < gx->frames)
        ginfo = (GINFO *) galloc(sizeof(GINFO));

    if (ginfo)
    {
        memset(ginfo, 0, sizeof(GINFO));

        ginfo->signature        = GMAKEID(0,0,'A','I');     /* Gimex type */
        ginfo->size             = sizeof(GINFO);            /* Length of GINFO */
        ginfo->version          = GIMEX_VERSION;
        ginfo->framenum         = framenum;                 /* Current frame */
        ginfo->width            = width;                    /* Width of bitmap in pixels */
        ginfo->height           = height;                   /* Height of bitmap in pixels */
        ginfo->subtype          = 0;                        /* Internal format sub-type 0=chunk, 1=nonchunk */
        ginfo->packed           = 0;                        /* Type of packing on original image. 0=key, 1=motion */
        ginfo->quality          = 100;                      /* Quality of lossy packing 0..100 */
        ginfo->alphabits        = 0;    /* Number of bits in alpha channel */
        ginfo->redbits          = 8;    /* Number of bits in red channel */
        ginfo->greenbits        = 8;    /* Number of bits in grreen channel */
        ginfo->bluebits         = 8;    /* Number of bits in blue channel */
        ginfo->centerx          = 0;    /* Center Point relative to upper left corner */
        ginfo->centery          = 0;
        ginfo->defaultx         = 0;    /* Default Coordinate Point */
        ginfo->defaulty         = 0;
        ginfo->numhotspots      = 0;    /* Number of Hot Spots defined */
        ginfo->bpp              = 32;   /* Bits per pixel (8 or 32) */
        ginfo->originalbpp      = 24;   /* Bits per pixel in original image (1 to 32) */
        ginfo->numcolours       = 0;    /* Number of colours in original indexed palette */
        strncpy(ginfo->framename, ai->layerdata[framenum].name, 31);
        ginfo->framename[31] = '\0';
    }

    return(ginfo);
}

int GCALL AI_read(GINSTANCE *gx, GINFO *ginfo, char *dest, int rowbytes)
{
    AIINFO *ai;
    int ok=0;
    GRefMod * GR;
    char tempname[L_tmpnam];
    GBITMAP tempbitmap;
    int PDFflag = 0;
    ARGB * argb;
    int i = 0;
    PSDOC *doc;

#define AISCALE 2
    GR = (GRefMod *)gx->gref;
    ai = GR->ai;
    doc = ai->doc;

    tmpnam(tempname);
    rewind(ai->file);
    if(isPDF(ai->file))
    {
       GSTREAM *tempfile;
       extract_pdf(ai,GR->pathname,tempname,"graw32",ginfo->width,ginfo->height, ginfo->framenum);

       tempfile = gopen(tempname);
       if (tempfile)
       {
                    tempbitmap.ginfo = galloc(sizeof(GINFO));//AIinfogimex(tempgx, ginfo->framenum);
                    if (tempbitmap.ginfo)
                    {
                        char RAW_width[4];
                        char RAW_height[4];
                        char databuf[4];
                        char * temp = 0;
                        int RAW_iwidth  = 0;
                        int RAW_iheight = 0;
                        int rawcount = 0;
                        int pos = 0;
                        FINSTANCE *fx;
                        FINSTANCE *cropfx;
                        GINFO *tempinfo;
                        GINFO *tempinfocrop;
                        FOP fop[2];
                        FOP cropop[8];
                        for(rawcount = 0; rawcount < ginfo->framenum;rawcount++)
                        {
                            gread(tempfile,RAW_width,4);
                            gread(tempfile,RAW_height,4);
                            pos = (int)gtell(tempfile);
                            gseek(tempfile,pos + ggeti(RAW_height,4)*ggeti(RAW_width,4)*4);
                        }
                        gread(tempfile,RAW_width,4);
                        gread(tempfile,RAW_height,4);
                        RAW_iwidth  = ggeti(RAW_width,4);
                        RAW_iheight = ggeti(RAW_height,4);
                        memset(tempbitmap.ginfo,0,sizeof(GINFO));
                        tempbitmap.ginfo->signature   = 0;
                        tempbitmap.ginfo->width       = RAW_iwidth;
                        tempbitmap.ginfo->height      = RAW_iheight;
                        tempbitmap.ginfo->bpp         = 32;
                        tempbitmap.ginfo->originalbpp = 32;
                        tempbitmap.ginfo->size        = sizeof(GINFO);
                        tempbitmap.ginfo->version     = GIMEX_VERSION;
                        tempbitmap.ginfo->framenum    = ginfo->framenum;
                        tempbitmap.ginfo->framesize   = RAW_iwidth*RAW_iheight*4;
                        tempbitmap.ginfo->quality     = 100;
                        tempbitmap.ginfo->alphabits   = 8;
                        tempbitmap.ginfo->redbits     = 8;
                        tempbitmap.ginfo->greenbits   = 8;
                        tempbitmap.ginfo->bluebits    = 8;
                        tempbitmap.ginfo->centerx     = RAW_iwidth/2;
                        tempbitmap.ginfo->centery     = RAW_iheight/2;
                        tempbitmap.ginfo->defaultx    = 0;
                        tempbitmap.ginfo->defaulty    = 0;
                        tempbitmap.ginfo->numhotspots = 0;
                        tempbitmap.rowbytes = RAW_iwidth*32/8;
                        tempbitmap.image = galloc(RAW_iheight * tempbitmap.rowbytes);
                        argb = (ARGB *)tempbitmap.image;
                        for(i = 0; i < RAW_iheight*RAW_iwidth;i++)
                        {
                            ok = gread(tempfile,databuf,4);
                            if(!ok)
                                break;
                            argb->a = databuf[0];
                            argb->r = databuf[1];
                            argb->g = databuf[2];
                            argb->b = databuf[3];
                            argb++;
                        }
                        /* Shrink it down to ginfo->width*ginfo->height */
                        fop[0].i = ginfo->width;
                        fop[1].i = ginfo->height;
                        cropop[0].i = doc->boundingbox[LLX];
                        cropop[1].i = RAW_iheight - (doc->boundingbox[LLY] + ginfo->height);
                        cropop[2].i = ginfo->width;
                        cropop[3].i = ginfo->height;
                        cropop[4].i = 0;
                        cropop[5].i = 0;
                        cropop[6].i = 0;
                        cropop[7].i = 0;
                        if(ok)
                        {
                            opencrop(&cropfx,1,&tempbitmap,8,cropop);
                            tempinfocrop = infocrop(cropfx,0);
                            ok = filtercrop(cropfx,tempinfocrop,dest,rowbytes);
                            closecrop(cropfx);

                            tempbitmap.rowbytes = ginfo->width * 4;

                        }
                        else
                            ok = 0;
                        gfree(tempinfocrop);
                        gfree(tempbitmap.ginfo);
                        gfree(tempbitmap.image);
                    }
            gclose(tempfile);
            remove(tempname);
        }

    }
    else
    {
    rewind(ai->file);
    if (extract_image(ai->file, ai, tempname, "png16m", AISCALE*ginfo->width, AISCALE*ginfo->height, ginfo->framenum))
    {
        GSTREAM *tempfile;

        tempfile = gopen(tempname);
        if (tempfile)
        {
            if (AIisgimex(tempfile))
            {

                GINSTANCE *tempgx;

                if (AIopengimex(&tempgx, tempfile, tempname, 0))
                {
                    tempbitmap.ginfo = AIinfogimex(tempgx, 0);
                    if (tempbitmap.ginfo)
                    {
                        FINSTANCE *fx;
                        GINFO *tempinfo;
                        FOP fop[2];

                        tempbitmap.rowbytes = AISCALE*ginfo->width*32/8;
                        tempbitmap.image = galloc(AISCALE*ginfo->height * tempbitmap.rowbytes);
                        ok = AIreadgimex(tempgx, tempbitmap.ginfo, tempbitmap.image, tempbitmap.rowbytes);

                        /* Shrink it down to ginfo->width*ginfo->height */
                        fop[0].i = ginfo->width;
                        fop[1].i = ginfo->height;
                        opensize(&fx, 1, &tempbitmap, 2, fop);
                        tempinfo = infosize(fx, 0);
                        filtersize(fx, tempinfo, dest, rowbytes);
                        closesize(fx);

                        gfree(tempinfo);
                        gfree(tempbitmap.ginfo);
                        gfree(tempbitmap.image);
                    }
                    AIclosegimex(tempgx);
                }
            }
            gclose(tempfile);
            remove(tempname);
        }
    }
    }
    return(ok);

#undef AISCALE

}

int GCALL AI_close(GINSTANCE *gx)
{
    int ok=0;
    AIINFO *ai;
    GRefMod * GR;

    if (gx)
    {
        GR = (GRefMod *)gx->gref;
        ai = GR->ai;  //(AIINFO*)gx->gref;
        if (ai)
        {
            fclose(ai->file);
            closefile(ai);
        }
        if(GR->pathname)
            gfree(GR->pathname);
        if(GR)
            gfree(GR);
        ok = gfree((void *) gx);
    }

    return(ok);
}

