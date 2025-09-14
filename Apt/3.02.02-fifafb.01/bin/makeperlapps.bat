@echo off
echo Generating PerlApps...
pushd ..\bin\Apt
p4 edit swfc.exe
call buildswfc.bat
popd

pushd ..\bin\AptAuxEAGLREAL
p4 edit EAGLREALc.exe
perlapp EAGLREALc.pl --force --nologo --lib ..\Apt
popd

pushd ..\bin\AptAuxPCOpenGL
p4 edit PCGLc.exe
perlapp PCGLc.pl --force --nologo --lib ..\Apt
popd
