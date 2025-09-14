// (c) 1998-2004 Electronic Arts Inc.

const char *SPIT_history[] =
{
"------------------------------------------------------------------",
"                                                                  ",
"                         Locomoto v1.29                           ",
"                                                                  ",
"                        by EAC Ignition                           ",
"                                                                  ",
"                   Main program - Feb 23, 2004                    ",
"                                                                  ",
"------------------------------------------------------------------",
"                                                                  ",
" Version SE Date   History:                                       ",
" ------- -- ------ --------                                       ",
"    1.00 GS 980604 initial version                                ",
"    1.01 GS 990309 index chunk, no .h option, check duplicate ids ",
"    1.02 GS 990410 int based index chunk                          ",
"    1.03 GS 990413 does not check id's if no header created       ",
"    1.04 FB 990420 abstract and -h different than -help           ",
"    1.05 GS 990422 32 bit align chunks                            ",
"    1.10 GS 990430 specified columns,conditional compiles,stats   ",
"    1.11 GS 990505 updated help                                   ",
"    1.20 GS 990603 converted parser, Unicode, tab delimiters      ",
"    1.21 GS 990618 -multifile option                              ",
"    1.22 GS 990623 -rc -lc options, left & right comment strings  ",
"    1.23 GS 991206 escape sequence, noheader, comment row, start# ",
"    1.24 GS 000208 nolanguagecount and nostringcount options      ",
"    1.25 GS 000221 =c:\test.loc sets output folder for .h & .loc  ",
"    1.26 GS 000306 -extraheaderinfo                               ",
"    1.27 FB 010129 WriteLong use little endian (puti)             ",
"    1.28 RG 010405 Abort nicely if bad column index specified     ",
"    1.29 JF 040224 Update to use teal-1.00.01, unicode is default ",
"                                                                  ",
"------------------------------------------------------------------",
"\0"
};

#include <windows.h>
#include <shlobj.h>
#include <stdarg.h>
#include <string>
#include <stdio.h>
#include <assert.h>

#include <eabase/eabase.h>
#include <teal/spit.h>
#include <teal/range.h>
#include <teal/ustr.h>

#include "DataTableIndex.h"

///////////////////////////////////////////////////////////////////////////
// SPIT Routines & Variables

// program string constants
const char k_AppName[] = "locomoto";
const char TextVersion[] = "1.29 (Feb. 24, 2004)";
const int TOOL_VERSION = 129;
const uint16_t UNICODE_SIGNATURE = 0xfeff;

// program options (set default values in SPIT_reset)
int  OPT_createheader;
int  OPT_cplusplus;
int  OPT_idcolumn;
int  OPT_valuecolumn;
int  OPT_multifile;
int  OPT_stats;
char OPT_stringcolumns[SPITMAXLINE];
int  OPT_definecolumn;
char OPT_define[SPITMAXLINE];
int  OPT_checkduplicates;
int  OPT_delimiter;
int  OPT_encoding;
char OPT_leftcomment;
char OPT_rightcomment;
int  OPT_escapesequence; 
int  OPT_comments;     
int  OPT_noheaderrow;    
int  OPT_startenumat;
int  OPT_nolanguagecount;
int  OPT_nostringcount;
int  OPT_extraheaderinfo;

const unsigned char locomoto_help[] = 
{
#include "htmlhelp.dat"
};

// Write out an html file into the temp directory and display it.
// This could be made into a shared function between all the programs
// in this package.
void DisplayDoc(const char *toolName, int toolVersion, const unsigned char *help, unsigned int helpsize)
{
    char tempPath[MAX_PATH];
    GetTempPath(MAX_PATH, tempPath);

    char DocFileName[MAX_PATH];

    sprintf(DocFileName, "%s%s-%.2f.htm", tempPath, toolName,
        static_cast<float>(toolVersion)/100.0f);

    HANDLE hDocFile = CreateFile(
        DocFileName,        // File Name
        FILE_ALL_ACCESS,    // Desired Access
        0,                  // Share mode
        NULL,               // Security Attributes
        CREATE_ALWAYS,      // Blow away the file if it was there already.
        FILE_ATTRIBUTE_NORMAL,
        NULL);              // Template File

    if (hDocFile == INVALID_HANDLE_VALUE)
    {
        printf("Could not open temp doc file\n");
        return;
    }
    
    DWORD NumberOfBytesWritten;
    WriteFile(hDocFile, help, helpsize, &NumberOfBytesWritten, NULL);

    if (NumberOfBytesWritten != helpsize)
    {
        printf("Failure writing temporary documentation file\n");
        CloseHandle(hDocFile);
        return;
    }

    CloseHandle(hDocFile);

    char ProcessCommandLine[MAX_PATH];
    sprintf(ProcessCommandLine, "/c start %s", DocFileName);

    char szAppName[MAX_PATH];
    SHGetFolderPath(NULL, CSIDL_SYSTEM, NULL, 0, szAppName);
    strcat(szAppName, "\\cmd.exe");

    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    memset(&si, 0, sizeof(si));
    memset(&pi, 0, sizeof(pi));

    CreateProcess(szAppName, ProcessCommandLine,
        NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi);
}

void SPIT_reset(void)
{
    // spit options
    OPTextrule = ".loc";

    // program options
    OPT_createheader     = 1;       // creates a .h file
    OPT_cplusplus        = 0;       // creates a C++ style .h file (ie, const int rather than #define)
    OPT_idcolumn         = 0;       // IDS_HELLO
    OPT_valuecolumn      = -1;      // int value of the IDS_HELLO label (ie, 23 in this example #define IDS_HELLO 23)
    OPT_multifile        = 0;       // all languages are stored in seperate .loc files
    OPT_stats            = 0;       // displays stats about the strings
    OPT_stringcolumns[0] = '\0';    // range of values (ie, 2..5)
    OPT_definecolumn     = -1;      // column that conditional compiles are in
    OPT_define[0]        = '\0';    // conditional compile defines (ie, PC|PSX)
    OPT_checkduplicates  = 0;       // analyze the source file to ensure no duplicate labels or bad data
    OPT_leftcomment      = 0;       // character used to specify left side of a comment
    OPT_rightcomment     = 0;       // character used to specify right side of a comment
    OPT_escapesequence   = 0;       // non-zero means to interpret HTML like escape sequences to place speicific bytes into a string (ie, &#32;)
    OPT_comments         = 0;       // non-zero indicates that rows that start with ; are ignored
    OPT_noheaderrow      = 0;       // non-zero means that .csv file contains no header rows (do not print out language #defines in .h file)
    OPT_startenumat      = 0;       // values to start .h enum values at (note: this does not effect data in the .loc file - only the .h file)
	OPT_nolanguagecount  = 0;		// do not write LOCALE_LANGUAGE_COUNT in .h file
	OPT_nostringcount    = 0;		// do not write LOCALE_STRING_COUNT in .h file
	OPT_extraheaderinfo  = 0;		// if non-zero write debug info in the header file to note where the string came from

    OPT_delimiter = DataTableIndex::k_DelimiterTab;     // character used to delimit data in source file (tab or comma)
    OPT_encoding  = DataTableIndex::k_EncodingUnicode;  // character encoding used in source file (ascii or unicode)
}

