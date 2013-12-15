################################################################################
#
#  Program: 3D Slicer
#
#  Copyright (c) Kitware Inc.
#
#  See COPYRIGHT.txt
#  or http://www.slicer.org/copyright/copyright.txt for details.
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.
#
#  This file was originally developed by
#   Dave Partyka and Jean-Christophe Fillion-Robin, Kitware Inc.
#  and was partially funded by NIH grant 3P41RR013218-12S1
#
################################################################################

#-----------------------------------------------------------------------------
# Git protocole option
#-----------------------------------------------------------------------------
option(${CMAKE_PROJECT_NAME}_USE_GIT_PROTOCOL "If behind a firewall turn this off to use http instead." ON)
set(git_protocol "git")
if(NOT ${CMAKE_PROJECT_NAME}_USE_GIT_PROTOCOL)
  set(git_protocol "http")

  # Verify that the global git config has been updated with the expected "insteadOf" option.
  function(_check_for_required_git_config_insteadof base insteadof)
    execute_process(
      COMMAND ${GIT_EXECUTABLE} config --global --get "url.${base}.insteadof"
      OUTPUT_VARIABLE output
      OUTPUT_STRIP_TRAILING_WHITESPACE
      RESULT_VARIABLE error_code
      )
    if(error_code OR NOT "${output}" STREQUAL "${insteadof}")
      message(FATAL_ERROR
"Since the ExternalProject modules doesn't provide a mechanism to customize the clone step by "
"adding 'git config' statement between the 'git checkout' and the 'submodule init', it is required "
"to manually update your global git config to successfully build ${CMAKE_PROJECT_NAME} with "
"option ${CMAKE_PROJECT_NAME}_USE_GIT_PROTOCOL set to FALSE. "
"See http://na-mic.org/Mantis/view.php?id=2731"
"\nYou could do so by running the command:\n"
"  ${GIT_EXECUTABLE} config --global url.\"${base}\".insteadOf \"${insteadof}\"\n")
    endif()
  endfunction()

  if("${ITK_VERSION_MAJOR}" LESS 4)
    _check_for_required_git_config_insteadof("http://itk.org/" "git://itk.org/")
  endif()

endif()

#-----------------------------------------------------------------------------
# Enable and setup External project global properties
#-----------------------------------------------------------------------------

set(ep_common_c_flags "${CMAKE_C_FLAGS_INIT} ${ADDITIONAL_C_FLAGS}")
set(ep_common_cxx_flags "${CMAKE_CXX_FLAGS_INIT} ${ADDITIONAL_CXX_FLAGS}")

#-----------------------------------------------------------------------------
# Define list of additional options used to configure Slicer
#------------------------------------------------------------------------------

if(DEFINED CTEST_CONFIGURATION_TYPE)
  mark_as_superbuild(CTEST_CONFIGURATION_TYPE)
endif()

if(DEFINED CMAKE_CONFIGURATION_TYPES)
  mark_as_superbuild(CMAKE_CONFIGURATION_TYPES)
endif()

# Provide a mechanism to disable one or more modules.
mark_as_superbuild(
  Slicer_QTLOADABLEMODULES_DISABLED:STRING
  Slicer_QTSCRIPTEDMODULES_DISABLED:STRING
  Slicer_CLIMODULES_DISABLED:STRING
  )

#------------------------------------------------------------------------------
# Slicer dependency list
#------------------------------------------------------------------------------

set(ITK_EXTERNAL_NAME ITKv${ITK_VERSION_MAJOR})

set(Slicer_DEPENDENCIES curl teem VTK ${ITK_EXTERNAL_NAME} CTK jqPlot LibArchive)

set(CURL_ENABLE_SSL ${Slicer_USE_PYTHONQT_WITH_OPENSSL})

if(Slicer_USE_OpenIGTLink)
  list(APPEND Slicer_DEPENDENCIES OpenIGTLink)
endif()

if(Slicer_BUILD_OpenIGTLinkIF)
  list(APPEND Slicer_DEPENDENCIES OpenIGTLinkIF)
endif()

if(Slicer_USE_SimpleITK)
  list(APPEND Slicer_DEPENDENCIES SimpleITK)
endif()

if(Slicer_BUILD_CLI_SUPPORT)
  list(APPEND Slicer_DEPENDENCIES SlicerExecutionModel)
endif()

if(Slicer_BUILD_BRAINSTOOLS)
    list(APPEND Slicer_DEPENDENCIES BRAINSTools)
endif()

if(Slicer_BUILD_EMSegment)
  list(APPEND Slicer_DEPENDENCIES EMSegment)
endif()

if(Slicer_BUILD_EXTENSIONMANAGER_SUPPORT)
  list(APPEND Slicer_DEPENDENCIES qRestAPI)
endif()

if(Slicer_BUILD_DICOM_SUPPORT)
  list(APPEND Slicer_DEPENDENCIES DCMTK)
endif()

if(Slicer_USE_BatchMake)
  list(APPEND Slicer_DEPENDENCIES BatchMake)
endif()

if(Slicer_USE_CTKAPPLAUNCHER)
  list(APPEND Slicer_DEPENDENCIES CTKAPPLAUNCHER)
endif()

if(Slicer_USE_PYTHONQT)
  set(PYTHON_ENABLE_SSL ${Slicer_USE_PYTHONQT_WITH_OPENSSL})
  list(APPEND Slicer_DEPENDENCIES python)
endif()

if(Slicer_USE_NUMPY)
  list(APPEND Slicer_DEPENDENCIES NUMPY)
endif()

if(Slicer_USE_PYTHONQT_WITH_TCL AND UNIX)
  list(APPEND Slicer_DEPENDENCIES incrTcl)
endif()

