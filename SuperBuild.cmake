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

  _check_for_required_git_config_insteadof("http://itk.org/" "git://itk.org/")
endif()

#-----------------------------------------------------------------------------
# Qt - Let's check if a valid version of Qt is available
#-----------------------------------------------------------------------------
include(SlicerBlockFindQtAndCheckVersion)

#-----------------------------------------------------------------------------
# Enable and setup External project global properties
#-----------------------------------------------------------------------------
include(ExternalProject)
include(ListToString)
include(SlicerMacroCheckExternalProjectDependency)

# With CMake 2.8.9 or later, the UPDATE_COMMAND is required for updates to occur.
# For earlier versions, we nullify the update state to prevent updates and
# undesirable rebuild.
set(${PROJECT_NAME}_EP_DISABLED_UPDATE UPDATE_COMMAND "")
if(CMAKE_VERSION VERSION_LESS 2.8.9)
  set(${PROJECT_NAME}_EP_UPDATE_IF_GREATER_288 ${${PROJECT_NAME}_EP_DISABLED_UPDATE})
else()
  set(${PROJECT_NAME}_EP_UPDATE_IF_GREATER_288 LOG_UPDATE 1)
endif()

set(ep_base        "${CMAKE_BINARY_DIR}")
#set(ep_install_dir "${ep_base}/Install")

set(ep_list_separator "^^")

set(ep_common_c_flags "${CMAKE_C_FLAGS_INIT} ${ADDITIONAL_C_FLAGS}")
set(ep_common_cxx_flags "${CMAKE_CXX_FLAGS_INIT} ${ADDITIONAL_CXX_FLAGS}")

# Compute -G arg for configuring external projects with the same CMake generator:
if(CMAKE_EXTRA_GENERATOR)
  set(gen "${CMAKE_EXTRA_GENERATOR} - ${CMAKE_GENERATOR}")
else()
  set(gen "${CMAKE_GENERATOR}")
endif()

#------------------------------------------------------------------------------
# Slicer dependency list
#------------------------------------------------------------------------------

set(ITK_EXTERNAL_NAME ITKv${ITK_VERSION_MAJOR})

set(Slicer_DEPENDENCIES cmcurl teem VTK ${ITK_EXTERNAL_NAME} CTK jqPlot LibArchive)

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
    list(APPEND Slicer_DEPENDENCIES BRAINSTools DoubleConvert)
endif()

if(Slicer_BUILD_EMSegment)
  list(APPEND Slicer_DEPENDENCIES EMSegment)
endif()

if(Slicer_BUILD_EXTENSIONMANAGER_SUPPORT)
  list(APPEND Slicer_DEPENDENCIES qMidasAPI)
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

if(Slicer_BUILD_DWIConvert)
  list(APPEND Slicer_DEPENDENCIES DWIConvert)
endif()

if(DEFINED Slicer_ADDITIONAL_DEPENDENCIES)
  list(APPEND Slicer_DEPENDENCIES ${Slicer_ADDITIONAL_DEPENDENCIES})
endif()

SlicerMacroCheckExternalProjectDependency(Slicer)

#-----------------------------------------------------------------------------
# Dump Slicer external project dependencies
#------------------------------------------------------------------------------
#set(ep_dependency_graph "# External project dependencies")
#foreach(ep ${Slicer_DEPENDENCIES})
#  set(ep_dependency_graph "${ep_dependency_graph}\n${ep}:${${ep}_DEPENDENCIES}")
#endforeach()
#file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/SlicerDependencies.txt "${ep_dependency_graph}\n")
#message(STATUS "Generated ${CMAKE_CURRENT_BINARY_DIR}/SlicerDependencies.txt")

#-----------------------------------------------------------------------------
# Set superbuild boolean args
#-----------------------------------------------------------------------------
set(ep_cmake_boolean_args
  DOCUMENTATION_TARGET_IN_ALL
  BUILD_TESTING
  BUILD_SHARED_LIBS
  WITH_COVERAGE
  WITH_MEMCHECK
  Slicer_BUILD_CLI
  Slicer_BUILD_CLI_SUPPORT
  Slicer_BUILD_DICOM_SUPPORT
  Slicer_BUILD_DIFFUSION_SUPPORT
  Slicer_BUILD_EXTENSIONMANAGER_SUPPORT
  Slicer_BUILD_QTLOADABLEMODULES
  Slicer_BUILD_QTSCRIPTEDMODULES
  Slicer_BUILD_OpenIGTLinkIF
  Slicer_UPDATE_TRANSLATION
  Slicer_USE_PYTHONQT
  Slicer_USE_PYTHONQT_WITH_TCL
  Slicer_USE_CTKAPPLAUNCHER
  Slicer_USE_BatchMake
  Slicer_USE_MIDAS
  Slicer_USE_OpenIGTLink
  Slicer_WITH_LIBRARY_VERSION
  Slicer_USE_NUMPY
  #Slicer_USE_WEAVE
  Slicer_USE_QtTesting
  Slicer_USE_SimpleITK
  Slicer_BUILD_BRAINSTOOLS
  Slicer_BUILD_EMSegment
  Slicer_BUILD_MultiVolumeExplorer
  Slicer_BUILD_MultiVolumeImporter
  Slicer_BUILD_Extensions
  )

