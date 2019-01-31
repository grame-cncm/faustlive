

mkdir build
cd build
cmake .. -G "Visual Studio 14 2015 Win64"
cpack -g NSIS64
move FaustLive-*.exe ..
