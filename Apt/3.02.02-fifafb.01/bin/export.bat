echo off

aTackCompiler %1.swf %1.eaf
set SUM=%errorlevel%
set REF=%SUM%

:LOOPTOP
	aTackCompiler %1.eaf %1.eaf
	set TEST=%errorlevel%
	if %SUM% == %TEST% goto DONE
	set SUM=%TEST%
	goto LOOPTOP

:DONE

echo mnemonic: %REF%/%TEST%
echo Compile complete
