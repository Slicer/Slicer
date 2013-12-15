
set(proj BRAINSTools)

# Set dependency list
set(${proj}_DEPENDENCIES ${ITK_EXTERNAL_NAME} SlicerExecutionModel VTK )

# Include dependent projects if any
ExternalProject_Include_Dependencies(${proj} PROJECT_VAR proj DEPENDS_VAR ${proj}_DEPENDENCIES)

if(${CMAKE_PROJECT_NAME}_USE_SYSTEM_${proj})
  message(FATAL_ERROR "Enabling ${CMAKE_PROJECT_NAME}_USE_SYSTEM_${proj} is not supported !")
endif()

# Sanity checks
if(DEFINED BRAINSTools_SOURCE_DIR AND NOT EXISTS ${BRAINSTools_SOURCE_DIR})
  message(FATAL_ERROR "BRAINSTools_SOURCE_DIR variable is defined but corresponds to non-existing directory")
endif()

if(NOT DEFINED BRAINSTools_SOURCE_DIR)

  set(GIT_TAG "bd755e2d82b7b8b8454fcb579d416e39b16dff80" CACHE STRING "" FORCE)

  if(NOT DEFINED git_protocol)
    set(git_protocol "git")
  endif()

  ExternalProject_Add(${proj}
    ${${proj}_EP_ARGS}
    GIT_REPOSITORY "${git_protocol}://github.com/BRAINSia/BRAINSTools.git"
    GIT_TAG "${GIT_TAG}"
    SOURCE_DIR ${proj}
    BINARY_DIR ${proj}-build
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    DEPENDS
      ${${proj}_DEPENDENCIES}
    )
  set(BRAINSTools_SOURCE_DIR ${CMAKE_BINARY_DIR}/${proj})

else()
  ExternalProject_Add_Empty(${proj} DEPENDS ${${proj}_DEPENDENCIES})
endif()

mark_as_superbuild(BRAINSTools_SOURCE_DIR:PATH)