void SPIT_usage(const char* ProgramName, const char *, const char *, int utype)
{
    printf("locomoto v%s by EAC Ignition\n", TextVersion);

    if (utype > 0)
    {
        printf("USAGE:   %s [-options] <sourcefile>\n", ProgramName);
        printf("type '%s -help' for full documentation in HTML format\n", ProgramName);
    }
}

char* SPIT_opt(const char* optptr)
{
    char* d =(char *) 0;

    switch (*optptr|(char)' ')
    {
        case 'a':
            if (strnicmp(optptr,"ascii",5) == 0)
            {
                int temp;
                d = SPIT_getopt(&temp,optptr+5, "ascii", 0, 1);
                OPT_encoding = DataTableIndex::k_EncodingAscii;
            }
            break;

        case 'c':
            if (strnicmp(optptr,"cpp",3) == 0)
            {
                d = SPIT_getopt(&OPT_cplusplus,optptr+3, "cpp", 0, 1);
            }
            else if (strnicmp(optptr,"check",5) == 0)
            {
                d = SPIT_getopt(&OPT_checkduplicates,optptr+5, "check", 0, 1);
            }
            else if (strnicmp(optptr,"csv",3) == 0)
            {
                int temp;
                d = SPIT_getopt(&temp,optptr+3, "csv", 0, 1);
                OPT_delimiter = DataTableIndex::k_DelimiterComma;
            }
            else if (strnicmp(optptr, "comments",8) == 0)
            {
                d = SPIT_getopt(&OPT_comments,optptr+8, "comments", 0, 1);
            }
            break;

        case 'd':
            if (strnicmp(optptr,"define",6) == 0)
            {
                d = SPIT_getopt(&OPT_definecolumn,optptr+6, "define", -1, 100);
            }
            else if (strnicmp(optptr,"D",1) == 0)
            {
                d = SPIT_getstropt(OPT_define,optptr+1, "D", "", SPITYES, SPITMAXLINE);
            }
            break;

        case 'e':
            if (strnicmp(optptr,"escape",6) == 0)
            {
                d = SPIT_getopt(&OPT_escapesequence,optptr+6, "escape", 0, 1);
            }
            else if (strnicmp(optptr,"extraheaderinfo",15) == 0)
            {
                d = SPIT_getopt(&OPT_extraheaderinfo,optptr+15, "extraheaderinfo", 0, 1);
            }
            break;

        case 'h':
            if (strnicmp(optptr,"header",6) == 0)
            {
                d = SPIT_getopt(&OPT_createheader,optptr+6, "header", 0, 1);
            }
            else if (!strnicmp(optptr, "help", 4))
            {
                d = const_cast<char *>(optptr) + 4;
                if (!SPIThelpgiven)
                {
                    DisplayDoc(k_AppName, TOOL_VERSION, locomoto_help, sizeof(locomoto_help));
                    SPIThelpgiven = 1;
                }
            }
            break;

        case 'i':
            if (strnicmp(optptr,"index",5) == 0)
            {
                d = SPIT_getopt(&OPT_valuecolumn,optptr+5, "index", -1, 100);
            }
            else if (strnicmp(optptr,"id",2) == 0)
            {
                d = SPIT_getopt(&OPT_idcolumn,optptr+2, "id", -1, 100);
            }
            break;
        
        case 'l':
            if (strnicmp(optptr, "lc",2) == 0)
            {
                OPT_leftcomment = *(optptr+2);
                d = (char*)(optptr + 3);
            }
            break;

        case 'm':
            if (strnicmp(optptr,"multifile",9) == 0)
            {
                d = SPIT_getopt(&OPT_multifile,optptr+9, "multifile", 0, 1);
            }
            break;

        case 'n':
            if (strnicmp(optptr,"noheader",8) == 0)
            {
                d = SPIT_getopt(&OPT_noheaderrow,optptr+8, "noheader", 0, 1);
            }
            else if (strnicmp(optptr,"nolanguagecount",15) == 0)
            {
                d = SPIT_getopt(&OPT_nolanguagecount,optptr+15, "nolanguagecount", 0, 1);
            }
            else if (strnicmp(optptr,"nostringcount",13) == 0)
            {
                d = SPIT_getopt(&OPT_nostringcount,optptr+13, "nostringcount", 0, 1);
            }
            break;

        case 'r':
            if (strnicmp(optptr, "rc",2) == 0)
            {
                OPT_rightcomment = *(optptr+2);
                d = (char*)(optptr + 3);
            }
            break;

        case 's':
            if (strnicmp(optptr,"stats",5) == 0)
            {
                d = SPIT_getopt(&OPT_stats,optptr+5, "stats", 0, 100);
            }
            else if (strnicmp(optptr,"strings",7) == 0)
            {
                d = SPIT_getstropt(OPT_stringcolumns,optptr+7, "strings", "", SPITYES, SPITMAXLINE);
            }
            else if (strnicmp(optptr,"startat",7) == 0)
            {
                d = SPIT_getopt(&OPT_startenumat,optptr+7, "startat", 0, 10000000);
            }
            break;

        case 't':
            if (strnicmp(optptr,"tab",3) == 0)
            {
                int temp;
                d = SPIT_getopt(&temp,optptr+3, "tab", 0, 1);
                OPT_delimiter = DataTableIndex::k_DelimiterTab;
            }
            break;

        case 'u':
            if (strnicmp(optptr,"unicode",7) == 0)
            {
                int temp;
                d = SPIT_getopt(&temp,optptr+7, "unicode", 0, 1);
                OPT_encoding = DataTableIndex::k_EncodingUnicode;
            }
            break;

        default:
            break;
    }
    return(d);
}

