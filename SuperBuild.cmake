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
# CMake https support
#-----------------------------------------------------------------------------
include(SlicerCheckCMakeHTTPS)
slicer_check_cmake_https()

#-----------------------------------------------------------------------------
# Git protocol option
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

set(ITK_EXTERNAL_NAME ITKv4)

set(VTK_EXTERNAL_NAME VTKv7)

set(Slicer_DEPENDENCIES curl teem ${VTK_EXTERNAL_NAME} ${ITK_EXTERNAL_NAME} CTK LibArchive RapidJSON)

set(CURL_ENABLE_SSL ${Slicer_USE_PYTHONQT_WITH_OPENSSL})

if(Slicer_USE_OpenIGTLink)
  list(APPEND Slicer_DEPENDENCIES OpenIGTLink)
endif()

if(Slicer_USE_SimpleITK)
  list(APPEND Slicer_DEPENDENCIES SimpleITK)
endif()

if(Slicer_BUILD_CLI_SUPPORT)
  list(APPEND Slicer_DEPENDENCIES SlicerExecutionModel)
endif()

if(Slicer_BUILD_EXTENSIONMANAGER_SUPPORT)
  list(APPEND Slicer_DEPENDENCIES qRestAPI)
endif()

if(Slicer_BUILD_DICOM_SUPPORT)
  list(APPEND Slicer_DEPENDENCIES DCMTK)
endif()

if(Slicer_BUILD_DICOM_SUPPORT AND Slicer_USE_PYTHONQT_WITH_OPENSSL)
  list(APPEND Slicer_DEPENDENCIES python-pydicom)
endif()

if(Slicer_USE_PYTHONQT AND Slicer_BUILD_EXTENSIONMANAGER_SUPPORT)
  list(APPEND Slicer_DEPENDENCIES python-GitPython python-chardet python-couchdb)
  if(Slicer_USE_PYTHONQT_WITH_OPENSSL OR Slicer_USE_SYSTEM_python)
    # python-PyGithub requires SSL support in Python
    list(APPEND Slicer_DEPENDENCIES python-PyGithub)
  else()
    message(STATUS "--------------------------------------------------")
    message(STATUS "Python was built without SSL support; "
                   "github integration will not be available. "
                   "Set Slicer_USE_PYTHONQT_WITH_OPENSSL=ON to enable this feature.")
    message(STATUS "--------------------------------------------------")
  endif()
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

if(DEFINED Slicer_ADDITIONAL_DEPENDENCIES)
  list(APPEND Slicer_DEPENDENCIES ${Slicer_ADDITIONAL_DEPENDENCIES})
endif()

mark_as_superbuild(Slicer_DEPENDENCIES:STRING)

#------------------------------------------------------------------------------
# Include remote modules
#------------------------------------------------------------------------------
include(ExternalProjectAddSource)

macro(list_conditional_append cond list)
  if(${cond})
    list(APPEND ${list} ${ARGN})
  endif()
endmacro()

Slicer_Remote_Add(jqPlot
  URL http://slicer.kitware.com/midas3/download?items=15065&dummy=jquery.jqplot.1.0.4r1120.tar.gz
  URL_MD5 5c5d73730145c3963f09e1d3ca355580
  SOURCE_DIR_VAR jqPlot_DIR
  LABELS FIND_PACKAGE
  )
list(APPEND Slicer_REMOTE_DEPENDENCIES jqPlot)

Slicer_Remote_Add(OpenIGTLinkIF
  GIT_REPOSITORY ${git_protocol}://github.com/openigtlink/OpenIGTLinkIF.git
  GIT_TAG bfe21de98dec2942b5077b7c910a2d1cb3d12008
  OPTION_NAME Slicer_BUILD_OpenIGTLinkIF
  OPTION_DEPENDS "Slicer_BUILD_QTLOADABLEMODULES;Slicer_USE_OpenIGTLink"
  LABELS REMOTE_MODULE
  )
list_conditional_append(Slicer_BUILD_OpenIGTLinkIF Slicer_REMOTE_DEPENDENCIES OpenIGTLinkIF)

