
# Make sure this file is included only once
get_filename_component(CMAKE_CURRENT_LIST_FILENAME ${CMAKE_CURRENT_LIST_FILE} NAME_WE)
if(${CMAKE_CURRENT_LIST_FILENAME}_FILE_INCLUDED)
  return()
endif()
set(${CMAKE_CURRENT_LIST_FILENAME}_FILE_INCLUDED 1)

# Set dependency list
set(OpenIGTLink_DEPENDENCIES "")

# Include dependent projects if any
SlicerMacroCheckExternalProjectDependency(OpenIGTLink)
set(proj OpenIGTLink)

# Set CMake OSX variable to pass down the external project
set(CMAKE_OSX_EXTERNAL_PROJECT_ARGS)
if(APPLE)
  list(APPEND CMAKE_OSX_EXTERNAL_PROJECT_ARGS
    -DCMAKE_OSX_ARCHITECTURES=${CMAKE_OSX_ARCHITECTURES}
    -DCMAKE_OSX_SYSROOT=${CMAKE_OSX_ARCHITECTURES}
    -DCMAKE_OSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET})
endif()

#message(STATUS "${__indent}Adding project ${proj}")

ExternalProject_Add(${proj}
  SVN_REPOSITORY "http://svn.na-mic.org/NAMICSandBox/trunk/OpenIGTLink"
  SOURCE_DIR OpenIGTLink
  BINARY_DIR OpenIGTLink-build
  CMAKE_GENERATOR ${gen}
  CMAKE_ARGS
    ${ep_common_flags}
    ${CMAKE_OSX_EXTERNAL_PROJECT_ARGS}
    -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
    -DBUILD_TESTING:BOOL=OFF
    -DBUILD_SHARED_LIBS:BOOL=ON
    -DOpenIGTLink_PROTOCOL_VERSION_2:BOOL=ON
  INSTALL_COMMAND ""
  DEPENDS
    ${OpenIGTLink_DEPENDENCIES}
)

set(OpenIGTLink_DIR ${CMAKE_BINARY_DIR}/OpenIGTLink-build)

