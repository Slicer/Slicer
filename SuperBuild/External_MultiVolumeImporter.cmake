
superbuild_include_once()

# Set dependency list
set(MultiVolumeImporter_DEPENDENCIES "")

# Include dependent projects if any
SlicerMacroCheckExternalProjectDependency(MultiVolumeImporter)
set(proj MultiVolumeImporter)

if(${CMAKE_PROJECT_NAME}_USE_SYSTEM_${proj})
  message(FATAL_ERROR "Enabling ${CMAKE_PROJECT_NAME}_USE_SYSTEM_${proj} is not supported !")
endif()

# Sanity checks
if(DEFINED MultiVolumeImporter_SOURCE_DIR AND NOT EXISTS ${MultiVolumeImporter_SOURCE_DIR})
  message(FATAL_ERROR "MultiVolumeImporter_SOURCE_DIR variable is defined but corresponds to non-existing directory")
endif()

if(NOT DEFINED MultiVolumeImporter_SOURCE_DIR)
  #message(STATUS "${__indent}Adding project ${proj}")

  if(NOT DEFINED git_protocol)
    set(git_protocol "git")
  endif()

  ExternalProject_Add(${proj}
    GIT_REPOSITORY "${git_protocol}://github.com/fedorov/MultiVolumeImporter.git"
    GIT_TAG "3ec521645718353226357c2cc8457f5535f1ef38"
    SOURCE_DIR ${CMAKE_BINARY_DIR}/${proj}
    BINARY_DIR ${proj}-build
    CMAKE_GENERATOR ${gen}
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    DEPENDS
      ${MultiVolumeImporter_DEPENDENCIES}
    )
  set(MultiVolumeImporter_SOURCE_DIR ${CMAKE_BINARY_DIR}/${proj})
else()
  # The project is provided using MultiVolumeImporter_DIR, nevertheless since other project may depend on EMSegment,
  # let's add an 'empty' one
  SlicerMacroEmptyExternalProject(${proj} "${MultiVolumeImporter_DEPENDENCIES}")
endif()
