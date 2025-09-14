// (c) Electronic Arts. All Rights Reserved.

/*------------------------------------------------------------------*/
/*                                                                  */
/*            Small Programs' Interface Tool (SPIT) v4.02           */
/*                                                                  */
/*                     by FrANK G. Barchard, EAC                    */
/*                                                                  */
/*                           Jul 23, 2008                           */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History:                                       */
/* ------- ----   -- --------                                       */
/*    1.00 901120 FB glorified font switch parser                   */
/*    1.10 901124 FB application option handler                     */
/*    1.11 901126 FB builtin options                                */
/*    1.14 901203 FB environment default options                    */
/*    1.15 901205 FB expand amiga wildcards                         */
/*    1.16 901206 FB expand ST/msdos wildcards                      */
/*    1.17 901212 FB don't expand non-wildcards                     */
/*    1.18 901212 FB rule string manipulation for SPITchangeext     */
/*    1.19 901216 FB block file read for long files on IBM          */
/*    1.20 901218 FB path rule                                      */
/*    1.21 901219 FB filter out 'TMPSPITX' from amiga wildcard list */
/*    1.22 901219 FB SPITsargv, SPITdargv list format               */
/*    1.23 901220 FB options->userbuf/files only for amiga wildcard */
/*    1.24 910109 FB -f option removed - use & instead              */
/*    1.25 910110 FB memsame added to spit                          */
/*    1.26 910113 FB strsame                                        */
/*    1.27 910123 FB memcopy                                        */
/*    1.28 910125 FB text formatted savefile option                 */
/*    1.29 910129 FB check stack size/return error codes to OS      */
/*    1.30 910130 FB SPIT_opts environment variable                 */
/*    1.31 910130 FB -v-1 no banner option                          */
/*    1.32 910130 FB CRC function                                   */
/*    1.33 910131 FB long options                                   */
/*    1.34 910205 FB percentstr, write modes                        */
/*    1.37 910206 FB SPITSTACK specify amount of stack to allocate  */
/*    1.38 910211 FB write to screen and file with hex option       */
/*    1.39 910212 FB read/write stats in load & save                */
/*    1.40 910212 FB spitlist return value for OS                   */
/*    1.41 910212 FB extension list for loadfile                    */
/*    1.42 910215 FB .bit spit Response file default extension      */
/*    1.43 910218 FB getstropt string options with rules            */
/*    1.44 910218 FB getfield with nested delimiters                */
/*    1.45 910219 FB end of sentence after al  defaults/environment */
/*    1.46 910224 FB delimited fields within undelimited fields     */
/*    1.48 910225 FB getfield for filenames                         */
/*    1.50 910319 FB getpath (v1.49 lost)/expand into full pathnames*/
/*    1.51 910326 FB @ and ! not wildcards                          */
/*    1.52 910402 FB create environment var name from program name  */
/*    1.53 910404 FB upper case OPTS                                */
/*    1.55 910406 FB openr with pathlist and extlist                */
/*    1.56 910407 FB pathlist for loadfile                          */
/*    1.57 910422 FB readfile bug fixed                             */
/*    1.58 910430 FB null pathrule to disable last arg directory    */
/*    1.59 910508 FB msc6.0 warning fixes, better strsame           */
/*    1.60 910509 FB fixed bug in new strsame                       */
/*    1.61 910512 FB system exec call                               */
/*    1.62 910514 FB getstropt allow '='                            */
/*    1.63 910522 FB stackavail using address of options            */
/*    1.64 910523 FB crctab public/accurate stack check/string rule */
/*    1.65 910524 FB smart extension trying                         */
/*    1.67 910529 FB ? for help, ?? for lots of help                */
/*    1.68 910618 FB -percent same as +percent in opposite dir      */
/*    1.69 910619 FB try to prevent dest name same as source name   */
/*    1.70 910624 FB amiga detect 'object not found' for wildcards  */
/*    1.71 910702 FB remove ext or _OPTS/-. extrule bug fix         */
/*    1.72 910703 FB loadfilemax to load n bytes of a file          */
/*    1.73 910801 FB don't avoid name conflicts if not writing file */
/*    1.76 910910 FB / for options for /? in MSDOS 5.0              */
/*    1.77 910915 FB /? give terse help                             */
/*    1.78 910925 FB -q for verbose level (quiet)                   */
/*    1.79 911008 FB (char)' ' bugfix                               */
/*    1.80 911016 FB Watcom C support                               */
/*    1.81 911021 FB Usage passes options for access to ext rule    */
/*    1.82 911021 FB delim goop removed/st version debugged         */
/*    1.83 911021 FB collision detect removed/=.ext                 */
/*    1.84 911104 FB spit stripped down/& changed to @ like watcom  */
/*    1.85 911111 FB spit .c file to create .o linkable file        */
/*    1.86 911112 FB require space after . to allow =. & ..\dir\    */
/*    1.87 911113 FB .directives not parsed/getfield eof check      */
/*    1.88 911119 FB translate crlr for console output/& or @ rsp   */
/*    1.89 911122 FB default extension routine/spitopts static      */
/*    1.90 911122 FB major revamp - Znaming, static vars            */
/*    1.91 911126 FB -b banner/-v verbose/nO for O0                 */
/*    1.92 911127 FB strsame removed, use stricmp now/enviro alias  */
/*    1.93 911204 FB 'Writing as' msg/SPIT_getfield bug fix         */
/*    1.94 911205 FB getenv for environment aliases (for ST)        */
/*    1.95 911206 FB append name to each include                    */
/*    1.96 911216 FB print option value -o?                         */
/*    1.97 911217 FB allow zero names in list for info              */
/*    2.00 920501 FB Microsoft overlayable calling conventions      */
/*    2.01 920618 FB Wildcards back in (removed for V2.00)          */
/*    2.02 920921 FB Write text to stdout with proper \n (had bug)  */
/*    2.03 920922 FB @envvar (watcom wmake compatible)              */
/*    2.04 920923 FB stricmp                                        */
/*    2.05 920925 FB Zwildcard for expand wildcards on MSDOS        */
/*    2.06 920928 FB if wildcard fails, use expression asis         */
/*    2.07 930708 FB moved stricmp define to machine.h              */
/*    2.08 930723 GS added ZByteFormat, Zputm handles longs         */
/*    2.09 930726 FB moved car to spit.c and renamed to OPTintel    */
/*    2.10 930812 FB Spin Cursor in for Mac - allows abort          */
/*    2.11 930903 FB Small model support                            */
/*    2.12 930910 GS envvar is converted to uppercase with toupper  */
/*    2.13 930921 FB `tool` to include stdout of tool in args       */
/*    2.14 930922 GS added SPIT_geti/Zputi, removed OPTintel        */
/*    2.15 931015 FB use lower case for filenames, for unix on SGI  */
/*    2.16 931019 FB - for options on SGI (not /) like Amiga        */
/*    2.17 931112 FB Semantic C for Mac MPW                         */
/*    2.18 931113 FB strnicmp C for Mac MPW                         */
/*    2.19 931117 FB wildcard expansion on PC converted tolower     */
/*    2.20 931130 FB spin cursor before fflush - more abortable     */
/*    2.21 931208 FB SPIT_rename                                    */
/*    2.22 931209 FB Allow .\ for dest dir on PC, -h for help       */
/*    2.23 931220 FB Mac file system io (ie Zfastopenw)             */
/*    2.24 940222 FB Preserve Mac spaces in filenames               */
/*    2.25 940301 FB -w8 for pause per page                         */
/*    2.26 940307 FB SPIT_getfield without strip delimiters         */
/*    2.27 940408 FB major reduction, ANSIC, machine.h removed      */
/*    2.28 940411 FB iswildcard to speed up wildcard expansion      */
/*    2.29 940411 FB bigger buffer for more files                   */
/*    2.30 940422 FB SPIT_blockmove                                 */
/*    2.31 940425 FB show parsed line for debugging                 */
/*    2.32 940609 FB Zwritefile clip console output to SPITMAXLINE  */
/*    2.33 940806 FB WCL 16 bit support                             */
/*    2.34 940816 FB OPTdestdir flag to enable/disable checking dir */
/*    2.35 950220 FB removed cdecl from all spit tools and headers  */
/*    2.36 950322 GS added Visual C support, removed MSC suppport   */
/*    2.37 950529 GS NT/Win95 wildcard support                      */
/*    2.38 950808 FB MWERK command line fetcher                     */
/*    2.39 950824 FB bump option verbose to 5                       */
/*    2.40 950911 FB fixed findnext                                 */
/*    2.41 950927 FB removed =.ext which conflicts with =.\name.ext */
/*    2.50 951002 FB rewrite using file/mem based on gimex api      */
/*    2.51 951007 FB MSVC friendly                                  */
/*    2.52 951008 FB Watcom win95 wildcards/SPITmachine             */
/*    2.53 951104 FB remove hard spaces from string options         */
/*    2.54 951106 FB OPTkeepopts to keep options in command line    */
/*    2.55 960113 FB -help for web help win95 and sgi               */
/*    2.56 960204 FB -sort names 1 alpha, 2 sgi numerical           */
/*    2.57 960220 FB help use start for win95                       */
/*    2.58 960318 FB OPTsortfirst, -sort 1 num, 2 alpha             */
/*    2.59 960423 FB -sort override user -s, fix in alnum compare   */
/*    2.60 960425 FB getopt allow % to follow number                */
/*    2.61 960509 FB remove extension naming. ie -.ext              */
/*    2.62 961009 FB use range api for wildcards                    */
/*    2.64 961114 FB iswildcard false for -opt                      */
/*    2.65 961118 FB -h reguardless of numargs                      */
/*    2.66 970109 FB count arguments with keepopts off too          */
/*    2.67 970210 FB -log0 to disable socket app logging            */
/*    2.68 970306 FB allow ^& for nt/95 compatibility               */
/*    2.69 970421 FB bumped to 1024 byte max lines                  */
/*    2.70 970827 FB getfield work properly on (parenthesis)        */
/*    2.71 980211 SK dynamically allocates SPITsargv,SPITdargv      */
/*    2.72 980216 SK -r, wildmatch.c, findfile.c                    */
/*    2.73 980310 SK SPITattribargv etc. OPTdirectory etc,qsortdate */
/*    2.74 980430 SK Improved speed of directory recurse            */
/*    2.75 980513 SK Added check for OPTkeepunfound to speed up     */
/*    2.76 980528 SK added SPIT_strstr, fixed OPTuseshortnames err  */
/*    2.77 980529 SK added OPTuselongnames to force long names      */
/*    2.78 980602 SK Fixed bug in Zoptionhelp function              */
/*    2.79 980615 SK env. vars. in response files works             */
/*    2.80 980616 SK parameters to response files, added OPThistory */
/*    2.81 980625 SK export SPIT_qsort; no case match in file name  */
/*    2.82 980708 SK SPIT_responsefile recognizes nested            */
/*    2.83 980717 SK Exports SPIT_typeofname function now.          */
/*    2.84 981029 FB SPIT_ispath require single drive letter ie c:  */
/*    2.85 981105 FB pipe into spit - treats as response file       */
/*    2.86 981117 FB -= and -. use next field to tolerate space     */
/*    2.87 990129 FB SPIT_ naming convention                        */
/*    2.88 990212 FB INT64 in spit.h                                */
/*    2.89 990413 GS fixed some string init bugs & warnings         */
/*    2.90 990528 LC find target if file is shortcut                */
/*    2.91 990607 LC ignore responsefile errors if con:             */
/*    2.92 990712 LC target directory can be anywhere in cmd line   */
/*    2.93 990714 LC OPTversion check if tool is more recent        */
/*    2.94 990720 FB SPIT_VERSION, SPIT_MACHINE                     */
/*    2.95 990816 FB directory detect rewritten                     */
/*    2.96 990917 FB sgi/linux shared __unix__ define               */
/*    2.97 991008 FB -nologo instead of -b0                         */
/*    2.98 000104 FB SPIT_url returns url of help                   */
/*    2.99 000125 FB SPIT_getopt2 default parm, OPTwrite=0 or 1     */
/*    3.00 000401 FB BeOS support __BEOS__ mostly unix              */
/*    3.01 000424 FB -time parse option in spit                     */
/*    3.02 000502 FB = treated like option on first option pass     */
/*    3.03 000526 FB history is SPIT_init argument                  */
/*    3.04 000601 FB getfield += -= == are not delimiters           */
/*    3.05 000614 FB apply name rules to nameless dargvs only       */
/*    3.06 000616 FB apply rules fix                                */
/*    3.07 000622 FB error messages say SPIT, so we know who dunnit */
/*    3.08 000713 FB options lowercase an option                    */
/*    3.09 000829 LC sorts with paths regardless of order of args   */
/*    3.10 000913 FB timeofdate for unix                            */
/*    3.11 000918 LC can now get wildcards for UNC paths            */
/*    3.12 000922 LC corrected sort documentation and qsort param   */
/*    3.13 000926 FB linux ifdef __i386__                           */
/*    3.14 000928 FB OSX ifdef __APPLE__                            */
/*    3.15 001017 LC fixed recurse on unix (findfile)               */
/*    3.16 001024 FB a&&b is expr, not response file                */
/*    3.17 001208 FB quote args for spaces                          */
/*    3.18 010220 ID -=dir\ =dir\ dir\ support fix                  */
/*    3.19 010222 ID -r recursion fix...                            */
/*    3.21 010412 ID added a=b=c binding to spit                    */
/*    3.22 010416 ID fixed up a =b =c =d binding                    */
/*    3.23 010517 FB -help point to new site                        */
/*    3.24 010528 ID spit now respect linux paths better, lastpath  */
/*    3.25 010612 ID fixed response file clumping with ""           */
/*    3.26 010720 LC fixed response file looking for null char      */
/*    3.27 010813 FB dont quote if arg contains " -" (for gx.lib)   */
/*    3.28 010917 FB c++ and shortcut code included                 */
/*    3.29 010925 FB extern "C" in header                           */
/*    3.30 010928 ID fixed issue with relative drive letter         */
/*    3.31 011001 FB version check done with ints                   */
/*    3.32 011009 FB bool                                           */
/*    3.33 011016 FB INT64 ps2                                      */
/*    3.34 011025 ID forced make optlist and assoc glob fix rerun   */
/*    3.35 011030 FB static for internals                           */
/*    3.36 011101 FB UINT64, more statics                           */
/*    3.37 011105 FB FORMAT64                                       */
/*    3.38 011108 FB CONST64                                        */
/*    3.39 011220 FB getopt2 dont clamp if min/max int              */
/*    3.40 020117 ID increased buffer size                          */
/*    3.41 020122 ID don't find directories by default              */
/*    3.42 020212 ID fixed issue with recursion keeping directory   */
/*    3.43 020214 ID fixed issue with recursion and outputfile      */
/*    3.44 020219 ID stopped one instance of infinite loop bug      */
/*    3.45 020221 FB return no more if helpgiven                    */
/*    3.46 020221 ID handle spaces in the output filename           */
/*    3.47 020226 ID new option for gxlib don't use hardspace       */
/*    3.50 020413 FB getfield delimit (1+3)*2 will get full field   */
/*    3.51 020414 FB getfield ='bob' dont strip inner quotes        */
/*    3.52 020416 FB -? show full help                              */
/*    3.53 020516 FB dont detect/expand suspitious shortcuts        */
/*    3.54 020527 ID -strippaths option added for output filename   */
/*    3.55 020703 ID FindRecursive no longer finds files twice      */
/*    3.56 020715 ID FindRecursive now uses OS / style for matches  */
/*    3.57 020717 FB SPIT_tempname append .bak                      */
/*    3.58 020717 FB SPIT_gettime cast carefully for vc             */
/*    3.59 020801 FB stdgimex 64 mac                                */
/*    3.60 020802 FB SPIT_percentstr reversable                     */
/*    3.61 020930 FB folder recursion case match                    */
/*    3.62 021002 FB SPIT_MACHINE PC instead of Windows             */
/*    3.63 021004 FB SPIT_parse version #, bloodhound               */
/*    3.64 021103 FB SPIT_loadstr, SPIT_load terminate for wchar_t  */
/*    3.65 030507 FB stripped out spit public header to whats used  */
/*    3.66 030604 FB one dest dir not special                       */
/*    3.67 030817 FB getfield dont start ' delimiter in midstring   */
/*    3.68 030825 FB SPITMAXLINE 16k for ea3x                       */
/*    3.70 030828 FB faster add path, falloc                        */
/*    3.72 031022 FB FollowDirs use galloc to avoid stack overflow  */
/*    3.73 031201 FB option parse use proper tolower                */
/*    3.74 031213 FB sort fixed for files that start with .         */
/*    3.75 040711 FB sort fixed for OPTstartsortat for glump        */
/*    3.76 040726 FB getfield strip matching lim, not delim         */
/*    3.77 040802 FB remove attrib,date,shortname                   */
/*    3.80 040809 FB verbose 1/2 no output per file                 */
/*    3.90 041102 FB sort directory sensitive fix                   */
/*    3.91 050520 FB -nologo fix, utf8 friendly, undef galloc       */
/*    3.92 050605 FB stdgimex self contained (not gimex.h)          */
/*    3.93 060426 FB ps3 extern c avoid string.h in headers         */
/*    3.94 060717 FB AddMatch alloc 1024 entries at a time          */
/*    3.95 060801 FB sort respect options and fix for numbers       */
/*    3.96 061116 FB sort allow first character to be digits        */
/*    3.97 070319 FB amd64, bloodhound removed                      */
/*    3.98 070710 FB -uni for unique names                          */
/*    3.99 071016 FB -sort2 for alphanumerical with variations      */
/*    4.00 080312 FB ps2 intptr_t, size_t for 64 bit, 100+ deep     */
/*    4.01 080704 FB -sort3 simple strcmp (was historially date)    */
/*    4.02 080723 FB -recurse:folder                                */
/*    4.03 090528 JF removal of stdgimex.h from spit header         */
/*                                                                  */
/*------------------------------------------------------------------*/

