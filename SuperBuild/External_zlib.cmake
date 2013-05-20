
# Make sure this file is included only once
get_filename_component(CMAKE_CURRENT_LIST_FILENAME ${CMAKE_CURRENT_LIST_FILE} NAME_WE)
if(${CMAKE_CURRENT_LIST_FILENAME}_FILE_INCLUDED)
  return()
endif()
set(${CMAKE_CURRENT_LIST_FILENAME}_FILE_INCLUDED 1)

# Sanity checks
if(DEFINED zlib_DIR AND NOT EXISTS ${zlib_DIR})
  message(FATAL_ERROR "zlib_DIR variable is defined but corresponds to non-existing directory")
endif()

# Set dependency list
set(zlib_DEPENDENCIES "")

# Include dependent projects if any
SlicerMacroCheckExternalProjectDependency(zlib)
set(proj zlib)

if(NOT DEFINED zlib_DIR)
  #message(STATUS "${__indent}Adding project ${proj}")

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

  if(NOT DEFINED git_protocol)
    set(git_protocol "git")
  endif()

  ExternalProject_Add(${proj}
    GIT_REPOSITORY "${git_protocol}://github.com/commontk/zlib.git"
    GIT_TAG "66a753054b356da85e1838a081aa94287226823e"
    SOURCE_DIR zlib
    BINARY_DIR zlib-build
    INSTALL_DIR zlib-install
    CMAKE_GENERATOR ${gen}
    CMAKE_ARGS
      ## CXX should not be needed, but it a cmake default test
      -DCMAKE_CXX_COMPILER:FILEPATH=${CMAKE_CXX_COMPILER}
      -DCMAKE_C_COMPILER:FILEPATH=${CMAKE_C_COMPILER}
      -DCMAKE_C_FLAGS:STRING=${ep_common_c_flags}
      -DZLIB_MANGLE_PREFIX:STRING=slicer_zlib_
      -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
      ${EXTERNAL_PROJECT_OPTIONAL_ARGS}
    DEPENDS
      ${zlib_DEPENDENCIES}
    )
  set(zlib_DIR ${CMAKE_BINARY_DIR}/zlib-install)
  set(SLICER_ZLIB_ROOT ${zlib_DIR})
  set(SLICER_ZLIB_INCLUDE_DIR ${zlib_DIR}/include )
  if(WIN32)
    set(SLICER_ZLIB_LIBRARY     ${zlib_DIR}/lib/zlib.lib )
  else()
    set(SLICER_ZLIB_LIBRARY     ${zlib_DIR}/lib/libzlib.a )
  endif()
else()
  # The project is provided using zlib_DIR, nevertheless since other project may depend on zlib,
  # let's add an 'empty' one
  SlicerMacroEmptyExternalProject(${proj} "${zlib_DEPENDENCIES}")
endif()
