
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

if(NOT DEFINED LibArchive_DIR)
  #message(STATUS "${__indent}Adding project ${proj}")

  #
  # WARNING - Before updating the version of LibArchive, please consider the following:
  #
  #   * LibArchive 2.7.1:
  #         - Patched version where '-Werror' is commented out - Available here [1]
  #         - Compiles on: All unix-like platform + windows 32bits
  #         - Doesn't compile on windows 64bits
  #
  #   * LibArchive 2.8.4
  #         - Patched version where '-Werror' is commented out - Available here [2]
  #         - Doesn't compile on all unix-like platform (See errors listed below)
  #         - Compiles compile on windows 64bits
  #
  #   * LibArchive trunk (r3461)
  #         - Compiles properly on all unix-like platform
  #         - Doesn't compile on windows 64bits
  #
  #   * LibArchive 3.0.3
  #         - No particular issues
  #         - Tested shared library build on MacOSX, Ubuntu 10.04, Windows7 32/64bit
  #
  # [1] http://svn.slicer.org/Slicer3-lib-mirrors/trunk/libarchive-2.7.1-patched.tar.gz
  # [2] http://svn.slicer.org/Slicer3-lib-mirrors/trunk/libarchive-2.8.4-patched.tar.gz
  #
  #
  # Listed below the errors occurring when compiling LibArchive2.8.4-patched.tar.gz
  #
  #   MacOSX:
  #    ../LibArchive/libarchive/archive_entry_copy_stat.c: \
  #    In function ‘archive_entry_copy_stat’:
  #    ../LibArchive/libarchive/archive_entry_copy_stat.c:67: \
  #    error: ‘const struct stat’ has no member named ‘st_birthtimespec’
  #    ../LibArchive/libarchive/archive_entry_copy_stat.c:67: \
  #    error: ‘const struct stat’ has no member named ‘st_birthtimespec’
  #
  #   Ubuntu 9.04 - gcc 4.3.3 :
  #    ../LibArchive/libarchive/archive_write_disk.c: In function ‘set_time’:
  #    ../LibArchive/libarchive/archive_write_disk.c:1859: \
  #     error: ‘AT_FDCWD’ undeclared (first use in this function)
  #    ../LibArchive/libarchive/archive_write_disk.c:1859: \
  #    error: (Each undeclared identifier is reported only once
  #    ../LibArchive/libarchive/archive_write_disk.c:1859: \
  #    error: for each function it appears in.)
  #    ../LibArchive/libarchive/archive_write_disk.c:1859: \
  #    error: ‘AT_SYMLINK_NOFOLLOW’ undeclared (first use in this function)
  #
  set(ADDITIONAL_CMAKE_ARGS)
  if(WIN32)
    set(LibArchive_URL http://cloud.github.com/downloads/libarchive/libarchive/libarchive-3.0.3.tar.gz)
    set(LibArchive_MD5 ca4090f0099432a9ac5a8b6618dc3892)
    # CMake arguments specific to LibArchive >= 2.8.4
    list(APPEND ADDITIONAL_CMAKE_ARGS
      -DBUILD_TESTING:BOOL=OFF
      -DENABLE_OPENSSL:BOOL=OFF
      )
  else()
    set(LibArchive_URL http://svn.slicer.org/Slicer3-lib-mirrors/trunk/libarchive-2.7.1-patched.tar.gz)
    set(LibArchive_MD5 fce7fc069ff7f7ecb2eaccac6bab3d7e)
    if(${CMAKE_VERSION} VERSION_GREATER "2.8.5")
      # Note that CMAKE_DISABLE_FIND_PACKAGE_* is only supported in CMake >= 2.8.6
      list(APPEND ADDITIONAL_CMAKE_ARGS
        -DCMAKE_DISABLE_FIND_PACKAGE_OpenSSL:BOOL=TRUE
        )
      message(STATUS "${proj} - Configured *WITHOUT* OpenSSL support.")
    else()
      message(STATUS "${proj} - Configured *WITH* OpenSSL support.")
    endif()
    list(APPEND ADDITIONAL_CMAKE_ARGS
      -DCRYPTO_LIBRARY:FILEPATH=
      )
  endif()

  # Set CMake OSX variable to pass down the external project
  set(CMAKE_OSX_EXTERNAL_PROJECT_ARGS)
  if(APPLE)
    list(APPEND CMAKE_OSX_EXTERNAL_PROJECT_ARGS
      -DCMAKE_OSX_ARCHITECTURES=${CMAKE_OSX_ARCHITECTURES}
      -DCMAKE_OSX_SYSROOT=${CMAKE_OSX_SYSROOT}
      -DCMAKE_OSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET})
  endif()

  ExternalProject_Add(${proj}
    URL ${LibArchive_URL}
    URL_MD5 ${LibArchive_MD5}
    SOURCE_DIR LibArchive
    BINARY_DIR LibArchive-build
    INSTALL_DIR LibArchive-install
    CMAKE_GENERATOR ${gen}
    CMAKE_ARGS
      ${CMAKE_OSX_EXTERNAL_PROJECT_ARGS}
      -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
      # -DCMAKE_CXX_FLAGS:STRING=${ep_common_cxx_flags} # Not used
      -DCMAKE_C_FLAGS:STRING=${ep_common_c_flags}
      -DBUILD_SHARED_LIBS:BOOL=ON
      -DENABLE_ACL:BOOL=OFF
      -DENABLE_CPIO:BOOL=OFF
      -DENABLE_TAR:BOOL=OFF
      -DENABLE_TEST:BOOL=OFF
      -DENABLE_XATTR:BOOL=OFF
      ${ADDITIONAL_CMAKE_ARGS}
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