/*
ABSTRACT

spit - Simple Program Interface Toolkit
@Spit
@SPIT

DESCRIPTION

       ------------------------------------------------------------------

                   Simple Program Interface Toolkit (SPIT) v4.02

                  Copyright (c) 1992-2005 Electronic Arts Canada
                   Copyright (c) 1990,1991 by FrANK G. Barchard

                         Documentation - Jun 11, 2005

        ------------------------------------------------------------------


          Table of Contents



          1.  Introduction ......................................    2

          2.  Built-in Options ..................................    3

          3.  Filenames .........................................    8
            3.1.  Simple Filenames ..............................    8
            3.2.  Wildcard Expressions ..........................    8
            3.3.  Advanced Wildcard Expressions .................    9

          4.  Response Files ....................................   10

          5.  Environment Variables .............................   10

          6.  Advanced Topics ...................................   12
            6.1.  String Rules ..................................   12
            6.2.  Sentences .....................................   13
            6.3.  Global Options ................................   13
            6.4.  GUI Usage .....................................   14
            6.5.  Delimiters ....................................   14
            6.6.  Lists .........................................   14

          Appendix A -- Quick Options Reference .................   15

          Appendix B -- Grammar Reference .......................   15






          SPIT Introduction                                          2


               THE  SCOPE  OF THIS DOCUMENT is limited to a functional
          description  of the Spit user interface.  This document does
          not  cover how to write Spit applications, nor does it cover
          DOS,  which  the user is assumed to have a working knowledge
          of.  For more information, please contact the author.




          1.  Introduction


               The  Simple Program Interface Toolkit,  refered  to as
          'Spit'  herein,  is  a command line and Response file processor
          that  provides  a  portable and consistant user interface to
          command  line driven utilities.  From the perspective of the
          user,  Spit standardizes how options and files are passed to
          and from applications.  Spit's primary features are:

               o  Machine independence
               o  Usage templates
               o  Options parsing
               o  Response file processing
               o  I/O handling/redirection
               o  Wildcard handling
               o  GUI support
               o  Environment variables



               THE  DESIGN  PHILOSOPHY  of the spit command line is an
          order   independent   list  of  source  files  and  options.
          Derivative   output   files   are   not  normally  specified
          explicitely  (although  they can be optionally).  The reason
          for  this  is  to  allow  wildcard  and GUI specification of
          files.   For  example,  we  might  launch  an application by
          dropping  several  files  into  it.  The output files of the
          application  will be named based on rules.  (See section 6.1
          String Rules)


           PARSING a command line occurs in the following sequence:

               o  Default options are set
               o  Each argument is parsed until the line is finished
                    o  If it is a filename, it is added to the list
                    o  If it is an option, it is parsed
               o  Each filename is expanded with wildcards
               o  The  name  list  and   options  are  passed  to  the
                  application

               Multiple  command  lines can be executed, in which case
          this sequence is repeated.  (See section 6.2 Sentences)



          SPIT Built-in Options                                      3



          2.  Built-in Options


               The  options  that follow are built into Spit, and thus
          are  available in all Spit applications.  The format of each
          option is indicated by example.  Most applications will have
          additional options that follow similar formats.  Running the
          application  in  question  will  list  available options and
          formats.

               Note  that  if  a numerical option is specified with no
          value,  then  the value for that option in incremented.  For
          most  numerical  options,  the default value and the value+1
          are  carefully  selected  so  that  this feature can be used
          often.

          EXAMPLES: spit_eg -v bob.txt
                    spit_eg -v2 bob.txt


               o  VERBOSE LEVEL

                  Default 2
                  Format  -v<n>

               The VERBOSE level tells the application how much output
          the user wants.  The values for this option are 0..6, with 0
          meaning  no  output  and 6 meaning the most possible output.
          The default  value is 2 which gives  one line of output.

          EXAMPLE: spit_eg -v3 bob.txt

          The following is a description for each verbose level:

               0 no console output
               1 no output except fatal errors
               2 terse output (default). 1 line summary
               3 verbose output. file level summary
               4 diagnostic output. file level diagnostics
               5 reserved for debugging info
               6 reserved for debugging info


          SPIT Built-in Options                                      4




               o  BANNER ON/OFF

                  Default ON
                  Format  -b<n>

               The  banner  for  an  application  is the applications'
          name,  version number and copyright notice.  This banner can
          be turned on and off.

          EXAMPLE: spit_eg -b0 bob.txt

          The following quiet levels affect the banner:

               0 no banner
               1 banner




               o  WRITE MODE

                  Default 1
                  Format  -w<n>

               File output can be to a file disabled.

          EXAMPLE: spit_eg -w0 bob.txt

          The following is a description for each write mode:

               0 no output
               1 output to file


          SPIT Built-in Options                                      5



               o  DESTINATION PATH

                  Default ""
                  Format  -=<str>

               If  a  destination  path  is  not  specified, files are
          written  to  the  same directory as the source files are in.
          This  option  allows  the  user  to  specify  an alternative
          directory  to write the output files to.  There are two ways
          to specify this option.

          EG 1: spit_eg -=c:\work\ bob.txt bob2.c

          EG 2: spit_eg bob.txt bob2.c c:\work\


               NOTE  that  in example 2, the last argument in the line
          must  end in a ':' or a slash '/' or '\'.


          SPIT Built-in Options                                      7



          SPIT Filenames                                             8



          3.  Filenames

               This    section   describes   the   input/output   file
          specification  for  Spit applications.  Sections 3.1 and 3.2
          are  useful  to  all Spit users, while section 3.3 describes
          the  workings  of  the  wildcard  expansion,  its powers and
          limitations.



          3.1.  Simple Filenames

               The  file names that you normally specify are of source
          files.  The actual file naming conventions vary slightly for
          each machine, but for this section we will assume MSDOS file
          naming  for  the examples.  The following would therefore be
          an example of a valid list of files:

          EXAMPLE: spit_eg apple.s pear.s orange.s

               In  this example, the output files from spit_eg would given
          names  based  on the extension rule for 'spit_eg'.  For example,
          if the extension rule is 'o', then the output files would be
          'apple.o  pear.o  orange.o'.  To explicitely name the output
          file,  the  user  can  follow any source file with '=' and a
          filename.

          EXAMPLE: spit_eg apple.s pear.s=newpear.o orange.s

               The  destination  filename only affects the source file
          preceding it, thus all other files use the extension rule or
          their own explicite destination file names.  To just  change
          the extension for one file you may type file=.ext

          EXAMPLE: spit_eg apple.s pear.s=.o orange.s



          3.2.  Wildcard Expressions

               Wildcard  expressions are supported on all machines, by
          attempting  to  expand  each  name  into  a  list  of names.
          Because  Spit  does  the  wildcard  expansion,  you  may use
          wildcard  expressions  in  command  lines,  Response  files and
          environment  variables,  and  you  may  have  any  number of
          wildcard expressions in a command line.

          EXAMPLE: spit_eg *.s *.c *.asm

               Some  machines  do  wildcard  expansion  for you.  Mac,
          Atari ST, and Xenix for example.  Thus Spit will be passed a
          simple  list  of files.  This is fine, but it may be limited
          in  number  of files it can expand to, and it will only work
          from  the  command  line.   To get Spit to do the expansion,
          rather   than   the  operating  system,  pass  the  wildcard
          expressions in quotes.

          EXAMPLE: spit_eg "*.s *.c *.asm"


          SPIT Filenames                                             9



          3.3.  Advanced Wildcard Expressions

               * matches zero or more characters, including special characters
               ? matches exactly 1 character
               # matches 1 or more digits


          SPIT Response Files                                       10


          4.  Response Files


               Response  files  in  Spit  are simply the same options and
          file  names  that  you  would type in from the command line,
          only they are stored in a text file.  A simple example of a response
          file  would  be  a file containing a list of filenames.  The
          command line format for reading a Response file is:

          EXAMPLE: spit_eg @my.rsp

               The  default  extension for Spit Response Files is '.rsp'.

               To  make  Response  files  more readable, two features are
          available:   white  spaces and comments.  White spaces allow
          you to separate each name or option with spaces, tabs or new
          lines.   Comments are a semi-colon and comment everything to
          the  end  of  line.   The following is an example of a Response
          file's contents.

          EXAMPLE:

               ; bigfile list

               -v1
               test1.dat
               data/file.viv


          5.  Environment Variables


               To  custumize  the way your Spit applications run, Spit
          provides  two  environment  variables with each application.
          The  first that Spit searches for is SPIT_OPTS, which may be
          assigned  a  string  with any Spit options that you want for
          ALL  Spit  applications.   The other environment variable is
          different for each application.  For example, FOO_OPTS would
          have  the  options that apply only to the application 'FOO'.
          The variables should be in upper case for Atari.


          SPIT Environment Variables                                11



               The  actual  command  to  set  an  environment variable
          varies  from  machine  to  machine.  To make the environment
          permanent,  you should put the SET commands in a DOS boot up
          Response  file.   The  following  are  the SET commands for the
          systems Spit currently supports:

               o  SET      for the Windows
               o  SETENV   for the Unix

          EXAMPLE: SET E_OPTS=-e"C:\Program Files\EditPlus 2\editplus.exe"


          SPIT Advanced Topics                                      12


          6.  Advanced Topics

               This  section  is  not  intended for the casual user of
          Spit.  Features in this section explain some of the workings
          of Spit and how to fully exploit the power of Spit.



          SPIT Advanced Topics                                      13



          6.2.  Sentences


               With  a single command line, all files are handled with
          the  same  set  of options and all at the same time.  Rather
          than  reenvoking  the  application  with  a different set of
          files  and  options,  the  user  can terminate a list with a
          SEMICOLIN ';' surrounded by white space(s).  This tells spit to
          stop parsing and call the application.  When the application
          returns,  Spit  resets  all  options  and the name list, and
          starts parsing a new set of filenames and options.


          EXAMPLE: spit_eg -o1 bob.txt ; -o2 bob2.c

                       |       | | |________|____ SECOND SENTENCE
                       |       | |_______________ END OF SENTENCE ONE
                       |_______|__________________FIRST SENTENCE








          6.5.  Delimiters

               For  any  string or numerical field, you may delimit it
          with any one the following special symbols:

               o  ()  []  ""  ''

               If  no  delimiters  are  given, the next white space or
          comma  is  used  to delimit the field.  Note that delimiters
          may be nested.

          EXAMPLE: gsr "-s'this is a test'" "-r'replace()'" bob.txt



          SPIT Appendix A -- Quick Options Reference                15



          Appendix A.  Quick Options Reference

                ;   end of sentence (allows multiple command lines)
                @   file include
                ;   comment to end of line (or arguement)
                =   specify destination file
               -v   quiet level
               -w   write mode
               -=   pathname rule


END ABSTRACT
*/

/****************************************************************/
/*  Spit Include                                                */
/****************************************************************/

#if defined(_MSC_VER)
#pragma warning(push,1)
#pragma warning(disable: 880)
#endif

#include "EAStdC/EAString.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#if defined(_MSC_VER)

#include <shlobj.h>
#include <winbase.h>

#pragma warning(pop)
#endif

#include "spit.h"
#include "findfile.h"
#include "wildmatch.h"

#if SPIT_VERSION < 396
#error SPIT needs SPIT_VERSION 3.96 or later
#endif
#if FINDFILE_VERSION < 102
#error SPIT needs FINDFILE_VERSION 1.02 or later
#endif
#if WILDMATCH_VERSION < 100
#error SPIT needs WILDMATCH_VERSION 1.00 or later
#endif

typedef struct
{
    char *s;                    /* Source name */
    char *d;                    /* Destination name */
    int size;                   /* Size of file */
    time_t date;                /* Date of file */
    char attrib;                /* File attributes */
} Match;

typedef struct
{
    char *wildin;               /* Input wildcard */
    char *wildout;              /* Output wildcard */
    char type;                  /* Type of input wildcard (SPITPATHNORM, SPITPATHABS, SPITPATHDIR, SPITPATHOPT) */
    char matched;               /* Temp variable used to flag whether file is matched in current dir */
    char iswild;                /* Specifies if wildin or wildout uses *, #, or ? */
    char hasout;                /* Has an output file been specified or not */
    int nummatches;             /* Number of files found matching wildin */
    int maxmatches;             /* Matches allocated for */
    Match **matches;            /* Array of match file criterion */
} FileInfo;

typedef struct
{
    FileInfo **files;           /* Array of search file criterion */
    int numfiles;               /* Number of search files */
    int maxfiles;               // Number of search files allocated
} SearchList;
/*******************************************************************/

// I know this is gross, it has to be done as these variables need to be reset to their
// defaults when spit restore is called.
// this is to enable multiple calls. Really spit parse needs to be rewritten, but
// at this point I will move these out to fix the problem temporarily until such
// time as we can rewrite spit in it's entirety.
static char *optptr = NULL;
static char firstcall = 1;
static char *namelist= NULL;
static char *optlist = NULL;



/********* Prototypes for SearchList manipulation ******************/
static SearchList *CreateSearchList(void);
static bool AddMatch(FileInfo *files, Match *newmatch);
static bool AddToSearchList(SearchList *s, char *in, char *out);
static bool RemoveFromSearchList(SearchList *s, int filenum, char *in, char *out);
static bool UpdateSearchList(SearchList *s, int filenum, char *in, char *out);
static void DestroySearchList(SearchList *s);
/*******************************************************************/

/********* File Finding Function Prototypes ************************/
static int FollowDirs(char *in, FileInfo *f, char *path, char *result);
static int FindDirs(SearchList *s, char *startdir, char depth);
static int FindAbsolutes(SearchList *s);
static int FindRecursive(char *startdir, char *origdir, SearchList *s, int depth);
static void MakeArgvs(SearchList *s);
static void ApplyExtRule(SearchList *s);
static void ApplyNameRules(SearchList *s);
static char *GetFirstDir(const char *fname, char *dir);
static void freeFiles(void);
static void replacePath(char *out, char *in, char *path);
void replacePathAbs(char *out, char *in, char *path);
static void replaceExt(char *out, char *in, char *ext);
/*******************************************************************/
static bool SPIT_isnextfieldpath(char *s);
static char *SPIT_makerelative(char *dest, const char *source,char *startdir);
static char *SPIT_makeunrelative(char hasout,char *out,char *startdir,char * complete);
//static void SPIT_figureoutRealPath(SearchList *s,char *path);
static char *SPIT_makerelativeAbs(char *dest, const char *source, char *origdir);

static void  (*SPIT_initfunc)(void) = NULL;
static void  (*SPIT_usagefunc)(const char *, const char *, const char *, int) = NULL;
static char *(*SPIT_optfunc)(const char *) = NULL;

static int initialized = 0;

const char **OPThistory = NULL;
char **SPITsargv;
char **SPITdargv;
int SPITargnum = 0;

extern int galloccount;

/* Setup the WILD_match functions used on different OS's */
#if defined(MSDOS) || defined(_WIN32) || defined(__WATCOMC__) || defined(_MSC_VER)
static int (*pathmatchfun)(char *, const char *, const char *, char *) = WILD_imatchpath;
static int (*stringmatchfun)(char *, const char *, const char *, char *) = WILD_imatch;
#else
static int (*pathmatchfun)(char *, const char *, const char *, char *) = WILD_imatchpath;
static int (*stringmatchfun)(char *, const char *, const char *, char *) = WILD_imatch;
#endif


/****************************************************************/
/*  Spit Options                                                */
/****************************************************************/

int  OPTexpandwildcards = 1;            /* Expand the wildcards */
int  OPTkeepunfound = 1;                /* Keep unfound search strings in x-argv lists */
int  OPTnamecombine = 1;                /* Move source name to destination name position */
int  OPTsorttype = 0;
int  OPTsortbydirs = 0;
int  OPTstartsortat = 0;
int  OPTuni = 0;
int  OPTskiprules = 0;                  /* dont apply rules to first n args */
int  OPToptlower = 1;                   /* make options lower case */
int  OPTstrippaths = 0;                 /* don't strip of paths from outputfilenames by default */

float OPTversion  = 1.00;
int  OPTverbose =   2;
int  OPTwrite =     1;
int  OPTbanner =    1;
int  OPTminargs =   1;
int  OPTdestdir =   1;
int  OPTkeepopts =  0;
int  OPTlog =       1;
int  OPTusehardspaces  = 1;
int  OPTlibnohardspace = 0;             // if I am using the gxlib I will only have one argv with everything in it so don't use hardspaces.
int  OPTtime =      0;
char emptyString[] = "";
char *OPTextrule =  emptyString;
char *OPTpathrule = emptyString;
char *OPTnamerule = emptyString;
char *OPTurl = NULL;
char *OPTbookmark = NULL;
char OPTstr[SPITMAXLINE];

char *OPTextlist =  emptyString;
char *OPTpathlist = emptyString;
char *OPTfname =    emptyString;
/*int OPTcreator =   'SPIT'; */
int OPTcreator =   'S'<<24 | 'P'<<16 | 'I'<<8 | 'T';
/*int OPTfiletype =  'TEXT'; */
int OPTfiletype =  'T'<<24 | 'E'<<16 | 'X'<<8 | 'T';
const char *SPITmachine =    SPIT_MACHINE;

char *OPTenvpath = emptyString;
int OPTrecurse = 0;
int OPTstartrecurseat = 0;
int OPTautoquote = 1;

int SPIThelpgiven=0;
int SPITerrorcount=0;

char OPTdot='\1'; // \1 is to disable it... not easily done by accident.  change to '.' to enable . as an option

/****************************************************************/
/*  Private Vars                                                */
/****************************************************************/

char *optnamebuf;

/****************************************************************/
/*  Private Defines                                             */
/****************************************************************/

#define MAXFNLEN  256
#define HARDSPACE 28

/* File Name Type Checking */

#define SPITPATHNORM        1                       /* Normal:              file.ext */
#define SPITPATHABS         2                       /* Absolute paths:      d:... OR d:\... OR \... OR \... */
#define SPITPATHDIR         3                       /* Multiple dir paths:  dir1\dir2\dir3\file.ext */
#define SPITPATHOPT         4                       /* Options:             -f5, .f4, ... */
static char    SPIT_typeofname(char *nm);

/****************************************************************/
/*  Load File Routine                                           */
/*    - loads file                                              */
/*    - stores ptr and len in memory struct                     */
/*    - null terminates file for easy ASCII use                 */
/****************************************************************/

bool SPIT_loadfilev(const char *fname, char **bufptr, size_t *fsize, int verbose)
{
    size_t flen;
    FILE *fhdl;
    char statbuf[SPITMAXLINE];
    bool ok;
    statbuf[0] = '\0';
    flen = 0;
    char *ptr = NULL;

    printf("%s", SPIT_loadstr(statbuf,fname,verbose));

    fhdl = fopen(fname, "rb");

    if (fhdl != NULL)
    {
        // Find file size
        fseek(fhdl, 0, SEEK_END);
        flen = static_cast<size_t>(ftell(fhdl));
        fseek(fhdl, 0, SEEK_SET);
        ptr = static_cast<char *>(malloc(flen + 4));
        if (ptr != NULL)
        {
            ptr[flen+0] = '\0';
            ptr[flen+1] = '\0';
            ptr[flen+2] = '\0';
            ptr[flen+3] = '\0';
            size_t dwNumberOfBytesRead = 0;
            dwNumberOfBytesRead = fread(ptr, 1, flen, fhdl);
            ok = dwNumberOfBytesRead == flen;
            fclose(fhdl);
            if (!ok)
            {
                free(ptr);
                ptr = NULL;
            }
        }
    }

    if (ptr == NULL)
    {
        if (verbose)
            printf("SPIT_loadfilev - COULD NOT READ FILE '%s'\n", fname);

        return false;
    }

    *bufptr = ptr;

    if (fsize != NULL)
    {
        *fsize = flen;
    }

    return true;
}

/****************************************************************/
/*  Private Functions                                           */
/****************************************************************/

char *  SPIT_stringrule(const char *rule, const char *old, char *New, int maxlen);

/****************************************************************/
/*  Time function for -time options                             */
/*  returns milliseconds                                        */
/****************************************************************/

#ifdef _MSC_VER
static double i64tod(__int64 it)
{
    return (double) it;
    //t = ((double)(unsigned int)(t1.HighPart)*4294967296.0) + ((double)(unsigned int)(t1.LowPart));
}
#endif

double SPIT_gettime(void)
{
    double t=0;

#ifdef _MSC_VER

    t = i64tod(GetTickCount());

#elif defined(__unix__) || defined(__BEOS__) || defined(__APPLE__)
    {
        struct timeval tv;
        struct timezone tz; // not used
        gettimeofday(&tv,&tz);
        t = (double)tv.tv_sec*1000.0+(double)tv.tv_usec/1000.0;
    }
#endif
    return(t);
}

/****************************************************************/
/*  Shortcut Expansion for Windows                              */
/*  -converts shortcuts to the full target pathname             */
/****************************************************************/

#if defined(__cplusplus) && defined(_MSC_VER)

static bool SPIT_isShortcut(const char *shortcut)
{
    int ext=0;
    if (!shortcut[0] || shortcut[0]=='-')  // its an option
        return false;

    for (int i=0; shortcut[i]; ++i)
    {
        if (shortcut[i]=='\\')
            ext = 0;
        if (shortcut[i]=='.')
            ext = i;
    }

    if (!strcmp(shortcut+ext, ".lnk"))
        return true;

    return false;
}

// if pszShortcutFile isnt a shorcut fullFilename will be the same as pszShortcutFile
// force forces a file with a non .lnk extension or no extension at all to be a shortcut and tries to get the target name
// if the shortcut is to a file on a floppy drive or CDROM drive, shortcuts will fail and return the shortcut name and not the target

static int SPIT_shortcut(const char *pszShortcutFile, char *fullFilename, int force)
{
    HRESULT hres;
    IShellLink *psl;
    WIN32_FIND_DATA wfd;
    char ShortcutFile[MAX_PATH*4+8];
    char Shortcut[MAX_PATH*4];
    HWND hwnd = NULL;
    int i = 0;
    char drive[4];
    unsigned int drivetype;
    int targetlen;

    bool isScut = SPIT_isShortcut(pszShortcutFile);
    if (!isScut && !force)
    {
        strcpy(fullFilename, pszShortcutFile);
        return (0);
    }

    strncpy(ShortcutFile, pszShortcutFile,MAX_PATH*4-1);
    ShortcutFile[MAX_PATH*4-1] = 0;

    if (!isScut && force)
        strcat(ShortcutFile, ".lnk");

    strncpy(Shortcut, pszShortcutFile,MAX_PATH*4);
    CoInitialize(NULL);

    // Get a pointer to the IShellLink interface.
    hres = CoCreateInstance (CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (void **)&psl);

    if (SUCCEEDED (hres))
    {
        IPersistFile *ppf;

        // Get a pointer to the IPersistFile interface.
        hres = psl->QueryInterface (IID_IPersistFile, (void **)&ppf);

        if (SUCCEEDED (hres))
        {
            wchar_t wsz [MAX_PATH]; // buffer for Unicode string

            // Ensure that the string consists of Unicode characters.
            MultiByteToWideChar (CP_ACP, 0, ShortcutFile, -1, wsz, MAX_PATH);

            // Load the shortcut.
            hres = ppf->Load (wsz, STGM_READ);

            if (SUCCEEDED (hres))
            {
                // Resolve the shortcut.
                hres = psl->Resolve (hwnd, SLR_NO_UI|SLR_NOUPDATE);
                if (SUCCEEDED (hres))
                {
                    strcpy (fullFilename, ShortcutFile);

                    // Get the path to the shortcut target.
                    hres = psl->GetPath (fullFilename, MAX_PATH,(WIN32_FIND_DATA *)&wfd, SLGP_UNCPRIORITY);
                    if (! SUCCEEDED (hres))
                        printf("GetPath failed!");
                }
            }
            // Release the pointer to IPersistFile.
            ppf->Release ();
        }
        // Release the pointer to IShellLink.
        psl->Release ();
    }
    else
        puts("CoCreateInstance failed");

    if (!SUCCEEDED(hres))
    {
        strncpy(fullFilename, Shortcut,MAX_PATH*4);
        return (0);
    }

    // checking which drive the target file is in
    targetlen = strlen(fullFilename);
    i = 0;
    while (i < targetlen)
    {
        if (fullFilename[i] == '\\' || fullFilename[i] == '/')
            break;
        i++;
    }

    if (i != targetlen)
    {
        i++;
        strncpy(drive,fullFilename,i);
        drive[3] = '\0';
        drivetype = GetDriveType(drive);

        // if the target is on a floppy or CD, return the shortcut name and shortcuts failed
        if ((drivetype == DRIVE_REMOVABLE)||(drivetype == DRIVE_CDROM))
        {
            strcpy(fullFilename, Shortcut);
            return(0);
        }
        else
            return(SUCCEEDED(hres));
    }
    else
    {
        strcpy(fullFilename, Shortcut);
        return(0);
    }
}
#endif

/****************************************************************/
/*  String Field Functions                                      */
/****************************************************************/

/* convert hard spaces to soft spaces */

static char inline tosoftspace(char hc)
{
    if (hc==HARDSPACE) return ' ';
    return hc;
}

/* check for start of delimited section of field */

static bool inline islim(char lim)
{
    if (lim==(char)'(' || lim==(char)'{' || lim==(char)'[' || lim==(char)'"' || lim==(char)'`' || lim==(char)'\'')
        return true;
    return false;
}

/* find matching close delimiter to opening delimiter */

static inline char limtodelim(char lim)
{
    char delim=lim;
    if (lim==(char)'(') delim = (char)')';
    if (lim==(char)'{') delim = (char)'}';
    if (lim==(char)'[') delim = (char)']';
    return delim;
}

char *SPIT_getfield( char *dest,const char *optptr, bool strip, bool list)
{
    char lim=0;
    char delim=0;
    bool more=true;
    char stripdelim=0;
    const char *s = optptr;
    char *d = dest;
    int depth = 0;
    int maxchars = SPITMAXLINE-1;

    while (more)
    {
#if 1
        // fast alphanums copy loop
        while ((*s>='a' && *s<='z') || (*s>='.' && *s<=':')) // ./0123456789:
        {
            *d++ = *s++;
            --maxchars;
        }
#endif
        if (*s)
        {
            if (!depth)
            {
                /* check for start of delimited section of field */

                if (islim(*s) && (*s!='\'' || d==dest))    // only respect ' as delimiter for first character
                {
                    ++depth;
                    lim = *s;
                    delim = limtodelim(lim);
                    if (d==dest && strip)
                        stripdelim = lim;

                    *d++ = tosoftspace(*s);
                    --maxchars;
                }

                /* check for white space delimiters */

                else
                {
                    if (((unsigned char)*s<=(unsigned char)' ' && *s!=HARDSPACE) || (*s==(char)',' && !list))
                    {
                        more = false;
                    }
                    else
                    {
                        if (*s==(char)'=' && !list)
                        {
                            if (s == optptr /* not delimiter if unary = */
                                || *(s-1)==(char)'+' || *(s-1)==(char)'-') // not a delimiter if a += or -=
                            {
                                *d++ = *s;
                                --maxchars;
                            }
                            else if (*(s+1)==(char)'=') // == is not a delimiter
                            {
                                *d++ = *s++;
                                *d++ = *s;
                                maxchars -= 2;
                            }
                            else
                                more = false;
                        }
                        else
                        {
                            *d++ = tosoftspace(*s);
                            --maxchars;
                        }
                    }
                }
            }
            else // nested in delimiter
            {
                if (*s==delim)
                {
                    --depth;
                }
                else if (*s==lim)
                {
                    ++depth;
                }
                *d++ = tosoftspace(*s);
                --maxchars;
            }
            if (maxchars <= 0)
                more = false;

            if (more)
                ++s;
        }
        else // null
            more = false;
    }

    *d = '\0';

    // strip delimiters
    if (stripdelim && d>=dest+2)
    {
        if (*(d-1)==stripdelim)
        {
            *(d-1) = 0;
            strcpy(dest,dest+1);
        }
    }

    return((char *)s);
}

