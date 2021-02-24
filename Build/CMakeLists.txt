cmake_minimum_required(VERSION 3.8.0 FATAL_ERROR)

project(FaustLive)

#######################################
get_filename_component(ROOT ${CMAKE_CURRENT_SOURCE_DIR} DIRECTORY)
set (SRCDIR  	 ${ROOT}/src)
set (RSRCDIR  	 ${ROOT}/Resources)
set (LOCALLIBDIR ${ROOT}/lib)

#######################################
# versions management
file (STRINGS "${ROOT}/version.txt" VERSION)
message (STATUS "Generates FaustLive version ${VERSION}")

#######################################
# Options 
option ( REMOTE 		"Includes remote computing" off )

set (CMAKE_CXX_STANDARD 11)

#######################################
# check for pkg-config 
# used later for libsndfile and libmicrohttpd
if (NOT WIN32)
include(FindPkgConfig)
if (NOT PKG_CONFIG_FOUND)
	message (FATAL "Cannot find pkg-config")
endif()
endif()


#######################################
# faust settings
execute_process(COMMAND faust  -libdir
                OUTPUT_VARIABLE FAUST_LIBRARY_DIR_TMP
                RESULT_VARIABLE FAUST_FOUND)
if (NOT FAUST_FOUND EQUAL 0)
	message (FATAL "faust not found: ${FAUST_FOUND}")
endif()
execute_process(COMMAND faust  -includedir OUTPUT_VARIABLE FAUST_INCLUDE_DIRS_TMP )
execute_process(COMMAND faust  -v OUTPUT_VARIABLE FAUST_VERSION_TMP )
string(REGEX REPLACE "\n$" "" FAUST_LIBRARY_DIR "${FAUST_LIBRARY_DIR_TMP}")
string(REGEX REPLACE "\n$" "" FAUST_INCLUDE_DIRS "${FAUST_INCLUDE_DIRS_TMP}")
string(REGEX MATCH "Version [0-9.]*" FAUST_VERSION ${FAUST_VERSION_TMP})
if (WIN32)
	string (REPLACE "\\" "/" WFAUST_LIBRARY_DIR ${FAUST_LIBRARY_DIR})
	string (REPLACE "\\" "/" WFAUST_INCLUDE_DIRS ${FAUST_INCLUDE_DIRS})
	set (FAUST_INCLUDE_DIRS ${WFAUST_INCLUDE_DIRS})
	set (FAUST_LIBRARIES ${WFAUST_LIBRARY_DIR}/libfaust.lib)
else()
	set (FAUST_LIBRARIES "-L${FAUST_LIBRARY_DIR}" -lfaust -lHTTPDFaust -lOSCFaust)
	set (FAUSTLIVE_RPATH ${FAUSTLIVE_RPATH} ${FAUST_LIBRARY_DIR})
endif()
message (STATUS "Using Faust ${FAUST_VERSION}")


#######################################
# llvm settings
if (NOT DEFINED LLVM_CONFIG)
	set (LLVM_CONFIG llvm-config)
endif()
execute_process(COMMAND ${LLVM_CONFIG}  --version
                OUTPUT_VARIABLE LLVM_VERSION_TMP
                RESULT_VARIABLE LLVM_FOUND)
if (NOT LLVM_FOUND EQUAL 0)
	message (FATAL " ${LLVM_CONFIG} not found: ${LLVM_FOUND}")
endif()
execute_process(COMMAND ${LLVM_CONFIG}  --ldflags OUTPUT_VARIABLE LLVM_LDFLAGS_TMP )
execute_process(COMMAND ${LLVM_CONFIG}  --libdir OUTPUT_VARIABLE LLVM_LIBDIR )
execute_process(COMMAND ${LLVM_CONFIG}  --libs OUTPUT_VARIABLE LLVM_LIBRARIES_TMP1 )
execute_process(COMMAND ${LLVM_CONFIG}  --system-libs OUTPUT_VARIABLE LLVM_LIBRARIES_TMP2 )
string(REGEX REPLACE "\n$" "" LLVM_VERSION "${LLVM_VERSION_TMP}")
string(REGEX REPLACE " *\n$" "" LLVM_LIBRARIES "${LLVM_LIBRARIES_TMP1}")
string(REGEX REPLACE " *\n$" "" LLVM_SYS_LIBRARIES "${LLVM_LIBRARIES_TMP2}")
string(REGEX REPLACE " *\n$" "" LLVM_LDFLAGS "${LLVM_LDFLAGS_TMP}")
string(REGEX REPLACE "\n$" "" LLVM_RPATH "${LLVM_LIBDIR}")
if (WIN32)
	set (LLVM_LIBRARIES ${LLVM_LIBRARIES} ${LLVM_SYS_LIBRARIES})
