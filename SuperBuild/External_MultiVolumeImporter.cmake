
set(proj MultiVolumeImporter)

# Set dependency list
set(${proj}_DEPENDENCIES "")

# Include dependent projects if any
ExternalProject_Include_Dependencies(${proj} PROJECT_VAR proj DEPENDS_VAR ${proj}_DEPENDENCIES)

if(${CMAKE_PROJECT_NAME}_USE_SYSTEM_${proj})
  message(FATAL_ERROR "Enabling ${CMAKE_PROJECT_NAME}_USE_SYSTEM_${proj} is not supported !")
endif()

# Sanity checks
if(DEFINED MultiVolumeImporter_SOURCE_DIR AND NOT EXISTS ${MultiVolumeImporter_SOURCE_DIR})
  message(FATAL_ERROR "MultiVolumeImporter_SOURCE_DIR variable is defined but corresponds to non-existing directory")
endif()

if(NOT DEFINED MultiVolumeImporter_SOURCE_DIR)

  if(NOT DEFINED git_protocol)
    set(git_protocol "git")
  endif()

  ExternalProject_Add(${proj}
    ${${proj}_EP_ARGS}
    GIT_REPOSITORY "${git_protocol}://github.com/fedorov/MultiVolumeImporter.git"
    GIT_TAG "3ec521645718353226357c2cc8457f5535f1ef38"
    SOURCE_DIR ${CMAKE_BINARY_DIR}/${proj}
    BINARY_DIR ${proj}-build
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    DEPENDS
      ${${proj}_DEPENDENCIES}
    )
  set(MultiVolumeImporter_SOURCE_DIR ${CMAKE_BINARY_DIR}/${proj})

else()
  ExternalProject_Add_Empty(${proj} DEPENDS ${${proj}_DEPENDENCIES})
endif()

mark_as_superbuild(MultiVolumeImporter_SOURCE_DIR:PATH)
