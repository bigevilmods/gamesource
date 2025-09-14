@echo off
p4 edit ../../include/rw/math/undefine_macros.h

REM Grep options to scan for defined macros
set GREP_OPTIONS=-h -e "#define.*("

REM Sed command (runs sed using the build_undef.sed script)
set SED_COMMAND=sed -f ../../src/scripts/build_undef.sed

REM Headers that should be parsed for all platforms
set COMMON_HEADERS=../../include/rw/math/fpu/detail/*.h         ../../include/rw/math/vpu/detail/*.h              ^
                   ../../include/rw/math/fpu/detail/ps3/ppu/*.h ../../include/rw/math/vpu/detail/ps3/ppu/*.h      ^
                   ../../include/rw/math/fpu/detail/ps3/spu/*.h ../../include/rw/math/vpu/detail/ps3/spu/*.h      ^
                   ../../include/rw/math/fpu/detail/xbox2/*.h   ../../include/rw/math/vpu/detail/xbox2/*.h        ^
                                                                ../../include/rw/math/vpu/detail/x86/*.h          ^
                   ../../include/rw/math/fpu/detail/cross/*.h   ../../include/rw/math/vpu/detail/cross/*.h        ^
                   ../../include/rw/math/fpu/detail/wii/*.h     ../../include/rw/math/vpu/detail/cross/*.h        ^
                     
REM Build undefine headers for all platforms.
echo Generating undefine_macros.h 
grep %GREP_OPTIONS% %COMMON_HEADERS% | sort | uniq | %SED_COMMAND% > ../../include/rw/math/undefine_macros.h
