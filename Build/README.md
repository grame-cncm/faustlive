FaustLive - How to compile
============================

### Prerequisites
- you must have [Faust](https://faust.grame.fr/) installed, the `faust` command available from the command line, and the compiled `libfaust` library. Be sure to compile `libfaust` with every static and dynamic libraries, that is using the `make all` target in the Faust distibution main folder.
- you must have [Qt](https://www.qt.io/) installed and the `qmake` command available from the command line.
- you must have [LLVM](http://llvm.org/) installed and the `llvm-config` command available from the command line.
- you must have [cmake](https://cmake.org/) installed and the `cmake` command available from the command line.
- you must have [libmicrohttpd](https://www.gnu.org/software/libmicrohttpd/) installed.
- you must have [libsndfile](http://www.mega-nerd.com/libsndfile/) installed.
- [Linux] you must have [JACK](http://jackaudio.org/) installed.
- [Windows] you must have [PortAudio](http://www.portaudio.com/) installed.

## Preparing a new release

Be sure to raise the `-DAPP_VERSION="X.X"` flag in the *CMakeLists.txt* file so that the .*FaustLive-CurrentSession-XX* cache folder is recreated with the new compiled and released version.

## Compilation

Make sure that the installed Faust package is using the same LLVM version than the one used to compile FaustLive. To check this point, you can query both faust and llvm:
~~~~~~~~~~
> faust --version
> llvm-config --version
~~~~~~~~~~

Once this preliminary verification have been made, from the `Build` folder type 
~~~~~~~~~~
> make
~~~~~~~~~~

For more details about the build process:
~~~~~~~~~~
> make help
~~~~~~~~~~

The **FaustLive** application is generated in the `bin` folder.

### Note for Windows

Some environment variables are necessary. Here are the steps. The implementation may need to change based on your system.

Put `llvm-config.exe` in our `PATH` ([INSTALL](https://github.com/DBraun/TD-Faust/releases/download/llvm-15.0.7-win11-x86_64/llvm-15.0.7-win11-x86_64.zip)).
```bash
set PATH=%USERPROFILE%\faustlive\llvm\bin;%PATH%
```

Set `LIBSNDFILE` to a path containing `bin/sndfile.dll` ([INSTALL](https://github.com/libsndfile/libsndfile/releases/download/1.2.0/libsndfile-1.2.0-win64.zip)).
```bash
set LIBSNDFILE=%USERPROFILE%\faustlive\libsndfile
```

Next we need Qt5. You can get Qt5 with with `pip install aqtinstall` and then `aqt install-qt windows desktop 5.15.2 win64_msvc2019_64`.

Set `Qt5_DIR` to a directory containing `Qt5Config.cmake`.
```bash
set Qt5_DIR=C:\SDKs\Qt\5.15.13\msvc2019_64\lib\cmake\Qt5
```

Then
~~~~~~~~~~
> Make.bat faustlive
~~~~~~~~~~

A `Make.bat` script provides services equivalent to the `Makefile`.
For more details:
~~~~~~~~~~
> Make.bat help
~~~~~~~~~~

## Distribution

A specific target is provided to build a FaustLive package:
~~~~~~~~~~
> make package
~~~~~~~~~~

on Windows:
~~~~~~~~~~
> Make.bat package
~~~~~~~~~~

There is no equivalent service for Linux.

--------------
#### [Grame](http://www.grame.fr) - Centre National de Creation Musicale
