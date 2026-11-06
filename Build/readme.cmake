# Generates README.html (installed next to FaustLive on macOS and Windows) from
# README.md and changelog.txt, rendered by strapdown.min.js in the browser.
# Called by Build/CMakeLists.txt at build time, so that the packages always
# hold the current README and change log.
#
# usage: cmake -DROOT=<FaustLive root> -DOUT=<README.html> -P readme.cmake

if (NOT DEFINED ROOT OR NOT DEFINED OUT)
	message (FATAL_ERROR "usage: cmake -DROOT=<dir> -DOUT=<file> -P readme.cmake")
endif()

file (READ ${ROOT}/README.md README)
file (READ ${ROOT}/changelog.txt CHANGELOG)
# skip the 7 header lines of the change log (title, Grame, copyright)
# (not with REGEX REPLACE: its '^' matches again after each replacement)
foreach (I RANGE 1 7)
	string (FIND "${CHANGELOG}" "\n" EOL)
	math (EXPR EOL "${EOL} + 1")
	string (SUBSTRING "${CHANGELOG}" ${EOL} -1 CHANGELOG)
endforeach()
file (READ ${ROOT}/Build/strapdown.min.js STRAPDOWN)

file (WRITE  ${OUT} "<!DOCTYPE html><html><xmp>\n")
file (APPEND ${OUT} "${README}")
file (APPEND ${OUT} "<br /><br />\n###Change log\n")
file (APPEND ${OUT} "${CHANGELOG}")
file (APPEND ${OUT} "</xmp>\n<script type=\"text/javascript\">\n")
file (APPEND ${OUT} "${STRAPDOWN}")
file (APPEND ${OUT} "</script>\n</html>\n")
