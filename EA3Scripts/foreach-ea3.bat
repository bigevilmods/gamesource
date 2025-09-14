@echo off
@echo.
if "%1"=="" goto PrintUsage
@echo Executing: %1
@echo.
for %%F in (ea3\*.*) do echo|set /p="%%~nxF," & ea3x.exe @%1 'ea3\%%~nxF' 2>nul
goto End
:PrintUsage
@echo Usage: batch_single_ea3 [script_name]
@echo.
:End