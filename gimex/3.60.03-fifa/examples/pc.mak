# (c) 1998-2007 Electronic Arts Inc.

#.silent:
.SUFFIXES: .cpp .c .a .o

EXAMPLES = \
 gext_eg.exe \
 many2many_eg.exe \
 saveany_eg.exe \
 showany_eg.exe \
 any2any_eg.exe \
 any2gsh_eg.exe \
 some2some_eg.exe \
 snapany_eg.exe \
 filesel.exe

#not working
# resampleimage_eg.exe
# tga2bmp_eg.exe \

# these examples need REAL
EXAMPLESWITHREAL = \
 savegimex_eg.exe \
 showlbm.exe \
 shape2gimex.exe
# replay.exe

ALL : delerr $(EXAMPLES)

REAL: delerr $(EXAMPLESWITHREAL)

# ICL should be set in the users environment to icl or cl
# only Intel C versions should be publically posted, if at all possible

!IFNDEF ICL
ICL=cl
!ENDIF

SPITOBJS=spit.obj stdgimex.obj hash.obj findfile.obj wildmatch.obj expr.obj range.obj ranger.obj gclip.obj
SPIT64OBJS=spit.obj unixgimex.obj hash.obj findfile.obj wildmatch.obj expr.obj range.obj ranger.obj gclip.obj
STDLIBS=user32.lib version.lib shell32.lib ole32.lib comdlg32.lib winmm.lib wsock32.lib gdi32.lib advapi32.lib tapi32.lib rasapi32.lib wininet.lib ddraw.lib dxguid.lib

.cpp.obj::
	$(ICL) /c /MT /O2 /W3 /WX /nologo $<

.c.obj::
	$(ICL) /c /MT /O2 /W3 /WX /nologo $<

.rc.res::
	@rc -r $<

.obj.exe:
    echo linking $*
	link /nologo /INCREMENTAL:NO /SUBSYSTEM:CONSOLE /OUT:$*.exe $*.obj spit.lib $(STDLIBS)
    mt.exe -nologo -manifest $*.exe.manifest -outputresource:$*.exe

delerr:
	@if exist pc.err del pc.err

gext_eg.exe: gext_eg.obj gopen.obj stdgimex.obj
    link /OUT:gext_eg.exe  /INCREMENTAL:NO /DEBUG gext_eg.obj gopen.obj stdgimex.obj user32.lib shell32.lib ole32.lib qtmlclient.lib 
    link /OUT:gext_egs.exe /INCREMENTAL:NO /DEBUG gext_eg.obj stdgimex.obj gimex.lib codex.lib vfw32.lib user32.lib advapi32.lib kernel32.lib gdi32.lib winspool.lib comdlg32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib qtmlclient.lib 
    mt.exe -nologo -manifest $*.exe.manifest -outputresource:$*.exe

many2many_eg.exe: many2many_eg.obj gopen.obj stdgimex.obj
    link /OUT:many2many_eg.exe  /INCREMENTAL:NO /DEBUG many2many_eg.obj gopen.obj stdgimex.obj user32.lib qtmlclient.lib
    link /OUT:many2manys_eg.exe /INCREMENTAL:NO /DEBUG many2many_eg.obj gopen.obj stdgimex.obj gimex.lib codex.lib vfw32.lib user32.lib advapi32.lib kernel32.lib gdi32.lib winspool.lib comdlg32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib qtmlclient.lib 
    mt.exe -nologo -manifest $*.exe.manifest -outputresource:$*.exe

resampleimage_eg.exe: resampleimage_eg.obj resample.obj gopen.obj stdgimex.obj
    link /OUT:resampleimage_eg.exe /INCREMENTAL:NO /DEBUG resampleimage_eg.obj resample.obj gopen.obj stdgimex.obj gfilter.lib crapi.lib user32.lib shell32.lib ole32.lib qtmlclient.lib
    link /OUT:resampleimages_eg.exe /INCREMENTAL:NO /DEBUG resampleimage_eg.obj resample.obj stdgimex.obj gimex.lib codex.lib gfilter.lib crapi.lib vfw32.lib user32.lib advapi32.lib kernel32.lib gdi32.lib winspool.lib comdlg32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib qtmlclient.lib 
    mt.exe -nologo -manifest $*.exe.manifest -outputresource:$*.exe

saveany_eg.exe: saveany_eg.obj gopen.obj stdgimex.obj
    link /OUT:saveany_eg.exe /INCREMENTAL:NO /DEBUG saveany_eg.obj gopen.obj stdgimex.obj user32.lib shell32.lib ole32.lib qtmlclient.lib
    link /OUT:saveanys_eg.exe /INCREMENTAL:NO /DEBUG saveany_eg.obj stdgimex.obj gimex.lib codex.lib vfw32.lib user32.lib advapi32.lib kernel32.lib gdi32.lib winspool.lib comdlg32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib qtmlclient.lib 
    mt.exe -nologo -manifest $*.exe.manifest -outputresource:$*.exe

showany_eg.exe: showany_eg.obj dib.obj gopen.obj stdgimex.obj
    link /OUT:showany_eg.exe /INCREMENTAL:NO /DEBUG showany_eg.obj dib.obj gopen.obj stdgimex.obj ddraw.lib dxguid.lib winmm.lib user32.lib gdi32.lib advapi32.lib version.lib wsock32.lib tapi32.lib rasapi32.lib shell32.lib ole32.lib 
    mt.exe -nologo -manifest $*.exe.manifest -outputresource:$*.exe

