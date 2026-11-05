# Stages the Faust libraries and examples exactly as FaustLive installs them in
# its session folder (~/.FaustLive-CurrentSession-<APP_VERSION>/{Libs,Examples}):
# FaustLive copies them from the ':/Libs' and ':/Examples' Qt resources, which are
# described by Resources/application.qrc (including the 'alias' renames).
#
# usage: cmake -DQRC=<application.qrc> -DDEST=<dir> -P stage_resources.cmake

if (NOT DEFINED QRC OR NOT DEFINED DEST)
	message (FATAL_ERROR "usage: cmake -DQRC=<application.qrc> -DDEST=<dir> -P stage_resources.cmake")
endif()

get_filename_component (QRCDIR ${QRC} DIRECTORY)
file (REMOVE_RECURSE ${DEST})
file (STRINGS ${QRC} ENTRIES REGEX "<file")

set (NLIBS 0)
set (NEXAMPLES 0)
foreach (ENTRY ${ENTRIES})
	if (ENTRY MATCHES "<file( +alias=\"([^\"]+)\")? *>([^<]+)</file>")
		set (SRC "${CMAKE_MATCH_3}")
		set (NAME "${CMAKE_MATCH_2}")
		if (NAME STREQUAL "")
			set (NAME "${SRC}")
		endif()
		if (NAME MATCHES "^Libs/")
			math (EXPR NLIBS "${NLIBS} + 1")
		elseif (NAME MATCHES "^Examples/")
			math (EXPR NEXAMPLES "${NEXAMPLES} + 1")
		else()
			continue()
		endif()
		if (NOT EXISTS ${QRCDIR}/${SRC})
			message (FATAL_ERROR "${QRC} references a missing file: ${SRC}")
		endif()
		if (EXISTS ${DEST}/${NAME})
			message (FATAL_ERROR "${QRC}: two entries are installed as ${NAME}")
		endif()
		configure_file (${QRCDIR}/${SRC} ${DEST}/${NAME} COPYONLY)
	endif()
endforeach()

if (NOT EXISTS ${DEST}/Libs/stdfaust.lib)
	message (FATAL_ERROR "${QRC} does not provide Libs/stdfaust.lib")
endif()
message (STATUS "Staged ${NLIBS} libraries and ${NEXAMPLES} examples in ${DEST}")
