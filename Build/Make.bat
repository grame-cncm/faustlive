
echo off

IF [%1]==[]     GOTO USAGE

echo "This script must be called from the adequate command.com"
echo "i.e. the one distributed with your target Visual Studio version"
echo "and corresponding to your target architecture (x64)"

set BUILD=%1
set PROJ=FaustLive.vcxproj

IF EXIST %BUILD% GOTO PROJECT
mkdir %BUILD%


:PROJECT
cd %BUILD%
IF EXIST %PROJ% GOTO COMPILE
echo "################# Generating FaustLive project #################"
qmake ..

:COMPILE
echo "###################### Building FaustLive ######################"
msbuild %PROJ%   /p:Configuration=Release /p:Platform=x64 /maxcpucount:4

cd ../FaustLive
windeployqt --release --compiler-runtime FaustLive.exe
cd ..

echo "DONE - Your package is available from the package folder"
GOTO END

:USAGE
echo "Usage: Make.bat <builddir>
echo "       <builddir>    is the windows build folder"

:END
