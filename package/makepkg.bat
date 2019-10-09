
@echo off

IF [%1]==[]     GOTO USAGE
IF %1==2015 (
    set GEN="Visual Studio 14 2015 Win64"
    GOTO NEXT
)
IF %1==2017 (
    set GEN="Visual Studio 15 2017 Win64"
    GOTO NEXT
)
GOTO USAGE

:NEXT

IF NOT EXIST build mkdir build
cd build

@echo on
cmake .. -G %GEN%
cpack -g NSIS64
@echo off
move FaustLive-*.exe ..
cd ..
GOTO END

:USAGE
echo "Usage: makepkg.bat <VS version>
echo "       <VS version>  is a visual studio version: 2015 or 2017"

:END
