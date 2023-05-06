
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
cmake --build . --config Release --target package
move FaustLive-*.exe ..
cd ..
EXIT /B

@rem --- fauslive compilation -------------------------
:COMPILE
	IF NOT EXIST builddir (
		echo Create output folder
		mkdir builddir
	)
	cd builddir
	cmake .. -DLIBSNDFILE=%LIBSNDFILE%
	cmake --build . --config Release
	cd ..
EXIT /B


:USAGE
echo Usage: %0 [COMMAND]
echo where COMMAND is in:
echo   faustlive : compiles faustlive
echo   install : install faustlive to the current folder
echo   package : build the faustlive package
echo   help    : display this help

:DONE
