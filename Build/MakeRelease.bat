
echo off

IF [%2]==[]     GOTO USAGE
IF %2==2015 (
    set GEN="Visual Studio 14 2015 Win64"
    GOTO NEXT
)
IF %2==2017 (
    set GEN="Visual Studio 15 2017 Win64"
    GOTO NEXT
)
GOTO USAGE

:NEXT
echo "Make sure you're on the correct branch"
echo "Make sure your repo is uptodate (git pull --rebase)"

set BUILD=%1

echo "###################### Building FaustLive ######################"
call Make.bat %BUILD%

echo "####################### Building package #######################"
cd ../package
call makepkg.bat %2
cd ../Build

echo DONE
GOTO END

:USAGE
echo "Usage: MakeRelease.bat <builddir> <VS version>
echo "       <builddir>    is the windows build folder"
echo "       <VS version>  is a visual studio version: 2015 or 2017"

:END