option(Slicer_BUILD_MULTIVOLUME_SUPPORT "Build MultiVolume support." ON)
mark_as_advanced(Slicer_BUILD_MULTIVOLUME_SUPPORT)

Slicer_Remote_Add(MultiVolumeExplorer
  GIT_REPOSITORY ${git_protocol}://github.com/fedorov/MultiVolumeExplorer.git
  GIT_TAG 7e4425257c8e8e9ad637ccca99249b1b36b12296
  OPTION_NAME Slicer_BUILD_MultiVolumeExplorer
  OPTION_DEPENDS "Slicer_BUILD_QTLOADABLEMODULES;Slicer_BUILD_MULTIVOLUME_SUPPORT;Slicer_USE_PYTHONQT"
  LABELS REMOTE_MODULE
  )
list_conditional_append(Slicer_BUILD_MultiVolumeExplorer Slicer_REMOTE_DEPENDENCIES MultiVolumeExplorer)

Slicer_Remote_Add(MultiVolumeImporter
  GIT_REPOSITORY ${git_protocol}://github.com/fedorov/MultiVolumeImporter.git
  GIT_TAG 2ebb4d2d8135d7d59314dcc6656e2c2b54266d60
  OPTION_NAME Slicer_BUILD_MultiVolumeImporter
  OPTION_DEPENDS "Slicer_BUILD_QTLOADABLEMODULES;Slicer_BUILD_MULTIVOLUME_SUPPORT;Slicer_USE_PYTHONQT"
  LABELS REMOTE_MODULE
  )
list_conditional_append(Slicer_BUILD_MultiVolumeImporter Slicer_REMOTE_DEPENDENCIES MultiVolumeImporter)

Slicer_Remote_Add(SimpleFilters
  GIT_REPOSITORY ${git_protocol}://github.com/SimpleITK/SlicerSimpleFilters.git
  GIT_TAG 0e0648faeea0b3cbb8c27a93be0d95253ce13b98
  OPTION_NAME Slicer_BUILD_SimpleFilters
  OPTION_DEPENDS "Slicer_BUILD_QTSCRIPTEDMODULES;Slicer_USE_SimpleITK"
  LABELS REMOTE_MODULE
  )
list_conditional_append(Slicer_BUILD_SimpleFilters Slicer_REMOTE_DEPENDENCIES SimpleFilters)

set(BRAINSTools_options
  BRAINSTools_SUPERBUILD:BOOL=OFF
  BRAINSTools_CLI_LIBRARY_OUTPUT_DIRECTORY:PATH=${CMAKE_BINARY_DIR}/${Slicer_BINARY_INNER_SUBDIR}/${Slicer_CLIMODULES_LIB_DIR}
  BRAINSTools_CLI_ARCHIVE_OUTPUT_DIRECTORY:PATH=${CMAKE_BINARY_DIR}/${Slicer_BINARY_INNER_SUBDIR}/${Slicer_CLIMODULES_LIB_DIR}
  BRAINSTools_CLI_RUNTIME_OUTPUT_DIRECTORY:PATH=${CMAKE_BINARY_DIR}/${Slicer_BINARY_INNER_SUBDIR}/${Slicer_CLIMODULES_BIN_DIR}
  BRAINSTools_CLI_INSTALL_LIBRARY_DESTINATION:PATH=${Slicer_INSTALL_CLIMODULES_LIB_DIR}
  BRAINSTools_CLI_INSTALL_ARCHIVE_DESTINATION:PATH=${Slicer_INSTALL_CLIMODULES_LIB_DIR}
  BRAINSTools_CLI_INSTALL_RUNTIME_DESTINATION:PATH=${Slicer_INSTALL_CLIMODULES_BIN_DIR}
  BRAINSTools_DISABLE_TESTING:BOOL=ON
  USE_BRAINSFit:BOOL=ON
  USE_BRAINSROIAuto:BOOL=ON
  USE_BRAINSResample:BOOL=ON
  USE_BRAINSDemonWarp:BOOL=ON
  # BRAINSTools comes with some extra tool that should not be compiled by default
  USE_AutoWorkup:BOOL=OFF
  USE_ReferenceAtlas:BOOL=OFF
  USE_ANTS:BOOL=OFF
  USE_GTRACT:BOOL=OFF
  USE_BRAINSABC:BOOL=OFF
  USE_BRAINSTalairach:BOOL=OFF
  USE_BRAINSConstellationDetector:BOOL=OFF
  USE_BRAINSMush:BOOL=OFF
  USE_BRAINSInitializedControlPoints:BOOL=OFF
  USE_BRAINSMultiModeSegment:BOOL=OFF
  USE_BRAINSCut:BOOL=OFF
  USE_BRAINSLandmarkInitializer:BOOL=OFF
  USE_ImageCalculator:BOOL=OFF
  USE_BRAINSSurfaceTools:BOOL=OFF
  USE_BRAINSSnapShotWriter:BOOL=OFF
  USE_ConvertBetweenFileFormats:BOOL=OFF
  USE_BRAINSMultiSTAPLE:BOOL=OFF
  USE_BRAINSCreateLabelMapFromProbabilityMaps:BOOL=OFF
  USE_BRAINSContinuousClass:BOOL=OFF
  USE_ICCDEF:BOOL=OFF
  USE_BRAINSPosteriorToContinuousClass:BOOL=OFF
  USE_DebugImageViewer:BOOL=OFF
  BRAINS_DEBUG_IMAGE_WRITE:BOOL=OFF
  USE_BRAINSTransformConvert:BOOL=ON
  USE_DWIConvert:BOOL=${Slicer_BUILD_DICOM_SUPPORT} ## Need to figure out library linking
  )