if(Slicer_BUILD_MultiVolumeExplorer)
  list(APPEND Slicer_DEPENDENCIES MultiVolumeExplorer)
endif()

if(Slicer_BUILD_MultiVolumeImporter)
  list(APPEND Slicer_DEPENDENCIES MultiVolumeImporter)
endif()

if(Slicer_BUILD_SimpleFilters)
  list(APPEND Slicer_DEPENDENCIES SimpleFilters)
endif()

if(DEFINED Slicer_ADDITIONAL_DEPENDENCIES)
  list(APPEND Slicer_DEPENDENCIES ${Slicer_ADDITIONAL_DEPENDENCIES})
endif()

mark_as_superbuild(Slicer_DEPENDENCIES:STRING)

#-----------------------------------------------------------------------------
# Define list of additional options used to configure Slicer
#------------------------------------------------------------------------------

# Projects that Slicer needs to download/configure/build/install...
list(APPEND Slicer_ADDITIONAL_PROJECTS ${Slicer_ADDITIONAL_DEPENDENCIES})
if(Slicer_ADDITIONAL_PROJECTS)
  list(REMOVE_DUPLICATES Slicer_ADDITIONAL_PROJECTS)
  foreach(additional_project ${Slicer_ADDITIONAL_PROJECTS})
    # needed to do find_package within Slicer
    mark_as_superbuild(${additional_project}_DIR:PATH)
  endforeach()
  mark_as_superbuild(Slicer_ADDITIONAL_PROJECTS:STRING)
endif()

include(ListToString)

ExternalProject_Include_Dependencies(Slicer DEPENDS_VAR Slicer_DEPENDENCIES)

set(EXTERNAL_PROJECT_OPTIONAL_ARGS)
if(WIN32)
  list(APPEND EXTERNAL_PROJECT_OPTIONAL_ARGS -DSlicer_SKIP_ROOT_DIR_MAX_LENGTH_CHECK:BOOL=ON)
endif()

#------------------------------------------------------------------------------
# Configure and build Slicer
#------------------------------------------------------------------------------
set(proj Slicer)

ExternalProject_Add(${proj}
  ${${proj}_EP_ARGS}
  DEPENDS ${Slicer_DEPENDENCIES}
  SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}
  BINARY_DIR ${Slicer_BINARY_INNER_SUBDIR}
  DOWNLOAD_COMMAND ""
  UPDATE_COMMAND ""
  CMAKE_CACHE_ARGS
    -DCMAKE_CXX_COMPILER:FILEPATH=${CMAKE_CXX_COMPILER}
    -DCMAKE_CXX_FLAGS:STRING=${ep_common_cxx_flags}
    -DCMAKE_C_COMPILER:FILEPATH=${CMAKE_C_COMPILER}
    -DCMAKE_C_FLAGS:STRING=${ep_common_c_flags}
    -DADDITIONAL_C_FLAGS:STRING=${ADDITIONAL_C_FLAGS}
    -DADDITIONAL_CXX_FLAGS:STRING=${ADDITIONAL_CXX_FLAGS}
    -DSlicer_REQUIRED_C_FLAGS:STRING=${Slicer_REQUIRED_C_FLAGS}
    -DSlicer_REQUIRED_CXX_FLAGS:STRING=${Slicer_REQUIRED_CXX_FLAGS}
    -DSlicer_SUPERBUILD:BOOL=OFF
    -DSlicer_SUPERBUILD_DIR:PATH=${Slicer_BINARY_DIR}
    -DSlicer_BUILD_WIN32_CONSOLE:BOOL=${Slicer_BUILD_WIN32_CONSOLE}
    -D${Slicer_MAIN_PROJECT}_APPLICATION_NAME:STRING=${${Slicer_MAIN_PROJECT}_APPLICATION_NAME}
    -D${Slicer_MAIN_PROJECT_APPLICATION_NAME}_VERSION_MAJOR:STRING=${${Slicer_MAIN_PROJECT_APPLICATION_NAME}_VERSION_MAJOR}
    -D${Slicer_MAIN_PROJECT_APPLICATION_NAME}_VERSION_MINOR:STRING=${${Slicer_MAIN_PROJECT_APPLICATION_NAME}_VERSION_MINOR}
    -D${Slicer_MAIN_PROJECT_APPLICATION_NAME}_VERSION_PATCH:STRING=${${Slicer_MAIN_PROJECT_APPLICATION_NAME}_VERSION_PATCH}
    -D${Slicer_MAIN_PROJECT_APPLICATION_NAME}_VERSION_TWEAK:STRING=${${Slicer_MAIN_PROJECT_APPLICATION_NAME}_VERSION_TWEAK}
    -D${Slicer_MAIN_PROJECT_APPLICATION_NAME}_VERSION_RC:STRING=${${Slicer_MAIN_PROJECT_APPLICATION_NAME}_VERSION_RC}
    -DSlicer_APPLICATIONS_DIR:PATH=${Slicer_APPLICATIONS_DIR}
    -DSlicer_EXTENSION_SOURCE_DIRS:STRING=${Slicer_EXTENSION_SOURCE_DIRS}
    ${EXTERNAL_PROJECT_OPTIONAL_ARGS}
  INSTALL_COMMAND ""
  )

# This custom external project step forces the build and later
# steps to run whenever a top level build is done...
ExternalProject_Add_Step(${proj} forcebuild
  COMMAND ${CMAKE_COMMAND} -E echo_append ""
  COMMENT "Forcing build step for '${proj}'"
  DEPENDEES configure
  DEPENDERS build
  ALWAYS 1
  )

#-----------------------------------------------------------------------------
# Slicer extensions
#-----------------------------------------------------------------------------
add_subdirectory(Extensions/CMake)
