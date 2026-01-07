
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

  # When updating the version of tbb, consider also
  # updating the soversion numbers hard-coded below in the
  # "fix_rpath" macOS external project step.
  #
  # To find out if the soversion number should be updated,
  # inspect the installed libraries and/or review the value
  # associated with
  # (1) __TBB_BINARY_VERSION in include/oneapi/tbb/version.h
  #     for libtbb
  # (2) TBBMALLOC_BINARY_VERSION variable in the top-level
  #     CMakeLists.txt for libtbbmalloc

  #------------------------------------------------------------------------------
  set(EP_SOURCE_DIR ${CMAKE_BINARY_DIR}/${proj})
  set(EP_BINARY_DIR ${CMAKE_BINARY_DIR}/${proj}-build)
  set(EP_INSTALL_DIR ${CMAKE_BINARY_DIR}/${proj}-install)

  ExternalProject_SetIfNotDefined(
    Slicer_${proj}_GIT_REPOSITORY
    "${EP_GIT_PROTOCOL}://github.com/uxlfoundation/oneTBB.git"
    QUIET
    )

  ExternalProject_SetIfNotDefined(
    Slicer_${proj}_GIT_TAG
    "v2022.3.0"
    QUIET
    )

  ExternalProject_Add(${proj}
    ${${proj}_EP_ARGS}
    GIT_REPOSITORY "${Slicer_${proj}_GIT_REPOSITORY}"
    GIT_TAG "${Slicer_${proj}_GIT_TAG}"
    SOURCE_DIR ${EP_SOURCE_DIR}
    BINARY_DIR ${EP_BINARY_DIR}
    CMAKE_CACHE_ARGS
      # Compiler settings
      -DCMAKE_CXX_COMPILER:FILEPATH=${CMAKE_CXX_COMPILER}
      -DCMAKE_CXX_FLAGS:STRING=${ep_common_cxx_flags}
      -DCMAKE_C_COMPILER:FILEPATH=${CMAKE_C_COMPILER}
      -DCMAKE_C_FLAGS:STRING=${ep_common_c_flags}
      # Install directories
      -DCMAKE_INSTALL_PREFIX:PATH=${EP_INSTALL_DIR}
      -DCMAKE_INSTALL_LIBDIR:STRING=lib  # Override value set in GNUInstallDirs CMake module
      # Options
      -DBUILD_SHARED_LIBS:BOOL=ON
      -DTBB_TEST:BOOL=OFF
      -DTBB_EXAMPLES:BOOL=OFF
      -DTBB_STRICT:BOOL=OFF
    )

  ExternalProject_GenerateProjectDescription_Step(${proj})

  # Set library and binary directories for the built libraries
  if (WIN32)
    set(tbb_libdir lib)
    set(tbb_bindir bin)
  else()
    set(tbb_libdir lib)
    set(tbb_bindir lib)
  endif()

  set(TBB_DIR ${EP_INSTALL_DIR}/${tbb_libdir}/cmake/TBB)
  set(TBB_BIN_DIR ${EP_INSTALL_DIR}/${tbb_bindir})
  set(TBB_LIB_DIR ${EP_INSTALL_DIR}/${tbb_libdir})

  if(APPLE)
    set(_tbb_suffix "$<$<CONFIG:Debug>:_debug>")
    ExternalProject_Add_Step(${proj} fix_rpath
      COMMAND install_name_tool -id ${TBB_LIB_DIR}/libtbb${_tbb_suffix}.12.dylib ${TBB_LIB_DIR}/libtbb${_tbb_suffix}.12.dylib
      COMMAND install_name_tool -id ${TBB_LIB_DIR}/libtbbmalloc${_tbb_suffix}.2.dylib ${TBB_LIB_DIR}/libtbbmalloc${_tbb_suffix}.2.dylib
      COMMAND install_name_tool -id ${TBB_LIB_DIR}/libtbbmalloc_proxy${_tbb_suffix}.2.dylib -change @rpath/libtbbmalloc${_tbb_suffix}.2.dylib ${TBB_LIB_DIR}/libtbbmalloc${_tbb_suffix}.2.dylib ${TBB_LIB_DIR}/libtbbmalloc_proxy${_tbb_suffix}.2.dylib
      DEPENDEES install
      )
  endif()

  #-----------------------------------------------------------------------------
  # Launcher setting specific to build tree

  set(${proj}_LIBRARY_PATHS_LAUNCHER_BUILD "${TBB_BIN_DIR}")
  mark_as_superbuild(
    VARS ${proj}_LIBRARY_PATHS_LAUNCHER_BUILD
    LABELS "LIBRARY_PATHS_LAUNCHER_BUILD"
    )

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
