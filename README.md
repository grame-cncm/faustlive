
FaustLive 
============================
## The swiss knife for [Faust](https://faust.grame.fr) development.
--------------

FaustLive is a stand-alone application that compiles and run Faust dsp code on the fly using [LLVM](http://llvm.org).

It supports also native applications generation using the [Faust online compiler](https://faust.grame.fr/tools/onlinecompiler/).

### Limitations and known bugs

 - crashes on Windows when compiling a dsp with input signals, on a machine without input audio device
 - may crash on startup due to incorrect cache. In this case, find the `FaustLive-CurrentSession-2.0` folder in your home folder, remove it and restart. On Unix like stations (Linux, MacOS), this folder is hidden and located at `$HOME/.FaustLive-CurrentSession-2.0` 
 

--------------
#### [Grame](http://www.grame.fr) - Centre National de Creation Musicale
