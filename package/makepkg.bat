
@echo off

set GEN="Visual Studio 14 2015 Win64"

IF [%1]==[]   GOTO PACKAGE
IF %1==2014   GOTO PACKAGE
set GEN="Visual Studio 15 2017 Win64"

:PACKAGE
mkdir build
cd build

@echo on
cmake .. -G %GEN%
cpack -g NSIS64
move FaustLive-*.exe ..