char *SPIT_nextfield(const char *optptr)
{
    const char *s;

    /* span white spaces */

    s = optptr;
    while ((((unsigned char)*s<=(unsigned char)' ' && *s!=HARDSPACE) && *s!=(char)'\0') || *s==(char)',')
    {
        ++s;
    }

    return((char *)s);
}

/* given a filename, create a temporary name that is unique.
historically, this has been DOS compatible by changing the extension to .bak
or .old.  The ANSI tmpname function tends to not work on network folders.
There are 2 problems with changing the extension to .bak
1. Does not work on .bak file names
2. Some editors etc will find .bak before .cpp files, which is annoying
The drawback to appending .bak is that some OS will not accept 2 extensions,
or long filenames.  If this comes up in practice, this function will deal
with it.
Some apps will write to this name.  Some will back up to this name first
and write to the intended target file.  Some apps will need several temporary
file names.  The rule parameter can suggest extensions
Some suggested names are ("tmp","bak","old","tm1" are
*/

char *SPIT_tempname( char *d,const char *s, const char *rule)
{
    if (!rule) rule="bak";

    sprintf(d,"%s.%s",s,rule);
    return d;
}

/* find the extension and applies rule to it.  Watchs for
   DOS path name to avoid confusion with .\file or ..\file
   Doesn't check first character, which is illegal DOS file
     name and may be a directive.  ie .stop
*/

char *SPIT_changeext( char *d,const char *s, const char *rule)
{
    char *x;
    char namebuf[SPITMAXLINE];
    char extbuf[SPITMAXLINE];
    namebuf[0] = '\0';
    extbuf[0]  = '\0';
    strncpy(namebuf, s,SPITMAXLINE);

    x = namebuf+(strlen(namebuf))-1;            /* find extension */
    while (x>namebuf)
    {
        if (*x=='.')
            break;
        if (*x==(char)':' || *x==(char)'/' || *x==(char)'\\')
            break;
        --x;
    }
    if (*x!='.')
    {
        x = namebuf+(strlen(namebuf));
        *(x+1) = '\0';
    }
    *x++ = '\0';                                /* remove '.' */
    SPIT_stringrule(rule, x, extbuf, 3);            /* apply rule to extension */

    strcpy(d, namebuf);                         /* join name and extension */
    if (*extbuf)
    {
        strcat(d, ".");
        strcat(d, extbuf);
    }
    return(d);
}

char *SPIT_changepath( char *d,const char *s, char *rule)
{
    char *dd;
    char *x;
    char dirsym;
    char namebuf[SPITMAXLINE];
    char pathbuf[SPITMAXLINE];
    char rulebuf[SPITMAXLINE];
    namebuf[0] = '\0';
    pathbuf[0] = '\0';
    rulebuf[0] = '\0';
    strncpy(pathbuf, s,SPITMAXLINE);
    strncpy(rulebuf, "",SPITMAXLINE);
    if (rule)
        strncpy(rulebuf, rule,SPITMAXLINE);

    x = emptyString;
    dd = pathbuf;
    while (*dd)                                 /* find last ':','/','\' */
    {
        if (*dd==(char)':' || *dd==(char)'/' || *dd==(char)'\\')
            x = dd;
        ++dd;
    }

    if (*x)                                     /* remove filename */
    {
        dirsym = *x;
        ++x;
        strncpy(namebuf, x,SPITMAXLINE);
        *x = '\0';
    }
    else
    {
        dirsym = '\0';
        strncpy(namebuf, pathbuf,SPITMAXLINE);
        *pathbuf = '\0';
    }

    if (*pathbuf)                               /* add separator if necessary */
    {
        dd = pathbuf + (strlen(pathbuf)) - 1;
        if (*dd!=(char)':' && *dd!=(char)'/' && *dd!=(char)'\\')
        {
            if (dirsym!=(char)'/' && dirsym!=(char)'\\')
                dirsym = SPITDIRSYM;
            dd[1] = dirsym;
            dd[2] = '\0';
        }
    }

    if ((*rulebuf==(char)'-') && *(rulebuf+1))              /* add separator if necessary */
    {
        dd = rulebuf + (strlen(rulebuf)) - 1;
        if (*dd!=(char)':' && *dd!=(char)'/' && *dd!=(char)'\\')
        {
            if (dirsym!=(char)'/' && dirsym!=(char)'\\')
                dirsym = SPITDIRSYM;
            dd[1] = dirsym;
            dd[2] = '\0';
        }
    }

    if (rule)
        SPIT_stringrule(rulebuf, pathbuf, pathbuf, MAXFNLEN);   /* apply rule to pathname */


    dd = pathbuf + (strlen(pathbuf)) - 1;                   /* add separator if necessary */
    if (dd>=pathbuf)
    {
        if (*dd!=(char)':' && *dd!=(char)'/' && *dd!=(char)'\\')
        {
            if (dirsym!=(char)'/' && dirsym!=(char)'\\')
                dirsym = SPITDIRSYM;
            dd[1] = dirsym;
            dd[2] = '\0';
        }
    }

    strcpy(d, pathbuf);                     /* join path and filename */
    strcat(d, namebuf);
    return(d);
}

/****************************************************************/
/*  Save File                                                   */
/*    - saves a binary file                                     */
/*    - fetches ptr and len from memory struct                  */
/****************************************************************/
bool SPIT_savefile(const char *fname, const void *buffer, size_t len)
{
    bool ok=true;
    char statbuf[SPITMAXLINE];
    FILE *fhdl;
    statbuf[0] = '\0';
    if (OPTwrite)
    {
        printf("%s", SPIT_savestr(statbuf, fname,OPTverbose));

        ok = false;

        {
            char nameBuffer[512];
            strcpy(nameBuffer, fname);

            for (int i=0; nameBuffer[i]; ++i)
            {
                if (nameBuffer[i] == '\\' || nameBuffer[i] == '/')
                {
                    nameBuffer[i] = '\0';
                    (void)RMkDir(nameBuffer);
                    nameBuffer[i] = '\\';
                }
            }
        }

        fhdl = fopen(fname, "w");

        if (fhdl != NULL)
        {
            size_t dwNumberOfBytesWritten = 0;

            dwNumberOfBytesWritten = fwrite(buffer, len, 1, fhdl);

            ok = dwNumberOfBytesWritten == len;

            fclose(fhdl);
        }

        if (!ok)
        {
            if (OPTverbose)
            {
                printf("SPIT_savefile - COULD NOT WRITE FILE '%s'\n", fname);
            }
        }
    }
    return(ok);
}

/****************************************************************/
/*  Filename parsing routines                                   */
/****************************************************************/


/* Case insensitive strstr */
char *SPIT_strstr(const char *str, const char *substr)
{
    int len1;
    int len2;
    int i = 0;
    char a = substr[0];
    char b;

    if (islower(a))
        b = (char)toupper(a);
    else
        b = (char)tolower(a);

    len2 = strlen(substr);
    len1 = strlen(str) - len2;

    while (i < len1)
    {
        if ((str[i] == a) || (str[i] == b))
        {
            if (!EA::StdC::Strnicmp(&(str[i]), substr, len2))
                return((char *)str+i);
        }
        ++i;
    }

    return(NULL);
}


char *SPIT_stringrule(const char *rule, const char *old, char *New, int maxlen)
{
    const char *left;
    const char *right;
    int   stripamt;
    int   stripleft;
    int   leftlen;
    int   rightlen;
    char  newstr[SPITMAXLINE];
    newstr[0] = '\0';
    if (*rule==(char)'-')
    {
        left = rule+1;
        right = old;
        stripleft = 0;
    }
    else if (*rule==(char)'+')
    {
        left = old;
        right = rule+1;
        stripleft = 1;
    }
    else if (*rule==(char)'=')
    {
        left = rule+1;
        right = emptyString;
        stripleft = 0;
    }
    else if (*rule==(char)'\0')
    {
        left = old;
        right = emptyString;
        stripleft = 0;
    }
    else
    {
        left = rule;
        right = emptyString;
        stripleft = 0;
    }

    leftlen =  (int) (strlen(left));
    rightlen = (int) (strlen(right));

    stripamt = leftlen+rightlen-maxlen;
    if (stripamt < 0)
        stripamt = 0;
    if (stripamt > rightlen)
        stripamt = rightlen;

    if (!stripleft)
        stripamt = 0;

    strcpy(newstr, left);
    strcat(newstr, (right+stripamt));

    strcpy(New, newstr);

    return(New);
}

char *SPIT_convertname( char *d,const char *s)
{
    return(SPIT_changeext(d, SPIT_getname(d, s),OPTextrule));
}

char *SPIT_getpath( char *newmatch,const char *old)
{
    char *d;
    char *x = NULL;

    strcpy(newmatch, old);

    d = newmatch;
    while (*d)                                  /* find last ':','/','\' */
    {
        if (*d==(char)':' || *d==(char)'/' || *d==(char)'\\')
            x = d;
        ++d;
    }

    if (x)                                     /* remove filename */
    {
        ++x;
        *x = '\0';
    }
    else
    {
        *newmatch = '\0';
    }

    return(newmatch);
}

char *SPIT_getname( char *New,const char *old)
{
    const char *nameptr;

    nameptr = old;

    while (*old)
    {
        if (*old==(char)'/' || *old==(char)'\\' || *old==(char)':')
            nameptr = old+1;
        ++old;
    }
    strcpy(New, nameptr);                           /* copy to destination */
    return(New);
}

/* get 2 names ie file=bob */

static char *SPIT_nextnames(char *names, char *inname, char *outname)
{
    char *namelist=names;

    if (!*namelist)
    {
        namelist = 0;
    }
    else
    {
        /* get source name */

        *inname = '\0';
        if (*namelist==(char)'=')   // =b
        {
            int c = 0;                                           // if it's a path I want to return the path
            if(SPIT_isnextfieldpath(names) && names[0] == '=')   // without the = sign in front.
                c++;
            Zop(namelist);
            namelist = SPIT_getfield(inname,namelist+c,true,true);
            namelist = SPIT_nextfield(namelist);
            strcpy(outname,inname);
        }
        else    // a=b
        {
            namelist = SPIT_getfield( inname,namelist,true,false);
            namelist = SPIT_nextfield(namelist);
        }

        /* is there an '=' directive? */

        *outname = '\0';
        if (*namelist==(char)'=' && *names!='-' && !SPIT_isnextfieldpath(namelist))    // a=b
        {
            Zop(namelist);
            namelist = SPIT_nextfield(namelist+1);
            namelist = SPIT_getfield( outname,namelist,true,false);
            namelist = SPIT_nextfield(namelist);
        }
        else    // option with commas
        {
            int c = 0;                                             // if it's a path I want to return the path
            if(SPIT_isnextfieldpath(names) && names[0] == '=')     // without the = sign in front.
                c++;
            namelist = SPIT_getfield( inname,names+c,true,true);  /* refetch with commas */
            namelist = SPIT_nextfield(namelist);
        }
    }
    return(namelist);
}


/****************************************************************/
/*  Expand File                                                 */
/*    - expands a text file's contents into the option list     */
/****************************************************************/

static bool SPIT_responsefile(char *name, char *optbuf)
{
    bool ok=true;
    char *oldextlist;
    char *envlist;
    char comment = 0;
    char envbuf[SPITMAXFNAME];
    char lim = 0;
    char delim = 0;
    int temp = 0;
    int depth;
    int i;
    int j;
    int k;
    int numparams = 0;
    char **params = NULL;
    char fname[SPITMAXFNAME];
    char *namefile;
    char defaultOPTextlist[] = "+rsp";

    oldextlist = OPTextlist;
    OPTextlist = defaultOPTextlist;

    /* Count parameters and get file name */
    i = j = 0;
    while (name[i])
    {
        if (!lim)
        {
            if (name[i] == '(' || name[i] == '{' || name[i] == '[')
            {
                lim = name[i];
                if (lim == '(') delim = ')';
                if (lim == '{') delim = '}';
                if (lim == '[') delim = ']';
                ++i;
            }
            else
                fname[j++] = name[i++];
        }
        else
        {
            depth = 0;
            while((name[i]!=delim || depth!=0) && (name[i]!=',' || depth!=0) && name[i] != '\0')
            {
                if (name[i] == lim)
                    ++depth;
                else if (name[i] == delim)
                    --depth;
                ++i;
            }

            if (name[i] == ',')
            {
                ++numparams;
                ++i;
            }
            else if (name[i] == delim)
            {
                temp = 1;
                ++numparams;
                if (name[i+1] != '\0')
                {
                    ok = false;
                    if (OPTverbose>0)
                        printf("spit - INVALID NAME FORMAT '%s'.\n", name);
                }
                ++i;
            }
        }
    }
    fname[j] = '\0';
    if (!temp && numparams)
    {
        ok = false;
        printf("spit - MISSING CLOSING BRACE ON '%s'.\n", name);
    }

    if (ok)
    {
        /* Allocate and get parameters */
        if (numparams > 0)
        {
            params = (char **)malloc(numparams * sizeof(char *));
            for (i = 0; i < numparams; ++i)
                params[i] = (char *)malloc(SPITMAXFNAME);

            i = numparams = 0;
            while (name[i] != lim)
                ++i;
            ++i;

            while (name[i])
            {
                j = depth = 0;
                while((name[i]!=delim || depth!=0) && (name[i]!=',' || depth!=0) && name[i] != '\0')
                {
                    if (name[i] == lim)
                        ++depth;
                    else if (name[i] == delim)
                        --depth;
                    params[numparams][j++] = name[i++];
                }
                params[numparams][j] = '\0';

                ++numparams;
                ++i;
            }
        }

        i = j = k = temp = 0;
        envlist = getenv(fname);               /* first try as environment var */
        if (envlist)
        {
            strcat(optbuf,envlist);
            strcat(optbuf, "\n");
        }
        else
        {
            if (SPIT_loadfilev(fname, &namefile, NULL, 0))
            {
                /* Do a preliminary pass over the file */
                while ((namefile[i] != '\0') && ok)
                {
                    /* Skip comments */
                    if (namefile[i] == ';')
                    {
                        if (i == 0)
                            comment = 1;
                        else if (namefile[i-1] == 10 || namefile[i-1] == 13)
                            comment = 1;
                        if (comment)
                        {
                            while (namefile[i] != 10 && namefile[i] != 13 && namefile[i] != '\0')
                                ++i;
                            while (namefile[i] == 10 || namefile[i] == 13)
                                ++i;
                            comment = 0;
                        }
                        else
                        {
                            ++i;
                        }
                    }
                    /* Find length of environment variables */
                    else if (namefile[i] == '%')
                    {
                        if (isdigit(namefile[i+1]))
                        {
                            k = 0;
                            ++i;
                            while(isdigit(namefile[i]))
                                envbuf[k++] = namefile[i++];
                            envbuf[k] = '\0';
                            temp = atoi(envbuf) - 1;
                            if (temp >= numparams)
                            {
                                ok = false;
                                if (OPTverbose>0)
                                    printf("spit - PARAMETER %%%i NOT GIVEN TO RESPONSEFILE.\n", temp+1);
                            }
                        }
                        else if (namefile[i+1] == '%')
                        {
                            i+=2;
                        }
                        else
                        {
                            k = 0;
                            temp = i;
                            ++i;
                            while ((namefile[i] != '%') && (isalnum((int)namefile[i]) || namefile[i] == '_'))
                                envbuf[k++] = namefile[i++];
                            envbuf[k] = '\0';
                            if (namefile[i] == '%' && k != 0)
                            {
                                if (!getenv(envbuf))
                                {
                                    ok = false;
                                    strupr(envbuf);
                                    if (OPTverbose>0)
                                        printf("SPIT - ENVIRONMENT VARIABLE '%s' NOT DEFINED.\n", envbuf);
                                }
                                ++i;
                            }
                        }
                    }
                    else
                    {
                        ++i;
                    }
                }

                if (ok)
                {
                    comment = (char) 0;
                    i = j = k = 0;

                    /* expand a text file - ripping out the comments (comments == first line has a ; at front) */
                    while (namefile[i] != '\0')
                    {
                        // fast copy ascii A..Z [ \ ] ^ _ ` a..z

                        while (namefile[i]>='A' && namefile[i]<='z')
                            *optbuf++ = namefile[i++];

                        /* Semi-colons mean comments... */
                        if (namefile[i] == ';')
                        {
                            /* But only if they are the first character of a line */
                            if (i == 0)
                                comment = 1;
                            else if (namefile[i-1] == 10 || namefile[i-1] == 13)
                                comment = 1;
                            if (comment)
                            {
                                while (namefile[i] != 10 && namefile[i] != 13 && namefile[i] != '\0')
                                    ++i;
                                while (namefile[i] == 10 || namefile[i] == 13)
                                    ++i;
                                comment = 0;
                            }
                            else
                            {
                                *optbuf++ = namefile[i++];
                                // strncat(optbuf, &namefile.ptr[i++], 1);
                            }
                        }
                        // parameter is %1
                        else if (namefile[i] == '%')
                        {
                            if (isdigit(namefile[i+1]))
                            {
                                k = 0;
                                ++i;
                                while(isdigit(namefile[i]))
                                    envbuf[k++] = namefile[i++];
                                envbuf[k] = '\0';
                                temp = atoi(envbuf) - 1;
                                if (temp>=0 && temp < numparams)
                                {
                                    strcpy(optbuf, params[temp]);
                                    optbuf += strlen(optbuf); // advance to end
                                }
                            }
                            else if (namefile[i+1] == '%')
                            {
                                *optbuf++ = '%';
                                //strcat(optbuf, "%");
                                i+=2;
                            }
                            else
                            {
                                k = 0;
                                ++i;
                                while ((namefile[i] != '%') && (isalnum((int)namefile[i]) || namefile[i] == '_'))
                                    envbuf[k++] = namefile[i++];
                                envbuf[k] = '\0';
                                if (namefile[i] == '%' && k != 0)
                                {
                                    if (getenv(envbuf))
                                    {
                                        strcpy(optbuf, getenv(envbuf));
                                        optbuf += strlen(optbuf); // advance to end
                                    }
                                    ++i;
                                }
                                else
                                {
                                    *optbuf++ = '%';
                                    //strcat(optbuf, "%");
                                    strcpy(optbuf, envbuf);
                                    optbuf += strlen(optbuf); // advance to end
                                }
                            }
                        }


                        // nested response file (require white space)
                        else if ((namefile[i] == '@' || namefile[i] == '&') && (i==0 || (unsigned char)namefile[i-1]<=(unsigned char)' '))
                        {
                            char rname[SPITMAXLINE];
                            int len;
                            int stop=0;
                            char *newptr;

                            /*char sname[SPITMAXLINE];  */
                            rname[0] = '\0';
                            newptr = SPIT_getfield( rname,&namefile[i+1],false,false);

                            /* Advance i by the advance of the pointer */
                            i = (int)(newptr - namefile);
                            len = strlen(rname);
                            while ((unsigned char)rname[len]<=(unsigned char)' ' || rname[len]=='\0' || rname[len] == ';')
                            {
                                if (rname[len] == ';')
                                {
                                    stop = 1;
                                    rname[len] = '\0';
                                }
                                --len;
                            }
#if defined(__cplusplus) && defined(_MSC_VER)
                            SPIT_shortcut(rname, rname, 0);
#endif
                            if (!SPIT_responsefile(rname, optbuf))
                                SPIThelpgiven = 1;

                            optbuf += strlen(optbuf); // advance to end

                            if (stop == 1)
                            {
                                stop = 0;
                                strcpy(optbuf, " ; ");
                                optbuf += strlen(optbuf); // advance to end
                            }
                        }
                        else if (namefile[i] == 10 || namefile[i] == 13 || namefile[i] == '\t')
                        {
                            *optbuf++ = ' ';
//                            strcat(optbuf, " ");
                            ++i;
                        }
                        else
                        {
                            *optbuf++ = namefile[i++];
                            //strncat(optbuf, &namefile.ptr[i++], 1);  // bottleneck!
                        }
                    }

                    *optbuf++ = ' ';
                    *optbuf = '\0';
                    //strcpy(optbuf, " ");
                    ok = true;
                }
                free(namefile);
                namefile = NULL;
            }
            else
                ok = false;
        }
    }

    if (numparams > 0 && ok && params)
    {
        for (i = 0; i < numparams; ++i)
            if (params[i])
                free(params[i]);
        free(params);
        params = NULL;
    }

    if (!ok && OPTverbose>0)
    {
        printf("WARNING - could not expand file <%s>\n", fname);
    }

    return(ok);
}


/****************************************************************/
/*  Check a Filename for Possible Wildcards                     */
/****************************************************************/

bool SPIT_iswildcards(const char *s)
{
    bool ok=false;

    if (*s!='-')  // options are not wildcards
    {
        while (*s)
        {
            if (*s=='*'
             || *s=='?'
             || *s=='#')
            {
                ok = true;
                break;
            }
            ++s;
        }
    }

    return(ok);
}


/****************************************************************/
/*  Check if string is a directory                              */
//  examples
//  c:          just a drive letter
//  dir\ dir/   folder
//
/****************************************************************/

