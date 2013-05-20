
# Make sure this file is included only once
get_filename_component(CMAKE_CURRENT_LIST_FILENAME ${CMAKE_CURRENT_LIST_FILE} NAME_WE)
if(${CMAKE_CURRENT_LIST_FILENAME}_FILE_INCLUDED)
  return()
endif()
set(${CMAKE_CURRENT_LIST_FILENAME}_FILE_INCLUDED 1)

# Set dependency list
set(cmcurl_DEPENDENCIES "")

# Include dependent projects if any
SlicerMacroCheckExternalProjectDependency(cmcurl)
set(proj cmcurl)

set(EXTERNAL_PROJECT_OPTIONAL_ARGS)

# Set CMake OSX variable to pass down the external project
if(APPLE)
  list(APPEND EXTERNAL_PROJECT_OPTIONAL_ARGS
    -DCMAKE_OSX_ARCHITECTURES=${CMAKE_OSX_ARCHITECTURES}
    -DCMAKE_OSX_SYSROOT=${CMAKE_OSX_SYSROOT}
    -DCMAKE_OSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET})
endif()

if(NOT CMAKE_CONFIGURATION_TYPES)
  list(APPEND EXTERNAL_PROJECT_OPTIONAL_ARGS
    -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE})
endif()

#message(STATUS "${__indent}Adding project ${proj}")
ExternalProject_Add(${proj}
  SVN_REPOSITORY "http://svn.slicer.org/Slicer3-lib-mirrors/trunk/cmcurl"
  SVN_REVISION -r "185"
  SOURCE_DIR cmcurl
  BINARY_DIR cmcurl-build
  CMAKE_GENERATOR ${gen}
  CMAKE_ARGS
  #Not needed -DCMAKE_CXX_COMPILER:FILEPATH=${CMAKE_CXX_COMPILER}
  #Not needed -DCMAKE_CXX_FLAGS:STRING=${ep_common_cxx_flags}
    -DCMAKE_C_COMPILER:FILEPATH=${CMAKE_C_COMPILER}
    -DCMAKE_C_FLAGS:STRING=${ep_common_c_flags}
    -DSLICERLIBCURL_TESTING:BOOL=OFF
  #Not used -DBUILD_TESTING:BOOL=OFF
    -DBUILD_SHARED_LIBS:BOOL=ON
    ${EXTERNAL_PROJECT_OPTIONAL_ARGS}
  INSTALL_COMMAND ""
  DEPENDS
    ${cmcurl_DEPENDENCIES}
  )

set(SLICERLIBCURL_DIR ${CMAKE_BINARY_DIR}/cmcurl-build)