set(ep_superbuild_boolean_args)
foreach(ep_cmake_arg ${ep_cmake_boolean_args})
  list(APPEND ep_superbuild_boolean_args -D${ep_cmake_arg}:BOOL=${${ep_cmake_arg}})
endforeach()

# message("CMake args:")
# foreach(arg ${ep_superbuild_boolean_args})
#   message("  ${arg}")
# endforeach()

#-----------------------------------------------------------------------------
# Define list of additional options used to configure Slicer
#------------------------------------------------------------------------------
set(ep_superbuild_extra_args)

if(DEFINED CTEST_CONFIGURATION_TYPE)
  list_to_string(${ep_list_separator} "${CTEST_CONFIGURATION_TYPE}" ep_CTEST_CONFIGURATION_TYPE)
  list(APPEND ep_superbuild_extra_args -DCTEST_CONFIGURATION_TYPE:STRING=${ep_CTEST_CONFIGURATION_TYPE})
endif()

if(DEFINED CMAKE_CONFIGURATION_TYPES)
  list_to_string(${ep_list_separator} "${CMAKE_CONFIGURATION_TYPES}" ep_CMAKE_CONFIGURATION_TYPES)
  list(APPEND ep_superbuild_extra_args -DCMAKE_CONFIGURATION_TYPES:STRING=${ep_CMAKE_CONFIGURATION_TYPES})
endif()

if(WIN32)
  list(APPEND ep_superbuild_extra_args -DSlicer_SKIP_ROOT_DIR_MAX_LENGTH_CHECK:BOOL=ON)
endif()

if(Slicer_USE_PYTHONQT)
  list(APPEND ep_superbuild_extra_args
    -DPYTHON_EXECUTABLE:FILEPATH=${slicer_PYTHON_EXECUTABLE}
    -DPYTHON_INCLUDE_DIR:PATH=${slicer_PYTHON_INCLUDE}
    -DPYTHON_LIBRARY:FILEPATH=${slicer_PYTHON_LIBRARY}
    )
endif()

if(Slicer_USE_PYTHONQT_WITH_TCL)
  list(APPEND ep_superbuild_extra_args
    -DSlicer_TCL_DIR:PATH=${tcl_build}
    -DTCL_TK_VERSION:STRING=${TCL_TK_VERSION}
    -DTCL_TK_VERSION_DOT:STRING=${TCL_TK_VERSION_DOT}
    )
  if(INCR_TCL_VERSION)
    list(APPEND ep_superbuild_extra_args
      -DINCR_TCL_VERSION:STRING=${INCR_TCL_VERSION}
      -DINCR_TCL_VERSION_DOT:STRING=${INCR_TCL_VERSION_DOT}
      )
  endif()
endif()

if(Slicer_BUILD_QTLOADABLEMODULES)
  # Provide a mechanism to disable one or more loadable modules.
  list(APPEND ep_superbuild_extra_args -DSlicer_QTLOADABLEMODULES_DISABLED:STRING=${Slicer_QTLOADABLEMODULES_DISABLED})
endif()

if(Slicer_BUILD_QTSCRIPTEDMODULES)
  # Provide a mechanism to disable one or more scripted modules.
  list(APPEND eq_superbuild_extra_args -DSlicer_QTSCRIPTEDMODULES_DISABLED:STRING=${Slicer_QTSCRIPTEDMODULES_DISABLED})
endif()

if(Slicer_USE_BatchMake)
  list(APPEND ep_superbuild_extra_args -DBatchMake_DIR:PATH=${BatchMake_DIR})
endif()

if(Slicer_USE_OpenIGTLink)
  list(APPEND ep_superbuild_extra_args -DOpenIGTLink_DIR:PATH=${OpenIGTLink_DIR})
endif()

if(Slicer_BUILD_OpenIGTLinkIF)
  list(APPEND ep_superbuild_extra_args -DOpenIGTLinkIF_SOURCE_DIR:PATH=${OpenIGTLinkIF_SOURCE_DIR})
