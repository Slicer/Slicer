
# Make sure this file is included only once
get_filename_component(CMAKE_CURRENT_LIST_FILENAME ${CMAKE_CURRENT_LIST_FILE} NAME_WE)
if(${CMAKE_CURRENT_LIST_FILENAME}_FILE_INCLUDED)
  return()
endif()
set(${CMAKE_CURRENT_LIST_FILENAME}_FILE_INCLUDED 1)

# Sanity checks
if(DEFINED BRAINSTools_SOURCE_DIR AND NOT EXISTS ${BRAINSTools_SOURCE_DIR})
  message(FATAL_ERROR "BRAINSTools_SOURCE_DIR variable is defined but corresponds to non-existing directory")
endif()

# Set dependency list
set(BRAINSTools_DEPENDENCIES ${ITK_EXTERNAL_NAME} SlicerExecutionModel VTK DoubleConvert)

# Include dependent projects if any
SlicerMacroCheckExternalProjectDependency(BRAINSTools)
set(proj BRAINSTools)

if(NOT DEFINED BRAINSTools_SOURCE_DIR)
  #message(STATUS "${__indent}Adding project ${proj}")
  set(GIT_TAG "49eda67a4bfc83dfff1ca1b102cc6281efaf6521" CACHE STRING "" FORCE)

  if(NOT DEFINED git_protocol)
    set(git_protocol "git")
  endif()

  ExternalProject_Add(${proj}
    GIT_REPOSITORY "${git_protocol}://github.com/BRAINSia/BRAINSTools.git"
    GIT_TAG "${GIT_TAG}"
    "${${PROJECT_NAME}_EP_UPDATE_IF_GREATER_288}"
    SOURCE_DIR ${proj}
    BINARY_DIR ${proj}-build
    CMAKE_GENERATOR ${gen}
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    DEPENDS
      ${BRAINSTools_DEPENDENCIES}
    )
  set(BRAINSTools_SOURCE_DIR ${CMAKE_BINARY_DIR}/${proj})
else()
  # The project is provided using BRAINSTools_DIR, nevertheless since other project may depend on BRAINSTools,
  # let's add an 'empty' one
  SlicerMacroEmptyExternalProject(${proj} "${BRAINSTools_DEPENDENCIES}")
endif()