/*
;
; ABSTRACT
;
; locomoto - convert a string database to .loc (locale) file
;
; SUMMARY
;
; locomoto v1.24 (NBA 2K1) Dec  6 1999 by Gerry Shaw, EAC
; USAGE:   locomoto [-options] <sourcefile>
; 
; VERSION: WIN NT v1.24 (NBA 2K1), Spit v0.00
; SUMMARY: locomoto string compiler tool
;          SPIT environment variable is LOCOMOTO_OPTS
; 
;      @<file>  response file, containing a list of options and files
;     -v#       verboseness 0=nothing,1=errors,2=default,3=extra,4=debug
;     -cpp      C++ style header files (const int vs default #define)
;     -header#  0=does not create .h file,1=creates .h file (default)
;     -id#      specifies what column id values in (default 1, -1 for none)
;     -index#   create an index, # specifies what column index (-1 for none)
;     -define#  specifies what column the conditionals are in (default -1)
;     -D...     specifies conditional
;     -strings#..# range of columns containing string values
;     -check    check for duplicate strings (default off - this is O(n^2))
;     -ascii    source file is ASCII encoded
;     -unicode  source file is Unicode encoded, must be tab delimited (default)
;     -csv      source file is a DataTableIndex, comma seperated file
;     -tab      source file is tab delimited (default)
;     -multifile languages are stored in multiple files (default is off)
;     -lc<char> character to indicate left side of a comment (default none)
;     -rc<char> character to indicate right side of a comment (default none)
;     -escape   use HTML stype escape sequences to enter raw bytes
;     -comments rows that start with a ; will be ignored
;     -noheader indicates that first row contains data (not header info)
;     -nostringcount prevents LOCALE_STRING_COUNT from appearing in header
;     -nolanguagecount prevents LOCALE_LANGUAGE_COUNT from appearing in header
;     -startat# value to start enumerations in .h file at
;     -extraheaderinfo prints where the string came from in the header file
;
; Notes: * Column values are 0 based.  Column 'A' in Excel is column 0.
;        * Use -1 to specify a column does not exist
;        * Range syntax for strings is 3..6 for 4 columns of strings
;        * If no string range is given than all left over columns with
;          a column header are considered to have strings.
;        * Conditionals are good when you want to share strings among
;          different platforms.  Put a pc or psx to limit the string
;          to a specific platform.  Use -Dpc to get only pc strings.
;        * If using Unicode you must use tab delimited files.
;        * In general tab delimited is better than comma delimited.
;        * In order to use the new LOCALE_getstr API you must use the
;          -multifile option to get each language in it's own .loc file.
;        * Pathnames are taken from the top row of each column when
;          using the multifiles.
;        * Use -lc and -rc to allow for comments in the strings
;          ie, "[winning team name] defeat [losing team name]."
;          Use these options to handle the above example -lc[ -rc]
;        * Escape sequences are in format of &#32;
;        * -startat option only effects .h file
;
; locomoto -h    # for this help
; locomoto -help # for web page with more help
; 
; 
; DESCRIPTION
;
; Locomoto converts a string database to .loc (locale) file.  The string database
; can be a comma seperated file (no unicode support) or tab delimited file 
; (prefered format).  The locale start page has examples and a walk through on 
; using Locomoto, go there for working examples (see below for URL).
;
; NOTES ABOUT CONDITIONAL COMPILING
;
; Locomoto supports the idea of "conditionally compiling" which allows you to use
; a single string database for all platforms of a game.  To do this you specify
; a column to be the "define" column.  In this define column you put labels of the 
; platforms you want.
;
; Here is an example file:
;
; id   ,platform  ,IDL_ENGLISH                     ,IDL_FRENCH
; IDS_A,pc        ,"Hello ""Again",                ,Bonjour
; IDS_B,pc|psx    ,"Hello2,,,, ""Again"""          ,Bonjour Duex  
; IDS_C,psx|pc|n64,"Hello, World!"                 ,Bonjour Mundo                
; IDS_D,n64       ,This is an English sentence     ,Das isch eh francis sentence
; IDS_E,          ,"""This string is in quotes"""  ,"dos isch eh string en quotes"
;
; To compile this database you could use the following Locomoto command:
;
; Locomoto -id0 -define1 -strings"2..3" -Dpc
;
; Note the -Dpc at the end of the command line will tell Locomoto to include
; strings only with "pc" or blank in the platform column (column 1 as specified 
; by the -define option).
;
; If for some strange reason you wanted to include all the strings for the pc
; and psx in a single .loc file you would use this command line:
;
; Locomoto -id0 -define1 -strings2..3 -Dpc|psx
;
; The use of the | character is VERY important here.  It acts as an OR operator.
; No there is no other operators (OR is strange enough if you ask me).
;
; Also note that in this example I use a platform this is not the only use for
; this feature.  It is the main use but it could be used for many other things.
;
; NOTES ABOUT ESCAPE SEQUENCES
;
; The -escape option turns on escape sequence processing.  This allows you to
; insert non-printable character codes directly in a string.  For example the
; if you wanted to insert character code 99 between Big and Doc you would use
; the following string format: "Big&#99;Doc".  The notation follows the notation
; used in HTML in case you were wondering.
;
; OTHER NOTES
;
; A lot of localization resources are available at the localization web site at:
; http://tools.eac.ea.com/locale
;
; RETURNS
;
; non zero if fatal error occured
;
; SEE ALSO
;
; txt, Spit, http://tools.eac.ea.com/locale/
;
; EXAMPLE
;
; > locomoto -id-1 -header0 -Dpc|n64 -cpp -define1 -index3 -id0 -strings"2,4..6" c:\real\example\locale_eg.index
; locomoto v1.10 (Earth) Apr 30 1999 by Gerry Shaw, EAC
; c:\real\example\locale_eg.index: Reading, Writing loc file (index, cpp, 4 language s, 5 strings)
;
; END ABSTRACT
;
*/

void Zprintf(int VerboseLevel, const char* format, ...)
{
    if (VerboseLevel <= OPTverbose)
    {
        va_list arglist;
        va_start(arglist, format);
        vprintf(format, arglist);
        va_end(arglist);
    }
}

///////////////////////////////////////////////////////////////////////////
// File Format Structures

#define LOCALEFILE_HEADERCHUNKID    GMAKEID('L','O','C','H')
#define LOCALEFILE_INDEXCHUNKID     GMAKEID('L','O','C','I')
#define LOCALEFILE_LANGUAGECHUNKID  GMAKEID('L','O','C','L')

typedef struct
{
    unsigned long   ChunkID;        // 'LOCH' LOCALEFILE_HEADERCHUNKID
    unsigned long   ChunkSize;      // size of chunk in bytes
    unsigned long   Flags;          // 0=no index chunk present,1=index chunk present
    unsigned long   LanguageCount;  // number of language chunks in this file
//  unsigned long   LanguageOffset[LanguageCount]; // offsets in bytes from start of file to language chunk
} LOCALEFILE_HEADERCHUNK;

typedef struct
{
    unsigned long   ChunkID;        // 'LOCI' LOCALEFILE_INDEXCHUNKID
    unsigned long   ChunkSize;      // size of chunk in bytes
    unsigned long   StringCount;    // number of string ids in this chunk (same value in all language chunks)
    unsigned long   pad;            // must be zero
//  STRINGID StringID[StringCount]; 
//  {
//      unsigned short ID;          // id that user gives to look up value
//      unsigned short Index;       // index to look up value in language chunks
//  }
} LOCALEFILE_INDEXCHUNK;

typedef struct
{
    unsigned long   ChunkID;        // 'LOCL' LOCALEFILE_LANGUAGECHUNKID
    unsigned long   ChunkSize;      // size of chunk in bytes including this header and all string data
    unsigned long   LanguageID;     // language strings are in for this bank
    unsigned long   StringCount;    // number of strings in this chunk
//  unsigned long   StringOffset[StringCount];   // offsets in bytes from start of chunk to string
//  const char*     Data[StringCount];           // StrinCount null terminated strings
} LOCALEFILE_LANGUAGECHUNK;


///////////////////////////////////////////////////////////////////////////
// util functions

char* FILE_readtextfile(char* pPathname)
{
    char* p = NULL;
    GSTREAM* g = gopen(pPathname);
    if (g)
    {
        int FileLength = (int) glen(g);
        p = (char*)galloc(FileLength+2);
        p[FileLength] = 0;  // double NULL terminate the file buffer
        p[FileLength+1] = 0;
        if (gread(g,p,FileLength) != FileLength)
        {
            gfree(p);
            p = NULL;
        }
        gclose(g);
    }
    return p;
}

///////////////////////////////////////////////////////////////////////////
// application functions

int  g_LanguageCount = 0;
int  g_StringCount   = 0;

int* g_ColumnMap        = NULL;
int  g_ColumnMapSize    = 0;        // number of entires in the map
const int k_EmptyColumn = -1;
const int k_IdColumn     = 1000;
const int k_ValueColumn  = 2000;
const int k_DefineColumn = 3000;
// string columns are 0..g_LanguageCount
const int k_MinStringColumn = 0;
const int k_MaxStringColumn = 999;

