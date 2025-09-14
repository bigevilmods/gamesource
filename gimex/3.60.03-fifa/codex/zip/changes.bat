:: (c) 2004-2007 Electronic Arts Inc.
REM This batch file can be used to alter the
REM zlib source to create the source that is
REM used for the codex zip module and the
REM chimp zip module.
REM
REM Also remove all FAR defines from zconf.h
REM by deleting lines 127..151 inclusive.

REM Remove FAR keyword from source
gsr -s'FAR' -x'' *.h *.c

REM Rename DEBUG to ZLIBDEBUG
gsr -i0 -c0 -s'DEBUG' -x'ZLIBDEBUG' *.h *.c
