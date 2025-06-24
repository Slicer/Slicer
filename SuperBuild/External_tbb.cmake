
set(proj tbb)

# Set dependency list
set(${proj}_DEPENDENCIES "")

# Include dependent projects if any
ExternalProject_Include_Dependencies(${proj} PROJECT_VAR proj DEPENDS_VAR ${proj}_DEPENDENCIES)

if(Slicer_USE_SYSTEM_${proj})
  # When adding support for finding TBB on the system, make sure to set TBB_BIN_DIR and TBB_LIB_DIR
  message(FATAL_ERROR "Enabling Slicer_USE_SYSTEM_${proj} is not supported!")
endif()

# Sanity checks
if(DEFINED TBB_DIR AND NOT EXISTS ${TBB_DIR})
  message(FATAL_ERROR "TBB_DIR variable is defined but corresponds to nonexistent directory")
endif()
if(DEFINED TBB_BIN_DIR AND NOT EXISTS ${TBB_BIN_DIR})
  message(FATAL_ERROR "TBB_BIN_DIR variable is defined but corresponds to nonexistent directory")
endif()
if(DEFINED TBB_LIB_DIR AND NOT EXISTS ${TBB_LIB_DIR})
  message(FATAL_ERROR "TBB_LIB_DIR variable is defined but corresponds to nonexistent directory")
endif()

if((NOT DEFINED TBB_DIR
    OR NOT DEFINED TBB_BIN_DIR
    OR NOT DEFINED TBB_LIB_DIR
    )
    AND NOT Slicer_USE_SYSTEM_${proj})

  if(NOT DEFINED git_protocol)
      set(git_protocol "https")
  set(tbb_ver "v2022.2.0-rc1")
  endif()

  ExternalProject_SetIfNotDefined(
     Slicer_${proj}_GIT_REPOSITORY
     ${git_protocol}://github.com/oneapi-src/oneTBB.git
     QUIET
  )

  ExternalProject_SetIfNotDefined(
    Slicer_${proj}_GIT_TAG
    v2022.2.0-rc1
    QUIET
    )

  set(EXTERNAL_PROJECT_OPTIONAL_CMAKE_CACHE_ARGS)

  set(EP_SOURCE_DIR ${CMAKE_BINARY_DIR}/${proj})
  set(EP_BINARY_DIR ${CMAKE_BINARY_DIR}/${proj}-${EXTERNAL_PROJECT_BUILD_TYPE}-build)

  ExternalProject_Add(${proj}
    ${${proj}_EP_ARGS}
    GIT_REPOSITORY "${Slicer_${proj}_GIT_REPOSITORY}"
    GIT_TAG "${Slicer_${proj}_GIT_TAG}"
    SOURCE_DIR ${EP_SOURCE_DIR}
    BINARY_DIR ${EP_BINARY_DIR}
    CMAKE_CACHE_ARGS
      ${EXTERNAL_PROJECT_DEFAULTS}
      -DTBB_CXX_OPTIMIZATION_FLAGS:STRING=${BRAINSToools_CXX_OPTIMIZATION_FLAGS}
      -DTBB_C_OPTIMIZATION_FLAGS:STRING=${BRAINSToools_C_OPTIMIZATION_FLAGS}
      -DTBB_DIR:PATH=${TBB_DIR}
      -DTACHYON_VERSION:STRING=tbb
      -DTBB4PY_BUILD:BOOL=OFF
      -DTBB_CPF:BOOL=OFF
      -DTBB_EXAMPLES:BOOL=OFF
      -DTBB_FIND_PACKAGE:BOOL=OFF
      -DTBB_INSTALL_VARS:BOOL=OFF
      -DTBB_NO_APPCONTAINER:BOOL=OFF
      -DTBB_SANITIZE:STRING=""
      -DTBB_STRICT:BOOL=OFF
      -DTBB_TEST:BOOL=OFF
      -DTBB_TEST_SPEC:BOOL=OFF
      -DTBB_VALGRIND_MEMCHECK:BOOL=OFF
      -DTBB_WINDOWS_DRIVER:BOOL=OFF
      #INSTALL_COMMAND ""
    DEPENDS
      ${${proj}_DEPENDENCIES}
    )

  message(STATUS "Building ${proj} against TBB_DIR:${TBB_DIR}:")
  set(TBB_DIR ${CMAKE_INSTALL_PREFIX}/lib/cmake/TBB)
  ExternalProject_GenerateProjectDescription_Step(${proj}
    VERSION ${tbb_ver}
    LICENSE_FILES "https://raw.githubusercontent.com/oneapi-src/oneTBB/v${tbb_ver}/LICENSE.txt"
    )

  #-----------------------------------------------------------------------------
  # Launcher setting specific to build tree

  set(${proj}_LIBRARY_PATHS_LAUNCHER_BUILD "${TBB_BIN_DIR}")
  mark_as_superbuild(
    VARS ${proj}_LIBRARY_PATHS_LAUNCHER_BUILD
    LABELS "LIBRARY_PATHS_LAUNCHER_BUILD"
    )

  set(TBB_DIR ${TBB_INSTALL_DIR}/lib/cmake/tbb)

else()
  # The project is provided using TBB_DIR, nevertheless since other project may depend on TBB,
  # let's add an 'empty' one
  ExternalProject_Add_Empty(${proj} DEPENDS ${${proj}_DEPENDENCIES})
endif()

mark_as_superbuild(
  VARS
    TBB_BIN_DIR:PATH
    TBB_LIB_DIR:PATH
  )

ExternalProject_Message(${proj} "TBB_DIR:${TBB_DIR}")
mark_as_superbuild(
  VARS TBB_DIR:PATH
  LABELS "FIND_PACKAGE"
  )