static bool SPIT_ispath(const char *s)
{
    int l;
    bool ok=false;

    l = strlen(s);

    if (l>=1)
    {
        if (s[0]!='-' &&
           ((s[l-1]=='\\' || s[l-1]=='/' || s[l-1]==':' || s[l-1]=='.') ||
            (l >= 2 && s[l-1] == '"' && (s[l-2]=='\\' || s[l-2]=='/' || s[l-2]==':' || s[l-2]=='.'))))
        {
            if (l>=2)
            {
                /* c: */
                if ((s[l-1]==':' && isalpha((unsigned char)s[0])
#if defined(_MSC_VER)
                   && l==2
#endif
                )
                /* c:. . .. c:\dir\. */
                || (s[l-1]=='.' && (s[l-2]=='\\' || s[l-2]=='/' || s[l-2]==':' || s[l-2]=='.'))
                || s[l-1]=='/'
                || s[l-1]=='\\'
                || (
                      l >= 2 &&
                      s[l-1] == '"' && (
                         (s[l-2]=='.' && (s[l-3]=='\\' || s[l-3]=='/' || s[l-3]==':' || s[l-3]=='.'))
                      || s[l-2]=='/'
                      || s[l-2]=='\\'
                                       )
                   )
                )
                    ok = true;
            }
            else
                ok = true;
        }
    }
    return(ok);
}

/****************************************************************/
/*  Status Messages                                             */
/****************************************************************/

/* load status message */

char *SPIT_loadstr(char *buf, const char *fname, int verbose)
{
    char statbuf[SPITMAXLINE];
    statbuf[0] = '\0';
    buf[0] = '\0';
    if (verbose>=5)
        sprintf(buf,"%s Reading, ",fname);
    else if (verbose>=4)
        sprintf(buf,"%s ",fname);
    else if (verbose>=3)
        sprintf(buf,"%s ",SPIT_getname( statbuf,fname));
    else if (verbose>=2)
        sprintf(buf,"%s ",SPIT_getname( statbuf,fname));

    return(buf);
}

/* save status message */

char *SPIT_savestr(char *buf, const char *fname, int verbose)
{
    char statbuf[SPITMAXLINE];
    statbuf[0] = '\0';
    buf[0] = '\0';
    if (verbose>=3)
    {
        if (verbose>=5)
            sprintf(buf,"Writing as %s, ",fname);
        else
        {
            if (verbose>=4)
                sprintf(buf,"Writing as %s, ",SPIT_getname( statbuf,fname));
            else
                sprintf(buf,"Writing, ");
        }
    }
    return(buf);
}

/* percentage calculation status */

char *SPIT_percentstr(char *buf, int olen, int nlen)
{
    double reduction=100.0;

    if (olen < nlen)
    {
        *buf = '-';
        SPIT_percentstr(buf+1, nlen, olen);
    }
    else
    {
        if (nlen && olen)
        {
            reduction = 100.0-(double)nlen*100.0/(double)olen;
        }
        sprintf(buf, "%.1f%%", reduction);
    }
    return(buf);
}

/****************************************************************/
/*  Get a String                                                */
/*  - used by SPIT_getnum                                           */
/****************************************************************/

static char *SPIT_getstring(char *buf, const char *s)
{
    char c;
    char n;
    char *d;
    char firstc;
    const char *olds;

    d = buf;
    firstc = *s;
    olds = s;
    if (firstc==(char)'"')
    {
        do
        {
            ++s;
            do
            {
                c = *s++;
                if (c==(char)'\\')
                {
                    c = *s++;
                    switch(c)
                    {
                        case 'n':   *d++ = (char)10;    break;
                        case 't':   *d++ = (char)'\t';  break;
                        case 'b':   *d++ = (char)'\b';  break;
                        case 'r':   *d++ = (char)13;    break;
                        case 'f':   *d++ = (char)'\f';  break;
                        case '\\':  *d++ = (char)'\\';  break;
                        case '<':   *d++ = (char)'<';   break;
                        case '\'':  *d++ = (char)'\'';   break;
                        case '"':   *d++ = (char)'"'; break;
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
                                        if ((unsigned char) n >= (unsigned char)' ')
                                            SPITerrorcount = 1; /*Error(loc+(int)(s-olds), SizeErr); */
                                        n = (char)(n*8+c);
                                        ++s;
                                    }
                                }
                                *d++ = n;
                            }
                            if (c==(char)8 || c==(char)9)
                                SPITerrorcount = 2; /*Error(loc+(int)(s-olds), BadDigit); */
                            --s;
                            c = '0';
                            break;
                    }
                }
                else
                {
                    *d++ = c;
                }
            } while (c!=(char)'\0' && c!=(char)'"');
        } while (*s==(char)'"');

        if (c==(char)'"')
            *--d = '\0';                        /* remove close quote or slash */
        else
            --s;
    }
    else
    {
        if (firstc==(char)'\'')
        {
            do
            {
                ++s;
                do
                {
                    c = *s++;
                    *d++ = c;
                } while (c!=(char)'\0' && c!=(char)'\'');
            } while (*s==(char)'\'');

            if (c==(char)'\'')
                *--d = '\0';                    /* remove close quote or slash */
            else
                --s;
        }
        else
        {
            if (firstc==(char)'\\')
            {
                ++s;
            }

            do
            {
                c = *s++;
                *d++ = c;
            } while ((unsigned char)c>(unsigned char)' ' && c!=(char)';' && c!=(char)',');

            if (c)
            {
                *--d = '\0';                    /* remove ; or , or white space */
                if (*(d-1)==(char)'\\')
                    *--d = '\0';                /* remove slash */
            }
            else
                --s;

            if (firstc==(char)'\\')
                SPITerrorcount = 3; /*Error(loc+(int)(s-olds), Undef); */

        }
    }

    return((char *)s);
}


/****************************************************************/
/*  Get a Single Number                                         */
/****************************************************************/

int SPIT_getnum(const char *s)
{
    char c;
    const char *opstart;
    const char *numstart;
    int  base;
    int nextval;
    int  stringlen;
    char buf[SPITMAXLINE];
    static int Radix=10;

    buf[0] = '\0';
    if (*s<='@' || Radix>10)
    {
        c = *s++;
        if (c=='-')                             /* negative */
            return(-SPIT_getnum(s));
        if (*s)                                 /* more than 1 char? */
        {
            opstart = s;
            if (c<'0')
            {
                base = 16;
                if (c!=(char)'$')                       /* hex  */
                {
                    switch (c)
                    {
                        case '"':               /* start a string */
                        case '\'':
                            nextval = 0L;
                            s = SPIT_getstring(buf, s-1);
                            if (*(s-1)!=(char)'"' && *(s-1)!=(char)'\'')    /* 0 closing delimiter */
                            {
                                SPITerrorcount = 4; /*Error(loc, 0StrEnd); */
                            }
                            else
                            {
                                stringlen = strlen(buf);
                                if (stringlen>4)
                                {
                                    SPITerrorcount = 5; /*Error(loc, SizeErr); */
                                }
                                else
                                {
                                    s = buf;
                                    while (stringlen--)
                                    {
                                        nextval = (nextval<<8) | (unsigned char) *s++;
                                    }
                                }
                            }
                            return(nextval);

                        case '%':               /* binary */
                            base = 2;
                            break;

                        case '&':               /* hex (arm style) */
                            base = 16;
                            break;

                        case 0:                 /* end of term */
                            SPITerrorcount = 6; /*Error(loc, OperErr); */
                            return(0L);

                        default:
                            base = 0;
                            break;
                    }
                }
            }
            else
            {
                base = Radix;
                --s;

                if (c==(char)'0')                       /* hex 0x */
                {
                    if (*(s+1)==(char)'x')
                    {
                        if (Radix<34)
                        {
                            base = 16;
                            s += 2;
                        }
                    }
                }
                else
                {
                    if (c==(char)'@')                   /* octal */
                    {
                        base = 8;
                        ++s;
                    }
                }
            }

            /* base 10 is optimized the most */

            nextval = 0L;
            if (base==10)
            {
                c = (char)(*s++-'0');
                if ((unsigned char) c < (unsigned char) 10)
                {
                    numstart = s;
                    nextval = (int) ((unsigned char) c);

                    c = (char)(*s++-'0');
                    if ((unsigned char) c < (unsigned char) 10)
                    {
                        nextval = (int) ((int) nextval * 10 + (unsigned char) c);

                        c = (char)(*s++-'0');
                        if ((unsigned char) c < (unsigned char) 10)
                        {
                            nextval = (int) ((int) nextval * 10 + (unsigned char) c);

                            c = (char)(*s++-'0');
                            if ((unsigned char) c < (unsigned char) 10)
                            {
                                do
                                {
                                    nextval = nextval * 10 + (unsigned char) c;

                                    c = (char)(*s++-'0');
                                } while ((unsigned char) c < (unsigned char) 10);
                            }
                        }
                    }
                    if (!*(s-1))    /* did we finish the number? */
                    {
                        /* more than 10 digits? */

                        if ((int)(s-numstart) > 10)
                          SPITerrorcount = 7; /*Error(loc, SizeErr); */

                        return(nextval);
                    }
                }
            }
            else if (base==16)
            {
                c = (char)(*s++-'0');
                if ((unsigned char) c >= (unsigned char) 10)
                    c = (char)(((c+'0')|' ')-'a'+10);

                if ((unsigned char) c < (unsigned char) 16)
                {
                    numstart = s;
                    do
                    {
                        do
                        {
                            do
                            {
                                nextval = (nextval<<4) + (unsigned char) c;

                                c = (char)(*s++-'0');
                            } while ((unsigned char) c < (unsigned char) 10);

                            c = (char)(((c+'0')|' ')-'a'+10);

                        } while ((unsigned char) c < (unsigned char) 16);

                        if (*(s-1)==(char)'_')
                        {
                            ++numstart;             /* adjust start for len calc */
                            c = (char)(*s++-'0');
                            if ((unsigned char) c >= (unsigned char) 10)
                                c = (char)(((c+'0')|' ')-'a'+10);
                        }
                    } while ((unsigned char) c < (unsigned char) 16);

                    if (!*(s-1))                    /* did we finish the number? */
                    {
                        /* more than 8 digits? */

                        if ((int)(s-numstart) > 8)
                            SPITerrorcount = 8; /*Error(loc, SizeErr); */

                        return(nextval);
                    }
                }
            }
            else
            {
                numstart = s;
                c = *s++;
                while (c)
                {
                    c -= '0';
                    if ((unsigned char) c >= (unsigned char) 10)
                        c = (char)(((c+'0')|' ')-'a'+10);

                    if ((unsigned char) c >= (unsigned char) base)
                    {
                        if (*(s-1)==(char)'_')
                        {
                            ++numstart;             /* adjust start for len calc */
                            continue;
                        }
                        break;
                    }

                    if ((unsigned int) nextval<3200L)
                    {
                        nextval = (int) nextval * base + (unsigned char) c;
                    }
                    else
                    {
                        nextval = (nextval*base) + (unsigned char) c;
                    }
                    c = *s++;
                }

                if (!*(s-1))                    /* did we finish the number? */
                {
                    /* more than 32 digits? */

                    if ((int)(s-numstart-1) > 32)
                        SPITerrorcount = 9; /*Error(loc, SizeErr); */

                    return(nextval);
                }
            }

            if (*(s-1)==(char)'$')                          /* set base */
            {
                if (nextval>=2 && nextval<=34)
                {
                    base = Radix;
                    Radix = nextval;
                    nextval = SPIT_getnum(s);
                    Radix = base;
                    return(nextval);
                }
            }

            s = opstart-1;
            if (isdigit(*s))                    /* was first char a digit */
            {
                while (*s)                      /* skip to the end */
                {
                    ++s;
                }

                c = *(s-1);
                *((char *)s-1) = '\0';
                switch (c|' ')
                {
                    case 'b':
                        base = Radix;
                        Radix = 2;
                        nextval = SPIT_getnum(opstart-1);
                        Radix = base;
                        *(char *)(s-1) = c;
                        return(nextval);

                    case 'd':
                        base = Radix;
                        Radix = 10;
                        nextval = SPIT_getnum(opstart-1);
                        Radix = base;
                        *(char *)(s-1) = c;
                        return(nextval);

                    case 'h':
                        base = Radix;
                        Radix = 16;
                        nextval = SPIT_getnum(opstart-1);
                        Radix = base;
                        *(char *)(s-1) = c;
                        return(nextval);

                    case 'o':
                    case 'q':
                        base = Radix;
                        Radix = 8;
                        nextval = SPIT_getnum(opstart-1);
                        Radix = base;
                        *(char *)(s-1) = c;
                        return(nextval);
                    default:;
                }
                *((char *)s-1) = c;
                s = opstart-1;
            }
        }
        else
        {
            c -= '0';
            if ((unsigned char) c >= (unsigned char) 10)
                c = (char)(((c+'0')|' ')-'a'+10);

            if ((unsigned char) c < (unsigned char) Radix)
            {
                return((int) c );
            }
            c = *--s;
        }
    }

    if (*s)
    {
        if (*s==(char)'^')                          /* control character */
        {
            if (!*(s+2))
            {
                c = (char)((*(s+1)|' ')-'a'+1);
                if ((unsigned char) c < (unsigned char) 27)
                {
                    return((int) c );
                }
            }
        }
    }
    SPITerrorcount = 10; /* Error(loc, Undef); */
    return(0L);
}

/****************************************************************/
/*  Is Buffer a Numerical Value?                                */
/****************************************************************/

bool SPIT_isdigit(char *operand)
{
    int val;

    if (*operand==(char)'-')
        ++operand;
    if (*operand>='0' && *operand<='9')
        return(true);

    SPITerrorcount = 0;
    val = SPIT_getnum(operand);
    if (val && !SPITerrorcount)
        return(true);

    return(false);
}


bool SPIT_isnumber(char *operand)
{
    char *oper = operand;

    if (*oper==(char)'-')
        ++oper;
    while (*oper != ' ' && *oper != ';' && *oper != '\0')
    {
        if (*oper<'0' || *oper>'9')
            return false;
        ++oper;
    }

    return true;
}

/****************************************************************/
/*  Numerical Options Parser                                    */
/****************************************************************/

char *SPIT_getopt2(int *optvalptr,const char *optptr,const  char *optname, int optmin, int optmax, int optdef)
{
    const char *s;
    int optval;
    char expbuf[SPITMAXLINE];
    expbuf[0] = '\0';
    s = SPIT_getfield( expbuf,optptr,false,false);

    if (s)  /* strip % from numbers */
    {
        int l;
        if (*(s-1)=='%')
            --s;
        l = strlen(expbuf);
        if (l)
            if (expbuf[l-1]=='%')
                expbuf[l-1] = 0;
    }

    if (!strcmp(expbuf,"?"))
    {
        optval = *optvalptr;
        if (OPTverbose)
        {
            printf("Numerical Option %s = %d. Valid range is %d..%d\n", optname, optval, optmin, optmax);
            SPIThelpgiven = 1;
        }
    }
    else
    {
        if (SPIT_isdigit(expbuf) || expbuf[0]=='\'')
        {
            SPITerrorcount = 0;
            optval = SPIT_getnum(expbuf);
            if (SPITerrorcount)
            {
                if (OPTverbose)
                {
                    printf("spit - NUMERICAL OPTION ERROR #%d IN %s OPTION VALUE. Using %d.\n", SPITerrorcount, optname, optval);
                    SPIThelpgiven = 1;
                }
            }
        }
        else
        {
//            if (OPTkeepopts)    // multipass, so don't increment twice
                optval = optdef;
//            else
//                optval = *optvalptr+1;
            if (optval > optmax)
                optval = optmax;
            s = optptr;
        }

        if (optmin!=static_cast<int>(0x80000000) && optval < optmin)
        {
            optval = optmin;
            if (OPTverbose)
            {
                printf("Value too small for %s option. Valid range is %d..%d.  Using %d.\n", optname, optmin, optmax, optval);
                SPIThelpgiven = 1;
            }
        }
        if (optmax!=0x7fffffff && optval > optmax)
        {
            optval = optmax;
            if (OPTverbose)
            {
                printf("Value too large for %s option. Valid range is %d..%d.  Using %d.\n", optname, optmin, optmax, optval);
                SPIThelpgiven = 1;
            }
        }

        *optvalptr = optval;

        if (OPTverbose>=6)
            printf("%s %d\n",optname, optval);
    }
    return((char *)s);
}

char *SPIT_getopt( int *optvalptr,const char *optptr,const  char *optname, int optmin, int optmax)
{
    return(SPIT_getopt2( optvalptr,optptr,optname,optmin,optmax,1));
}



char *SPIT_getstropt(char *optvalptr,const char *optptr,const char * /*optname*/, const char * /*separator*/, int /*doit*/, int /*optmax*/)
{
    return(SPIT_getfield( optvalptr,optptr,true,true));
}

void SPIT_restore(void)
{
    if (OPTurl)
    {
        free(OPTurl);
        OPTurl = NULL;
    }
    if (OPTbookmark)
    {
        free(OPTbookmark);
        OPTbookmark = NULL;
    }
    if (optnamebuf != NULL)
    {
        free(optnamebuf);
        optnamebuf = NULL;
    }

    // gross but has to be done. On each successive call to spit if it's going
    // to be used for parsing passed options inside another program.
    optptr = NULL;
    firstcall = 1;
    namelist= NULL;
    optlist = NULL;

    freeFiles();
}

/****************************************************************/
/*  OpenURL                                                     */
/*  -used for -help option                                      */
/****************************************************************/

static bool SPIT_findtool(char *browser, const char *tool)
{

#if defined(_WIN32)
    tool;
    strcpy(browser,"start");
    return(true);
#else

    char toolfile[SPITMAXLINE];
    strcpy(toolfile,tool);

#if defined(MSDOS) || defined(_WIN32)

    strcat(toolfile,".exe");

    browser[0] = 0;
    _searchenv(toolfile,"PATH",browser);
    if (browser[0])
        return(true);

    sprintf(browser,"C:\\Program Files\\Netscape\\Navigator\\Program\\%s",toolfile);
    if (SPIT_exists(browser))
        return(true);
    sprintf(browser,"D:\\Program Files\\Netscape\\Navigator\\Program\\%s",toolfile);
    if (SPIT_exists(browser))
        return(true);
    sprintf(browser,"C:\\Program Files\\Netscape\\Program\\%s",toolfile);
    if (SPIT_exists(browser))
        return(true);
    sprintf(browser,"D:\\Program Files\\Netscape\\Program\\%s",toolfile);
    if (SPIT_exists(browser))
        return(true);


    sprintf(browser,"C:\\Program Files\\plus!\\Microsoft Internet\\%s",toolfile);
    if (SPIT_exists(browser))
        return(true);
    sprintf(browser,"D:\\Program Files\\plus!\\Microsoft Internet\\%s",toolfile);
    if (SPIT_exists(browser))
        return(true);
    sprintf(browser,"C:\\Program Files\\Microsoft Internet\\%s",toolfile);
    if (SPIT_exists(browser))
        return(true);
    sprintf(browser,"D:\\Program Files\\Microsoft Internet\\%s",toolfile);
    if (SPIT_exists(browser))
        return(true);


    sprintf(browser,"C:\\Progra~1\\Netscape\\Naviga~1\\Program\\%s",toolfile);
    if (SPIT_exists(browser))
        return(true);
    sprintf(browser,"D:\\Progra~1\\Netscape\\Naviga~1\\Program\\%s",toolfile);
    if (SPIT_exists(browser))
        return(true);
    sprintf(browser,"C:\\Progra~1\\Netscape\\Program\\%s",toolfile);
    if (SPIT_exists(browser))
        return(true);
    sprintf(browser,"D:\\Progra~1\\Netscape\\Program\\%s",toolfile);
    if (SPIT_exists(browser))
        return(true);


    sprintf(browser,"C:\\Progra~1\\plus!\\Micros~1\\%s",toolfile);
    if (SPIT_exists(browser))
        return(true);
    sprintf(browser,"D:\\Progra~1\\plus!\\Micros~1\\%s",toolfile);
    if (SPIT_exists(browser))
        return(true);
    sprintf(browser,"C:\\Progra~1\\Micros~1\\%s",toolfile);
    if (SPIT_exists(browser))
        return(true);
    sprintf(browser,"D:\\Progra~1\\Micros~1\\%s",toolfile);
    if (SPIT_exists(browser))
        return(true);

    return(false);
#else
    strcpy(browser,tool);
    return(true);
#endif
#endif

}

bool SPIT_openurl(const char *url)
{
    char browser[SPITMAXLINE];
    char sysline[SPITMAXLINE];
    bool ok=false;

    if (!ok)
        ok = SPIT_findtool(browser,"netscape");

    if (!ok)
    {
        ok = SPIT_findtool(browser,"iexplore");
    }

    if (ok)
    {
/*#if defined(_WIN32) */
/*    int Result = (int)ShellExecute(NULL, NULL, url, NULL, NULL, 1);\\SW_SHOWNORMAL); */

/*#else */
/*    if (Result < 32) */
/*                printf("did not work"); */

#if defined(MSDOS) || defined(_WIN32)
//        if (ok==2)
//            sprintf(sysline,"\"%s\" %s",browser,url);
//        else
            sprintf(sysline,"%s %s",browser,url);
#else
/* sgi */
        sprintf(sysline,"%s %s &",browser,url);
#endif

        if (OPTverbose>=6)
            printf("%s\n",sysline);
#if !defined(__PPU__)
        int ret = system(sysline);
        (void)ret;
#endif


/*#endif */
    }
    else
    {
        if (OPTverbose>=5)
            printf("spit - CANT FIND BROWSER FOR OPENURL\n\n");
    }
    return(ok);
}

char *SPIT_geturl(char *url, const char *ToolNameBuf)
{
    if (OPTurl[0])
    {
        strcpy(url,OPTurl);
    }
    else
    {
        char namehtm[SPITMAXLINE];
        char *ext;

        strcpy(namehtm,ToolNameBuf);
        ext = strrchr(namehtm,'.');
        if (ext)
            *ext = 0;

        sprintf(url,"http://tools.eac.ea.com/util/%s.htm",namehtm);
    }

    if (*OPTbookmark)
        strcat(url, OPTbookmark);

    return(url);
}


bool SPIT_webhelp(const char *ToolNameBuf)
{
    static char url[SPITMAXLINE];

    return(SPIT_openurl(SPIT_geturl(url, ToolNameBuf)));
}



/* Checks if an option has a trailing question mark */

bool SPIT_optionhelp(char *out, const char *optptr)
{
    bool ok = false;

    *out = '-';
    ++out;
    while ((unsigned char)*optptr > (unsigned char)' ' && *optptr != ';')
    {
        *out = *optptr;
        ++optptr;
        ++out;
    }
    ok = *(out-1)=='?' && (*(out-2)!='-' && *(out-2)!='/');
    *(out-1) = '\0';

    return ok;
}