endif()

if(Slicer_USE_CTKAPPLAUNCHER)
  list(APPEND ep_superbuild_extra_args -DCTKAPPLAUNCHER_DIR:PATH=${CTKAPPLAUNCHER_DIR})
endif()

if(Slicer_BUILD_CLI_SUPPORT)  
  # Provide a mechanism to disable one or more CLI modules.
  list(APPEND ep_superbuild_extra_args -DSlicer_CLIMODULES_DISABLED:STRING=${Slicer_CLIMODULES_DISABLED})
  list(APPEND ep_superbuild_extra_args -DSlicerExecutionModel_DIR:PATH=${SlicerExecutionModel_DIR})
endif()

if(Slicer_BUILD_BRAINSTOOLS)
  list(APPEND ep_superbuild_extra_args -DBRAINSTools_SOURCE_DIR:PATH=${BRAINSTools_SOURCE_DIR})
endif()

if(Slicer_BUILD_MultiVolumeExplorer)
  list(APPEND ep_superbuild_extra_args -DMultiVolumeExplorer_SOURCE_DIR:PATH=${MultiVolumeExplorer_SOURCE_DIR})
endif()

if(Slicer_BUILD_MultiVolumeImporter)
  list(APPEND ep_superbuild_extra_args -DMultiVolumeImporter_SOURCE_DIR:PATH=${MultiVolumeImporter_SOURCE_DIR})
endif()

if(Slicer_BUILD_EMSegment)
  list(APPEND ep_superbuild_extra_args -DEMSegment_SOURCE_DIR:PATH=${EMSegment_SOURCE_DIR})
endif()

if(Slicer_BUILD_EXTENSIONMANAGER_SUPPORT)
  list(APPEND ep_superbuild_extra_args
    -DqMidasAPI_DIR:PATH=${qMidasAPI_DIR}
    )
endif()

# Projects that Slicer needs to download/configure/build/install...
list(APPEND Slicer_ADDITIONAL_PROJECTS ${Slicer_ADDITIONAL_DEPENDENCIES})
if (Slicer_ADDITIONAL_PROJECTS)
  list(REMOVE_DUPLICATES Slicer_ADDITIONAL_PROJECTS)
  set(Slicer_ADDITIONAL_PROJECTS_STRING)
  foreach(additional_project ${Slicer_ADDITIONAL_PROJECTS})
    # needed to do find_package within Slicer
    list(APPEND ep_superbuild_extra_args
      -D${additional_project}_DIR:PATH=${${additional_project}_DIR})
    if(Slicer_ADDITIONAL_PROJECTS_STRING)
      set(Slicer_ADDITIONAL_PROJECTS_STRING "${Slicer_ADDITIONAL_PROJECTS_STRING}^^${additional_project}")
    else()
      set(Slicer_ADDITIONAL_PROJECTS_STRING ${additional_project})
    endif()
  endforeach()
  # needed for packaging
  list(APPEND ep_superbuild_extra_args
    -DSlicer_ADDITIONAL_PROJECTS:STRING=${Slicer_ADDITIONAL_PROJECTS_STRING})
endif()

# Set CMake OSX variable to pass down the external project
set(CMAKE_OSX_EXTERNAL_PROJECT_ARGS)
if(APPLE)
  list(APPEND CMAKE_OSX_EXTERNAL_PROJECT_ARGS
    -DCMAKE_OSX_ARCHITECTURES=${CMAKE_OSX_ARCHITECTURES}
    -DCMAKE_OSX_SYSROOT=${CMAKE_OSX_SYSROOT}
    -DCMAKE_OSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET})
endif()

if(Slicer_BUILD_DWIConvert)
  list(APPEND ep_superbuild_extra_args
    -DDWIConvert_SOURCE_DIR:PATH=${DWIConvert_SOURCE_DIR}
    )
endif()

#------------------------------------------------------------------------------
# Configure and build Slicer
#------------------------------------------------------------------------------
set(proj Slicer)

