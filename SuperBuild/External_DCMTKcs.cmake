
set(proj DCMTKcs)

# DCMTKcs provides community-supported external modules for DCMTK.
# It is a source-only download: the dcmjp2kcs module is built as part of the
# DCMTK build by copying the source into DCMTK's ExternalModules directory.
# See External_DCMTK.cmake for how this source is used.

# Set dependency list
set(${proj}_DEPENDENCIES "")

# Include dependent projects if any
ExternalProject_Include_Dependencies(${proj} PROJECT_VAR proj DEPENDS_VAR ${proj}_DEPENDENCIES)

# Sanity checks
if(DEFINED DCMTKcs_SOURCE_DIR AND NOT EXISTS ${DCMTKcs_SOURCE_DIR})
  message(FATAL_ERROR "DCMTKcs_SOURCE_DIR variable is defined but corresponds to nonexistent directory")
endif()

if(NOT DEFINED DCMTKcs_SOURCE_DIR AND NOT Slicer_USE_SYSTEM_${proj})

  ExternalProject_SetIfNotDefined(
    Slicer_${proj}_GIT_REPOSITORY
    "${EP_GIT_PROTOCOL}://github.com/lassoan/DCMTKcs.git"
    QUIET
    )

  ExternalProject_SetIfNotDefined(
    Slicer_${proj}_GIT_TAG
    "416bc63e929dd17d629a9952d93295dc239e8e9d" # main 2026-04-01
    QUIET
    )

  set(EP_SOURCE_DIR ${CMAKE_BINARY_DIR}/${proj})

  # Source-only download: DCMTKcs modules are built as part of DCMTK,
  # not as a standalone ExternalProject.
  ExternalProject_Add(${proj}
    ${${proj}_EP_ARGS}
    GIT_REPOSITORY "${Slicer_${proj}_GIT_REPOSITORY}"
    GIT_TAG "${Slicer_${proj}_GIT_TAG}"
    SOURCE_DIR ${EP_SOURCE_DIR}
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    DEPENDS
      ${${proj}_DEPENDENCIES}
    )

  ExternalProject_GenerateProjectDescription_Step(${proj})

  set(DCMTKcs_SOURCE_DIR ${EP_SOURCE_DIR})

else()
  ExternalProject_Add_Empty(${proj} DEPENDS ${${proj}_DEPENDENCIES})
endif()

mark_as_superbuild(
  VARS DCMTKcs_SOURCE_DIR:PATH
  )