const char* GetColumnDescription(int mapid)
{
    // these function is used to give meaningful error messages
    if (mapid == k_EmptyColumn)
        return "unassigned";
    else if (mapid >= k_MinStringColumn && mapid < k_MaxStringColumn)
        return "string";
    else if (mapid == k_IdColumn)
        return "id";
    else if (mapid == k_ValueColumn)
        return "index value";
    else if (mapid == k_DefineColumn)
        return "conditional";
    else 
        return "unknown";
}

bool AllocColumnMap(int ColumnCount)
{
    bool ok = false;

    g_ColumnMapSize = ColumnCount;
    g_ColumnMap = static_cast<int*>(galloc(g_ColumnMapSize * sizeof(int)));
    if (g_ColumnMap)
    {
        // make all columns empty
        for (int i = 0; i < g_ColumnMapSize; ++i)
            g_ColumnMap[i] = k_EmptyColumn;

        if (OPT_idcolumn >= 0)
            g_ColumnMap[OPT_idcolumn] = k_IdColumn;
        if (OPT_valuecolumn >= 0)
            g_ColumnMap[OPT_valuecolumn] = k_ValueColumn;
        if (OPT_definecolumn >= 0)
            g_ColumnMap[OPT_definecolumn] = k_DefineColumn;

        int column = 0;
        for (int i = 0; i < g_LanguageCount; ++i)
        {
            if (strlen(OPT_stringcolumns) == 0)
            {
                // assume every column not specified is a string
                while (g_ColumnMap[column] != -1)
                    column++;
            }
            else
            {
                // user stringcolumns range
                // int RANGE_match(const char *rangestr, char *name, int num);
                // count = RANGE_count(OPT_stringcolumns, k_MinStringColumn,k_MaxStringColumn);

                while (!RANGE_match(OPT_stringcolumns, NULL, column))
                    column++;
            }

            if (g_ColumnMap[column] != -1)
            {
                Zprintf(1, " - ERROR (no files created)\n");
                Zprintf(1, "Conflict in column assignments.\n");
                Zprintf(1, "Column %d is specified as a string column and a %s column\n",
                    column, GetColumnDescription(g_ColumnMap[column]));

                gfree(g_ColumnMap);
                g_ColumnMap = NULL;
                g_ColumnMapSize = 0;
                return false;
            }
            g_ColumnMap[column++] = i;
        }
        ok = true;
    }

    // print column map for debugging
    if (OPTverbose >= 4 && ok)
    {
        printf("\n\nColumn Map:\n===========\n");
        for (int i = 0; i < g_ColumnMapSize; ++i)
        {
            printf("%2d %s\n", i, GetColumnDescription(g_ColumnMap[i]));
        }
    }

    return ok;
}

void FreeColumnMap(void)
{
    gfree(g_ColumnMap);
    g_ColumnMap = 0;
    g_ColumnMapSize = 0;

}

int GetLanguageColumn(int LanguageIndex)
{
    int column = -1;
    for (int i = 0; i < g_ColumnMapSize; ++i)
    {
        if (g_ColumnMap[i] == LanguageIndex)
        {
            column = i;
            break;
        }
    }

    return column;
}

// return the pos of ch in str
int find(const char* str, char ch, int startingpos = 0)
{
    int pos = startingpos;
    while (str[pos] != '\0' && str[pos] != ch)
    {
        pos++;
    }
    if (str[pos] == '\0')
        return -1;
    else
        return pos;
}

int GetTokenCount(const std::string& TokenList, char delimiter)
{
    int Count = 0;

    // there is at least one token in the list if the string is not empty
    if (TokenList.size() > 0)
    {
        int pos = 0;
        while (pos >= 0)
        {
            Count++;
            pos = TokenList.find(delimiter,pos+1);
        }
    }

    return Count;
}

static void trim(std::string& str)
{
    /* trim any trailing whitespace */
    int i = str.size() - 1;
    while (i >= 0)
    {
        if (str[i] != ' ')
            break;

        str[i--] = 0;
    }
}

std::string GetToken(const std::string& TokenList, int TokenIndex, char delimiter)
{
    assert(TokenIndex >= 0 && TokenIndex < GetTokenCount(TokenList, delimiter));

    int startpos = 0;
    for (int i = 0; i < TokenIndex; i++)
    {
        startpos = TokenList.find(delimiter, startpos) + 1;
        assert(startpos  >= 0);
    }

    int endpos = TokenList.find(delimiter, startpos);
    if (endpos < 0)
        endpos = TokenList.size();

    std::string token = TokenList.substr(startpos,endpos-startpos);
    trim(token);
    return token;
}


bool AnyTokenInTokenList(const std::string& t1list, const std::string& t2list, char delimiter)
{
    // return true if any token in tokenlist t1list is in token list t2list
    int t1count = GetTokenCount(t1list, delimiter);
    int t2count = GetTokenCount(t2list, delimiter);

    for (int i = 0; i < t1count; ++i)
    {
        std::string t1 = GetToken(t1list, i, delimiter);
        for (int j = 0; j < t2count; ++j)
        {
            std::string t2 = GetToken(t2list, j, delimiter);

            if (t1.compare(t2) == 0)
            {
                return true;
            }
        }
    }
    return false;
}

std::string GetAsciiString(DataTableIndex& index, int r, int c)
{
    std::string str;

    int initialoffset = index.getCellOffset(r,c);
    int offset = initialoffset;
    int size   = index.getCellSize(r,c);
    str.reserve(size);

    while (offset-initialoffset < size)
    {
        int ch = index.getCharacter(offset);
        char character;
        if (ch < 256)
        {
            character = static_cast<char>(ch);
        }
        else
        {
            character = 'X';
        }
        str += character;
        offset += index.getCharacterSize();
    }

    if ((index.getDelimiter() == DataTableIndex::k_DelimiterComma) && (index.getEncoding() == DataTableIndex::k_EncodingAscii))
    {
        // translate CSV string into a normal string
        // 1. remove quotes if first character is quoted
        // 2. replace double quotes with a single quote
        // only have to handle the ASCII case as .csv files can't be unicode

        if (str[0] == '"')
        {
            // first and last quote characters
            str.replace(str.size()-1,1,"");
            str.replace(0,1,"");
        }

        unsigned int pos = str.find("\"\"");
        while (pos != std::string::npos)
        {
            str.replace(pos,2,"\"");
            pos = str.find("\"\"");
        }
    }

    // remove any characters in comment characters
    // ie, given "The %1[team name] kick the %2[losing team] into yesterday"
    // and that comment characters are [ and ] we want to return
    // "The %1 kick the %2 into yesterday"
    if (OPT_leftcomment != 0 && OPT_rightcomment != 0)
    {
        // find left comment
        unsigned int pos = str.find(OPT_leftcomment);
        while (pos != std::string::npos)
        {
            // find right comment
            unsigned int rightpos = str.find(OPT_rightcomment, pos+1);

            // find length of comment to remove
            unsigned int len;
            if (rightpos == std::string::npos)
            {
                len = str.size() - pos;
            }
            else
            {
                len = rightpos - pos + 1;
            }

            // remove everything starting at pos for len characters
            str.replace(pos,len,"");

            // find next left comment
            pos = str.find(OPT_leftcomment);
        }
    }

	// translate HTML escape sequences into raw character codes
	// format is: &#32;
	// ie, given "Press the '&#65;&#32;&#66;' button to continue." we want
	// "Press the 'A B' button to continue."
	if (OPT_escapesequence != 0)
	{
		// find escape sequence
		unsigned int pos = str.find("&#");
		while (pos != std::string::npos)
		{
			unsigned int endpos = str.find(";", pos+2);
			unsigned int len    = endpos-pos-2;

			// check to see if this really is a valid escape sequence
			if (endpos == std::string::npos || len > 6)
				break;

			std::string number;
			number.assign(str, pos+2, len);

			int code = atoi(number.c_str());

			char codestr[2];
			codestr[0] = static_cast<char>(code);
			codestr[1] = '\0';

			str.replace(pos, len+3, codestr); // +3 is for the #& and ; characters

			// find next escape sequence
			pos = str.find("&#");
		}
	}

    return str;
}

