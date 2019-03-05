cmake_minimum_required(VERSION 3.4.0)
project (faustlive)

macro (get_major_minor_patch version)
	string( REGEX REPLACE "([0-9]*)\\.([0-9]*)\\.([0-9]*)" "\\1"  VERSION_MAJOR ${version} )
	string( REGEX REPLACE "([0-9]*)\\.([0-9]*)\\.([0-9]*)" "\\2"  VERSION_MINOR ${version} )
	string( REGEX REPLACE "([0-9]*)\\.([0-9]*)\\.([0-9]*)" "\\3"  VERSION_PATCH ${version} )
endmacro()

get_filename_component(ROOT ${CMAKE_CURRENT_SOURCE_DIR} DIRECTORY)
set (FLDIR ${ROOT}/Build/FaustLive)
set (FLDOC ${ROOT}/Documentation)

#######################################
# versions management
file (READ ${ROOT}/version.txt VERSION)
get_major_minor_patch( ${VERSION} )

message (STATUS "Configuring version ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")


####################################
# application files
file (GLOB APP ${FLDIR}/*)


####################################
# install section
####################################
set (CMAKE_INSTALL_PREFIX .)
if (WIN32)
	set (DEST .)
else()
	set (DEST FaustLive-${VERSION})
endif()
set (CPACKDEST FaustLive-${VERSION})

####################################
# install headers
if(APPLE)
install (
    DIRECTORY ${FLDIR}/FaustLive.app DESTINATION ${DEST}
	FILE_PERMISSIONS USE_SOURCE_PERMISSIONS
    DIRECTORY_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
    	GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
)
elseif (WIN32)
install (
    DIRECTORY ${FLDIR} DESTINATION ${DEST}
    FILE_PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
    DIRECTORY_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
    	GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
    PATTERN ".DS_Store" EXCLUDE
)
endif()

install (
    FILES     ${FLDOC}/faust-quick-reference.pdf ${FLDOC}/UserManual.pdf DESTINATION ${DEST}/Documentation
    PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
)
install (
    FILES     README.html DESTINATION ${DEST}
    PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
)

####################################
# install VS redistributables
if (WIN32)
set (VS "C:/Program Files (x86)/Microsoft Visual Studio 14.0/VC/redist/x64/Microsoft.VC140.CRT")
install (
    FILES  "${VS}/msvcp140.dll" "${VS}/vcruntime140.dll" DESTINATION ${DEST}/FaustLive
    PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
)
endif()


####################################
# cpack section
####################################
set (CPACK_PACKAGE_NAME   "FaustLive")
set (CPACK_PACKAGE_VENDOR "GRAME")
set (CPACK_PACKAGE_DESCRIPTION_SUMMARY "FAUSTLive - an environment for the design of Faust DSP.")
set (CPACK_PACKAGE_VERSION   ${VERSION})
set (CPACK_PACKAGE_VERSION_MAJOR   ${VERSION_MAJOR})
set (CPACK_PACKAGE_VERSION_MINOR   ${VERSION_MINOR})
set (CPACK_PACKAGE_VERSION_PATCH   ${VERSION_PATCH})

set (CPACK_MONOLITHIC_INSTALL true)
set (CPACK_RESOURCE_FILE_LICENSE  ${ROOT}/GPL.txt)
set (CPACK_PACKAGE_INSTALL_DIRECTORY  ${CPACKDEST})

####################################
# NSIS specific section
set (CPACK_NSIS_INSTALL_ROOT "C:\\\\Program Files")
set (CPACK_NSIS_MODIFY_PATH on)
set (CPACK_NSIS_MUI_ICON "${ROOT}/Resources/Images/FaustLiveIcon.ico")

include (CPack)
