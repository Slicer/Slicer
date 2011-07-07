
# Make sure this file is included only once
get_filename_component(CMAKE_CURRENT_LIST_FILENAME ${CMAKE_CURRENT_LIST_FILE} NAME_WE)
if(${CMAKE_CURRENT_LIST_FILENAME}_FILE_INCLUDED)
  return()
endif()
set(${CMAKE_CURRENT_LIST_FILENAME}_FILE_INCLUDED 1)

# Set dependency list
set(LibArchive_DEPENDENCIES "")

# Include dependent projects if any
SlicerMacroCheckExternalProjectDependency(LibArchive)
set(proj LibArchive)

# Set CMake OSX variable to pass down the external project
set(CMAKE_OSX_EXTERNAL_PROJECT_ARGS)
if(APPLE)
  list(APPEND CMAKE_OSX_EXTERNAL_PROJECT_ARGS
    -DCMAKE_OSX_ARCHITECTURES=${CMAKE_OSX_ARCHITECTURES}
    -DCMAKE_OSX_SYSROOT=${CMAKE_OSX_SYSROOT}
    -DCMAKE_OSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET})
endif()

if(NOT DEFINED LibArchive_DIR)
  #message(STATUS "${__indent}Adding project ${proj}")
  # Note: On windows, version smaller than 2.8.4 can't be built on 64bits architecture.
  set(LibArchive_SVN_REPOSITORY http://libarchive.googlecode.com/svn/trunk/)
  set(LibArchive_SVN_REVISION -r 3461)
  ExternalProject_Add(${proj}
    SVN_REPOSITORY ${LibArchive_SVN_REPOSITORY}
    SVN_REVISION ${LibArchive_SVN_REVISION}
    SOURCE_DIR LibArchive
    BINARY_DIR LibArchive-build
    INSTALL_DIR LibArchive-install
    CMAKE_GENERATOR ${gen}
    CMAKE_ARGS
      ${CMAKE_OSX_EXTERNAL_PROJECT_ARGS}
      -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
      -DCMAKE_CXX_FLAGS:STRING=${ep_common_cxx_flags}
      -DCMAKE_C_FLAGS:STRING=${ep_common_c_flags}
      -DBUILD_SHARED_LIBS:BOOL=ON
      -DBUILD_TESTING:BOOL=ON
      -DENABLE_ACL:BOOL=OFF
      -DENABLE_CPIO:BOOL=OFF
      -DENABLE_ICONV:BOOL=OFF
      -DENABLE_OPENSSL:BOOL=OFF
      -DENABLE_TAR:BOOL=OFF
      -DENABLE_TEST:BOOL=OFF
      -DENABLE_XATTR:BOOL=OFF
      -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
    DEPENDS
      ${LibArchive_DEPENDENCIES}
    )
  set(LibArchive_DIR ${CMAKE_BINARY_DIR}/LibArchive-install)
else()
  # The project is provided using LibArchive_DIR, nevertheless since other project may depend on LibArchive,
  # let's add an 'empty' one
  SlicerMacroEmptyExternalProject(${proj} "${LibArchive_DEPENDENCIES}")
endif()

