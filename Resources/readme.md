# Faust Live Resources

This folder contains various resources for FaustLive consisting mainly in Qt `.qrc` files.
A Makefile is included to re-generate the `application.qrc` file that is used to embed the Faust libraries in the FaustLive bundle.

### Usage:
~~~~~~~~~~~~~
make
~~~~~~~~~~~~~
regenerates the  `application.qrc` file.  
Make sure the `Libs` submodule is up-to-date. 

or 
~~~~~~~~~~~~~
make help
~~~~~~~~~~~~~