static bool SPIT_nameneedsquotes(const char *s)
{
    bool ok=false; // assume no quotes needed

    if (OPTautoquote && strchr(s,' '))
    {
        int hasquote=0;

        if (*s==(char)'(' || *s==(char)'{' || *s==(char)'[' || *s==(char)'"' || *s==(char)'`' || *s==(char)'\'')
            hasquote = 1;
        if (!hasquote)
        {
            int isopt=0;

            if (s[0]=='-' || (s[0]==OPTdot && s[1]!='.' && s[1]!='/' && s[1]!='\\'))
            {
                isopt = Zop(s+1)?1:0;
            }
            else if (s[0]=='=' || s[0]=='+' )
            {
                isopt = Zop(s)?1:0;
            }

            if (!isopt)
                ok = true;
        }
    }
    return(ok);
}

static char     OPTextrulebuf[SPITMAXLINE];
static char     OPTnamerulebuf[SPITMAXLINE];
static char     OPTpathrulebuf[SPITMAXLINE];
int  autohelp;
int  opterror;

static void SPIT_ini(void)
{
    if (!OPTurl)
        OPTurl = (char *) malloc(SPITMAXLINE);
    OPTurl[0] = '\0';

    if (!OPTbookmark)
    {
        OPTbookmark = (char *) malloc(SPITMAXFNAME);
        OPTbookmark[0] = '\0';
    }

    OPTexpandwildcards = 1;            /* Expand the wildcards */
    OPTkeepunfound = 1;                /* Keep unfound search strings in x-argv lists */
    OPTsorttype = 0;
    OPTuni = 0;
    OPTstrippaths = 0;                 /* Don't strip off paths from output files by default */
    OPTsortbydirs = 0;
    OPTuni = 0;
    OPTstartsortat = 0;
    OPTskiprules = 0;                  /* dont apply rules to first n args */
    OPTverbose =   2;
    OPTwrite =     1;
    OPTbanner =    1;
    OPTminargs =   1;
    OPTdestdir =   1;
    OPTkeepopts =  0;
    OPTlog =       1;
    OPTusehardspaces  = 1;
    OPTtime =      0;

    OPTextlist  = emptyString;
    OPTpathlist = emptyString;
    OPTfname    = emptyString;
    OPTenvpath  = emptyString;

    *OPTextrulebuf = '\0';
    *OPTpathrulebuf = '\0';
    *OPTnamerulebuf = '\0';

    /*OPTcreator =   'SPIT'; */
    OPTcreator =   'S'<<24 | 'P'<<16 | 'I'<<8 | 'T';
    /*OPTfiletype =  'TEXT'; */
    OPTfiletype =  'T'<<24 | 'E'<<16 | 'X'<<8 | 'T';
    opterror = 0;
    OPTrecurse = 0;
    OPTstartrecurseat = 0;
//    OPThistory = NULL;
    SPITmachine = SPIT_MACHINE;

    if (SPIT_initfunc)
        SPIT_initfunc();

    return;
}


/* Used to be in SPIT_parse (but are needed in Zop now too) */
char ToolNameBuf[SPITMAXLINE];


char *Zop(const char *opt)
{
    const char *optptr=opt;
    char optchar;
    char *tmpptr;
    char namebuf[SPITMAXLINE];
    char helpbuf[SPITMAXLINE];
    char urlbuf[SPITMAXLINE];

    SPIT_geturl(urlbuf, ToolNameBuf);

    if (!EA::StdC::Strnicmp(optptr,"sort",4) && !isalpha((unsigned char)*(optptr+4))) /* override -s user options */
    {
        optptr = SPIT_getopt( &OPTsorttype,optptr+4, "sort names", 0, 6);
        OPTsortbydirs = 1;
    }
    else if (!EA::StdC::Strnicmp(optptr,"nologo",6) && !isalpha((unsigned char)*(optptr+6))) /* override -n user options */
    {
        int OPTnologo;
        OPTnologo=!OPTbanner;
        optptr = SPIT_getopt( &OPTnologo,optptr+6, "disable logo", 0, 1);
        OPTbanner = !OPTnologo;
    }
    else
    {
        if (SPIT_optionhelp(helpbuf, optptr) && SPIT_usagefunc && !SPIThelpgiven)
        {
            SPIT_usagefunc(ToolNameBuf, helpbuf, urlbuf, SPITOPTIONHELP);
            SPIThelpgiven = 1;
            tmpptr = SPIT_getstropt( helpbuf,optptr+1, "? field", "", SPITYES, SPITMAXLINE);
        }
        else if (SPIT_optfunc)
        {
            tmpptr = SPIT_optfunc(optptr);
        }
        else
            tmpptr = (char *)0;
        if (tmpptr)
        {
            optptr = tmpptr;
        }
        else
        {
            optchar = (char) tolower((int) *optptr);
            switch (optchar)                    /* built-in options */
            {
                case 'b':
                    optptr = SPIT_getopt( &OPTbanner,optptr+1, "banner", 0, 1);
                    break;

                case '?':
                    optptr = SPIT_getstropt( helpbuf,optptr+1, "? field", "", SPITYES, SPITMAXLINE);
                    if (SPIT_isnumber(helpbuf))
                    {
                        optptr = SPIT_getopt2( &OPTverbose,optptr - strlen(helpbuf), "help", 0, 6, 3);
                        autohelp = 2;
                    }
                    else if (SPIT_usagefunc && !SPIThelpgiven)
                    {
                        SPIT_usagefunc(ToolNameBuf, helpbuf, urlbuf, SPITOPTIONHELP);
                        SPIThelpgiven = 1;
                    }
                    break;
                case 'h':
                    if (!EA::StdC::Strnicmp(optptr,"history",7))
                    {
                        optptr += 7;
                        if (!SPIThelpgiven)
                        {
                            if (SPIT_usagefunc)
                                SPIT_usagefunc(ToolNameBuf, NULL, NULL, 0);
                            if (OPThistory)
                            {
                                int i = 0;
                                while (OPThistory[i][0])
                                    printf("%s\n", OPThistory[i++]);
                            }
                            SPIThelpgiven = 1;
                        }
                    }
                    else if (!EA::StdC::Strnicmp(optptr,"help",4))
                    {
                        if (*(optptr+4) == '#')
                        {
                            optptr = SPIT_getstropt( OPTbookmark,optptr+4, "bookmark", "", SPITYES, SPITMAXFNAME);
                        }
                        else
                        {
                            OPTbookmark[0] = 0;
                            if ((unsigned char)optptr[4]>(unsigned char)' ')
                                OPTbookmark[0] = '#';

                            optptr = SPIT_getstropt( &(OPTbookmark[1]),optptr+4, "bookmark", "", SPITYES, SPITMAXFNAME);
                        }
                        strlwr(OPTbookmark);
                        autohelp = 3;
                    }
                    else
                    {
                        optptr = SPIT_getopt2( &OPTverbose,optptr+1, "help", 0, 6, 3);
                        autohelp = 2;
                    }
                    break;

                case 'n':
                    if (!EA::StdC::Strnicmp(optptr, "nologo", 6))
                    {
                        OPTbanner = !OPTbanner;
                        optptr = SPIT_getopt( &OPTbanner,optptr+6, "no logo", 0, 1);
                        OPTbanner = !OPTbanner;
                        break;
                    }
                    optptr = 0;
                    break;

                case 'r':
                     if (!EA::StdC::Strnicmp(optptr,"recurse:",8))
					     optptr += 8;
				     else
						 ++optptr;
                     {
                         /* Only works for environment variables without spaces */
                         int len = 0;

                         while (*(optptr + len)!=' ' && *(optptr + len)!='\0')
                             ++len;
                         if (len > 0)
                         {
                             OPTenvpath = (char *) malloc((size_t)len + 1);
                             OPTenvpath[len] = '\0';
                             strncpy(OPTenvpath, optptr, len);
                             optptr += len;
                             OPTrecurse = 2;
                         }
                         else
                             OPTrecurse = 1;
                     }
                     break;
                case 's':
                    if (!EA::StdC::Strnicmp(optptr,"sort",4))
                    {
                        optptr = SPIT_getopt( &OPTsorttype,optptr+4, "sort names", 0, 6);
                        OPTsortbydirs = 1;
                        break;
                    }
                    if (!EA::StdC::Strnicmp(optptr,"strippaths",10))
                    {
                        optptr = SPIT_getopt(&OPTstrippaths,optptr+10, "strip paths of destination filename", 0, 1);
                        break;
                    }
                    optptr = 0;
                    break;

                case 't':
                    if (!EA::StdC::Strnicmp(optptr,"time",4))
                    {
                        optptr = SPIT_getopt( &OPTtime,optptr+4, "enable timing diagnostics", 0, 1);
                        break;
                    }
                    optptr = 0;
                    break;

                case 'l':
                    if (!EA::StdC::Strnicmp(optptr,"log",3) && optptr[3]>='0' && optptr[3]<='1')
                    {
                        optptr = SPIT_getopt( &OPTlog,optptr+3, "log", 0, 2);
                        break;
                    }
                    optptr = 0;
                    break;

                case 'u':
                    if (!EA::StdC::Strnicmp(optptr,"uni",3))
                    {
                        optptr = SPIT_getopt( &OPTuni,optptr+3, "unique names", 0, 1);
                        break;
                    }
                    optptr = 0;
                    break;

                case 'v':
                    if (!EA::StdC::Strnicmp(optptr,"version",7))
                    {
                        /* Please note that this is just a hack */
                        /* the version check was already done in SPIT_parse and this is here just to acknowledge  */
                        /* that version is a valid option */
                        optptr = SPIT_getfield( OPTstr,SPIT_nextfield(optptr+7),true,true);
                    }
                    else
                    {
                        optptr = SPIT_getopt2( &OPTverbose,optptr+1, "verbose", 0, 6, 3);
                        if (*(optptr-1)!='?')
                            autohelp = 1;
                    }
                    break;

                case 'w':
                    optptr = SPIT_getopt( &OPTwrite,optptr+1, "write mode", 0, 1);
                    break;


                case '.':
                    optptr = SPIT_getfield(namebuf,SPIT_nextfield(optptr+1),true,true);
                    strcpy(OPTextrulebuf, namebuf);
                    OPTextrule = OPTextrulebuf;
                    break;

                case '=':
                    optptr = SPIT_getfield( namebuf,SPIT_nextfield(optptr+1),true,true);
                    if (SPIT_ispath(namebuf))
                    {
                        strcpy(OPTpathrulebuf,namebuf);
                        OPTpathrule = OPTpathrulebuf;
                    }
                    else
                    {
                        strcpy(OPTnamerulebuf,namebuf);
                        OPTnamerule = OPTnamerulebuf;
                    }
                    break;
                default:
                    if (OPTverbose)
                    {
                        SPIT_getfield( namebuf,opt,true,true);
                        printf("spit - 1 UNKNOWN OPTION <%s>\n", namebuf);
                        SPIThelpgiven = 1;
                    }

                    optptr = 0;
                    opterror = -1;
                    autohelp = 1;
            }
        }
    }

    if (optptr)
    {
        if ((unsigned char)*optptr>(unsigned char)' ' && *optptr!=HARDSPACE)
        {
            if (OPTverbose>=2 && !SPIThelpgiven)
            {
                SPIT_getfield( namebuf,opt,true,true);
                printf("spit - 2 UNKNOWN OPTION <%s>\n", namebuf);
                SPIThelpgiven = 1;
            }
            optptr = 0;
        }
    }
    return((char *)optptr);
}


void SPIT_init(void(*initfunc)(void),
           void(*usagefunc)(const char *,
           const char *, const char *, int),
           char *(*optfunc)(const char *),
           const char **spithistory
           )
{
    SPIT_initfunc = initfunc;
    SPIT_usagefunc = usagefunc;
    SPIT_optfunc = optfunc;
    OPThistory = spithistory;

    if (!initialized)
    {
        initialized = 1;
    }

    SPIT_ini();

    return;
}


//****************************************************************//
// Check if the the next field int the buffer is a path or a file.//
//****************************************************************//

bool SPIT_isnextfieldpath(char *s)
{
    char tempbuf[SPITMAXLINE];
    bool rc;
    SPIT_getfield(tempbuf,s, true,false);  // list?
    if(tempbuf[0] == '=')
        rc = SPIT_ispath(&(tempbuf[1]))?1:0;
    else
        rc = SPIT_ispath(tempbuf)?1:0;
    return rc;

}

// function unclumps a string of equals a=b=c=d and passes this as the innamebuf.
int Unclump(char *innamebuf)
{
    char * d = (innamebuf)+1;    // start at the begining minus the first = sign.
    int count = strlen(d);       // loop over the entire string.
    while(count && *d != '=')    // if I hit a second = sign
    {
        d++;                     // move over the aray
        count--;                 // and decrement my current position counter.
    }
    if(count)                    // if I didn't hit the end of the array then.
        *d = '\0';               // shorten my string.
    return(count?(count+1):(count));             // return my position relative to the end of the array including the =sign.
}