Slicer_Remote_Add(BRAINSTools
  GIT_REPOSITORY "${git_protocol}://github.com/Slicer/BRAINSTools.git"
  GIT_TAG "a5cfa21addf98d86da72f216d93e08913fa396d7" # master (v4.7.1) + Fix for BRAINSia/BRAINSTools#285 + fix to update DCMTK to 3.6.1_2016.10.12
  OPTION_NAME Slicer_BUILD_BRAINSTOOLS
  OPTION_DEPENDS "Slicer_BUILD_CLI_SUPPORT;Slicer_BUILD_CLI"
  LABELS REMOTE_MODULE
  VARS ${BRAINSTools_options}
  )
list_conditional_append(Slicer_BUILD_BRAINSTOOLS Slicer_REMOTE_DEPENDENCIES BRAINSTools)

Slicer_Remote_Add(EMSegment
  SVN_REPOSITORY "http://svn.slicer.org/Slicer3/branches/Slicer4-EMSegment"
  SVN_REVISION -r "17126"
  OPTION_NAME Slicer_BUILD_EMSegment
  OPTION_DEPENDS "Slicer_BUILD_BRAINSTOOLS;Slicer_BUILD_QTLOADABLEMODULES;Slicer_USE_PYTHONQT_WITH_TCL"
  LABELS REMOTE_MODULE
  )
list_conditional_append(Slicer_BUILD_EMSegment Slicer_REMOTE_DEPENDENCIES EMSegment)

Slicer_Remote_Add(OtsuThresholdImageFilter
  GIT_REPOSITORY "${git_protocol}://github.com/Slicer/Slicer-OtsuThresholdImageFilter"
  GIT_TAG "cf39e5064472af31809ec1fa2f93fb97dc9a606e"
  OPTION_NAME Slicer_BUILD_OtsuThresholdImageFilter
  OPTION_DEPENDS "Slicer_BUILD_EMSegment"
  LABELS REMOTE_MODULE
  )
list_conditional_append(Slicer_BUILD_OtsuThresholdImageFilter Slicer_REMOTE_DEPENDENCIES OtsuThresholdImageFilter)

Slicer_Remote_Add(DataStore
  GIT_REPOSITORY "${git_protocol}://github.com/Slicer/Slicer-DataStore"
  GIT_TAG "6c3fb92da5b4a9f8c13781add66a29f9b2bf8ab0"
  OPTION_NAME Slicer_BUILD_DataStore
  LABELS REMOTE_MODULE
  )
list_conditional_append(Slicer_BUILD_DataStore Slicer_REMOTE_DEPENDENCIES DataStore)

