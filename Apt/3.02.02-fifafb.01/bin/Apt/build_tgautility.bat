@echo off
setlocal
IF "%1"=="2" goto fw2
set CSCPATH=c:\windows\Microsoft.NET\Framework\v3.5\csc.exe
IF EXIST %CSCPATH%  goto build
:fw2
set CSCPATH=c:\windows\Microsoft.NET\Framework\v2.0.50727\csc.exe
IF EXIST %CSCPATH%  goto build
echo Error - Cannot find C# compiler, csc.exe.
goto done

:build
%CSCPATH% /out:tgautility.exe TgaUtility.cs

:done