void USTR_replace(USTR* dst, int count, const USTR* src);

void WriteUnicodeString(GSTREAM* g, DataTableIndex& index, int r, int c)
{
    int offset = index.getCellOffset(r,c);
    int size   = index.getCellSize(r,c);

    if (size > 0)
    {
        // copy Unicode string into temp buffer
        USTR* str = (USTR*)(galloc(size+2));
        memset(str,0, size+2);
        memcpy(str, index.getByteBuffer() + offset, size);

        // remove any comments from the string
        if (OPT_leftcomment != 0 && OPT_rightcomment != 0)
        {
            // find left comment
            int pos = USTR_findchar(str, OPT_leftcomment);
            while (pos != USTR_NPOS)
            {
                // find right comment
                int rightpos = USTR_findchar(str+1, OPT_rightcomment);

                // find length of comment to remove
                int len;
                if (rightpos == USTR_NPOS)
                {
                    len = USTR_length(str) - pos;
                }
                else
                {
                    len = rightpos - pos + 1;
                }

                // remove everything starting at pos for len characters
                int taillen = USTR_length(str) - len - pos;
                USTR_ncopy(str+pos, str+pos+len+1, taillen);

                // find next left comment
                pos = USTR_findchar(str, OPT_leftcomment);
            }
        }

		// translate HTML escape sequences into raw character codes
		// format is: &#32;
		// ie, given "Press the '&#65;&#32;&#66;' button to continue." we want
		// "Press the 'A B' button to continue."
		if (OPT_escapesequence != 0)
		{
			USTR escapeStr[3];
			escapeStr[0] = '&';
			escapeStr[1] = '#';
			escapeStr[2] = 0;

			// find escape sequence
			int pos = USTR_find(str, escapeStr);
			while (pos != USTR_NPOS)
			{
				int endpos = USTR_findchar(str+pos, ';') + pos;
				int len    = endpos-pos-2;

				// check to see if this really is a valid escape sequence
				if (endpos == USTR_NPOS || len > 6)
					break;

				USTR digits[7];
				USTR_ncopy(digits, str+pos+2, len);
				digits[len] = 0;

				// convert unicode string into ascii string
				int code;
				{
					char cdigits[7];
					int i = 0;
					while (digits[i] != 0)
					{
						int n = digits[i];
						if (n < 0 || n > 255)
							n = 0;
						cdigits[i++] = static_cast<char>(n);
					}
					cdigits[i] = 0;
					code = atoi(cdigits);
				}

				USTR codestr[2];
				codestr[0] = static_cast<char>(code);
				codestr[1] = '\0';

				USTR_replace(str+pos, len+3, codestr);  // +3 is for the #& and ; characters

				// find next escape sequence
				pos = USTR_find(str, escapeStr);
			}
		}
        // write string buffer to given stream
		size = USTR_length(str)*2 + 2; // *2 because length returns number of letters, add 2 bytes for double byte null character
        gwrite(g, str, size);

        // free string
        gfree(str);
    }
}

void USTR_replace(USTR* dst, int count, const USTR* src)
{
	int srcLen = USTR_length(src);
	int dstLen = USTR_length(dst);

	// we can't grow a USTR so don't do anything if replacement string is bigger than what we are replacing
	if (srcLen > count)
		return;

	// copy src over dst
	memcpy(dst, src, srcLen*2);

	// copy rest of dst to end of src
	int i;
	for (i = 0; i < dstLen-count; ++i)
	{
		*(dst+srcLen + i) = *(dst + count + i);
	}
	*(dst+srcLen+i) = 0;
}

void WriteAsciiString(GSTREAM* g, DataTableIndex& index, int r, int c)
{
    std::string str = GetAsciiString(index, r,c);
    gwrite(g, str.c_str(), str.size());
}

void WriteString(GSTREAM* g, DataTableIndex& index, int r, int c)
{
    // do NOT write a null character, caller will write one if need be
    if (OPT_encoding == DataTableIndex::k_EncodingUnicode)
    {
        WriteUnicodeString(g, index, r,c);
    }
    else
    {
        WriteAsciiString(g, index, r,c);
    }
}

int GetNextStringRow(DataTableIndex& index, int currentRow)
{
    int r = currentRow+1;
    int csvRowCount = index.getRowCount();

    if (OPT_definecolumn >= 0)
    {
        const char* optionTokenList = OPT_define;

        if (r >= csvRowCount)
            return -1;

        std::string rowTokenList = GetAsciiString(index, r, OPT_definecolumn);
        while (rowTokenList.size() > 0 && !AnyTokenInTokenList(rowTokenList.c_str(), optionTokenList, '|'))
        {
            r++;
            if (r >= csvRowCount)
                return -1;

            rowTokenList = GetAsciiString(index, r, OPT_definecolumn);
        }
    }
    else
    {
        if (r >= csvRowCount)
            return -1;
    }

    return r;
}

int GetFirstStringRow(DataTableIndex& index)
{
	int startRow = OPT_noheaderrow ? 0 : 1;
    return GetNextStringRow(index, startRow-1);
}

int GetStringCount(DataTableIndex& index)
{
    int rowCount = index.getRowCount();

	int startRow = OPT_noheaderrow ? 0 : 1;
    int count    = rowCount - startRow;

    if (OPT_definecolumn >= 0)
    {
        const char* optionTokenList = OPT_define;
        for (int r = startRow; r < rowCount; ++r)
        {
            std::string rowTokenList = GetAsciiString(index, r, OPT_definecolumn);

            if (rowTokenList.size() > 0)
            {
                if (!AnyTokenInTokenList(rowTokenList, optionTokenList, '|'))
                {
                    count--;
                }
            }
        }
    }

    return count;
}

int GetLanguageCount(DataTableIndex& index)
{
    int count = 0;

    if (strlen(OPT_stringcolumns) == 0)
    {
        count = index.getColumnCount();
        if (OPT_idcolumn >= 0)
        {
            count--;
        }
        if (OPT_valuecolumn >= 0)
        {
            count--;
        }
        if (OPT_definecolumn >= 0)
        {
            count--;
        }
    }
    else
    {
        count = RANGE_count(OPT_stringcolumns, k_MinStringColumn,k_MaxStringColumn);
    }
    return count;
}