bool SPIT_parse(int argc, char * const * argv, const char *ToolName, int ToolVersion)
{
    char optchar;

    char *tmpptr;
    int  index = -1;
    int  i;
    int  j;
    int  stop = 0;
    int  inoption;
    int  numopts=0;
    int optbufsize;
    char *newToolName = NULL;
    char envnamebuf[SPITMAXLINE];
    char namebuf[SPITMAXLINE];
    char path[SPITMAXLINE];

    char *namelistptr = NULL;
    int  sargnum;

    char urlbuf[SPITMAXLINE];
    char innamebuf[SPITMAXLINE];
    char innamebufholder[SPITMAXLINE];                 // in case we have multiple bindings then
    char outnamebuf[SPITMAXLINE];                      // store the inname for future use.
    char helpbuf[SPITMAXLINE];

    SearchList *s;
    memset(innamebuf,0,SPITMAXLINE);
    memset(namebuf,0,SPITMAXLINE);
    memset(envnamebuf,0,SPITMAXLINE);
    memset(path,0,SPITMAXLINE);
    memset(urlbuf,0,SPITMAXLINE);
    memset(innamebufholder,0,SPITMAXLINE);
    memset(outnamebuf,0,SPITMAXLINE);
    memset(helpbuf,0,SPITMAXLINE);

    if (!initialized)
    {
        if (OPTverbose)
        {
            printf("spit - SPIT NOT INITIALIZED.  CALL SPIT_init(... )\n");
            return false;
        }
    }

    SPIT_ini();

    if(!optnamebuf)   // do this to keep namebuf in sync with
    {
        namelist = NULL;  // optnamebuf. There are cases where
        optlist  = NULL;  // SPIT_parse gets called multiple times
                          // and namebuf must be null if optnamebuf
                          // is.
    }
    if (optptr == NULL && firstcall)
    {
#ifdef __MWERKS__
        argc = ccommand(&argv);
#endif

        opterror = 0;
        autohelp = 1;
        SPIThelpgiven = 0;
        SPITargnum = 0;

/****************************************************************/
/*  Get Options Buffer                                          */
/****************************************************************/
        optbufsize = ((int) SPITMAXOPTBUF)*2L;
        optnamebuf = static_cast<char *>(malloc(optbufsize));
        if (!optnamebuf)
        {
            if (OPTverbose)
                printf("spit - ERROR ALLOCATING NAME BUFFER\n");
        }
        else
        {
            namelist =         optnamebuf;
            optlist =          optnamebuf + (int) SPITMAXOPTBUF;

            *namelist = '\0';
            *optlist = '\0';


/****************************************************************/
/*  Get Name of Program                                         */
/*  - make sure name points to something                        */
/*  - default to ToolName for name                              */
/*  - strip off path name and extension                         */
/****************************************************************/

            newToolName = (char *)*argv;
            if (newToolName)
            {   if (!*newToolName)
                    newToolName = 0;
            }
            if (!newToolName)
                newToolName = (char *)ToolName;
            SPIT_changeext(ToolNameBuf,SPIT_getname(ToolNameBuf,newToolName),"=");
            strlwr(ToolNameBuf);

/****************************************************************/
/*  Search For Environment Opts                                 */
/****************************************************************/

            sprintf(envnamebuf,"%s_OPTS",ToolNameBuf);
            strupr(envnamebuf);
            optptr = getenv(envnamebuf);   /* get application options */
            if (optptr)
            {
                strcat(optlist, optptr);
                strcat(optlist, " ");
            }

/****************************************************************/
/*  Fill in URL                                                 */
/****************************************************************/

            SPIT_geturl(urlbuf, ToolNameBuf);

/****************************************************************/
/*  Check for piped stdin                                       */
/****************************************************************/

#if defined(_MSC_VER) || defined(__WATCOMC__)
            {
                struct _stat buf;
                int result;

                result = _fstat( _fileno( stdin ), &buf );
                if (!result && buf.st_mode & _S_IFREG)
                {
                    /*if (!SPIT_responsefile("con:", optlist)) */
                    /*    SPIThelpgiven = 1; */

                    /* we dont need to worry about errors with SPIT_responsefile if coming form standard input */
                    SPIT_responsefile("con:", optlist+strlen(optlist));

/*                    strcat(optlist, "@con: "); */

                    /* Set "stdin" to have binary mode: */
                    result = _setmode( _fileno( stdin ), _O_BINARY );
                }
            }
#endif

/****************************************************************/
/*  Make Argument List into Option List                         */
/*  And expand any response files                               */
/****************************************************************/

            for (i=1; i<argc; ++i)
            {

                if (SPIT_nameneedsquotes(argv[i]))
                    sprintf(namebuf,"\"%s\"",argv[i]);
                else
                    strcpy(namebuf,argv[i]);


                if (!OPTlibnohardspace)
                {
                    if (OPTusehardspaces)
                    {
                        for (j=0; namebuf[j]; ++j)
                        {
                            if (namebuf[j]==' ')
                                namebuf[j] = HARDSPACE;     // convert spaces to preserve
                        }
                    }
                }
                // did the user specify a response file?
                if (namebuf[0] == '@' || namebuf[0] == '&' || (namebuf[0] == '\"' && (namebuf[1] == '@' || namebuf[1] == '&')))
                {
                    char rname[SPITMAXLINE];
                    int len;
                    // if we added quotes then we are going to have to strip off the quotes for our response file
                    // so that we can open it and process it.
                    if(namebuf[0] == '\"' && (namebuf[1] == '@' || namebuf[1] == '&'))
                    {
                        char *p = 0;
                        SPIT_getfield( rname,namebuf+2,false,false); // go two chars over to skip quotes and @ symbol.
                        p = rname + strlen(rname) - 1;       // jump to end of string and get rid of extra quote.
                        *p = '\0';
                    }
                    else
                        SPIT_getfield( rname,namebuf+1,false,false);
                    len = strlen(rname);
                    while ((unsigned char)rname[len]<=(unsigned char)' ' || rname[len]=='\0' || rname[len] == ';')
                    {
                        if (rname[len] == ';')
                        {
                            stop = 1;
                            rname[len] = '\0';
                        }
                        --len;
                    }
#if defined(__cplusplus) && defined(_MSC_VER)
                    SPIT_shortcut(rname, rname, 0);
#endif

                    if(!SPIT_responsefile(rname, optlist+strlen(optlist)))
                        SPIThelpgiven = 1;
                    if (stop == 1)
                    {
                        stop = 0;
                        strcat(optlist, " ; ");
                    }
                }
                else
                {
                    strcat(optlist, namebuf);
                    strcat(optlist, " ");
                }
            }
            if (!opterror)
                optptr = optlist;
            else
                optptr = NULL;
        }
    }
    else
    {
        freeFiles();
        if (namelist)
            *namelist = '\0';
    }

/****************************************************************/
/*  Pass 1:                                                     */
/*  - parse options                                             */
/*  - expand response files                                     */
/*  - build file name list for pass 2                           */
/****************************************************************/

    if (optptr)
    {
        namelistptr = namelist;

        inoption = 0;
        while (*optptr && !stop)
        {
            optchar = (char) tolower((int) *optptr);

            if (!inoption)
            {
                switch (optchar)
                {

#if defined(_MSC_VER)
                    case '/':
                        if (optptr[1]=='/')
                            goto notoptdefault;
#endif
                    case '-':
                        inoption = 1;
                        ++optptr;
                        ++numopts;
                        break;

                    case '^':
                        if (optptr[1]!='&')
                            goto notoptdefault;
                        ++optptr;

                    case ';':
                        stop = 1;
                        ++optptr;
                        break;

                    // = behaves like an option as well as a name
                    case '=':
                        optptr = SPIT_getfield( namebuf,SPIT_nextfield(optptr+1),true,true);

                        if (OPTkeepopts)
                        {
                            if (namebuf[0])
                            {
                                strcat(namelistptr,"=");
                                strcat(namelistptr,"\"");
                                strcat(namelistptr,namebuf);
                                strcat(namelistptr,"\"");
                                strcat(namelistptr," ");
                                namelistptr += strlen(namelistptr); // advance to end
                            }
                        }

                        if (SPIT_ispath(namebuf))
                        {
                            strcpy(OPTpathrulebuf,namebuf);
                            OPTpathrule = OPTpathrulebuf;
                        }
                        else
                        {
                            strcpy(OPTnamerulebuf,namebuf);
                            OPTnamerule = OPTnamerulebuf;
                        }
                        break;

                    case '?':
                        if ((unsigned char)*(optptr+1)<=(unsigned char)' ' && *(optptr+1)!=(char)HARDSPACE)
                        {
                            OPTverbose = 3;
                            ++optptr;
                            autohelp = 2;
                            break;
                        }

                    default:
notoptdefault:
                        if ((unsigned char)optchar<=(unsigned char)' ' || optchar==(char)',')
                            ++optptr;
                        else
                        {

                            int len;
                            optptr = SPIT_getfield( namebuf,optptr,false,true);

                            len = strlen(namebuf);
                            while (((unsigned char)namebuf[len]<=(unsigned char)' ' || namebuf[len]=='\0' || namebuf[len] == ';') && (len >= 0))
                            {
                                if (namebuf[len] == ';')
                                {
                                    stop = 1;
                                    namebuf[len] = '\0';
                                }
                                --len;
                            }

                            strcat(namelistptr,namebuf);
                            strcat(namelistptr," ");
                            namelistptr += strlen(namelistptr); // advance to end
                        }
                        break;
                }
            }
            else /* inoption */
            {
                int len;

                if (OPToptlower)
                    *optptr = (char)(tolower(*optptr));

                SPIT_getfield( namebuf,optptr,false,true);
                len = strlen(namebuf);
                while (len>=0 && (((unsigned char)namebuf[len]<=(unsigned char)' ' || namebuf[len]=='\0' || namebuf[len] == ';')))
                {
                    if (namebuf[len] == ';')
                    {
                        stop = 1;
                        namebuf[len] = ' ';
                        optptr[len] = ' ';
                    }
                    --len;
                }

                if (OPTkeepopts)
                {
                    if (namebuf[0])
                    {
                        strcat(namelistptr,"-");
                        strcat(namelistptr,namebuf);
                        strcat(namelistptr," ");
                        namelistptr += strlen(namelistptr); // advance to end
                    }
                }

                if (!EA::StdC::Strnicmp(optptr,"sort",4) && !isalpha((unsigned char)*(optptr+4))) /* override -s user options */
                {
                    optptr = SPIT_getopt( &OPTsorttype,optptr+4, "sort names", 0, 6);
                    OPTsortbydirs = 1;
                }
                else if (!EA::StdC::Strnicmp(optptr,"nologo",6))
                {
                    OPTbanner = !OPTbanner;
                    optptr = SPIT_getopt( &OPTbanner,optptr+6, "no logo", 0, 1);
                    OPTbanner = !OPTbanner;
                }
                else
                {
                    if (SPIT_optionhelp(helpbuf, optptr) && SPIT_usagefunc && !SPIThelpgiven)
                    {
                        SPIT_usagefunc(ToolNameBuf, helpbuf, urlbuf, SPITOPTIONHELP);
                        SPIThelpgiven = 1;
                        tmpptr = SPIT_getstropt( helpbuf,optptr+1, "? field", "", SPITYES, SPITMAXLINE);
                    }
                    else if (SPIT_optfunc)
                    {
                        tmpptr = SPIT_optfunc(optptr);
                    }
                    else
                        tmpptr = (char *)0;
                    if (tmpptr)
                    {
                        optptr = tmpptr;
                        inoption = 0;
                    }
                    else
                    {
                        optchar = (char) tolower((int) *optptr);
                        switch (optchar)                    /* built-in options */
                        {
                            case 'b':
                                optptr = SPIT_getopt( &OPTbanner,optptr+1, "banner", 0, 1);
                                break;

                            case 'n':
                                if (!EA::StdC::Strnicmp(optptr, "nologo", 6))
                                {
                                    OPTbanner = !OPTbanner;
                                    optptr = SPIT_getopt( &OPTbanner,optptr+6, "no logo", 0, 1);
                                    OPTbanner = !OPTbanner;
                                    break;
                                }
                                goto optdefault;

                            case 'l':
                                if (!EA::StdC::Strnicmp(optptr,"log",3) && optptr[3]>='0' && optptr[3]<='1')
                                {
                                    optptr = SPIT_getopt( &OPTlog,optptr+3, "log", 0, 2);
                                    break;
                                }
                                goto optdefault;

                            case '?':
                                optptr = SPIT_getstropt( helpbuf,optptr+1, "? field", "", SPITYES, SPITMAXLINE);
                                if (SPIT_isnumber(helpbuf))
                                {
                                    optptr = SPIT_getopt2( &OPTverbose,optptr - strlen(helpbuf), "help", 0, 6, 3);
                                    autohelp = 2;
                                }
                                else if (SPIT_usagefunc && !SPIThelpgiven)
                                {
                                    SPIT_usagefunc(ToolNameBuf, helpbuf, urlbuf, SPITOPTIONHELP);
                                    SPIThelpgiven = 1;
                                }
                                break;
                            case 'h':
                                if (!EA::StdC::Strnicmp(optptr,"history",7))
                                {
                                    optptr += 7;
                                    if (!SPIThelpgiven)
                                    {
                                        if (SPIT_usagefunc)
                                            SPIT_usagefunc(ToolNameBuf, NULL, NULL, 0);
                                        if (OPThistory)
                                        {
                                            int i = 0;
                                            while (OPThistory[i][0])
                                                printf("%s\n", OPThistory[i++]);
                                        }
                                        SPIThelpgiven = 1;
                                    }
                                }
                                else if (!EA::StdC::Strnicmp(optptr,"help",4))
                                {
                                    if (*(optptr+4) == '#')
                                    {
                                        optptr = SPIT_getstropt( OPTbookmark,optptr+4, "bookmark", "", SPITYES, SPITMAXFNAME);
                                    }
                                    else
                                    {
                                        OPTbookmark[0] = 0;
                                        if ((unsigned char)optptr[4]>(unsigned char)' ')
                                            OPTbookmark[0] = '#';

                                        optptr = SPIT_getstropt( &(OPTbookmark[1]),optptr+4, "bookmark", "", SPITYES, SPITMAXFNAME);
                                    }
                                    strlwr(OPTbookmark);
                                    autohelp = 3;
                                }
                                else
                                {
                                    optptr = SPIT_getopt2( &OPTverbose,optptr+1, "help", 0, 6, 3);
                                    autohelp = 2;
                                }
                                break;

                            case 'r':
			                     if (!EA::StdC::Strnicmp(optptr,"recurse:",8))
								     optptr += 8;
							     else
									 ++optptr;
                                 {
                                    /* Only works for environment variables without spaces */
                                    int len = 0;

                                    while (*(optptr + len)!=' ' && *(optptr + len)!='\0')
                                        ++len;
                                    if (len > 0)
                                    {
                                        OPTenvpath = (char *)malloc((size_t)len + 1);
                                        OPTenvpath[len] = '\0';
                                        strncpy(OPTenvpath, optptr, len);
                                        optptr += len;
                                        OPTrecurse = 2;
                                    }
                                    else
                                        OPTrecurse = 1;
                                }
                                break;

                            case 's':
                                if (!EA::StdC::Strnicmp(optptr,"sort",4))
                                {
                                    optptr = SPIT_getopt( &OPTsorttype,optptr+4, "sort names", 0, 6);
                                    OPTsortbydirs = 1;
                                    break;
                                }
                                if (!EA::StdC::Strnicmp(optptr,"strippaths",10))
                                {
                                    optptr = SPIT_getopt(&OPTstrippaths,optptr+10, "strip paths of destination filename", 0, 1);
                                    break;
                                }
                                goto optdefault;

                            case 't':
                                if (!EA::StdC::Strnicmp(optptr,"time",4))
                                {
                                    optptr = SPIT_getopt( &OPTtime,optptr+4, "enable timing diagnostics", 0, 1);
                                    break;
                                }
                                goto optdefault;

                            case 'u':
                                if (!EA::StdC::Strnicmp(optptr,"uni",3))
                                {
                                    optptr = SPIT_getopt( &OPTuni,optptr+3, "unique names", 0, 1);
                                    break;
                                }
                                goto optdefault;

                            case 'v':
                                if (!EA::StdC::Strnicmp(optptr,"version",7))
                                {
//                                    int ToolVersion=0;
//                                    int ccmajor=0, ccminor=0, args;
                                    optptr = SPIT_getfield( OPTstr,SPIT_nextfield(optptr+7),true,false);
                                    OPTversion = (float) atof(OPTstr);

//                                    args = sscanf(versionstr,"%d.%d",&ccmajor,&ccminor);
//                                    if (args==2)
//                                        ToolVersion = ccmajor*100+ccminor;

                                    if (!ToolVersion)
                                    {
                                        /* error - version number expected in textversion */
                                        printf("spit - No version number in TextVersion string\n");
                                        exit (101);
                                    }
                                    if ((ToolVersion) < (int)(OPTversion*100.0f+0.5f))
                                    {
                                        if (OPTverbose)
                                            printf("spit - The current version is %.2f; Please get the latest version of %s\n", (float)ToolVersion/100.0f, ToolName);
                                        exit(101);
                                    }
                                }
                                else
                                {
                                    optptr = SPIT_getopt2( &OPTverbose,optptr+1, "verbose", 0, 6, 3);
                                    if (*(optptr-1)!='?')
                                        autohelp = 1;
                                }
                                break;

                            case 'w':
                                optptr = SPIT_getopt( &OPTwrite,optptr+1, "write mode", 0, 1);
                                break;

                            case '.':
                                optptr = SPIT_getfield(namebuf,SPIT_nextfield(optptr+1),true,true);
                                strcpy(OPTextrulebuf, namebuf);
                                OPTextrule = OPTextrulebuf;
                                break;

                            case '=':
                                optptr = SPIT_getfield( namebuf,SPIT_nextfield(optptr+1),true,true);
                                if (SPIT_ispath(namebuf))
                                {
                                    strcpy(OPTpathrulebuf,namebuf);
                                    OPTpathrule = OPTpathrulebuf;
                                }
                                else
                                {
                                    strcpy(OPTnamerulebuf,namebuf);
                                    OPTnamerule = OPTnamerulebuf;
                                }
                                break;

                            case ',':
                                ++optptr;
                                break;

                            case ';':
                                stop = 1;
                                ++optptr;
                                break;

                            default: ;
optdefault:
                                if ((unsigned char)optchar<=(unsigned char)' ' && optchar!=(char)HARDSPACE)
                                {
                                    inoption = 0;
                                    ++optptr;
                                }
                                else
                                {
                                    SPIT_getfield( namebuf,optptr-1,true,true);
                                    if (OPTverbose)
                                    {
                                        printf("spit - 3 UNKNOWN OPTION <%s>\n", namebuf);
                                        SPIThelpgiven = 1;
                                    }

                                    optptr = emptyString;
                                    opterror = -1;
                                    autohelp = 1;
                                }
                                break;
                        }
                    }
                }
            }
        }

/****************************************************************/
/*  Pass 2:                                                     */
/*  - show banner or help                                       */
/*  - expand wildcards                                          */
/*  - process file name list into SPITsargv, SPITdargv and SPITargnum       */
/****************************************************************/

        /* count args, removing last path */

        {
            char *nextname;
            char *lastpath=0;
            int filenamecount=0;

            sargnum = 0;
            namelistptr = namelist;
            while (namelistptr)
            {
                nextname = SPIT_nextnames(namelistptr,innamebuf,outnamebuf);
                if (nextname)
                {
                    ++sargnum;
                    if (SPIT_ispath(innamebuf))
                        lastpath = namelistptr;
                    if(innamebuf[0] != '-')     // if we have another file name and this is indeed not the
                    {
                        lastpath = 0;           // last non option on the line the reset the lastpath so we don't
                        ++filenamecount;        // use what's currently there.
                    }
                }
                namelistptr = nextname;
            }
            if (lastpath && filenamecount>1)
            {
                int i = 0;

                nextname = SPIT_nextnames(lastpath,OPTpathrulebuf,outnamebuf);
                // getting rid of the -= to get the actual path
                if (OPTpathrulebuf[0] == '-')
                    i++;
                if (OPTpathrulebuf[1] == '=')
                    i++;

                OPTpathrule = OPTpathrulebuf+i;
                strcpy(lastpath,nextname);      /* remove from arglist */
                --sargnum;                      /* drop count by 1 */

                if (OPTverbose>=6)
                    printf("destdir '%s'\n",OPTpathrule);
            }
        }

        inoption = 0; /* Use inoption to count number of "source.ext -options =dest.ext" combined */
        SPITargnum = 0;
        namelistptr = namelist;
        s = CreateSearchList();

        {
            int disablerule=OPTskiprules;
            int IsPaired = 0;                       // for bookeeping if I have one binding already a=b
            for (i=0; i<sargnum; ++i)
            {
                int isopt=0;
                if (namelistptr[0]=='-' || (namelistptr[0]==OPTdot && namelistptr[1]!='.' && namelistptr[1]!='/' && namelistptr[1]!='\\'))
                {
                    isopt = Zop(namelistptr+1)?1:0;
                }
                else if (namelistptr[0]=='=' || namelistptr[0]=='+' )
                {
                    isopt = Zop(namelistptr)?1:0;
                }

                namelistptr = SPIT_nextnames(namelistptr,innamebuf,outnamebuf);

                /* Replace hard spaces */
                for (j=0; innamebuf[j]; ++j)
                {
                    if (innamebuf[j]==HARDSPACE)
                        innamebuf[j] = ' ';
                }
                for (j=0; outnamebuf[j]; ++j)
                {
                    if (outnamebuf[j]==HARDSPACE)
                        outnamebuf[j] = ' ';
                }

                if (!outnamebuf[0] && OPTnamerule[0] && !disablerule)
                {
                    if(innamebuf[0] == '=' && IsPaired)         // if we get another =b we want to check
                    {                                           // if there was a pervious pair... a=c=b so that
                        int otheroptsamount = Unclump(innamebuf);     // unclump any clumped together =signs
                        namelistptr = namelistptr - otheroptsamount;  // decrement my namelist pointer to deal with this unclumped amount.
                        //removeQuotes(innamebuf);                // since we might keep quotes around the string to fudge quote support in Win2K
                        strcpy(outnamebuf,(innamebuf+1));       // I can duplicate the entry in the search list.
                        strcpy(innamebuf,innamebufholder);      // make the innamebuf the old file and make the new binding the outname buf.
                        if(otheroptsamount)                     // if we discovered that we needed to unclump then increase loop
                            sargnum++;                          // amount by one.

                    }
                    else
                        strcpy(outnamebuf,OPTnamerule);         // otherwise set to default binding if there is one.
                }

                if (innamebuf[0] && !outnamebuf[0])
                {
                    if (SPIT_typeofname(innamebuf) != SPITPATHOPT)
                        index = i-inoption;

                    AddToSearchList(s, innamebuf, innamebuf);
                    IsPaired = 0;                               // nope no more bindings after a=b so reset here as well just to be sure.
                }
                else if (!innamebuf[0] && outnamebuf[0])
                {
                    if (index >= 0)
                    {
                        /* If combining to the destination name */
                        if (OPTnamecombine == 1)
                        {
                            RemoveFromSearchList(s, index, innamebuf, NULL);
                            AddToSearchList(s, innamebuf, outnamebuf);
                            IsPaired = 0;                           // nope no more bindings after the first reset.
                            ++inoption;
                        }
                        /* Else if combining to the source name */
                        else
                        {
                            UpdateSearchList(s, index, NULL, outnamebuf);
                            IsPaired = 0;                           // nope no more bindings after the first reset
                        }
                        index = -1;
                    }
                    else
                    {
                        if (OPTverbose>3)
                            printf("WARNING - No source filename match to <%s>, using as source and destination.\n", outnamebuf);
                        AddToSearchList(s, outnamebuf, outnamebuf);
                        IsPaired = 0;                                   // nope no more bindings after the first reset.
                    }
                }
                else if (innamebuf[0] && outnamebuf[0])
                {
                    if (innamebuf[0] == '=' && IsPaired)                    // if had a =b =c =d  we will get innamebuf = "=c" outnamebuf = "d"
                    {
                        char holderbuf[SPITMAXLINE];                    // to hold my second search list entry.
                        strcpy(holderbuf,outnamebuf);                   // make a backup copy of second search list entry.
                        strcpy(outnamebuf,(innamebuf+1));               // wasteful I know but do in order and put innamebuf (without the = sign) into search list.
                        strcpy(innamebuf,innamebufholder);              // copy over the current file name and get ready to add to search list.
                        AddToSearchList(s, innamebuf, outnamebuf);      // add this particular entry to the search list in question.
                        strcpy(outnamebuf,holderbuf);                   // make a second entry in the search list with the second out parameter.
                        AddToSearchList(s, innamebuf, outnamebuf);      // add this second entry to the search list.
                        index = -1;                                     // do my bookeeping for this case and then continue.
                    }
                    else
                    {
                        AddToSearchList(s, innamebuf, outnamebuf);
                        IsPaired = 1;                                   // we have a binding a=b do this just in case there is another
                        strcpy(innamebufholder,innamebuf);              // binding following this a=b=c
                        index = -1;
                    }
                }
                if (disablerule && !isopt)
                    --disablerule;
            }
        }
        //SPIT_figureoutRealPath(s,path);
        /* Apply name and path rules */
        ApplyNameRules(s);

        /* Find absolute path names */
        FindAbsolutes(s);

        /* Find directory and normal names */
        RGetCWD(path, SPITMAXLINE);

        if (OPTrecurse == 2)
        {
            char *envpointer = OPTenvpath;
            char *envstart = OPTenvpath;
            char envstring[SPITMAXLINE];
            int cont = 1;
            while (cont)
            {
                if (*envpointer == (char)';' || *envpointer == (char)'\0')
                {
                    strncpy(envstring, envstart, envpointer - envstart);
                    envstring[envpointer - envstart] = '\0';
                    FindRecursive(envstring, emptyString, s, 0);
                    envstart = envpointer + 1;
                }
                cont = !(*envpointer == (char)'\0');
                ++envpointer;
            }
        }
        else
        {
            FindRecursive(path, path, s, 0);
        }

        /* Apply extension rule to destination names */
        ApplyExtRule(s);

        /* Turn the SearchList 's' into SPITsargv, SPITdargv */
        MakeArgvs(s);

        DestroySearchList(s);



        if (OPTverbose>=6)
            printf("OPTsorttype %d\n",OPTsorttype);

        /* Sort alphabetically by directory */
        if (OPTsortbydirs)
            SPIT_qsort(OPTstartsortat, SPITargnum - 1, OPTsorttype);

        /* Sort with user method excluding path (keeping directories distinct) */
        if (OPTsorttype != 0)
        {
            char lastdir[SPITMAXLINE];
            char dir[SPITMAXLINE];
            int i;
            int start = OPTstartsortat;
            int sorted = 0;

            SPIT_getpath( lastdir,SPITsargv[0]);
            for (i = OPTstartsortat; i < SPITargnum; ++i)
            {
                SPIT_getpath( dir,SPITsargv[i]);
                // this ensures that all the files within the same folder are sorted separately
                // from files in other folders
                // NOTE: this will sort all preceding options if the input files are given with full paths.
                if (strcmp(dir, lastdir))
                {
                    sorted = 1;
                    SPIT_qsort(start, i-1, OPTsorttype);
                    start = i;
                    strcpy(lastdir, dir);
                }
                // if its the last argument and the path is given, sort it! otherwise it will never get sorted when it should
                else if ((dir != '\0') && (i == (SPITargnum-1)))
                {
                    SPIT_qsort(start, SPITargnum - 1, OPTsorttype);
                }
            }
            if (!sorted)
                SPIT_qsort(OPTstartsortat, SPITargnum - 1, OPTsorttype);
        }

        {
            long nameargs;
            nameargs = SPITargnum;
            if (OPTkeepopts)
                nameargs -= numopts;

            if (!SPIThelpgiven && autohelp<2)  /* is there a name list? */
            {
                if (SPITargnum<OPTminargs)
                    autohelp = 1;
                else
                    autohelp = 0;

                if (OPTbanner && firstcall && nameargs>0 && OPTverbose>=3 && SPIT_usagefunc)
                    SPIT_usagefunc(ToolNameBuf, envnamebuf, urlbuf, 0);
            }
        }

        if (OPTverbose>=6)
            printf("OPTuni %d\n",OPTuni);

        /* Sort with user method excluding path (keeping directories distinct) */
        if (OPTuni)
        {
            int k=0; // insert point
            for (int i = 0; i < SPITargnum; ++i)
            {
                bool isDuplicate=false;

                int isopt=0;

                if (SPITsargv[i][0]=='-' || (SPITsargv[i][0]==OPTdot && SPITsargv[i][1]!='.' && SPITsargv[i][1]!='/' && SPITsargv[i][1]!='\\'))
                {
                    isopt = Zop(SPITsargv[i]+1)?1:0;
                }
                else if (SPITsargv[i][0]=='=' || SPITsargv[i][0]=='+' )
                {
                    isopt = Zop(SPITsargv[i])?1:0;
                }

                if (!isopt)
                {
                    for (int j = 0; j < i; ++j)
                    {
                        if (!EA::StdC::Stricmp(SPITsargv[i],SPITsargv[j]))
                        {
                            isDuplicate = true;
                            break;
                        }
                    }
                }
                if (!isDuplicate)
                {
                    SPITsargv[k] = SPITsargv[i];
                    SPITdargv[k] = SPITdargv[i];
                    ++k;
                }
            }


            if (OPTverbose>=6)
                printf("OPTuni %d removed %d duplicates\n",OPTuni,SPITargnum-k);

            SPITargnum = k;
        }

        if (!SPIThelpgiven && autohelp)
        {
            bool ok=false;

            if (autohelp==3)
            {
                ok = SPIT_webhelp(ToolName);
            }

            if (!ok)
            {
                if (OPTverbose>=2 && SPIT_usagefunc)
                    SPIT_usagefunc(ToolNameBuf, envnamebuf, urlbuf, OPTverbose);
            }
            SPIThelpgiven = 1;
        }
    }

    if (firstcall == 1)
        firstcall = 0;

    {
        /* calculate return value before restoring since restore clears the optbuffer */
        bool return_value = false;
        if (opterror)
            return_value = opterror!=0;
        else if (*optptr)
            return_value = true;
        else
            return_value = false;

        if (OPTrecurse == 2)
        {
            if (OPTenvpath) free(OPTenvpath);
            OPTenvpath = NULL;
        }

        if (SPIThelpgiven)
        {
            return_value = false;
            SPIT_restore();
        }

        return return_value;
    }
}

/* Applies the extension rule to each destination match for each file */
void ApplyExtRule(SearchList *s)
{
    char temp[SPITMAXLINE];
    int i;
    int j;

    if (*OPTextrule != '\0')
    {
        for (i = 0; i < s->numfiles; ++i)
        {
            /* This is for the matched files */
            if (s->files[i]->nummatches > 0)
            {
                for (j = 0; j < s->files[i]->nummatches; ++j)
                {
                    replaceExt(temp, s->files[i]->matches[j]->d, OPTextrule);
                    free(s->files[i]->matches[j]->d);
                    s->files[i]->matches[j]->d = (char *) malloc(strlen(temp)+1);
                    strcpy(s->files[i]->matches[j]->d, temp);
                }
            }
            /* This is for all others (including those that would have matched */
            /* but for efficiency sake were passed over - ex OPTkeepunfound=1) */
            else
            {
                    replaceExt(temp, s->files[i]->wildout, OPTextrule);
                    free(s->files[i]->wildout);
                    s->files[i]->wildout = (char *)malloc(strlen(temp)+1);
                    strcpy(s->files[i]->wildout, temp);
            }
        }
    }

    return;
}

