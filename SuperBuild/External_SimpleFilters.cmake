
set(proj SimpleFilters)

# Set compile time dependency list
set(${proj}_DEPENDENCIES "")

# Include dependent projects if any
ExternalProject_Include_Dependencies(${proj} PROJECT_VAR proj DEPENDS_VAR ${proj}_DEPENDENCIES)

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
    ${${proj}_EP_ARGS}
    GIT_REPOSITORY "${git_protocol}://github.com/SimpleITK/SlicerSimpleFilters.git"
    GIT_TAG "fc1f06ce52aec9272f928f3d1e0fa59b1a1c8bd6"
    SOURCE_DIR ${CMAKE_BINARY_DIR}/${proj}
    BINARY_DIR ${CMAKE_BINARY_DIR}/${proj}
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    DEPENDS
      ${${proj}_DEPENDENCIES}
    )
  set(SimpleFilters_SOURCE_DIR ${CMAKE_BINARY_DIR}/${proj})

else()
  ExternalProject_Add_Empty(${proj} DEPENDS ${${proj}_DEPENDENCIES})
endif()

mark_as_superbuild(SimpleFilters_SOURCE_DIR:PATH)