int IsStringEmpty(const char* p)
{
    int empty = 1;
    while (p && (*p != '\0'))
    {
        if (*p == ' ')
            ++p;
        else
        {
            empty = 0;
            break;
        }
    }
    return empty;
}


void WriteLong(GSTREAM* g, unsigned long l)
{
    unsigned char buf[4];
    gputi(buf,l,4);         // force little endian
    gwrite(g, buf, 4);
}

void WriteChunkID(GSTREAM* g, unsigned long l)
{
    unsigned char buf[4];
    gputm(buf, l, 4);       // IDs is big endian
    gwrite(g, buf, 4);
}


void WriteLine(GSTREAM* g, const char* str)
{
    gwrite(g, (void*)str, strlen(str));
    gwrite(g, "\r\n", 2);
}

void WriteHeaderEntry(GSTREAM* g, const char* label, int value, const char* pathname = NULL, int linenumber = -1)
{
    char line[512];

    if (OPT_cplusplus != 0)
    {
		if (OPT_extraheaderinfo != 0 && pathname != NULL && linenumber >= 0)
		{
			sprintf(line, "const int %-40s = %d; // %s(%d)", label, value, pathname, linenumber);
		}
		else
		{
			sprintf(line, "const int %-40s = %d;", label, value);
		}
    }
    else
    {
 		if (OPT_extraheaderinfo != 0 && pathname != NULL && linenumber >= 0)
		{
	       sprintf(line, "#define %-40s %d /* %s(%d) */", label, value, pathname, linenumber);
		}
		else
		{
	       sprintf(line, "#define %-40s %d", label, value);
		}
    }
    WriteLine(g, line);
}

void WriteHeaderFile(DataTableIndex& index, const char* HeaderPathName, const char* srcPathname)
{
    GSTREAM* g = gwopen(HeaderPathName);
    if (g)
    {
        // write header comment line
        {
            char s[512];
            sprintf(s, "/* automatically generated by %s v%s - http://tools.eac.ea.com/locale/ */", k_AppName, TextVersion);
            WriteLine(g, s);
            WriteLine(g, "");
        }

        // write langauge #defines (unless -noheader specified)
		if (OPT_noheaderrow == 0)
		{
			WriteLine(g, "/* Language ID's */");

			for (int i = 0; i < g_LanguageCount; ++i)
			{
				std::string str = GetAsciiString(index, 0, GetLanguageColumn(i));
				WriteHeaderEntry(g, str.c_str(), i, srcPathname, 0);
			}
			if (OPT_nolanguagecount == 0)
			{
				WriteLine(g, "");
				WriteHeaderEntry(g, "LOCALE_LANGUAGE_COUNT", g_LanguageCount);
				WriteLine(g, "");
			}
			WriteLine(g, "");
		}

        // write string #defines
        WriteLine(g, "/* string ID's */");

        int row = GetFirstStringRow(index);
        for (int i = 0; i < g_StringCount; ++i)
        {
            std::string str = GetAsciiString(index, row, OPT_idcolumn);

            int value = i;
            if (OPT_valuecolumn >= 0)
            {
                std::string valuestr = GetAsciiString(index, row, OPT_valuecolumn);
                value = atoi(valuestr.c_str());  // if we are hear then CheckDuplicates has verified that this is a valid unsigned short
            }

			// offset value by -startat value
			value += OPT_startenumat;

            WriteHeaderEntry(g, str.c_str(), value, srcPathname, index.getLineNumber(row));

            row = GetNextStringRow(index, row);
        }
		if (OPT_nostringcount == 0)
		{
	        WriteLine(g, "");
			WriteHeaderEntry(g, "LOCALE_STRING_COUNT", g_StringCount);
		}
	    WriteLine(g, "");

        gclose(g);
    }
}

void WriteHeaderChunk(GSTREAM* g, int LanguageCount)
{
    int ChunkSize = 16 + LanguageCount*4;

    WriteChunkID(g, LOCALEFILE_HEADERCHUNKID);  // chunkID
    WriteLong(g, ChunkSize);                    // chunk size
    WriteLong(g, (OPT_valuecolumn >= 0) ? 1:0); // flags
    WriteLong(g, LanguageCount);                // bank count

    for (int i = 0; i < LanguageCount; ++i)
    {
        WriteLong(g, 0);
    }
}


int compareid(const void *arg1, const void *arg2)
{
    const unsigned short* s1 = static_cast<const unsigned short*>(arg1);
    const unsigned short* s2 = static_cast<const unsigned short*>(arg2);

    return (*s1) - (*s2);
}

void WriteIndexChunk(DataTableIndex& index, GSTREAM* g)
{
    // calculate size of chunk
    int ChunkSize = sizeof(LOCALEFILE_INDEXCHUNK) // size of fixed fields in chunk
        + g_StringCount*4;      // size of sorted string id list and index values (both stored as 2 byte unsigned shorts)

    // write chunk header
    WriteChunkID(g, LOCALEFILE_INDEXCHUNKID);
    WriteLong(g, ChunkSize);                
    WriteLong(g, g_StringCount);             
    WriteLong(g, 0); // reserved value

    // write string id table
    {
        // create string id value table in memory
        int StringIDTableSize = g_StringCount*4;
        unsigned short* pStringIDTable = static_cast<unsigned short*>(galloc(StringIDTableSize));
        unsigned short* pTable = pStringIDTable;
        if (pStringIDTable != NULL)
        {
            memset(pStringIDTable, 0, StringIDTableSize);

            int row = GetFirstStringRow(index);
            for (int i = 0; i < g_StringCount; ++i)
            {
                // copy index into table
                std::string value = GetAsciiString(index, row,OPT_valuecolumn);

                // copy id value and index value into table
                *pTable++ = static_cast<unsigned short>(atoi(value.c_str()));
                *pTable++ = static_cast<unsigned short>(i);

                row = GetNextStringRow(index, row);
            }

            // sort string id value table
            qsort(pStringIDTable, g_StringCount, 4, compareid);

            // write string id value table
            gwrite(g, pStringIDTable, StringIDTableSize);
            gfree(pStringIDTable);
        }
    }
}

void WriteLanguageChunk(DataTableIndex& index, int LanguageIndex, GSTREAM* g)
{
    int StartPos = (int) gtell(g);

    // write chunk header
    WriteChunkID(g, LOCALEFILE_LANGUAGECHUNKID);// chunkID
    WriteLong(g, 0);                            // chunk Size (fill in later)
    WriteLong(g, 0);                            // flags
    WriteLong(g, g_StringCount);                // string count (calculated when header was written)

    // write g_StringCount offset as a place holder.  These will be filled in as each string gets written
    int OffsetIndexPos = (int) gtell(g);
    {
        for (int i = 0; i < g_StringCount; ++i)
            WriteLong(g, 0);
    }

    int row = GetFirstStringRow(index);
    for (int i = 0; i < g_StringCount; ++i)
    {
        // write offset as index
        int StringOffset = (int) gtell(g);
        gseek(g, OffsetIndexPos);
        WriteLong(g, StringOffset - StartPos);
        OffsetIndexPos += 4;            // update offset postion to next index location

        // write string
        gseek(g, StringOffset);
        {
            int r = row;
            int c = GetLanguageColumn(LanguageIndex);
            WriteString(g, index, r,c);

            // write null at the end of the string
            int nul = 0;
            if (index.getEncoding() == DataTableIndex::k_EncodingAscii)
            {
                gwrite(g, &nul, 1);
            }
            else
            {
                gwrite(g, &nul, 2);
            }
        }

        row = GetNextStringRow(index,row);
    }

    // ensure that language chunk ends on a 32 bit boundary
    int EndPos = (int) gtell(g);
    while (EndPos % 4 != 0)
    {
        int fill = 0;
        gwrite(g, &fill, 1);
        EndPos = (int) gtell(g);
    }
    int ChunkSize = EndPos - StartPos;

    // update chunk size
    gseek(g, StartPos + 4);
    WriteLong(g, ChunkSize);

    // update header offset
    if (OPT_multifile)
    {
        LanguageIndex = 0;
    }
    gseek(g, 16+4*LanguageIndex);
    WriteLong(g, StartPos);

    gseek(g, EndPos);
}

