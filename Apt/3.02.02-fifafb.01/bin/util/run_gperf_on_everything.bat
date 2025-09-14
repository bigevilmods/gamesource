@echo off
rem Check out all of the output files
p4 edit ..\..\source\Apt\AptObjects.h
p4 edit ..\..\source\Apt\AptObject\AptSpriteMembers.cpp
p4 edit ..\..\source\Apt\AptValue\AptTextMembers.h
p4 edit ..\..\source\Apt\AptObject\AptXmlMembers.cpp
p4 edit ..\..\source\Apt\AptObject\AptDateMembers.h
p4 edit ..\..\source\Apt\AptObject\AptKeyMembers.h
p4 edit ..\..\source\Apt\AptObject\AptUtilMembers.h
p4 edit ..\..\source\Apt\AptObject\AptLoadVarsMembers.h
p4 edit ..\..\source\Apt\AptObject\AptMathMembers.h
p4 edit ..\..\source\Apt\AptObject\AptSoundMembers.h
p4 edit ..\..\source\Apt\AptObject\AptStageMembers.cpp
p4 edit ..\..\source\Apt\AptValue\AptStringMembers.h
p4 edit ..\..\source\Apt\AptObject\AptTextFormatMembers.h
p4 edit ..\..\source\Apt\AptValue\AptArrayMembers.h

rem Run gperf on everything
run_gperf.pl --all

rem Pause so user can see the output
pause