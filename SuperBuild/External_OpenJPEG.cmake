
set(proj OpenJPEG)

# Set dependency list
set(${proj}_DEPENDENCIES "")

# Include dependent projects if any
ExternalProject_Include_Dependencies(${proj} PROJECT_VAR proj DEPENDS_VAR ${proj}_DEPENDENCIES)

if(Slicer_USE_SYSTEM_${proj})
  unset(${proj}_DIR CACHE)
  find_package(${proj} REQUIRED)
endif()

# Sanity checks
if(DEFINED ${proj}_DIR AND NOT EXISTS ${${proj}_DIR})
  message(FATAL_ERROR "${proj}_DIR variable is defined but corresponds to nonexistent directory")
endif()
if(DEFINED ${proj}_INSTALL_DIR AND NOT EXISTS ${${proj}_INSTALL_DIR})
  message(FATAL_ERROR "${proj}_INSTALL_DIR variable is defined but corresponds to nonexistent directory")
endif()

if(NOT DEFINED ${proj}_DIR AND NOT Slicer_USE_SYSTEM_${proj})

  ExternalProject_SetIfNotDefined(
    Slicer_${proj}_GIT_REPOSITORY
    "${EP_GIT_PROTOCOL}://github.com/uclouvain/openjpeg.git"
    QUIET
    )

  ExternalProject_SetIfNotDefined(
    Slicer_${proj}_GIT_TAG
    "v2.5.4"
    QUIET
    )

  set(EP_SOURCE_DIR ${CMAKE_BINARY_DIR}/${proj})
  set(EP_BINARY_DIR ${CMAKE_BINARY_DIR}/${proj}-build)
  set(EP_INSTALL_DIR ${CMAKE_BINARY_DIR}/${proj}-install)

  ExternalProject_Add(${proj}
    ${${proj}_EP_ARGS}
    GIT_REPOSITORY "${Slicer_${proj}_GIT_REPOSITORY}"
    GIT_TAG "${Slicer_${proj}_GIT_TAG}"
    SOURCE_DIR ${EP_SOURCE_DIR}
    BINARY_DIR ${EP_BINARY_DIR}
    INSTALL_DIR ${EP_INSTALL_DIR}
    CMAKE_CACHE_ARGS
      # Compiler settings
      -DCMAKE_CXX_COMPILER:FILEPATH=${CMAKE_CXX_COMPILER}
      -DCMAKE_CXX_FLAGS:STRING=${ep_common_cxx_flags}
      -DCMAKE_C_COMPILER:FILEPATH=${CMAKE_C_COMPILER}
      -DCMAKE_C_FLAGS:STRING=${ep_common_c_flags}
      -DCMAKE_POSITION_INDEPENDENT_CODE:BOOL=ON
      # Options
      -DBUILD_SHARED_LIBS:BOOL=ON
      -DBUILD_STATIC_LIBS:BOOL=OFF  # Remove static OpenJPEG library target to avoid conflict with ITK GDCM OpenJPEG
      -DBUILD_TESTING:BOOL=OFF
      -DBUILD_CODEC:BOOL=OFF
      # Install directories
      -DCMAKE_INSTALL_LIBDIR:STRING=lib  # Override value set in GNUInstallDirs CMake module
      -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
    DEPENDS
      ${${proj}_DEPENDENCIES}
    )

  ExternalProject_GenerateProjectDescription_Step(${proj})

  # OpenJPEG installs its cmake config to lib/cmake/openjpeg-<MAJOR>.<MINOR>
  string(REGEX REPLACE "^v([0-9]+\\.[0-9]+).*$" "\\1" _openjpeg_version "${Slicer_${proj}_GIT_TAG}")
  set(OpenJPEG_DIR ${EP_INSTALL_DIR}/lib/cmake/openjpeg-${_openjpeg_version})
  set(${proj}_INSTALL_DIR ${EP_INSTALL_DIR})

  set(_lib_subdir lib)
  if(WIN32)
    set(_lib_subdir bin)
  endif()

  #-----------------------------------------------------------------------------
  # Launcher setting specific to build tree

  set(${proj}_LIBRARY_PATHS_LAUNCHER_BUILD ${EP_INSTALL_DIR}/${_lib_subdir})
  mark_as_superbuild(
    VARS ${proj}_LIBRARY_PATHS_LAUNCHER_BUILD
    LABELS "LIBRARY_PATHS_LAUNCHER_BUILD"
    )

else()
  # The project is provided using OpenJPEG_DIR, nevertheless since other projects may depend on OpenJPEG,
  # let's add an 'empty' one
  ExternalProject_Add_Empty(${proj} DEPENDS ${${proj}_DEPENDENCIES})
endif()

mark_as_superbuild(
  VARS ${proj}_INSTALL_DIR:PATH
  )

ExternalProject_Message(${proj} "OpenJPEG_DIR:${OpenJPEG_DIR}")
mark_as_superbuild(
  VARS ${proj}_DIR:PATH
  LABELS "FIND_PACKAGE"
  )
