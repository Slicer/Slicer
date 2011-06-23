
# Make sure this file is included only once
get_filename_component(CMAKE_CURRENT_LIST_FILENAME ${CMAKE_CURRENT_LIST_FILE} NAME_WE)
if(${CMAKE_CURRENT_LIST_FILENAME}_FILE_INCLUDED)
  return()
endif()
set(${CMAKE_CURRENT_LIST_FILENAME}_FILE_INCLUDED 1)

# Set dependency list
set(libarchive_DEPENDENCIES "")

if(WIN32)
  set(libarchive_URL http://libarchive.googlecode.com/files/libarchive-2.8.4.zip)
  set(libarchive_MD5 bb416ba2e35693394a546e460797d87e)
else()
  set(libarchive_URL http://libarchive.googlecode.com/files/libarchive-2.7.1.tar.gz)
  set(libarchive_MD5 f43382413b4457d0e192771b100a66e7)
endif()

# Include dependent projects if any
SlicerMacroCheckExternalProjectDependency(libarchive)
set(proj libarchive)

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
  URL ${libarchive_URL}
  URL_MD5 ${libarchive_MD5}
  SOURCE_DIR libarchive
  BINARY_DIR libarchive-build
  INSTALL_DIR libarchive-install
  CMAKE_GENERATOR ${gen}
  CMAKE_ARGS
    ${CMAKE_OSX_EXTERNAL_PROJECT_ARGS}
    -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
    -DCMAKE_CXX_FLAGS:STRING=${ep_common_cxx_flags}
    -DCMAKE_C_FLAGS:STRING=${ep_common_c_flags}
    -DBUILD_SHARED_LIBS:BOOL=ON
    -DENABLE_TAR_SHARED:BOOL=ON
    -DENABLE_ACL:BOOL=OFF
    -DENABLE_TEST:BOOL=OFF
    -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
  DEPENDS
    ${libarchive_DEPENDENCIES}
)

set(LIBARCHIVE_DIR ${CMAKE_BINARY_DIR}/libarchive-install)
set(LIBARCHIVE_INCLUDE_DIR ${LIBARCHIVE_DIR}/include)
set(LIBARCHIVE_LIBRARY)

if(WIN32)
  set(LIBARCHIVE_LIBRARY ${LIBARCHIVE_DIR}/lib/archive.lib)
elseif(APPLE)
  set(LIBARCHIVE_LIBRARY ${CMAKE_BINARY_DIR}/libarchive-build/libarchive/libarchive.dylib)
else()
  set(LIBARCHIVE_LIBRARY ${LIBARCHIVE_DIR}/lib/libarchive.so)
endif()