snapany_eg.exe: snapany_eg.obj gopen.obj stdgimex.obj
    link /OUT:snapany_eg.exe /INCREMENTAL:NO /DEBUG snapany_eg.obj gopen.obj stdgimex.obj user32.lib shell32.lib ole32.lib qtmlclient.lib
    link /OUT:snapanys_eg.exe /INCREMENTAL:NO /DEBUG snapany_eg.obj stdgimex.obj gimex.lib codex.lib vfw32.lib user32.lib advapi32.lib kernel32.lib gdi32.lib winspool.lib comdlg32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib qtmlclient.lib 
    mt.exe -nologo -manifest $*.exe.manifest -outputresource:$*.exe

tga2bmp_eg.exe: tga2bmp_eg.obj stdgimex.obj tgaread.obj bmpwrite.obj
    link /OUT:tga2bmp_eg.exe /INCREMENTAL:NO /DEBUG tga2bmp_eg.obj tgaread.obj bmpwrite.obj stdgimex.obj user32.lib qtmlclient.lib
    mt.exe -nologo -manifest $*.exe.manifest -outputresource:$*.exe

any2any_eg.exe: any2any_eg.obj gopen.obj stdgimex.obj
    link /OUT:any2any_eg.exe /INCREMENTAL:NO /DEBUG any2any_eg.obj gopen.obj stdgimex.obj user32.lib shell32.lib ole32.lib qtmlclient.lib
    link /OUT:any2anys_eg.exe /INCREMENTAL:NO /DEBUG any2any_eg.obj stdgimex.obj gimex.lib codex.lib vfw32.lib user32.lib advapi32.lib kernel32.lib gdi32.lib winspool.lib comdlg32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib qtmlclient.lib 
    mt.exe -nologo -manifest $*.exe.manifest -outputresource:$*.exe

any2gsh_eg.exe: any2gsh_eg.obj gopen.obj stdgimex.obj
    link /OUT:any2gsh_eg.exe /INCREMENTAL:NO /DEBUG any2gsh_eg.obj gopen.obj stdgimex.obj user32.lib shell32.lib ole32.lib qtmlclient.lib
    link /OUT:any2gshs_eg.exe /INCREMENTAL:NO /DEBUG any2gsh_eg.obj stdgimex.obj gimex.lib codex.lib vfw32.lib user32.lib advapi32.lib kernel32.lib gdi32.lib winspool.lib comdlg32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib qtmlclient.lib 
    mt.exe -nologo -manifest $*.exe.manifest -outputresource:$*.exe

some2some_eg.exe: some2some_eg.obj stdgimex.obj
    link /OUT:some2some_eg.exe /INCREMENTAL:NO /DEBUG some2some_eg.obj stdgimex.obj gimex.lib codex.lib vfw32.lib user32.lib advapi32.lib kernel32.lib gdi32.lib winspool.lib comdlg32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib qtmlclient.lib 
    mt.exe -nologo -manifest $*.exe.manifest -outputresource:$*.exe

filesel.exe: filesel.obj gopen.obj stdgimex.obj
    link /OUT:filesel.exe /INCREMENTAL:NO /DEBUG filesel.obj gopen.obj stdgimex.obj user32.lib shell32.lib ole32.lib qtmlclient.lib 
    mt.exe -nologo -manifest $*.exe.manifest -outputresource:$*.exe

# these are the examples with REAL

replay_eg.exe: replay_eg.obj gopen.obj stdgimex.obj
    link /OUT:replay_eg.exe /INCREMENTAL:NO /DEBUG replay_eg.obj gopen.obj stdgimex.obj realpcm.lib gimex.lib codex.lib vfw32.lib user32.lib advapi32.lib kernel32.lib gdi32.lib winspool.lib comdlg32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib qtmlclient.lib 
    mt.exe -nologo -manifest $*.exe.manifest -outputresource:$*.exe

showlbm_eg.exe: showlbm_eg.obj lbmread.obj libgimex.obj
    link /OUT:showlbm_eg.exe  /INCREMENTAL:NO /DEBUG showlbm_eg.obj lbmread.obj libgimex.obj user32.lib realpcm.lib qtmlclient.lib
    mt.exe -nologo -manifest $*.exe.manifest -outputresource:$*.exe

shape2gimex_eg.exe: shape2gimex_eg.obj gopen.obj stdgimex.obj
    link /OUT:shape2gimex_eg.exe /INCREMENTAL:NO /DEBUG shape2gimex_eg.obj gopen.obj stdgimex.obj user32.lib realpcm.lib gimex.lib codex.lib qtmlclient.lib 
    mt.exe -nologo -manifest $*.exe.manifest -outputresource:$*.exe

savegimex_eg.exe: savegimex_eg.obj stdgimex.obj bmpwrite.obj
    link /OUT:savegimex_eg.exe /INCREMENTAL:NO /DEBUG savegimex_eg.obj stdgimex.obj bmpwrite.obj realpcm.lib user32.lib shell32.lib ole32.lib qtmlclient.lib 
    mt.exe -nologo -manifest $*.exe.manifest -outputresource:$*.exe
