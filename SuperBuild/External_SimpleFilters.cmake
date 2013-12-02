
superbuild_include_once()

# Set compile time dependency list
set(SimpleFilters_DEPENDENCIES "")

# Include dependent projects if any
SlicerMacroCheckExternalProjectDependency(SimpleFilters)
set(proj SimpleFilters)

if(${CMAKE_PROJECT_NAME}_USE_SYSTEM_${proj})
  message(FATAL_ERROR "Enabling ${CMAKE_PROJECT_NAME}_USE_SYSTEM_${proj} is not supported !")
endif()

# Sanity checks
if(DEFINED SimpleFilters_SOURCE_DIR AND NOT EXISTS ${SimpleFilters_SOURCE_DIR})
  message(FATAL_ERROR "SimpleFilters_SOURCE_DIR variable is defined but corresponds to non-existing directory")
endif()

if(NOT DEFINED SimpleFilters_SOURCE_DIR)

  if(NOT DEFINED git_protocol)
    set(git_protocol "git")
  endif()

  ExternalProject_Add(${proj}
    GIT_REPOSITORY "${git_protocol}://github.com/SimpleITK/SlicerSimpleFilters.git"
    GIT_TAG "fc1f06ce52aec9272f928f3d1e0fa59b1a1c8bd6"
    SOURCE_DIR ${CMAKE_BINARY_DIR}/${proj}
    BINARY_DIR ${CMAKE_BINARY_DIR}/${proj}
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    DEPENDS
      ${SimpleFilters_DEPENDENCIES}
    )
  set(SimpleFilters_SOURCE_DIR ${CMAKE_BINARY_DIR}/${proj})
else()
  SlicerMacroEmptyExternalProject(${proj} "${SimpleFilters_DEPENDENCIES}")
endif()
