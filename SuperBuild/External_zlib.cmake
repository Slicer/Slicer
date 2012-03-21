
# Make sure this file is included only once
get_filename_component(CMAKE_CURRENT_LIST_FILENAME ${CMAKE_CURRENT_LIST_FILE} NAME_WE)
if(${CMAKE_CURRENT_LIST_FILENAME}_FILE_INCLUDED)
  return()
endif()
set(${CMAKE_CURRENT_LIST_FILENAME}_FILE_INCLUDED 1)

# Set dependency list
set(zlib_DEPENDENCIES "")

# Include dependent projects if any
SlicerMacroCheckExternalProjectDependency(zlib)
set(proj zlib)

if(NOT DEFINED zlib_DIR)
  #message(STATUS "${__indent}Adding project ${proj}")

  set(ADDITIONAL_CMAKE_ARGS)
  set(zlib_c_flags ${ep_common_c_flags})
  if(WIN32)
    set(zlib_c_flags "${ep_common_c_flags} /DZLIB_WINAPI")
  endif()

  # Set CMake OSX variable to pass down the external project
  set(CMAKE_OSX_EXTERNAL_PROJECT_ARGS)
  if(APPLE)
    list(APPEND CMAKE_OSX_EXTERNAL_PROJECT_ARGS
      -DCMAKE_OSX_ARCHITECTURES=${CMAKE_OSX_ARCHITECTURES}
      -DCMAKE_OSX_SYSROOT=${CMAKE_OSX_SYSROOT}
      -DCMAKE_OSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET})
  endif()

  if(NOT DEFINED git_protocol)
    set(git_protocol "git")
  endif()

  ExternalProject_Add(${proj}
    GIT_REPOSITORY "${git_protocol}://github.com/commontk/zlib.git"
    GIT_TAG "66a753054b356da85e1838a081aa94287226823e"
    SOURCE_DIR zlib
    BINARY_DIR zlib-build
    UPDATE_COMMAND ""
    INSTALL_DIR zlib-install
    CMAKE_GENERATOR ${gen}
    CMAKE_ARGS
      ${CMAKE_OSX_EXTERNAL_PROJECT_ARGS}
      -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
      # -DCMAKE_CXX_FLAGS:STRING=${ep_common_cxx_flags} # Not used
      -DCMAKE_C_FLAGS:STRING=${zlib_c_flags}
      -DZLIB_MANGLE_PREFIX:STRING=la_zlib_
      ${ADDITIONAL_CMAKE_ARGS}
      -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
    DEPENDS
      ${zlib_DEPENDENCIES}
    )
  set(zlib_DIR ${CMAKE_BINARY_DIR}/zlib-install)
else()
  # The project is provided using zlib_DIR, nevertheless since other project may depend on zlib,
  # let's add an 'empty' one
  SlicerMacroEmptyExternalProject(${proj} "${zlib_DEPENDENCIES}")
endif()