/* Applies name rules to each wildin/wildout in a SearchList */
void ApplyNameRules(SearchList *s)
{
    char temp[SPITMAXLINE];
    int i;
    int disablerule=OPTskiprules;

    for (i=0; i<s->numfiles; ++i)
    {
        if (s->files[i]->wildout[0] == '\0')
        {
            if (*OPTnamerule!='\0' && !disablerule)
            {
                free(s->files[i]->wildout);
                s->files[i]->wildout = (char *)malloc(strlen(OPTnamerule)+1);
                strcpy(s->files[i]->wildout, OPTnamerule);
            }
            else
            {
                free(s->files[i]->wildout);
                s->files[i]->wildout = (char *)malloc(strlen(s->files[i]->wildin)+1);
                strcpy(s->files[i]->wildout, s->files[i]->wildin);
            }
        }
        if (OPTpathrule && *OPTpathrule!='\0' && !disablerule)
        {
            if(s->files[i]->type == SPITPATHDIR && !SPIT_haswild(s->files[i]->wildout))
                replacePathAbs(temp, s->files[i]->wildout, OPTpathrule);
            else
                replacePath(temp, s->files[i]->wildout, OPTpathrule);
            free(s->files[i]->wildout);
            s->files[i]->wildout = (char *)malloc(strlen(temp)+1);
            strcpy(s->files[i]->wildout, temp);
        }
        if (disablerule && s->files[i]->wildin[0]!='-' && s->files[i]->wildin[0]!='+')
            --disablerule;
    }
    return;
}

//--------------------------------------------------------------------------------------
// figureout where the use wants me to recurse from.

/* Rips directory off of front of path */
char *GetFirstDir(const char *fname, char *dir)
{
    const char *s = fname;

    while(*s != '\\' && *s != '/' && *s != '\0')
        ++s;
    strncpy(dir, fname, s - fname);
    dir[s - fname] = '\0';
    return((char *)s);
}


/* Traces multiple directory filenames that can include wildcards */
int FollowDirs(char *in, FileInfo *f, char *path, char *result)
{
    RFindData c_file;
    RHandle hFile;

    char *dir     = (char *) malloc(SPITMAXLINE);
    char *curdir  = (char *) malloc(SPITMAXLINE);
    char *newname = (char *) malloc(SPITMAXLINE);

    char *ptr;
    int numfiles = 0;
    int getdir;
    newname[0] = '\0';
    dir[0]     = '\0';
    curdir[0]  = '\0';
    /* Get the first directory and assign getdir */
    ptr = GetFirstDir(in, dir);
    getdir = (*ptr != '\0');

    /* Check if the first directory is a drive (like D:\) */
    if (isalpha((unsigned char)dir[0]) && (dir[1] == ':') && (dir[2] == '\0'))
    {
        dir[2] = '\\';
        dir[3] = '\0';
        (void)RChDir(dir);
        strncpy(newname, dir,SPITMAXLINE);

        RGetCWD(curdir, SPITMAXLINE);
        numfiles += FollowDirs(ptr+1, f, emptyString, newname);
        (void)RChDir(curdir);
    }
    /* Else if first directory is a relative drive path like D:BOB.MOV */
    else if (isalpha((unsigned char)dir[0]) && (dir[1] == ':') && (dir[2] != '\0'))
    {
        ptr -= strlen(&dir[2]);
        dir[2] = '\0';
        (void)RChDir(dir);
        strncpy(newname, dir,SPITMAXLINE);

        RGetCWD(curdir, SPITMAXLINE);
        numfiles += FollowDirs(ptr, f, emptyString, newname);
        (void)RChDir(curdir);
    }
    /* Else if first directory is a UNC name that begins with \\... */
    /* just take the entire UNC path */
    else if((dir[0] == '\0') && (ptr[0] == '\\') && (ptr[1] == '\\'))
    {
        {
            char *root = (char *) malloc(SPITMAXLINE);

            strncpy(root, ptr,SPITMAXLINE);

            int length = strlen(in);
            ptr = ptr+length;

            // look for the entire path but not the filename
#if defined(MSDOS) || defined(_WIN32) || defined(__WATCOMC__) || defined(_MSC_VER)
            while (*ptr != '\\')
            {
                ptr--;
                length--;
            }
#else
            while (*ptr != '/')
            {
                ptr--;
                length--;
            }
#endif

            strncpy(newname, root, ++length);
            newname[length] = '\0'; //  make sure that its null terminated
            free(root);
        }
        (void)RChDir(newname);
        RGetCWD(curdir, SPITMAXLINE);
        numfiles += FollowDirs(ptr+1, f, emptyString, newname);
        (void)RChDir(curdir);
    }
    /* Else if first directory is an absolute path like \games\... */
    else if(dir[0] == '\0')
    {
        RGetCWD(curdir, SPITMAXLINE);
        strncpy(dir, curdir,SPITMAXLINE);
#if defined(MSDOS) || defined(_WIN32) || defined(__WATCOMC__) || defined(_MSC_VER)
        dir[2] = '\\';
        dir[3] = '\0';
#endif
        (void)RChDir(dir);

#if defined(MSDOS) || defined(_WIN32) || defined(__WATCOMC__) || defined(_MSC_VER)
        strcpy(newname, "\\");
#else
        strcpy(newname, "/");
#endif

        numfiles += FollowDirs(ptr + 1, f, emptyString, newname);
        (void)RChDir(curdir);
    }
    else if ((hFile = RFindFirst(&c_file)) != RNOTFOUND)
    {
        char iswild = (char)(SPIT_iswildcards(dir));
        Match match;

        // malloc'ed because recursion uses too much stack
        char *tmpname    = (char *) malloc(SPITMAXLINE);
        char *destbuf    = (char *) malloc(SPITMAXLINE);
        match.s = (char *)malloc(SPITMAXLINE);
        match.d = (char *)malloc(SPITMAXLINE);

        do
        {
            f->matched = (char)0;
            if (getdir)
            {
                char *currentdir = (char *) malloc(SPITMAXLINE);

                strcpy(tmpname, RName(c_file));

                /* Don't allow .. or . unless the dir they specified is .. or . */
                if (!( (!strcmp(tmpname, "..") && strcmp(dir, "..")) ||  (!strcmp(tmpname, ".") && strcmp(dir, ".")) ) )
                {
                    if (RAttrib(c_file) & RSUBDIR && !(RAttrib(c_file) & RHIDDEN))
                    {
                        if (!strcmp(tmpname, ".."))
                        {
                            RGetCWD(currentdir, SPITMAXLINE);
                        }
                        else if (!strcmp(tmpname, "."))
                            strcpy(currentdir, ".");
                        else
                            strcpy(currentdir, "..");
                        if (pathmatchfun(destbuf, tmpname, dir, emptyString))
                        {
                            (void)RChDir(RName(c_file));

#if defined(MSDOS) || defined(_WIN32) || defined(__WATCOMC__) || defined(_MSC_VER)  // linux or DOS? do the slash accordingly.
                            sprintf(newname, "%s%s\\", result, RName(c_file));
#else
                            sprintf(newname, "%s%s/", result, RName(c_file));
#endif

                            numfiles += FollowDirs(ptr+1, f, path, newname);
                            (void)RChDir(currentdir);
                            f->matched = (char)1;
                        }
                    }
                }
                free(currentdir);
            }
            else
            {
                /* I know... it's yucky - but it does powerful checking */
                if ((strcmp(RName(c_file), ".") && strcmp(RName(c_file), "..")))
                {
                    if (!(RAttrib(c_file) & RHIDDEN))
                    {
                        sprintf(tmpname, "%s%s", result, RName(c_file));
                        if (pathmatchfun(destbuf, tmpname, f->wildin, f->wildout))
                        {
                            f->matched = (char)1;
                            sprintf(newname, "%s%s", result, RName(c_file));
                        }
                        if (f->matched)
                        {
                            ++numfiles;

                            if (*path != '\0')
                            {
                                char * destbufunrel = 0;
                                int len = strlen(path)-1;

                                if(OPTstrippaths)    // if I'm going to strip off the path information then setup my buffer.
                                    destbufunrel = SPIT_makeunrelative(f->hasout,f->wildout,result,destbuf);
                                if (path[len] == '\\' || path[len] == '/')
                                {
                                    sprintf(match.s, "%s%s", path, newname);
                                    if(OPTstrippaths)
                                        sprintf(match.d, "%s%s", path, destbufunrel);
                                    else
                                        sprintf(match.d, "%s%s", path, destbuf);
                                }
                                else
                                {
#if defined(MSDOS) || defined(_WIN32) || defined(__WATCOMC__) || defined(_MSC_VER)
                                    sprintf(match.s, "%s\\%s", path, newname);
                                    if(OPTstrippaths)
                                        sprintf(match.d, "%s\\%s", path, destbufunrel);
                                    else
                                        sprintf(match.d, "%s\\%s", path, destbuf);
#else
                                    sprintf(match.s, "%s/%s", path, newname);
                                    if(OPTstrippaths)
                                        sprintf(match.d, "%s/%s", path, destbufunrel);
                                    else
                                        sprintf(match.d, "%s/%s", path, destbuf);
#endif
                                }
                                /* If match.d now contains (ex. d:\temp\c:\temp) then change it */
                                if (SPIT_typeofname(destbuf) != SPITPATHABS)
                                {
                                    AddMatch(f, &match);
                                }
                                else
                                {
                                    if(OPTstrippaths)
                                        strcpy(match.d, destbufunrel);
                                    else
                                        strcpy(match.d, destbuf);
                                    AddMatch(f, &match);
                                }
                                if(destbufunrel)
                                    free(destbufunrel);
                            }
                            else
                            {
                                char * destbufunrel = 0;

                                strcpy(match.s, newname);
                                if(OPTstrippaths)
                                {
                                    destbufunrel = SPIT_makeunrelative(f->hasout,f->wildout,result,destbuf);
                                    strcpy(match.d, destbufunrel);
                                }
                                else
                                    strcpy(match.d, destbuf);
                                AddMatch(f, &match);
                                if(destbufunrel)
                                    free(destbufunrel);
                            }
                        }
                    }
                }
            }
        }while(RFindNext(hFile, &c_file) && !(!(iswild) && f->matched));
        RFindClose(hFile, &c_file);
        free(match.s);
        free(match.d);
        free(tmpname);
        free(destbuf);
        match.s = NULL;
        match.d = NULL;
        tmpname = NULL;
        destbuf = NULL;
    }
    free(dir);
    free(curdir);
    free(newname);

    return numfiles;
}


/* Finds files of type SPITPATHABS */
int FindAbsolutes(SearchList *s)
{
    char currentdir[SPITMAXLINE];
    char fname[SPITMAXLINE];
    int numfiles = 0;
    int i;

    if (!OPTrecurse)  // if we are recursing we will find files with absolute paths, don't do it here.
    {
        RGetCWD(currentdir, SPITMAXLINE);
        for (i = 0; i < s->numfiles; ++i)
        {
            if (s->files[i]->iswild || !OPTkeepunfound)
            {
                if (s->files[i]->type == SPITPATHABS)
                {
                    numfiles += FollowDirs(s->files[i]->wildin, s->files[i], currentdir, fname);
                }
            }
        }
        (void)RChDir(currentdir);
    }
    return numfiles;
}


/* Finds files of type SPITPATHDIR (called by FindRecursive) */
int FindDirs(SearchList *s, char *startdir, char depth)
{
    char currentdir[SPITMAXLINE];
    char fname[SPITMAXLINE];
    int numfiles = 0;
    int i;
    currentdir[0] = '\0';
    fname[0]      = '\0';
    RGetCWD(currentdir, SPITMAXLINE);
    for (i = 0; i < s->numfiles; ++i)
    {
        if ((s->files[i]->type == SPITPATHDIR) && ((OPTstartrecurseat <= i) || !depth))
        {
            if (s->files[i]->iswild || !OPTkeepunfound)
            {
                numfiles += FollowDirs(s->files[i]->wildin, s->files[i], startdir, fname);
            }
        }
    }
    (void)RChDir(currentdir);

    return numfiles;
}


/* Recursively finds files of type SPITPATHDIR and SPITPATHNORM */
int FindRecursive(char *startdir, char *origdir, SearchList *s, int depth)
{
    RFindData c_file;
    RHandle hFile;
    int numfiles = 0;

    bool anywild=false;
    for (int i = 0; i < s->numfiles; ++i)
    {
        if (s->files[i]->iswild)
        {
            anywild = true;
            break;
        }
    }
    if (OPTrecurse==0 && (!anywild || !OPTexpandwildcards))
        return s->numfiles;
    if (OPTverbose>=6)
        printf("SPIT - recurse start at %s\n",startdir);

    // failsafe
    if (strlen(startdir)>(SPITMAXLINE-12)) // 12 for basic filenames
        return 0;

    if (RChDir(startdir))
    {
        int len = strlen(startdir)-1;
        if (startdir[len] != '\\' && startdir[len] != '/')
        {
#if defined(MSDOS) || defined(_WIN32) || defined(__WATCOMC__) || defined(_MSC_VER)
            strcat(startdir, "\\");
#else
            strcat(startdir, "/");
#endif
        }

        /* Strip startdir to be relative to origdir */
        int j=0;
        while (tolower(startdir[j]) == tolower(origdir[j]) && origdir[j] != '\0' && startdir[j] != '\0')
            ++j;
        if (startdir[j] == '/' || startdir[j] == '\\')
            ++j;

        /* Clear all matched flags */
        for (int i = 0; i < s->numfiles; ++i)
            s->files[i]->matched = (char)0;

        /* First find all the type=SPITPATHDIR from this directory */
        if (OPTrecurse != 1) // we will find these again if I  recurse.
            numfiles += FindDirs(s, startdir+j, (char)depth);

        /* Now find all the type=SPITPATHNORM from this directory */
        if ((hFile = RFindFirst(&c_file)) != RNOTFOUND)
        {
            char destbuf[SPITMAXLINE];

            Match match;
            match.s = (char *)malloc(SPITMAXLINE);
            match.d = (char *)malloc(SPITMAXLINE);

            do
            {
                for (int i = 0; i < s->numfiles; ++i)
                {
                    if (((s->files[i]->type == SPITPATHNORM) || (s->files[i]->type == SPITPATHDIR) || (s->files[i]->type == SPITPATHABS)) && ((OPTstartrecurseat <= i) || !depth))
                    {
                        if ((strcmp(RName(c_file), ".") && strcmp(RName(c_file), "..")))
                        {
                            if (!(RAttrib(c_file) & RHIDDEN))
                            {
                                char wildintempbuf[SPITMAXLINE];
                                char wildouttempbuf[SPITMAXLINE];

                                if (s->files[i]->iswild && OPTexpandwildcards)
                                {
                                    char filenametempbuf[SPITMAXLINE];

                                    s->files[i]->matched = (char)0;

                                    // make the wildout relative to current working directory.
                                    SPIT_makerelative(wildouttempbuf, s->files[i]->wildout,startdir+j);
                                    SPIT_makerelative(filenametempbuf,RName(c_file),startdir+j);

                                    // absolute checking is still broken cannot be fixed easily.
                                    if(s->files[i]->type == SPITPATHABS) // if an absolute type then have to strip off absolute information.
                                        SPIT_makerelativeAbs(wildintempbuf, s->files[i]->wildin,origdir);
                                    else                                 // otherwise we want it relative to current working directory.
                                        SPIT_makerelative(wildintempbuf, s->files[i]->wildin,startdir+j);
                                    // match up and see if I found anything.
                                    if (pathmatchfun(destbuf, filenametempbuf/*RName(c_file)*/, wildintempbuf/*s->files[i]->wildin*/, wildouttempbuf))
                                        s->files[i]->matched = (char) 1;
                                }
                                else if (!s->files[i]->matched && (OPTkeepunfound == 0 || OPTrecurse == 1))
                                {
                                    if (!EA::StdC::Stricmp(RName(c_file), s->files[i]->wildin))
                                        /* 1 means it is matched to RName */
                                        s->files[i]->matched = (char) 1;

                                    // we have to make the wildout relative to the current working directory.
                                    SPIT_makerelative(destbuf, s->files[i]->wildout,startdir+j);
                                }
                                if (s->files[i]->matched != 3 && s->files[i]->matched != 0)
                                {
                                    if (startdir[j] != '\0')
                                    {
                                        char * destbufunrel = 0;
                                        if (s->files[i]->matched == 1)
                                            sprintf(match.s, "%s%s", startdir+j, RName(c_file));
                                        if(OPTstrippaths)
                                        {
                                            destbufunrel = SPIT_makeunrelative(s->files[i]->hasout,s->files[i]->wildout,startdir+j,destbuf);
                                            sprintf(match.d, "%s", destbufunrel);
                                        }
                                        else
                                            sprintf(match.d, "%s", destbuf);

                                        if (SPIT_typeofname(destbuf) != SPITPATHABS)
                                        {
                                            AddMatch(s->files[i], &match);
                                        }
                                        else
                                        {
                                            if(OPTstrippaths)
                                                sprintf(match.d, "%s", destbufunrel);
                                            else
                                                strcpy(match.d, destbuf);
                                            AddMatch(s->files[i], &match);
                                        }
                                        if(destbufunrel)
                                            free(destbufunrel);
                                    }
                                    else
                                    {
                                        char * destbufunrel = 0;
                                        if (s->files[i]->matched == 1)
                                            strcpy(match.s, RName(c_file));

                                        if(OPTstrippaths)
                                        {
                                            destbufunrel = SPIT_makeunrelative(s->files[i]->hasout,s->files[i]->wildout,startdir+j,destbuf);
                                            strcpy(match.d, destbufunrel);
                                        }
                                        else
                                            strcpy(match.d, destbuf);

                                        AddMatch(s->files[i], &match);
                                        if(destbufunrel)
                                            free(destbufunrel);
                                    }
                                    /* 3 means its been matched already for this directory */
                                    s->files[i]->matched = 3;
                                }
                            }
                        }
                    }
                }
            }
            while(RFindNext(hFile, &c_file));
            RFindClose(hFile, &c_file);

            free(match.s);
            free(match.d);
            match.s = NULL;
            match.d = NULL;
        }

        if (OPTrecurse == 1)
        {
            if ((hFile = RFindFirst(&c_file)) != RNOTFOUND)
            {
                do
                {
                    if (RAttrib(c_file) & RSUBDIR)
                    {
                        const char *subDir = RName(c_file);
                        if (strcmp(subDir, ".") && strcmp(subDir, ".."))
                        {
                            const int SPITMAXPATH = 256;
                            char newdir[SPITMAXPATH];
                            sprintf(newdir,"%s%s", startdir, subDir);

                            numfiles += FindRecursive(newdir, origdir, s, ++depth);
                            (void)RChDir("..");
                        }
                    }
                }while(RFindNext(hFile, &c_file));
                RFindClose(hFile, &c_file);
            }
        }
    }

    return numfiles;
}

void replacePathAbs(char *out, char *in, char *path)
{
    char *ptr = in + strlen(in);
    char *drivecheck = path + strlen(path) - 1;       // jump to the last character of the line.
    char temp[SPITMAXLINE];
    int  isdrive = 0;                                 // check if this is a drive letter or path?
    isdrive = (*drivecheck == ':')?1:0;               // if this is a drive letter then we want
    if(!isdrive)                                      // to keep relative path information so
    {                                                 // scan back differently than if not.
        while (ptr != in && *ptr != '\\' && *ptr != '/'
#if defined(MSDOS) || defined(_WIN32) || defined(__WATCOMC__) || defined(_MSC_VER)
                && *ptr != ':'
#endif
              )
              --ptr;                                  // scan back till we hit first occurence of
    }                                                 // any delimeter so we can strip this info
    else                                              // off and tack on the path.
    {
        while (ptr != in
#if defined(MSDOS) || defined(_WIN32) || defined(__WATCOMC__) || defined(_MSC_VER)
                && *ptr != ':'
#endif
              )
              --ptr;
    }

    if (ptr != in)
        ++ptr;

    strcpy(temp, path);
    if (*(path + strlen(path) - 1) != '\\' && *(path + strlen(path) - 1) != '/'
#if defined(MSDOS) || defined(_WIN32) || defined(__WATCOMC__) || defined(_MSC_VER)
            && *(path + strlen(path) - 1) != ':'
#endif
        )
#if defined(MSDOS) || defined(_WIN32) || defined(__WATCOMC__) || defined(_MSC_VER)
        strcat(temp, "\\");
#else
        strcat(temp, "/");
#endif
    strcat(temp, ptr);
    strcpy(out, temp);

    return;
}

/* Replace the path in "in" with "path" */
/* Used to apply OPTpathrule */
void replacePath(char *out, char *in, char *path)
{
    char *ptr = in + strlen(in);
    char temp[SPITMAXLINE];

    while (*ptr != '\\' && *ptr != '/' && ptr != in
#if defined(MSDOS) || defined(_WIN32) || defined(__WATCOMC__) || defined(_MSC_VER)
            && *ptr != ':'
#endif
            )
        --ptr;

    if (ptr != in)
        ++ptr;

    strcpy(temp, path);
    if (*(path + strlen(path) - 1) != '\\' && *(path + strlen(path) - 1) != '/'
#if defined(MSDOS) || defined(_WIN32) || defined(__WATCOMC__) || defined(_MSC_VER)
            && *(path + strlen(path) - 1) != ':'
#endif
        )
#if defined(MSDOS) || defined(_WIN32) || defined(__WATCOMC__) || defined(_MSC_VER)
        strcat(temp, "\\");
#else
        strcat(temp, "/");
#endif
    strcat(temp, ptr);
    strcpy(out, temp);

    return;
}


/* Replace the extension in "in" with "ext" */
/* Applys path rules */
void replaceExt(char *out, char *in, char *ext)
{
    char *ptr;
    char temp[SPITMAXLINE];
    char tmpext[SPITMAXLINE] = "";

    /* Remove any prefixed '.' (used from command line to indicate extension rule) */
    if (*ext == '.')
        ++ext;

    /* Leave filenames with '.' at end and copy extension to tmpext */
    strcpy(temp, in);
    ptr = temp + strlen(temp);
    while (*ptr != '.' && ptr != temp && *ptr != '\\' && *ptr != '/')
        --ptr;
    if (*ptr != '.')
    {
        ptr = temp + strlen(temp);
        strcpy(ptr, ".");
        ++ptr;
    }
    else
    {
        ++ptr;
        strcpy(tmpext, ptr);
    }

    if (*ext == '\0')
        --ptr;

    /* Apply extension rule to extension */
    stringmatchfun(ptr, tmpext, "*", ext);

    strcpy(out, temp);

    return;
}

/* get date in form yyyymmdd */

