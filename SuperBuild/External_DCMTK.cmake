
# Make sure this file is included only once
get_filename_component(CMAKE_CURRENT_LIST_FILENAME ${CMAKE_CURRENT_LIST_FILE} NAME_WE)
if(${CMAKE_CURRENT_LIST_FILENAME}_FILE_INCLUDED)
  return()
endif()
set(${CMAKE_CURRENT_LIST_FILENAME}_FILE_INCLUDED 1)

# Set dependency list
set(DCMTK_DEPENDENCIES "zlib")

# Include dependent projects if any
SlicerMacroCheckExternalProjectDependency(DCMTK)
set(proj DCMTK)

if(NOT DEFINED DCMTK_DIR)
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

  set(CMAKE_PROJECT_INCLUDE_EXTERNAL_PROJECT_ARG)
  if(CTEST_USE_LAUNCHERS)
    set(CMAKE_PROJECT_INCLUDE_EXTERNAL_PROJECT_ARG
      "-DCMAKE_PROJECT_DCMTK_INCLUDE:FILEPATH=${CMAKE_ROOT}/Modules/CTestUseLaunchers.cmake")
  endif()

  if(UNIX)
    list(APPEND EXTERNAL_PROJECT_OPTIONAL_ARGS
      -DDCMTK_FORCE_FPIC_ON_UNIX:BOOL=ON
      -DDCMTK_WITH_WRAP:BOOL=OFF   # CTK does not build on Mac with this option turned ON due to library dependencies missing
      )
  endif()

  set(${proj}_REPOSITORY ${git_protocol}://github.com/commontk/DCMTK.git)
  set(${proj}_GIT_TAG "f461865d1759854db56e4c840991c81c77e45bb9")

  ExternalProject_Add(${proj}
    GIT_REPOSITORY ${DCMTK_REPOSITORY}
    GIT_TAG ${DCMTK_GIT_TAG}
    SOURCE_DIR ${proj}
    BINARY_DIR ${proj}-build
    "${${PROJECT_NAME}_EP_UPDATE_IF_GREATER_288}"
    CMAKE_GENERATOR ${gen}
    CMAKE_ARGS
      -DCMAKE_CXX_COMPILER:FILEPATH=${CMAKE_CXX_COMPILER}
      -DCMAKE_CXX_FLAGS:STRING=${ep_common_cxx_flags}
      -DCMAKE_C_COMPILER:FILEPATH=${CMAKE_C_COMPILER}
      -DCMAKE_C_FLAGS:STRING=${ep_common_c_flags}
      ${CMAKE_PROJECT_INCLUDE_EXTERNAL_PROJECT_ARG}
      -DBUILD_SHARED_LIBS:BOOL=ON
      -DDCMTK_WITH_DOXYGEN:BOOL=OFF
      -DDCMTK_WITH_ZLIB:BOOL=OFF # see CTK github issue #25
      -DDCMTK_WITH_OPENSSL:BOOL=OFF # see CTK github issue #25
      -DDCMTK_WITH_PNG:BOOL=OFF # see CTK github issue #25
      -DDCMTK_WITH_TIFF:BOOL=OFF  # see CTK github issue #25
      -DDCMTK_WITH_XML:BOOL=OFF  # see CTK github issue #25
      -DDCMTK_WITH_ICONV:BOOL=OFF  # see CTK github issue #178
      -DDCMTK_OVERWRITE_WIN32_COMPILER_FLAGS:BOOL=OFF
      ${EXTERNAL_PROJECT_OPTIONAL_ARGS}
    INSTALL_COMMAND ""
    DEPENDS
      ${DCMTK_DEPENDENCIES}
  )
  set(DCMTK_DIR ${CMAKE_BINARY_DIR}/${proj}-build)
else()
  # The project is provided with DCMTK_DIR, nevertheless since other project may depend on DCMTK_DIR,
  # let's add an 'empty' one
  SlicerMacroEmptyExternalProject(${proj} "${DCMTK_DEPENDENCIES}")
endif()
