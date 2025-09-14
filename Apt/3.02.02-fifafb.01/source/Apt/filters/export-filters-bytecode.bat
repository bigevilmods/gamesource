@echo off
setlocal

REM This batch file takes the bytecode of the flash.filter.* classes from 
REM Apt-Flash-Native-Filters.swf and dumps them as hex values into a C header file.
REM So, the flash.filter.* classes are built right into the Apt library

set SWFC_PATH=..\..\..\bin\Apt
set UTIL_PATH=..\..\..\bin\util
set clean_all=0
set SWFFILE=Apt-Flash-Native-Filters

if "%1"=="" GOTO err_usage
if not exist %UTIL_PATH%\NUL GOTO err_missing_util_path
if not exist %SWFC_PATH%\NUL GOTO err_missing_swfc_path
if "%2"=="clean" set clean_all=1

set OUTPUT=%SWFFILE%-%1

REM Export the 32-bit format
%SWFC_PATH%\swfc.exe --optimize --endian=%1 -o %OUTPUT% %SWFFILE%.swf
move /y %SWFFILE%.eaf %OUTPUT%.eaf
del /q %SWFFILE%.flm
del /q %SWFFILE%.tac
rmdir %OUTPUT%_geometry
rmdir %OUTPUT%_textures
rmdir %OUTPUT%_sounds
rmdir %OUTPUT%_videos

%UTIL_PATH%\quickdump.pl %OUTPUT%.apt 32   > filters_%1_apt.h
%UTIL_PATH%\quickdump.pl %OUTPUT%.const 32 > filters_%1_const.h

REM Export the 64-bit format
%SWFC_PATH%\swfc.exe --optimize --endian=%1 --ptrsize=8 -o %OUTPUT% %SWFFILE%.swf
move /y %SWFFILE%.eaf %OUTPUT%.eaf
del /q %SWFFILE%.flm
del /q %SWFFILE%.tac
rmdir %OUTPUT%_geometry
rmdir %OUTPUT%_textures
rmdir %OUTPUT%_sounds
rmdir %OUTPUT%_videos

%UTIL_PATH%\quickdump.pl %OUTPUT%.apt 32   > filters_%1_apt_64.h
%UTIL_PATH%\quickdump.pl %OUTPUT%.const 32 > filters_%1_const_64.h

if %clean_all%==0 GOTO done

del /q %OUTPUT%.apt
del /q %OUTPUT%.const
del /q %OUTPUT%.eaf

GOTO done

:err_missing_util_path
echo Cannot find folder %UTIL_PATH%.

GOTO done

:err_missing_swfc_path
echo Cannot find swfc.exe in %SWFC_PATH%.

GOTO done

:err_usage
echo export-filters-bytecode.bat ENDIAN
echo Where ENDIAN is 'little' or 'big'


:done