int SPIT_filedate(const char *fname)
{
    RFindData c_file;
    RHandle   hFile;
    char temp[256];
    char name[256];
    int date=0;
    int done = 0;

    /* Just use name, because we should be in the propper directory when called */
    SPIT_getname( name,fname);
    if (name[0] == '\0')
        name[0] = '.';

    if ((hFile = RFindFirst(&c_file)) != RNOTFOUND)
    {
        do
        {
            if (WILD_imatch(temp, RName(c_file), name, name))
            {
                time_t       t;
                struct tm *st;

                t = RDate(c_file);
                st = localtime(&t);

                date = ((st->tm_year+1900)*10000)+((st->tm_mon)*100)+(st->tm_mday);
                done = 1;
            }
        }while(RFindNext(hFile, &c_file) && !done);
        RFindClose(hFile, &c_file);
    }

    if (OPTverbose>=6)
        printf("%s %d\n",fname,date);

    return(date);
}


bool SPIT_isdirectory(unsigned char attrib)
{
    return (attrib & 0x01)!=0;
}


bool SPIT_isreadonly(unsigned char attrib)
{
    return (attrib & 0x02)!=0;
}


bool SPIT_issystem(unsigned char attrib)
{
    return (attrib & 0x04)!=0;
}

bool SPIT_ishidden(unsigned char attrib)
{
    return (attrib & 0x08)!=0;
}


/*****************************************************************************/
/* Sorting algorithm                                                         */

/* Swaps 2 elements of the argv lists */
static void SPIT_exchange(int p, int q)
{
    char *str;

    str = SPITsargv[p];
    SPITsargv[p] = SPITsargv[q];
    SPITsargv[q] = str;

    str = SPITdargv[p];
    SPITdargv[p] = SPITdargv[q];
    SPITdargv[q] = str;

    return;
}

// SPIT_strcmp compares alphanumerically
// case sensitive
// digits compare as greater than non-digits
// zero padding ignored
// numbers limited to 2^31

#define isdigit(c) (c>='0' && c<='9')

static int SPIT_strcmp(const char *s0, const char *s1)
{
    int diff = 0;
    if (*s0=='-' || *s1=='-') return 0; // treat options as same
    do
    {
        if (isdigit(*s0) || isdigit(*s1))
        {
            int n0=-1;
            if (isdigit(*s0))
            {
                char digits[100];
                char *d=digits;
                while (isdigit(*s0) && d<digits+sizeof(digits)-1)
                    *d++ = *s0++;
                *d = '\0';
                n0 = atoi(digits);
            }

            int n1=-1;
            if (isdigit(*s1))
            {
                char digits[100];
                char *d=digits;
                while (isdigit(*s1) && d<digits+sizeof(digits)-1)
                    *d++ = *s1++;
                *d = '\0';
                n1 = atoi(digits);
            }
            diff = n0-n1;
        }
        else
        {
            diff = (int)(unsigned char)*s0 - (int)(unsigned char)*s1;

            ++s0;
            ++s1;
        }
        if (diff) break;

        if (!diff && (!*s0 || !*s1))
            diff = (int)(unsigned char)*s0 - (int)(unsigned char)*s1;

    } while (!diff && *s0 && *s1);

    return diff;
}

/* 1 by path (with numerical awareness), 3 by path (straightforward strcmp), 5 by destination name, 6 by destination extension */
static int SPIT_sortcompare(int p, int r, int sorttype)
{
    if (sorttype == 1)
    {
        return SPIT_strcmp(SPITsargv[p], SPITsargv[r]);
    }
    else if (sorttype == 2)
    {
        printf("SPIT - sort by name with variations not supported\n");
        return 0;
    }
    else if (sorttype == 3)
    {
        return strcmp(SPITdargv[p], SPITdargv[r]);
    }
    else if (sorttype == 4)
    {
        printf("SPIT - size sort not supported\n");
        return 0;
    }
    else if (sorttype == 5)
    {
        return SPIT_strcmp(SPITdargv[p], SPITdargv[r]);
    }
    else if (sorttype == 6)
    {
        char *p_ptr = SPITdargv[p] + strlen(SPITdargv[p]);
        char *r_ptr = SPITdargv[r] + strlen(SPITdargv[r]);
        while (*p_ptr!='\\' && *p_ptr!= '/' && *p_ptr!='.' && p_ptr!=SPITdargv[p])
            --p_ptr;
        while (*r_ptr!='\\' && *r_ptr!= '/' && *r_ptr!='.' && r_ptr!=SPITdargv[r])
            --r_ptr;
        if (*p_ptr != '.')
            p_ptr = SPITdargv[p] + strlen(SPITdargv[p]);
        if (*r_ptr != '.')
            r_ptr = SPITdargv[r] + strlen(SPITdargv[r]);
        return SPIT_strcmp(p_ptr, r_ptr);
    }

    return 0;
}

/* modified bubble sort in order to maintain option order/boundaries and because file names are usually almost sorted */
void SPIT_qsort(int first, int last, int sorttype)
{
    int i;
    while (first<last)
    {
        bool swapped = false;
        for (i=first; i<last; ++i)
            if (SPIT_sortcompare(i,i+1,sorttype)>0)
            {
                SPIT_exchange(i,i+1);
                swapped = true;
            }
        if (!swapped) break;
        for (i=last; i>first; --i)
            if (SPIT_sortcompare(i-1,i,sorttype)>0)
                SPIT_exchange(i,i-1);
        ++first;
        --last;
    }
}

/*****************************************************************************/

/******** FILE SEARCHLIST CONTROL ********************************************/

SearchList *CreateSearchList(void)
{
    SearchList *s;
    s = (SearchList *)malloc(sizeof(SearchList));
    s->files = NULL;
    s->numfiles = s->maxfiles = 0;
    return s;
}


static char SPIT_typeofname(char *nm)
{
    char type = SPITPATHNORM;

    if (strlen(nm) > 1)
    {
        if ((nm[0] == '-') || (nm[0] == '+') || (nm[0] == OPTdot && nm[1] != '\\' && nm[1] != '/' && nm[1] != '.'))
            type = SPITPATHOPT;
        /* This covers the <drive>:\<path> and <drive>:<filename> possibilities */
        else if (isalpha((unsigned char)nm[0]) && nm[1] == ':')
            type = SPITPATHABS;
        /* This covers the \<path> possibility */
        else if (nm[0] == '\\' || nm[0] == '/')
            type = SPITPATHABS;
        else
        {
            /* Check if it has multiple directories */
            while (*nm != '\0')
            {
                if (*nm == '\\' || *nm == '/')
                {
                    type = SPITPATHDIR;
                    break;
                }
                ++nm;
            }
        }
    }
    return type;
}

#define MATCHALLOCSIZE 1024

void IncreaseMatch(FileInfo *files)
{
    ++files->nummatches;

    if (files->nummatches == 1)
    {
        files->maxmatches = MATCHALLOCSIZE;

        files->matches = (Match **)malloc((files->maxmatches)*sizeof(Match *));
    }
    else
    {
        if (files->nummatches >= files->maxmatches)
        {
            files->maxmatches += MATCHALLOCSIZE;

            Match ** t_matches = (Match **)malloc((files->maxmatches)*sizeof(Match *));

            memcpy((void *)t_matches, (void *)files->matches, (files->nummatches)*sizeof(Match *));
            free((void *)files->matches);
            files->matches = t_matches;
        }
    }
}



bool AddMatch(FileInfo *files, Match *newmatch)
{
    int oldnum = files->nummatches;
    bool ok = false;
    char dest[SPITMAXLINE];

    if (files)
    {
        IncreaseMatch(files);

        /* just in case newmatch->s and newmatch->d are pointing to the same place */
        strcpy(dest,newmatch->d);

#if defined(__cplusplus) && defined(_MSC_VER)
        SPIT_shortcut(newmatch->s, newmatch->s, 0);
#endif

        Match *t_match = (Match *) malloc(sizeof(Match));
        t_match->s = (char *)malloc(strlen(newmatch->s)+1);
        t_match->d = (char *)malloc(strlen(dest)+1);
        strcpy(t_match->s, newmatch->s);
        strcpy(t_match->d, dest);
        t_match->size = newmatch->size;
        t_match->date = newmatch->date;
        t_match->attrib = newmatch->attrib;
        files->matches[oldnum] = t_match;
        ok = true;
    }

    return ok;
}


bool UpdateSearchList(SearchList *s, int filenum, char *in, char *out)
{
    int i;
    bool ok = false;

    if (s)
    {
        if (filenum < s->numfiles)
        {
            /* Update wildin if necessary */
            if (in)
            {
                if (s->files[filenum]->wildin)
                    free(s->files[filenum]->wildin);
                s->files[filenum]->wildin = (char *)malloc(strlen(in)+1);
                strcpy(s->files[filenum]->wildin, in);
                s->files[filenum]->type = SPIT_typeofname(in);
            }

            /* Update wildout if necessary */
            if (out)
            {
                if (s->files[filenum]->wildout)
                    free(s->files[filenum]->wildout);
                s->files[filenum]->wildout = (char *)malloc(strlen(out)+1);
                strcpy(s->files[filenum]->wildout, out);
            }

            /* Delete all matches */
            if (s->files[filenum]->nummatches)
            {
                for(i = 0; i < s->files[filenum]->nummatches; ++i)
                {
                    free((void *)s->files[filenum]->matches[i]->s);
                    free((void *)s->files[filenum]->matches[i]->d);
                    free((void *)s->files[filenum]->matches[i]);
                }
                free((void *)s->files[filenum]->matches);
                s->files[filenum]->matches = NULL;
                s->files[filenum]->nummatches = 0;
            }

            /* Update all other fields */
            s->files[filenum]->iswild = (char)((SPIT_iswildcards(in) || SPIT_iswildcards(out)));
            s->files[filenum]->matched = 0;

            ok = true;
        }
    }

    return ok;
}


bool RemoveFromSearchList(SearchList *s, int filenum, char *in, char *out)
{
    FileInfo **t_files = NULL;
    int i;
    int j = 0;
    bool ok = false;

    if (in)  in[0]  = '\0';
    if (out) out[0] = '\0';
    if (s)
    {
        if (filenum < s->numfiles)
        {
            if (s->files[filenum]->wildin)
            {
                if (in)
                    strcpy(in, s->files[filenum]->wildin);
                free((void *)s->files[filenum]->wildin);
                s->files[filenum]->wildin = NULL;
            }

            if (s->files[filenum]->wildout)
            {
                if (out)
                    strcpy(out, s->files[filenum]->wildout);
                free((void *)s->files[filenum]->wildout);
                s->files[filenum]->wildout = NULL;
            }

            if (s->files[filenum]->nummatches)
            {
                for(i = 0; i < s->files[filenum]->nummatches; ++i)
                {
                    free((void *)s->files[filenum]->matches[i]->s);
                    free((void *)s->files[filenum]->matches[i]->d);
                    free((void *)s->files[filenum]->matches[i]);
                }
                free((void *)s->files[filenum]->matches);
                s->files[filenum]->matches = NULL;
            }

            --s->numfiles;
            s->maxfiles = s->numfiles;
            if (s->numfiles > 0)
            {
                t_files = (FileInfo **)malloc((s->numfiles)*sizeof(FileInfo *));
                for (i = 0; i < s->numfiles+1; ++i)
                {
                    if (i != filenum)
                    {
                        t_files[j] = s->files[i];
                        ++j;
                    }
                    else
                    {
                        free(s->files[i]);
                        s->files[i] = NULL;
                    }
                }
                free((void *)s->files);
                s->files = t_files;
            }
            else
            {
                free((void *)s->files);
                s->files = NULL;
            }

            ok = true;
        }
    }

    return ok;
}

static const int NUMSEARCHFILESTOALLOC=1024;
bool AddToSearchList(SearchList *s, char *in, char *out)
{
    FileInfo **t_files = NULL;
    int oldnum;
    bool ok = false;

    if (s)
    {
        oldnum = s->numfiles;
        ++s->numfiles;

        t_files = s->files;
        if (s->numfiles>s->maxfiles)
        {
            s->maxfiles += NUMSEARCHFILESTOALLOC;

            t_files = (FileInfo **)malloc((s->maxfiles)*sizeof(FileInfo *));

            if (oldnum)
                memcpy((void *)t_files, (void *)s->files, (oldnum)*sizeof(FileInfo *));

            if (s->files)
                free((void *)s->files);

            s->files = NULL;
        }

#if defined(__cplusplus) && defined(_MSC_VER)
        SPIT_shortcut(in, in, 0);
#endif
        t_files[oldnum] = (FileInfo *)malloc(sizeof(FileInfo));
        t_files[oldnum]->wildin = (char *)malloc(strlen(in)+1);
        t_files[oldnum]->wildout = (char *)malloc(strlen(out)+1);
        strcpy(t_files[oldnum]->wildin, in);
        strcpy(t_files[oldnum]->wildout, out);
        t_files[oldnum]->iswild     = (char)((SPIT_iswildcards(in) || SPIT_iswildcards(out)));
        t_files[oldnum]->matched    = 0;
        t_files[oldnum]->type       = SPIT_typeofname(in);
        t_files[oldnum]->nummatches = 0;
        t_files[oldnum]->maxmatches = 0;
        t_files[oldnum]->matches    = NULL;
        // for makeunrelative, if it has an output filename then have to stip off path correctly.
        if (in == out)
            t_files[oldnum]->hasout = 0;
        else
            t_files[oldnum]->hasout = 1;



        s->files = t_files;
        ok = true;
    }

    return ok;
}


void DestroySearchList(SearchList *s)
{
    int i;
    int j;

    if (s)
    {
        if (s->files)
        {
            for(i = 0; i < s->numfiles; ++i)
            {
                if (s->files[i]->matches)
                {
                    for(j = 0; j < s->files[i]->nummatches; ++j)
                    {
                        free((void *)s->files[i]->matches[j]->s);
                        free((void *)s->files[i]->matches[j]->d);
                        free((void *)s->files[i]->matches[j]);
                    }
                    free((void *)s->files[i]->matches);
                    s->files[i]->matches = NULL;
                }
                free((void *)s->files[i]->wildin);
                free((void *)s->files[i]->wildout);
                free((void *)s->files[i]);
            }
            free((void *)s->files);
            s->files = NULL;
        }
        free((void *)s);
        s = NULL;
    }

    return;
}

void MakeArgvs(SearchList *s)
{
    int i;
    int j;
    int c = 0;
    SPITargnum = 0;


    if (s)
    {
        if (s->files)
        {

            for (i = 0; i < s->numfiles; ++i)
            {
                if (s->files[i]->nummatches)
                {
                    SPITargnum += s->files[i]->nummatches;
                }
                else if (OPTkeepunfound || (s->files[i]->type == SPITPATHOPT && OPTkeepopts))
                {
                    ++SPITargnum;
                }
            }

            if (SPITargnum > 0)
            {
                SPITsargv = (char **)malloc(SPITargnum*sizeof(char *));
                SPITdargv = (char **)malloc(SPITargnum*sizeof(char *));

                for(i = 0; i < s->numfiles; ++i)
                {
                    if (s->files[i]->nummatches)
                    {
                        if (s->files[i]->matches)
                        {
                            for (j = 0; j < s->files[i]->nummatches; ++j)
                            {
                                SPITsargv[c] = (char *)malloc(strlen(s->files[i]->matches[j]->s)+1);
                                SPITdargv[c] = (char *)malloc(strlen(s->files[i]->matches[j]->d)+1);
                                strcpy(SPITsargv[c], s->files[i]->matches[j]->s);
                                strcpy(SPITdargv[c], s->files[i]->matches[j]->d);
                                ++c;
                            }
                        }
                    }
                    else if (OPTkeepunfound || (s->files[i]->type == SPITPATHOPT && OPTkeepopts))
                    {
                        SPITsargv[c] = (char *)malloc(strlen(s->files[i]->wildin)+1);
                        SPITdargv[c] = (char *)malloc(strlen(s->files[i]->wildout)+1);
                        strcpy(SPITsargv[c], s->files[i]->wildin);
                        strcpy(SPITdargv[c], s->files[i]->wildout);
                        ++c;
                    }
                }
            }
        }
    }
    return;
}


/* Frees up memory allocated to SPITsargv, SPITdargv */
void freeFiles(void)
{
    int i;

    if (SPITsargv != NULL)
    {
        for (i = 0; i < SPITargnum; ++i)
            if (SPITsargv[i] != NULL)
                free(SPITsargv[i]);
        free(SPITsargv);
        SPITsargv = NULL;
    }

    if (SPITdargv != NULL)
    {
        for (i = 0; i < SPITargnum; ++i)
            if (SPITdargv[i] != NULL)
                free(SPITdargv[i]);
        free(SPITdargv);
        SPITdargv = NULL;
    }

    SPITargnum = 0;

    return;
}


bool SPIT_haswild(char *s)
{
    char *d = s;
    while(*d != '\0')
    {
        if(*d == '*' || *d == '?')
            return true;
        d++;
    }
    return false;
}

//-----------------------------------------------------------------------------
// this matches two characters in a directory, ie / and \ are regarded
// as the same character.
inline bool dirs_areequal(char a, char b)
{
    if (tolower(a) == tolower(b) || ((a == '\\' || a == '/') && (b == '\\' || b == '/')))
        return true;
    return false;
}

//-----------------------------------------------------------------------------
// make my start directory relative to what is already in the current string.
void MakeRelative(char ** startdir, char* enddir,const char *startstring, const char * endstring)
{
    char *dirs = *startdir;
    const char *start = startstring;
    int inc = 0;
    // scan along till we find a spot where the two strings match.
    while(!dirs_areequal(*start,*dirs) && dirs != enddir && start != endstring)
    {
        start++;
        if(*start == ':')
            inc = 1;
        if((*start == '.' && *(start+1) != '\\') || *(start+1) != '/') // just in case we hit the file extension.
            return;
    }
    if(!inc)
        start = startstring;
    while(dirs_areequal(*start,*dirs) && dirs != enddir && start != endstring)
    {
        dirs++;
        start++;
    }
    if(*(start - 1) == '\\' || *(start - 1) == '/')
        *startdir = dirs;
    return;
}

//---------------------------------------------------------------------------------------
// this will automatically tack on a CWD at the front of any path after drive letters.
// for instance e:*.ffs  STARTDIR=folder2\ becomes e:folder2\*.ffs
char *SPIT_makerelative(char *dest, const char *source, char *startdir)
{
    char *startdirectory = startdir;
    char *enddirectory = startdir + strlen(startdir);

    // find end of folder/start of file name
    const char *ptr = source + strlen(source);
    while (*ptr != '\\' && *ptr != '/' && ptr != source && *ptr != ':')
        --ptr;

    if (ptr != source)
    {
        ++ptr;                      // advance over slash.. point to file name.
        const char *ptr2 = source;
        int CopyIndex = 0;

        while (ptr2 != ptr)          // copy path
            dest[CopyIndex++] = *ptr2++;

        dest[CopyIndex] = '\0';  // null terminate so I can use strcat.
        MakeRelative(&startdirectory,enddirectory,source,ptr);
    }
    else
        dest[0] = '\0';          // otherwise not first bit so set for copy.

    strcat(dest,startdirectory);   // copy in the CWD information relative to origdir.
    strcat(dest,ptr);            // copy over filename.

    for (int i = 0; dest[i]; ++i)
        if (dest[i] == '\\' || dest[i] == '/')
            dest[i] = '\\';

    return dest;
}

//destbufunrel = SPIT_makeunrelative(s->files[i]->wildout,startdir+j,destbuf)
//---------------------------------------------------------------------------------------
// This will strip out startdir from complete. It will look for the startdir
// and strip out the startdir from the string.
char *SPIT_makeunrelative(char hasout,char *out,char *startdir,char * complete)
{
    unsigned int clen = strlen(complete);
    unsigned int blen = 0;
    char * retstring  = (char *)malloc(clen+1);
    char * loc        = 0;
    char * p          = out + strlen(out);
    char * sdir       = startdir;
    char * tempsdir   = 0;

    memset(retstring,0,clen+1);

    if(hasout)
    {
        // find the outpath that we have specified, discard
        // the filename.
        while(p != out && *p != '\\' && *p != '/' && *p != ':')
            p--;

        // if the startdir == outdir then I don't want to strip off the
        // startdir from the destination.
        if(!strncmp(out,startdir,(unsigned int)(p - out)) && strlen(startdir) == ((unsigned int)(p-out)+1))
        {
            strncpy(retstring,complete,clen);
            return(retstring);
        }
        else
        {
            unsigned int plen = (unsigned int)(p-out);
            unsigned int vlen = 0;
            char * v = complete + strlen(complete);

            // go back and find the start of the file.
            while(v != complete && *v != '\\' && *v != '/' && *v != ':')
                v--;
            // if I didn't hit the begining omit the slash.
            if(v != complete)
                v++;
            // figure out the length of my filename.
            vlen = ((unsigned int)(v-complete) == 0)?strlen(complete):(unsigned int)(v-complete);

            // create the tempbuf for the path.
            tempsdir = (char *)malloc(plen+3);
            memset(tempsdir,0,plen+3);
            if(plen)
                strncpy(tempsdir,out,plen+1);
            if(retstring)
                free(retstring);
            retstring = (char *)malloc(clen + 2 + strlen(tempsdir));
            memset(retstring,0,clen + 2 + strlen(tempsdir));
            if(plen)
                strncpy(retstring,tempsdir,plen+1);                                          // copy over this begining string.
            if(vlen)
                strncat(retstring,v,vlen);                                                   // copy over the filename from the completestr.

            if(tempsdir)
                free(tempsdir);
            return(retstring);
        }
    }

    // otherwise lets strip off the startdir
    // from the destination so we behave more
    // like copy does.
    loc = SPIT_strstr(complete,sdir);
    if(loc)   // I found the startdir, strip it out.
    {
        blen = (unsigned int)(loc - complete);      // length of string before the startdir.
        strncpy(retstring,complete,blen);           // copy over this begining string.
        strncat(retstring,loc+strlen(sdir),(unsigned int)(clen-blen-strlen(sdir)));  // copy over the rest after startdir.
    }
    else      // nope no startdir leave be.
    {
        strncpy(retstring,complete,clen);
    }
    return(retstring);
}




//---------------------------------------------------------------------
// strip off absolute file name information.
char *SPIT_makerelativeAbs(char *dest, const char *source, char *origdir)
{
    char *end = origdir + strlen(origdir);
    char *ptr = origdir;
    const char *s = source;
    const char *endsource = source + strlen(source);

    // loop along till I find the end of origdir in the absolute path.
    while (*ptr == *s && ptr != end && s != endsource)
        ++ptr,s++;
    strcpy(dest,s);
    return dest;
}