void WriteSingleLocaleFile(DataTableIndex& index, const char* LocPathName)
{
    GSTREAM* g = gwopen(LocPathName);
    if (g)
    {
        WriteHeaderChunk(g, g_LanguageCount);

        if (OPT_valuecolumn >= 0)
        {
            WriteIndexChunk(index, g);
        }

        for (int i = 0; i < g_LanguageCount; ++i)
        {
            WriteLanguageChunk(index, i, g);
            Zprintf(2, ".");
        }

        gclose(g);
    }
}

void WriteMultiLocaleFile(std::string LocPathName, DataTableIndex& index, int LanguageIndex)
{
	// append filename from first cell in the column, ie, IDL_ENGLISH
	LocPathName.append(GetAsciiString(index, 0, GetLanguageColumn(LanguageIndex)));

    // append the .loc extension
    LocPathName.append(".loc");

    GSTREAM* g = gwopen(LocPathName.c_str());
    if (g)
    {
        WriteHeaderChunk(g, 1);
        if (OPT_valuecolumn >= 0)
        {
            WriteIndexChunk(index, g);
        }
        WriteLanguageChunk(index, LanguageIndex, g);
        Zprintf(2, "\n%s", LocPathName.c_str());
        gclose(g);
    }
}

void WriteLocaleFile(DataTableIndex& index, const char* StringPathName)
{
    if (OPT_multifile == 0)
    {
        WriteSingleLocaleFile(index, StringPathName);
    }
    else
    {
		std::string LocPathName(StringPathName);
		// truncate all characters after the last /
		std::string::size_type pos = LocPathName.rfind(SPITDIRSYM);
		if (pos == std::string::npos)
		{
			// no pathname specified, clear the entire string
			LocPathName.erase();
		}
		else
		{
			// erase all the charcers from pos to end
			LocPathName.erase(LocPathName.begin() + pos + 1, LocPathName.end());
		}

        for (int i = 0; i < g_LanguageCount; ++i)
        {
            WriteMultiLocaleFile(LocPathName, index, i);
        }
    }
}

///////////////////////////////////////////////////////////////////////////
// check for duplictes routines

static bool IsValidUShort(const char* pNumber)
{
    if (strlen(pNumber) <= 0)
        return false;

    const char* str = pNumber;
    // check to see if every character is a digit
    while (*str != '\0')
    {
        if (!isdigit(*str))
        {
            return false;
        }
        ++str;
    }

    // check to see if the number is the range of an unsigned short
    int i = atoi(pNumber);
    if (i < 0 || i > 0xffff) // 65535
        return false;

    return true;
}

bool CheckForDuplicates(DataTableIndex& index, const char* SourcePathName)
{
    Zprintf(2, ", Checking");

    if (OPT_createheader != 0 && OPT_idcolumn < 0)
    {
        Zprintf(1, " - ERROR (no files created)\n");
        Zprintf(1, "header (.h file) cannot be created unless a valid id column is specified\n");

        Zprintf(1, "id column is currently specified as column %d\n", OPT_idcolumn);
        return true;
    }

    // return true if duplicate language id is found
    // return true if duplicte string id is found
    // return true if duplicte string id value is found
    // return true if string id is missing (strlen()<=0) AND check is active

    // check language id's
    if (OPT_createheader != 0)
    {
        for (int i = 0; i < g_LanguageCount-1; ++i)
        {
            std::string id = GetAsciiString(index, 0, GetLanguageColumn(i)); // set to the first cell with a language id in the first column

            // check to see if id exist as any other language id
            for (int j = i+1; j < g_LanguageCount; ++j)
            {
                std::string testid = GetAsciiString(index, 0, GetLanguageColumn(j));
                if (testid.compare(id) == 0)
                {
                    Zprintf(1, " - ERROR (no files created)\n");
                    Zprintf(1, "%s(1) : duplicate language id %s (columns %d and %d)\n",
                        SourcePathName, id.c_str(), GetLanguageColumn(i),GetLanguageColumn(j));
                    return true;
                }
            }
        }
    }

    // check string id's and id values
    int row = GetFirstStringRow(index);
    for (int i = 0; i < g_StringCount; ++i)
    {
        if (i % 10 == 0)
            Zprintf(2, ".");

        std::string id;
        std::string value;

        if ((OPT_idcolumn >= 0) && (OPT_createheader != 0))
        {
            id = GetAsciiString(index, row,OPT_idcolumn); // set to the first cell in the current row

            if (id.size() <= 0)
            {
                Zprintf(1, " - ERROR (no files created)\n");
                Zprintf(1, "%s(%d) : string id empty (must be a unqiue non-empty string)\n", SourcePathName,row+1);
                return true;
            }
        }

        if (OPT_valuecolumn >= 0)
        {
            value = GetAsciiString(index, row,OPT_valuecolumn); // set to the first cell in the current row

            if (!IsValidUShort(value.c_str()))
            {
                Zprintf(1, " - ERROR (no files created)\n");
                Zprintf(1, "%s(%d) : bad string id index '%s' (must be unique integer in range of 0..65535)\n", SourcePathName,row+1, value.c_str());
                return true;
            }
        }

        // don't check for duplicates when we are the last item
        // we need to do the previous checks though so we can't use the -1 trick in the language check above

        if (i < g_StringCount-1)
        {
            // check to see if id exist as any other string id
            int nextrow = GetNextStringRow(index, row);
            for (int j = i+1; j < g_StringCount; ++j)
            {
                if (OPT_createheader != 0)
                {
                    std::string testid = GetAsciiString(index, nextrow,OPT_idcolumn);
                    if (testid.size() <= 0)
                    {
                        Zprintf(1, " - ERROR (no files created)\n");
                        Zprintf(1, "%s(%d) : string id empty (must be a unqiue non-empty string)\n", SourcePathName,nextrow+1);
                        return true;
                    }

                    if (testid.compare(id) == 0)
                    {
                        Zprintf(1, " - ERROR (no files created)\n");
                        Zprintf(1, "%s(%d) : duplicate string id '%s' (rows %d and %d)\n", SourcePathName,i+2, id.c_str(),i+2,j+2);
                        Zprintf(1, "%s(%d) :\n", SourcePathName,j+2);
                        return true;
                    }
                }

                if (OPT_valuecolumn >= 0)
                {
                    std::string testValue = GetAsciiString(index, nextrow,OPT_valuecolumn);
                    if (!IsValidUShort(testValue.c_str()))
                    {
                        Zprintf(1, " - ERROR (no files created)\n");
                        Zprintf(1, "%s(%d) : bad string id index '%s' (must be unique integer in range of 0..65535)\n", SourcePathName,nextrow+1, testValue.c_str());
                        return true;
                    }


                    if (testValue.compare(value) == 0)
                    {
                        Zprintf(1, " - ERROR (no files created)\n");
                        Zprintf(1, "%s(%d) : duplicate string id value '%s' (rows %d and %d)\n", SourcePathName,i+2, testValue.c_str(),i+2,j+2);
                        Zprintf(1, "%s(%d) :\n", SourcePathName,j+2);

                        return true;
                    }
                }
                nextrow = GetNextStringRow(index, nextrow);
            }
        }
    }

    return false; // no duplicate found
}

