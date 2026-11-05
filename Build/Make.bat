
@echo off

set VERSION="2.5.15"

IF [%1]==[]     GOTO USAGE
IF %1==faustlive 	(
	CALL :COMPILE
	GOTO DONE
)
IF %1==install (
	CALL :INSTALL
	GOTO DONE
)
IF %1==package (
	CALL :PACKAGE
	GOTO DONE
)

GOTO USAGE

@rem --- installation -------------------------------
:INSTALL
cd builddir
cmake .. -DPREFIX=../FaustLive-%VERSION%
cmake --build . --config Release --target install
cd ..
EXIT /B

@rem --- packaging ----------------------------------
:PACKAGE
cd builddir
cmake .. -DPACK=on
IF ERRORLEVEL 1 GOTO FAILED
cmake --build . --config Release --target package
IF ERRORLEVEL 1 GOTO FAILED
move FaustLive-*.exe ..
IF ERRORLEVEL 1 GOTO FAILED
cd ..
EXIT /B 0

@rem --- fauslive compilation -------------------------
:COMPILE
	IF NOT EXIST builddir (
		echo Create output folder
		mkdir builddir
	)
	cd builddir
	cmake .. -DCMAKE_VERBOSE_MAKEFILE=ON -DLIBSNDFILE=%LIBSNDFILE%
	IF ERRORLEVEL 1 GOTO FAILED
	cmake --build . --config Release
	IF ERRORLEVEL 1 GOTO FAILED
	cd ..
EXIT /B 0

@rem --- a step failed: back to the Build folder, report the error -----
:FAILED
cd ..
echo Make.bat: a build step failed
EXIT /B 1


:USAGE
echo Usage: %0 [COMMAND]
echo where COMMAND is in:
echo   faustlive : compiles faustlive
echo   install : install faustlive to the current folder
echo   package : build the faustlive package
echo   help    : display this help

:DONE