else()
	set (LLVM_LIBRARIES ${LLVM_LDFLAGS} ${LLVM_LIBRARIES} ${LLVM_SYS_LIBRARIES})
	set (FAUSTLIVE_RPATH ${FAUSTLIVE_RPATH} ${LLVM_RPATH})
endif()
set (LLVM_DEFINES -DLLVM_VERSION="${LLVM_VERSION}")
message (STATUS "Using LLVM version ${LLVM_VERSION}")

#######################################
# Qt settings
set (CMAKE_AUTOMOC ON)
set (CMAKE_AUTOUIC ON)
set (CMAKE_AUTORCC ON)
find_package(Qt5 COMPONENTS Core Gui Widgets Network REQUIRED)
set (QTINCDIRS 	${Qt5Core_INCLUDE_DIRS} ${Qt5Gui_INCLUDE_DIRS} ${Qt5Widgets_INCLUDE_DIRS} ${Qt5Network_INCLUDE_DIRS})
set (QTLIBS 	Qt5::Core Qt5::Gui Qt5::Widgets Qt5::Network)
message (STATUS "Using Qt version ${Qt5Core_VERSION}")

#######################################
# set sources and include dirs
file (GLOB FAUSTLIVE_SRC  RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${SRCDIR}/*/*.cpp )
file (GLOB FAUSTLIVE_HEADERS  RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}  ${SRCDIR}/*/*.h)
file (GLOB FAUSTLIVE_INCLUDE_DIRS  RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${SRCDIR}/* "/usr/local/include")
set (FAUSTLIVE_HEADERS ${FAUSTLIVE_HEADERS} ${FAUST_INCLUDE_DIRS}/faust/gui/QTUI.h)
#message ("FAUSTLIVE_HEADERS ${FAUSTLIVE_HEADERS}")

#######################################
# libmicrohttpd settings
if (WIN32)
	set (FAUSTLIVE_LIBRARIES ${FAUSTLIVE_LIBRARIES} ${LOCALLIBDIR}/libmicrohttpd/x64/libmicrohttpd.lib)
	set (FAUSTLIVE_INCLUDE_DIRS ${FAUSTLIVE_INCLUDE_DIRS} ${LOCALLIBDIR}/libmicrohttpd)
else()
	pkg_check_modules (LMHTTPD REQUIRED libmicrohttpd)
	set (FAUSTLIVE_LIBRARIES ${FAUSTLIVE_LIBRARIES} ${LMHTTPD_LDFLAGS})
	set (FAUSTLIVE_INCLUDE_DIRS ${FAUSTLIVE_INCLUDE_DIRS} ${LMHTTPD_INCLUDE_DIRS})
	set (FAUSTLIVE_RPATH ${FAUSTLIVE_RPATH} ${LMHTTPD_LIBRARY_DIRS})
endif()

#######################################
# sndfile support
if (WIN32)
	if (NOT DEFINED LIBSNDFILE)
		set ( LIBSNDFILE "C:/Program Files/Mega-Nerd/libsndfile" )
	endif()
	set (FAUSTLIVE_LIBRARIES ${FAUSTLIVE_LIBRARIES}  winmm.lib ws2_32.lib ${LIBSNDFILE}/lib/libsndfile-1.lib)
	set (FAUSTLIVE_INCLUDE_DIRS ${FAUSTLIVE_INCLUDE_DIRS}  ${LIBSNDFILE}/include)
	set (FAUSTLIVE_DEFINITIONS ${FAUSTLIVE_DEFINITIONS} -D_WIN32 -DUSEWINMAIN)
else()
	pkg_check_modules (SNDFILE REQUIRED sndfile)
	set (FAUSTLIVE_LIBRARIES ${FAUSTLIVE_LIBRARIES} ${SNDFILE_LDFLAGS} -lcurl)
	set (FAUSTLIVE_INCLUDE_DIRS ${FAUSTLIVE_INCLUDE_DIRS} ${SNDFILE_INCLUDE_DIRS})
	set (FAUSTLIVE_RPATH ${FAUSTLIVE_RPATH} ${SNDFILE_LIBRARY_DIRS})
endif()

#######################################
# audio settings
if ((NOT DEFINED AUDIO) OR (AUDIO STREQUAL ""))
	if (WIN32)
		set(AUDIO portaudio)
	elseif (APPLE)
		set(AUDIO coreaudio)
	else()
		set(AUDIO jack)
	endif()
endif()

if (AUDIO STREQUAL coreaudio)
	message (STATUS "Using Core Audio drivers")
	set (FAUSTLIVE_LIBRARIES ${FAUSTLIVE_LIBRARIES} "-framework CoreAudio" "-framework AudioUnit" "-framework CoreServices" "-framework CoreMIDI" )
	set (FAUSTLIVE_DEFINITIONS ${FAUSTLIVE_DEFINITIONS} -DCOREAUDIO)
	set (FAUSTLIVE_INCLUDE_DIRS ${FAUSTLIVE_INCLUDE_DIRS} ${SRCDIR}/Audio/CA)
	file (GLOB AUDIO_SRC  RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${SRCDIR}/Audio/CA/*.cpp )
	file (GLOB AUDIO_HEADERS  RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}  ${SRCDIR}/Audio/CA/*.h )

elseif (AUDIO STREQUAL portaudio)
	message (STATUS "Using PortAudio drivers")
	set (FAUSTLIVE_DEFINITIONS ${FAUSTLIVE_DEFINITIONS} -DPORTAUDIO)
	set (FAUSTLIVE_INCLUDE_DIRS ${FAUSTLIVE_INCLUDE_DIRS} ${SRCDIR}/Audio/PA ${ROOT}/lib/portaudio/include)
	file (GLOB AUDIO_SRC  RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${SRCDIR}/Audio/PA/*.cpp )
	file (GLOB AUDIO_HEADERS  RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}  ${SRCDIR}/Audio/PA/*.h )
	if (WIN32)
		set (FAUSTLIVE_LIBRARIES ${FAUSTLIVE_LIBRARIES} ${ROOT}/lib/portaudio/lib/portaudio.lib )
		set (FAUSTLIVE_INCLUDE_DIRS ${FAUSTLIVE_INCLUDE_DIRS} ${LOCALLIBDIR}/portaudio/include)
	else()
		set (FAUSTLIVE_LIBRARIES ${FAUSTLIVE_LIBRARIES} -lportaudio )
	endif()

elseif (AUDIO STREQUAL alsa)
	if (APPLE OR NOT UNIX)
		message (FATAL "Can't use alsa drivers on non Unix platform.")	
	endif()
	message (STATUS "Using Alsa drivers")
	set (FAUSTLIVE_DEFINITIONS ${FAUSTLIVE_DEFINITIONS} -DALSA)
	set (FAUSTLIVE_INCLUDE_DIRS ${FAUSTLIVE_INCLUDE_DIRS} ${SRCDIR}/Audio/AL)
	file (GLOB AUDIO_SRC  RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${SRCDIR}/Audio/AL/*.cpp )
	file (GLOB AUDIO_HEADERS  RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}  ${SRCDIR}/Audio/AL/*.h )
	set (FAUSTLIVE_LIBRARIES ${FAUSTLIVE_LIBRARIES} -lasound )

elseif (AUDIO STREQUAL jack)
	message (STATUS "Using Jack Audio drivers")
	set (FAUSTLIVE_DEFINITIONS ${FAUSTLIVE_DEFINITIONS} -DJACK)
	set (FAUSTLIVE_INCLUDE_DIRS ${FAUSTLIVE_INCLUDE_DIRS} ${SRCDIR}/Audio/JA)
	file (GLOB AUDIO_SRC  RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${SRCDIR}/Audio/JA/*.cpp )
	file (GLOB AUDIO_HEADERS  RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}  ${SRCDIR}/Audio/JA/*.h )
	set (FAUSTLIVE_LIBRARIES ${FAUSTLIVE_LIBRARIES} -ljack )

elseif (AUDIO STREQUAL netjack)
	message (STATUS "Using NetJack Audio drivers")
	set (FAUSTLIVE_DEFINITIONS ${FAUSTLIVE_DEFINITIONS} -DNETJACK)
	set (FAUSTLIVE_INCLUDE_DIRS ${FAUSTLIVE_INCLUDE_DIRS} ${SRCDIR}/Audio/JA)
	file (GLOB AUDIO_SRC  RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${SRCDIR}/Audio/NJ_Master/*.cpp ${SRCDIR}/Audio/NJ_Slave/*.cpp )
	file (GLOB AUDIO_HEADERS  RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${SRCDIR}/Audio/NJ_Master/*.h ${SRCDIR}/Audio/NJ_Slave/*.h )
	set (FAUSTLIVE_LIBRARIES ${FAUSTLIVE_LIBRARIES} -ljacknet )

else ()
	message (WARNING "Unknown Audio driver '${AUDIO}'")
endif()


#######################################
set ( FAUSTLIVE_SRC 	${FAUSTLIVE_SRC} 	 ${AUDIO_SRC})
set ( FAUSTLIVE_HEADERS ${FAUSTLIVE_HEADERS} ${AUDIO_HEADERS})

set (INCLUDE_DIRS ${FAUSTLIVE_INCLUDE_DIRS} ${QTINCDIRS} ${FAUST_INCLUDE_DIRS})
set (FAUSTLIVE_DEFINITIONS ${FAUSTLIVE_DEFINITIONS} -DVERSION="${VERSION}" -DAPP_VERSION="2.0" -DHTTPCTRL -DQRCODECTRL ${LLVM_DEFINES})
set (FAUSTLIVE_LIBRARIES ${FAUSTLIVE_LIBRARIES} ${FAUST_LIBRARIES} ${QTLIBS} ${LLVM_LIBRARIES} ${LIBMICROHPPTD_LIBRARIES})
#message ("FAUSTLIVE_LIBRARIES ${FAUSTLIVE_LIBRARIES}")


#######################################
# add ui and resources
set (FAUSTLIVE_RSRC ${RSRCDIR}/application.qrc ${RSRCDIR}/styles.qrc)
set (FAUSTLIVE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/rsrc/FaustLiveInfo.plist)
set (FAUSTLIVE_ICON_FILE ${RSRCDIR}/Images/FaustLiveIcon.icns)
set_source_files_properties(${FAUSTLIVE_RSRC} ${FAUSTLIVE_ICON_FILE} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
set(FAUSTLIVE_SRC ${FAUSTLIVE_SRC} ${FAUSTLIVE_RSRC} ${FAUSTLIVE_ICON_FILE})

if (WIN32)
	enable_language(RC)
	set(FAUSTLIVE_SRC ${FAUSTLIVE_SRC} ${CMAKE_CURRENT_SOURCE_DIR}/rsrc/FaustLive.rc)
	set(FAUSTLIVE_EXE WIN32)
elseif(APPLE)
	set(FAUSTLIVE_EXE MACOSX_BUNDLE)
endif()

if(APPLE)
	set (OSXTARGET 10.11)
	set (CMAKE_C++_FLAGS -mmacosx-version-min=${OSXTARGET})
	set (CMAKE_LDFLAGS -mmacosx-version-min=${OSXTARGET})
	set (CMAKE_OSX_DEPLOYMENT_TARGET ${OSXTARGET})
	set (MACOSX_DEPLOYMENT_TARGET ${OSXTARGET})
	message (STATUS "MacOSX deployment target set to ${OSXTARGET}")
elseif(UNIX)
	set (FAUSTLIVE_LIBRARIES ${FAUSTLIVE_LIBRARIES} -lpthread -lasound)
endif()

#message ("FAUSTLIVE_LIBRARIES ${FAUSTLIVE_LIBRARIES}")

set (FAUSTLIVE_RPATH ${FAUSTLIVE_RPATH} "/usr/lib")
set (CMAKE_BUILD_RPATH ${FAUSTLIVE_RPATH})
set (CMAKE_SKIP_BUILD_RPATH FALSE)
#message ("FAUSTLIVE_RPATH ${FAUSTLIVE_RPATH}")

set (BINDIR ${CMAKE_CURRENT_SOURCE_DIR}/bin)
#######################################
# add FaustLive
set (faustlive FaustLive)
add_executable(${faustlive} ${FAUSTLIVE_EXE} ${FAUSTLIVE_SRC} ${FAUSTLIVE_HEADERS})
target_include_directories (${faustlive} PRIVATE ${INCLUDE_DIRS})
target_compile_definitions (${faustlive} PRIVATE ${FAUSTLIVE_DEFINITIONS})
target_link_libraries (${faustlive} PRIVATE ${FAUSTLIVE_LIBRARIES} )
set_target_properties (${faustlive} PROPERTIES
	BUNDLE true
	MACOSX_BUNDLE_BUNDLE_NAME ${faustlive}
	MACOSX_BUNDLE_ICON_FILE ${FAUSTLIVE_ICON_FILE}
	MACOSX_BUNDLE_INFO_PLIST ${FAUSTLIVE_INFO_PLIST}
	MACOSX_BUNDLE_BUNDLE_VERSION "v.${VERSION}"
	MACOSX_BUNDLE_SHORT_VERSION_STRING "${VERSION}"
	MACOSX_BUNDLE_INFO_PLIST ${FAUSTLIVE_INFO_PLIST}
	RUNTIME_OUTPUT_DIRECTORY  ${BINDIR}
	RUNTIME_OUTPUT_DIRECTORY_RELEASE  ${BINDIR}
)

