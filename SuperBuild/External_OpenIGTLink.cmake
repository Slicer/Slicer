
# Make sure this file is included only once
get_filename_component(CMAKE_CURRENT_LIST_FILENAME ${CMAKE_CURRENT_LIST_FILE} NAME_WE)
IF(${CMAKE_CURRENT_LIST_FILENAME}_FILE_INCLUDED)
  RETURN()
ENDIF()
SET(${CMAKE_CURRENT_LIST_FILENAME}_FILE_INCLUDED 1)

# Set dependency list
set(OpenIGTLink_DEPENDENCIES "")

set(proj OpenIGTLink)
include(${Slicer_SOURCE_DIR}/CMake/SlicerBlockCheckExternalProjectDependencyList.cmake)

set(${proj}_EXTERNAL_PROJECT_INCLUDED TRUE)
#message(STATUS "Adding project '${proj}'")

ExternalProject_Add(${proj}
  SVN_REPOSITORY "http://svn.na-mic.org/NAMICSandBox/trunk/OpenIGTLink"
  SOURCE_DIR OpenIGTLink
  BINARY_DIR OpenIGTLink-build
  CMAKE_GENERATOR ${gen}
  CMAKE_ARGS
    ${ep_common_flags}
    -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
    -DBUILD_SHARED_LIBS:BOOL=ON
    -DOpenIGTLink_DIR:FILEPATH=${CMAKE_BINARY_DIR}/OpenIGTLink-build
    -DOpenIGTLink_PROTOCOL_VERSION_2:BOOL=ON
  INSTALL_COMMAND ""
  DEPENDS 
    ${OpenIGTLink_DEPENDENCIES}
)

set(OpenIGTLink_DIR ${CMAKE_BINARY_DIR}/OpenIGTLink-build)

