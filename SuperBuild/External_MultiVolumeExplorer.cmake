
superbuild_include_once()

# Set dependency list
set(MultiVolumeExplorer_DEPENDENCIES "")

# Include dependent projects if any
SlicerMacroCheckExternalProjectDependency(MultiVolumeExplorer)
set(proj MultiVolumeExplorer)

if(${CMAKE_PROJECT_NAME}_USE_SYSTEM_${proj})
  message(FATAL_ERROR "Enabling ${CMAKE_PROJECT_NAME}_USE_SYSTEM_${proj} is not supported !")
endif()

# Sanity checks
if(DEFINED MultiVolumeExplorer_SOURCE_DIR AND NOT EXISTS ${MultiVolumeExplorer_SOURCE_DIR})
  message(FATAL_ERROR "MultiVolumeExplorer_SOURCE_DIR variable is defined but corresponds to non-existing directory")
endif()

if(NOT DEFINED MultiVolumeExplorer_SOURCE_DIR)
  #message(STATUS "${__indent}Adding project ${proj}")

  if(NOT DEFINED git_protocol)
    set(git_protocol "git")
  endif()

  ExternalProject_Add(${proj}
    GIT_REPOSITORY "${git_protocol}://github.com/fedorov/MultiVolumeExplorer.git"
    GIT_TAG "8e1ad5f885a32a05c34b1699f945239cdba099b5"
    SOURCE_DIR ${CMAKE_BINARY_DIR}/${proj}
    BINARY_DIR ${proj}-build
    CMAKE_GENERATOR ${gen}
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    DEPENDS
      ${MultiVolumeExplorer_DEPENDENCIES}
    )
  set(MultiVolumeExplorer_SOURCE_DIR ${CMAKE_BINARY_DIR}/${proj})
else()
  # The project is provided using MultiVolumeExplorer_DIR, nevertheless since other project may depend on EMSegment,
  # let's add an 'empty' one
  SlicerMacroEmptyExternalProject(${proj} "${MultiVolumeExplorer_DEPENDENCIES}")
endif()
