FaustLive - How to compile
============================


### Prerequisites
- you must have [Faust](https://faust.grame.fr/) installed, the `faust` command available from the command line, and the compiled `libfaust` library. Be sure to compile `libfaust` with every static and dynamic libraries, that is using the `make all` target in the Faust distibution main folder.
- you must have [Qt](https://www.qt.io/) installed and the `qmake` command available from the command line.
- you must have [LLVM](http://llvm.org/) installed and the `llvm-config` command available from the command line.
- you must have [libmicrohttpd](https://www.gnu.org/software/libmicrohttpd/) installed.
- you must have [libsndfile](http://www.mega-nerd.com/libsndfile/) installed.
- [Linux] you must have [JACK](http://jackaudio.org/) installed.
- [Windows] you must have [PortAudio](http://www.portaudio.com/) installed.


## Compilation steps

It is recommended to build the project in a separate subfolder. It isolates the compilation process from the rest of the project.

### Project generation

After a fresh `git clone`, you'll have to retrieve the Faust libraries git submodule. Type `make update` once in the Build folder.

Be sure to have all library files (located in **Resources/Libs**) and all examples files (located **Resources/Examples**) correctly listed in the **application.qrc** file.

Assuming that you compile in a subfolder named `fldir`, simply do the following:

~~~~
$ cd fldir
$ qmake .. [an optionnal project specification]
~~~~

The project specification determines the output project type: a Makefile, an Xcode project, a Visual Studio project etc. The specification has the following form: 

`-spec` __qmake-spec__ 

where __qmake-spec__ depends on your platform. You should find a folder named `mkspecs` in your Qt distribution, that contains all the supported Qt spec (e.g. macx-xcode, win32-msvc, linux-g++,...). `qmake` has default values for the -spec option (generates a Makefile on Unix like platforms).


### Compilation

It depends on your project type. On most cases you'll only have to type `make`. 
Otherwise open the project with the chosen IDE and proceed as usual with your IDE.

The **FaustLive** application is generated in a subfolder name FaustLive.


### Finalisation

The finalisation step ensures that all the resources and libraries necessary to FaustLive are present for distribution. This finalisation step is required on Windows, required on MacOS for distribution and doesn'texist on Linux. Qt includes tools for that. 

On MacOS type :

~~~~
$ macdeployqt FaustLive/FaustLive.app
~~~~

On Windows type :

~~~~
$ windeployqt FaustLive/FaustLive.exe
~~~~


## Compilation using the Makefile

Works on Unix like platforms. It performs all the above steps with 2 targets:

~~~~
$ make
$ make deploy
~~~~

## Note for Windows

You must make sure you're using the same compilation environment than the one used by Faust and LLVM. The current Faust distribution is based on LLVM 7.0.0, both build with MS Visual Studio 14 2015. If you want to use another environment, you'll have to recompile Faust and LLVM with this environment.

To generate a Visual Studio project with qmake, it is recommended to run `qmake` from a command.com

--------------
#### [Grame](http://www.grame.fr) - Centre National de Creation Musicale
