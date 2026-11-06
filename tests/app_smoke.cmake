# Smoke test of the FaustLive application: starts it headless (Qt 'offscreen'
# platform) with a DSP file, in a temporary HOME so that the user's own
# ~/.FaustLive-CurrentSession-<APP_VERSION> folder is never touched, lets it run
# for a few seconds, then checks that:
#   - it did not exit or crash on its own,
#   - the session folder was created with the bundled libraries (flattened Libs),
#   - the DSP was compiled (LLVM bitcode written in SHAFolder),
# then starts it again in the same session and checks that the bitcode cache
# was read back (the bitcode file is not written again).
# Needs a working audio device (CoreAudio on macOS, JACK on Linux).
#
# usage: cmake -DAPP=<FaustLive binary> -DWORKDIR=<dir> -DDSP=<file.dsp>
#              -DAPP_VERSION=<x.y> [-DDURATION=<seconds>] -P app_smoke.cmake

foreach (VAR APP WORKDIR DSP APP_VERSION)
	if (NOT DEFINED ${VAR})
		message (FATAL_ERROR "missing -D${VAR}=...")
	endif()
endforeach()
if (NOT DEFINED DURATION)
	set (DURATION 10)
endif()

file (REMOVE_RECURSE ${WORKDIR})
file (MAKE_DIRECTORY ${WORKDIR}/home)
set (ENV{HOME} ${WORKDIR}/home)
set (ENV{USERPROFILE} ${WORKDIR}/home)
set (ENV{QT_QPA_PLATFORM} offscreen)

function (run_faustlive)
	execute_process (COMMAND ${APP} ${DSP}
		TIMEOUT ${DURATION}
		RESULT_VARIABLE RESULT
		OUTPUT_FILE ${WORKDIR}/stdout.txt
		ERROR_FILE ${WORKDIR}/stderr.txt)
	if (NOT RESULT MATCHES "timeout")
		file (READ ${WORKDIR}/stderr.txt ERR)
		message (FATAL_ERROR "FaustLive exited before ${DURATION}s (result: ${RESULT})\n${ERR}")
	endif()
endfunction()

run_faustlive()

if (WIN32)
	set (SESSION ${WORKDIR}/home/FaustLive-CurrentSession-${APP_VERSION})
else()
	set (SESSION ${WORKDIR}/home/.FaustLive-CurrentSession-${APP_VERSION})
endif()

foreach (FILE Libs/stdfaust.lib Libs/dx7.lib Libs/music.lib Settings.ini)
	if (NOT EXISTS ${SESSION}/${FILE})
		message (FATAL_ERROR "missing ${SESSION}/${FILE}")
	endif()
endforeach()

file (GLOB_RECURSE BITCODE ${SESSION}/SHAFolder/*_bitcode_process.bc)
if (NOT BITCODE)
	file (READ ${WORKDIR}/stderr.txt ERR)
	message (FATAL_ERROR "no compiled DSP in ${SESSION}/SHAFolder\n${ERR}")
endif()

# Second run, same session: the DSP must come from the bitcode cache
# (FLSessionManager::createFactory rewrites the bitcode only when it compiles)
list (GET BITCODE 0 FIRST_BITCODE)
file (TIMESTAMP ${FIRST_BITCODE} FIRST_TIME "%s")
run_faustlive()
file (TIMESTAMP ${FIRST_BITCODE} SECOND_TIME "%s")
if (NOT FIRST_TIME STREQUAL SECOND_TIME)
	message (FATAL_ERROR "${FIRST_BITCODE} was written again: the bitcode cache was not used")
endif()
message (STATUS "FaustLive ran twice ${DURATION}s, session in ${SESSION}")
