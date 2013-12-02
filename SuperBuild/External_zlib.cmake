
superbuild_include_once()

# Set dependency list
set(zlib_DEPENDENCIES "")

# Include dependent projects if any
SlicerMacroCheckExternalProjectDependency(zlib)
set(proj zlib)

if(${CMAKE_PROJECT_NAME}_USE_SYSTEM_${proj})
  unset(zlib_DIR CACHE)
  find_package(ZLIB REQUIRED)
  set(ZLIB_INCLUDE_DIR ${ZLIB_INCLUDE_DIRS})
  set(ZLIB_LIBRARY ${ZLIB_LIBRARIES})
endif()

# Sanity checks
if(DEFINED zlib_DIR AND NOT EXISTS ${zlib_DIR})
  message(FATAL_ERROR "zlib_DIR variable is defined but corresponds to non-existing directory")
endif()

if(NOT DEFINED zlib_DIR AND NOT ${CMAKE_PROJECT_NAME}_USE_SYSTEM_${proj})
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

  set(EP_SOURCE_DIR ${CMAKE_BINARY_DIR}/zlib)
  set(EP_BINARY_DIR ${CMAKE_BINARY_DIR}/zlib-build)
  set(EP_INSTALL_DIR ${CMAKE_BINARY_DIR}/zlib-install)

  ExternalProject_Add(${proj}
    GIT_REPOSITORY "${git_protocol}://github.com/commontk/zlib.git"
    GIT_TAG "66a753054b356da85e1838a081aa94287226823e"
    SOURCE_DIR ${EP_SOURCE_DIR}
    BINARY_DIR ${EP_BINARY_DIR}
    INSTALL_DIR ${EP_INSTALL_DIR}
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
  set(zlib_DIR ${EP_INSTALL_DIR})
  set(ZLIB_ROOT ${zlib_DIR})
  set(ZLIB_INCLUDE_DIR ${zlib_DIR}/include)
  if(WIN32)
    set(ZLIB_LIBRARY ${zlib_DIR}/lib/zlib.lib)
  else()
    set(ZLIB_LIBRARY ${zlib_DIR}/lib/libzlib.a)
  endif()
else()
  # The project is provided using zlib_DIR, nevertheless since other project may depend on zlib,
  # let's add an 'empty' one
  SlicerMacroEmptyExternalProject(${proj} "${zlib_DEPENDENCIES}")
endif()

message(STATUS "${__${proj}_superbuild_message} - ZLIB_INCLUDE_DIR:${ZLIB_INCLUDE_DIR}")
message(STATUS "${__${proj}_superbuild_message} - ZLIB_LIBRARY:${ZLIB_LIBRARY}")
if(ZLIB_ROOT)
  message(STATUS "${__${proj}_superbuild_message} - ZLIB_ROOT:${ZLIB_ROOT}")
endif()