///////////////////////////////////////////////////////////////////////////
// main program

void printoptions(void)
{
    if (OPTverbose >= 4)
    {
        printf("\nOptions:\n========\n");
        printf("OPT_createheader   = %d\n", OPT_createheader);
        printf("OPT_cplusplus	   = %d\n", OPT_cplusplus);
        printf("OPT_idcolumn	   = %d\n", OPT_idcolumn);
        printf("OPT_valuecolumn    = %d\n", OPT_valuecolumn);
        printf("OPT_multifile      = %d\n", OPT_multifile);
        printf("OPT_stats          = %d\n", OPT_stats);
        printf("OPT_definecolumn   = %d\n", OPT_definecolumn);
        printf("OPT_define         = '%s'\n", OPT_define);
        printf("OPT_stringcolumns  = '%s'\n", OPT_stringcolumns);
        printf("OPT_leftcomment    = '%c' 0x%02x\n", OPT_leftcomment,OPT_leftcomment);
        printf("OPT_rightcomment   = '%c' 0x%02x\n", OPT_rightcomment,OPT_rightcomment);
        printf("OPT_escapesequence = %d\n", OPT_escapesequence);
        printf("OPT_comments       = %d\n", OPT_comments);
        printf("OPT_noheaderrow    = %d\n", OPT_noheaderrow);
        printf("OPT_nostringcount   = %d\n", OPT_nostringcount);
        printf("OPT_nolanguagecount = %d\n", OPT_nolanguagecount);
        printf("OPT_startenumat    = %d\n\n", OPT_startenumat);

        for (int i = 0; i < SPITargnum; ++i)
        {
            printf("SPITsargv[%d] = '%s' SPITdargv[%d] = '%s'\n", i,SPITsargv[i], i,SPITdargv[i]);
        }
    }
}

void printfile(DataTableIndex& index)
{
    if (OPTverbose >= 5)
    {
        int rowCount = index.getRowCount();
        int colCount = index.getColumnCount();

        for (int r = 0; r < rowCount; ++r)
        {
            printf("%4d:=================================================\n", r);
            for (int c = 0; c < colCount; ++c)
            {
                printf("%4d,%4d: '%s'\n", r,c, GetAsciiString(index, r,c).c_str());
            }
            printf("\n");
        }
    }
}

bool checkoptions(void)
{
    bool ok = true;

    if ((OPT_delimiter == DataTableIndex::k_DelimiterComma) && (OPT_encoding != DataTableIndex::k_EncodingAscii))
    {
        Zprintf(1, "ERROR: CSV files can only be ASCII encoded\n");
        ok = false;
    }

    return ok;
}

int main(int argc, char** argv)
{
    // init libs
    SPIT_init(SPIT_reset, SPIT_usage, SPIT_opt, NULL);

    int more;
    do
    {
        more = SPIT_parse(argc, argv, k_AppName, TOOL_VERSION);

        printoptions();


        for (int i = 0; i < SPITargnum; ++i)
        {
            if (checkoptions())
            {
                // read source file
                Zprintf(2, "%s: Reading", SPITsargv[i]);
                char* pData = FILE_readtextfile(SPITsargv[i]);

                // JF 02/21/04 check for a UNICODE file.  I am not 100% sure
                // this will work in all cases.  There's a comment in 
                // DataTableIndex::parse that unicode files only *sometimes*
                // have this in them, and it may indicate text direction.  In
                // any case, files produced by Microsoft Excel have it and
                // we can probably safely use it.  If not, someone will surely
                // complain. :)
                if (*reinterpret_cast<uint16_t *>(pData) == UNICODE_SIGNATURE)
                {
                    if (OPT_encoding == DataTableIndex::k_EncodingAscii)
                    {
                        printf("locomoto error: -ascii was specified as the input file type but the input data file is in unicode format.\n");
                        continue;
                    }

                    if (OPT_delimiter == DataTableIndex::k_DelimiterComma)
                    {
                        printf("locomoto error: -csv was specified as the delimiter type but a unicode file requires a tab delimiter.\n");
                        continue;
                    }
                }
                else
                {
                    // Make sure they didn't tell us this is a UNICODE file
                    if (OPT_encoding == DataTableIndex::k_EncodingUnicode)
                    {
                        printf("locomoto error: the file type appears to be ASCII, but -ascii was not specified as the input file type.\n");
                        continue;
                    }

                    // ASCII files can have either tab or comma-delimited fields.
                }
                
                if (pData != NULL)
                {
                    DataTableIndex index(pData, OPT_delimiter, OPT_encoding, OPT_comments ? ';' : 0);

					// rcg: Check that columns are accessible.
					int nCols = index.getColumnCount();

					#define IS_COLUMN_BAD(_colid) ((_colid) >= nCols)

					if( IS_COLUMN_BAD(OPT_definecolumn) ||
						IS_COLUMN_BAD(OPT_idcolumn) ||
						IS_COLUMN_BAD(OPT_valuecolumn))
					{
						printf("\nerror: bad column index specified!\n");
						gfree(pData);
						more = 0;
						break;
					}

                    printfile(index);

#ifdef _DEBUG
					index.printSize();
#endif

                    g_StringCount   = GetStringCount(index);
                    g_LanguageCount = GetLanguageCount(index);

#ifdef _DEBUG
					index.printSize();
#endif

                    if (AllocColumnMap(index.getColumnCount()))
                    {
                        // check for duplicate id's
                        if (OPT_checkduplicates && CheckForDuplicates(index, SPITsargv[i]))
                            break;

                        Zprintf(2, ", Writing");

                        // create header file
                        if (OPT_createheader)
                        {
                            Zprintf(2, " h file,");
                            char HeaderPathName[SPITMAXFNAME];
                            SPIT_changeext(HeaderPathName, SPITdargv[i], "h");
                            WriteHeaderFile(index, HeaderPathName, SPITsargv[i]);
                        }

                        // create string banks
                        Zprintf(2, " loc file");
                        WriteLocaleFile(index, SPITdargv[i]);

                        // write results
                        Zprintf(2, " (%sindex, %s%d languages, %d strings)\n", 
                            (OPT_valuecolumn >= 0) ? "" : "no ",
                            OPT_cplusplus ? "cpp, " : "",
                            g_LanguageCount, 
                            g_StringCount);

                        FreeColumnMap();
                    }

                    // free data
                    gfree(pData);
                }
                else
                {
                    Zprintf(1, " Could not open file %s\n", SPITsargv[i]);
                }
            }
        }
    } while (more > 0);
    SPIT_restore();

    return 0;
}
