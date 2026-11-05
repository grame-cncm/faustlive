# Checks that every library referenced by stdfaust.lib (ba = library("basics.lib"); ...)
# exists in the staged session Libs folder, i.e. is listed in Resources/application.qrc.
#
# usage: cmake -DLIBS=<session>/Libs -P check_stdfaust.cmake

if (NOT DEFINED LIBS)
	message (FATAL_ERROR "usage: cmake -DLIBS=<dir> -P check_stdfaust.cmake")
endif()

file (STRINGS ${LIBS}/stdfaust.lib LINES REGEX "library\\(\"[^\"]+\"\\)")
set (MISSING)
set (COUNT 0)
foreach (LINE ${LINES})
	if (LINE MATCHES "^[ \t]*//")
		continue()
	endif()
	string (REGEX MATCH "([A-Za-z_][A-Za-z0-9_]*)[ \t]*=[ \t]*library\\(\"([^\"]+)\"\\)" M "${LINE}")
	if (M)
		math (EXPR COUNT "${COUNT} + 1")
		if (NOT EXISTS ${LIBS}/${CMAKE_MATCH_2})
			list (APPEND MISSING "${CMAKE_MATCH_1}. -> ${CMAKE_MATCH_2}")
		endif()
	endif()
endforeach()

if (COUNT EQUAL 0)
	message (FATAL_ERROR "no library() declaration found in ${LIBS}/stdfaust.lib")
endif()
if (MISSING)
	string (REPLACE ";" "\n  " MISSING "${MISSING}")
	message (FATAL_ERROR "stdfaust.lib prefixes without a bundled library (add them to Resources/application.qrc):\n  ${MISSING}")
endif()
message (STATUS "All ${COUNT} stdfaust.lib prefixes resolve in ${LIBS}")