Slicer_Remote_Add(CompareVolumes
  GIT_REPOSITORY "${git_protocol}://github.com/pieper/CompareVolumes"
  GIT_TAG "6d46b39048bf556ed54ea6dcae8870c6c13f311b"
  OPTION_NAME Slicer_BUILD_CompareVolumes
  OPTION_DEPENDS "Slicer_USE_PYTHONQT"
  LABELS REMOTE_MODULE
  )
list_conditional_append(Slicer_BUILD_CompareVolumes Slicer_REMOTE_DEPENDENCIES CompareVolumes)

Slicer_Remote_Add(LandmarkRegistration
  GIT_REPOSITORY "${git_protocol}://github.com/pieper/LandmarkRegistration"
  GIT_TAG "b2ecd4b99ad16a228bcaa23dcf17fc9da27b157a"
  OPTION_NAME Slicer_BUILD_LandmarkRegistration
  OPTION_DEPENDS "Slicer_BUILD_CompareVolumes;Slicer_USE_PYTHONQT"
  LABELS REMOTE_MODULE
  )
list_conditional_append(Slicer_BUILD_LandmarkRegistration Slicer_REMOTE_DEPENDENCIES LandmarkRegistration)

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
  DEPENDS ${Slicer_DEPENDENCIES} ${Slicer_REMOTE_DEPENDENCIES}
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
    -D${Slicer_MAIN_PROJECT}_APPLICATION_NAME:STRING=${${Slicer_MAIN_PROJECT}_APPLICATION_NAME}
    -D${Slicer_MAIN_PROJECT_APPLICATION_NAME}_VERSION_MAJOR:STRING=${${Slicer_MAIN_PROJECT_APPLICATION_NAME}_VERSION_MAJOR}
    -D${Slicer_MAIN_PROJECT_APPLICATION_NAME}_VERSION_MINOR:STRING=${${Slicer_MAIN_PROJECT_APPLICATION_NAME}_VERSION_MINOR}
    -D${Slicer_MAIN_PROJECT_APPLICATION_NAME}_VERSION_PATCH:STRING=${${Slicer_MAIN_PROJECT_APPLICATION_NAME}_VERSION_PATCH}
    -D${Slicer_MAIN_PROJECT_APPLICATION_NAME}_VERSION_TWEAK:STRING=${${Slicer_MAIN_PROJECT_APPLICATION_NAME}_VERSION_TWEAK}
    -D${Slicer_MAIN_PROJECT_APPLICATION_NAME}_VERSION_RC:STRING=${${Slicer_MAIN_PROJECT_APPLICATION_NAME}_VERSION_RC}
    -DSlicer_APPLICATIONS_DIR:PATH=${Slicer_APPLICATIONS_DIR}
    -DSlicer_EXTENSION_SOURCE_DIRS:STRING=${Slicer_EXTENSION_SOURCE_DIRS}
    -DExternalData_OBJECT_STORES:PATH=${ExternalData_OBJECT_STORES}
    ${EXTERNAL_PROJECT_OPTIONAL_ARGS}
  INSTALL_COMMAND ""
  )

# This custom external project step forces the build and later
# steps to run whenever a top level build is done...
#
# BUILD_ALWAYS flag is available in CMake 3.1 that allows force build
# of external projects without this workaround. Remove this workaround
# and use the CMake flag instead, when Slicer's required minimum CMake
# version will be at least 3.1.
#
if(CMAKE_CONFIGURATION_TYPES)
  set(BUILD_STAMP_FILE "${CMAKE_CURRENT_BINARY_DIR}/${proj}-prefix/src/${proj}-stamp/${CMAKE_CFG_INTDIR}/${proj}-build")
else()
  set(BUILD_STAMP_FILE "${CMAKE_CURRENT_BINARY_DIR}/${proj}-prefix/src/${proj}-stamp/${proj}-build")
endif()
ExternalProject_Add_Step(${proj} forcebuild
  COMMAND ${CMAKE_COMMAND} -E remove ${BUILD_STAMP_FILE}
  COMMENT "Forcing build step for '${proj}'"
  DEPENDEES build
  ALWAYS 1
  )
