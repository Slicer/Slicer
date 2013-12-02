
superbuild_include_once()

# Set dependency list
set(LibArchive_DEPENDENCIES "zlib")
if(WIN32)
  list(APPEND LibArchive_DEPENDENCIES zlib)
endif()

# Include dependent projects if any
SlicerMacroCheckExternalProjectDependency(LibArchive)
set(proj LibArchive)

if(${CMAKE_PROJECT_NAME}_USE_SYSTEM_${proj} AND (WIN32 OR APPLE))
  message(FATAL_ERROR "Enabling ${CMAKE_PROJECT_NAME}_USE_SYSTEM_${proj} is not supported !")
endif()

if(${CMAKE_PROJECT_NAME}_USE_SYSTEM_${proj})
  unset(LibArchive_DIR CACHE)
  find_package(LibArchive REQUIRED MODULE)
endif()

# Sanity checks
if(DEFINED LibArchive_DIR AND NOT EXISTS ${LibArchive_DIR})
  message(FATAL_ERROR "LibArchive_DIR variable is defined but corresponds to non-existing directory")
endif()

if(NOT DEFINED LibArchive_DIR AND NOT ${CMAKE_PROJECT_NAME}_USE_SYSTEM_${proj})
  #message(STATUS "unset(DCMTK_DIR CACHE)${__indent}Adding project ${proj}")
  #
  # NOTE: - a stable, recent release (3.0.4) of LibArchive is now checked out from git
  #         for all platforms.  For notes on cross-platform issues with earlier versions
  #         of LibArchive, see the repository for earlier revisions of this file.

  set(EXTERNAL_PROJECT_OPTIONAL_ARGS)

  # CMake arguments specific to LibArchive >= 2.8.4
  list(APPEND EXTERNAL_PROJECT_OPTIONAL_ARGS
    -DBUILD_TESTING:BOOL=OFF
    -DENABLE_OPENSSL:BOOL=OFF
    )

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
    GIT_REPOSITORY "${git_protocol}://github.com/libarchive/libarchive.git"
    GIT_TAG "v3.0.4"
    SOURCE_DIR ${CMAKE_BINARY_DIR}/${proj}
    BINARY_DIR ${proj}-build
    INSTALL_DIR LibArchive-install
    CMAKE_GENERATOR ${gen}
    CMAKE_ARGS
    # Not used -DCMAKE_CXX_COMPILER:FILEPATH=${CMAKE_CXX_COMPILER}
    # Not used -DCMAKE_CXX_FLAGS:STRING=${ep_common_cxx_flags}
      -DCMAKE_C_COMPILER:FILEPATH=${CMAKE_C_COMPILER}
      -DCMAKE_C_FLAGS:STRING=${ep_common_c_flags}
      -DBUILD_SHARED_LIBS:BOOL=ON
      -DENABLE_ACL:BOOL=OFF
      -DENABLE_CPIO:BOOL=OFF
      -DENABLE_ICONV:BOOL=OFF
      -DENABLE_NETTLE:BOOL=OFF
      -DENABLE_TAR:BOOL=OFF
      -DENABLE_TEST:BOOL=OFF
      -DENABLE_XATTR:BOOL=OFF
      -DCMAKE_DISABLE_FIND_PACKAGE_BZip2:BOOL=ON
      -DCMAKE_DISABLE_FIND_PACKAGE_LibXml2:BOOL=ON
      -DCMAKE_DISABLE_FIND_PACKAGE_EXPAT:BOOL=ON
      -DCMAKE_DISABLE_FIND_PACKAGE_LZMA:BOOL=ON
      -DZLIB_ROOT:PATH=${ZLIB_ROOT}
      -DZLIB_INCLUDE_DIR:PATH=${ZLIB_INCLUDE_DIR}
      -DZLIB_LIBRARY:FILEPATH=${ZLIB_LIBRARY}
      -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
      ${EXTERNAL_PROJECT_OPTIONAL_ARGS}
    DEPENDS
      ${LibArchive_DEPENDENCIES}
    )

  if(APPLE)
    ExternalProject_Add_Step(${proj} fix_rpath
      COMMAND install_name_tool -id ${CMAKE_BINARY_DIR}/${proj}-install/lib/libarchive.12.dylib ${CMAKE_BINARY_DIR}/${proj}-install/lib/libarchive.12.dylib
      DEPENDEES install
      )
  endif()

  set(LibArchive_DIR ${CMAKE_BINARY_DIR}/LibArchive-install)

  set(LibArchive_INCLUDE_DIR ${LibArchive_DIR}/include)
  if(WIN32)
    set(LibArchive_LIBRARY ${LibArchive_DIR}/lib/${CMAKE_SHARED_LIBRARY_PREFIX}archive.lib)
  else()
    set(LibArchive_LIBRARY ${LibArchive_DIR}/lib/${CMAKE_SHARED_LIBRARY_PREFIX}archive${CMAKE_SHARED_LIBRARY_SUFFIX})
  endif()

else()
  # The project is provided using LibArchive_DIR, nevertheless since other project may depend on LibArchive,
  # let's add an 'empty' one
  SlicerMacroEmptyExternalProject(${proj} "${LibArchive_DEPENDENCIES}")
endif()

if(${CMAKE_PROJECT_NAME}_USE_SYSTEM_${proj})
  message(STATUS "${__${proj}_superbuild_message} - LibArchive_INCLUDE_DIR:${LibArchive_INCLUDE_DIR}")
  message(STATUS "${__${proj}_superbuild_message} - LibArchive_LIBRARY:${LibArchive_LIBRARY}")
endif()
