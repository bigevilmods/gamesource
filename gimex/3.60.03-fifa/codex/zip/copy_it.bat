:: (c) 2004-2007 Electronic Arts Inc.
@echo off

REM c_files
copy %1\trees.c
copy %1\adler32.c
copy %1\deflate.c
copy %1\infblock.c
copy %1\infcodes.c
copy %1\inffast.c
copy %1\inflate.c
copy %1\zutil.c
copy %1\inftrees.c
copy %1\infutil.c
copy %1\crc32.c

REM h_files
copy %1\deflate.h
copy %1\infblock.h
copy %1\infcodes.h
copy %1\inffast.h
copy %1\inftrees.h
copy %1\infutil.h
copy %1\zconf.h
copy %1\zlib.h
copy %1\zutil.h