ExternalProject_Add(${proj}
  DEPENDS ${Slicer_DEPENDENCIES}
  DOWNLOAD_COMMAND ""
  SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}
  BINARY_DIR ${Slicer_BINARY_INNER_SUBDIR}
  CMAKE_GENERATOR ${gen}
  UPDATE_COMMAND ""
  LIST_SEPARATOR ${ep_list_separator}
  CMAKE_ARGS
    -DCMAKE_CXX_COMPILER:FILEPATH=${CMAKE_CXX_COMPILER}
    -DCMAKE_CXX_FLAGS:STRING=${ep_common_cxx_flags}
    -DCMAKE_C_COMPILER:FILEPATH=${CMAKE_C_COMPILER}
    -DCMAKE_C_FLAGS:STRING=${ep_common_c_flags}
    ${ep_superbuild_boolean_args}
    ${ep_superbuild_extra_args}
    ${CMAKE_OSX_EXTERNAL_PROJECT_ARGS}
    -DBUILD_SHARED_LIBS:BOOL=${BUILD_SHARED_LIBS}
    -DADDITIONAL_C_FLAGS:STRING=${ADDITIONAL_C_FLAGS}
    -DADDITIONAL_CXX_FLAGS:STRING=${ADDITIONAL_CXX_FLAGS}
    -DSlicer_REQUIRED_C_FLAGS:STRING=${Slicer_REQUIRED_C_FLAGS}
    -DSlicer_REQUIRED_CXX_FLAGS:STRING=${Slicer_REQUIRED_CXX_FLAGS}
    -DWITH_COVERAGE:BOOL=${WITH_COVERAGE}
    -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
    -DSubversion_SVN_EXECUTABLE:FILEPATH=${Subversion_SVN_EXECUTABLE}
    -DGIT_EXECUTABLE:FILEPATH=${GIT_EXECUTABLE}
    -DSlicer_SUPERBUILD:BOOL=OFF
    -DSlicer_SUPERBUILD_DIR:PATH=${Slicer_BINARY_DIR}
    -DSlicer_BUILD_WIN32_CONSOLE:BOOL=${Slicer_BUILD_WIN32_CONSOLE}
    -DSlicer_MAIN_PROJECT:STRING=${Slicer_MAIN_PROJECT}
    -D${Slicer_MAIN_PROJECT}_APPLICATION_NAME:STRING=${${Slicer_MAIN_PROJECT}_APPLICATION_NAME}
    -D${Slicer_MAIN_PROJECT_APPLICATION_NAME}_VERSION_MAJOR:STRING=${${Slicer_MAIN_PROJECT_APPLICATION_NAME}_VERSION_MAJOR}
    -D${Slicer_MAIN_PROJECT_APPLICATION_NAME}_VERSION_MINOR:STRING=${${Slicer_MAIN_PROJECT_APPLICATION_NAME}_VERSION_MINOR}
    -D${Slicer_MAIN_PROJECT_APPLICATION_NAME}_VERSION_PATCH:STRING=${${Slicer_MAIN_PROJECT_APPLICATION_NAME}_VERSION_PATCH}
    -D${Slicer_MAIN_PROJECT_APPLICATION_NAME}_VERSION_TWEAK:STRING=${${Slicer_MAIN_PROJECT_APPLICATION_NAME}_VERSION_TWEAK}
    -D${Slicer_MAIN_PROJECT_APPLICATION_NAME}_VERSION_RC:STRING=${${Slicer_MAIN_PROJECT_APPLICATION_NAME}_VERSION_RC}
    -DSlicer_APPLICATIONS_DIR:PATH=${Slicer_APPLICATIONS_DIR}
    -DSlicer_EXTENSION_SOURCE_DIRS:STRING=${Slicer_EXTENSION_SOURCE_DIRS}
    -DDOCUMENTATION_ARCHIVES_OUTPUT_DIRECTORY:PATH=${DOCUMENTATION_ARCHIVES_OUTPUT_DIRECTORY}
    -DDOXYGEN_EXECUTABLE:FILEPATH=${DOXYGEN_EXECUTABLE}
    # ITK
    -DITK_VERSION_MAJOR:STRING=${ITK_VERSION_MAJOR}
    -DITK_DIR:PATH=${ITK_DIR}
    # Teem
    -DTeem_DIR:PATH=${Teem_DIR}
    # VTK
    -DVTK_DIR:PATH=${VTK_DIR}
    -DVTK_DEBUG_LEAKS:BOOL=${Slicer_USE_VTK_DEBUG_LEAKS}
    # cmcurl
    -DSLICERLIBCURL_DIR:PATH=${SLICERLIBCURL_DIR}
    # Qt
    -DSlicer_REQUIRED_QT_VERSION:STRING=${Slicer_REQUIRED_QT_VERSION}
    -DQT_QMAKE_EXECUTABLE:PATH=${QT_QMAKE_EXECUTABLE}
    # CTK
    -DCTK_DIR:PATH=${CTK_DIR}
    # DCMTK
    -DDCMTK_DIR:PATH=${DCMTK_DIR}
    # jqPlot
    -DjqPlot_DIR:PATH=${jqPlot_DIR}
    # LibArchive
    -DLibArchive_DIR:PATH=${LibArchive_DIR}
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
