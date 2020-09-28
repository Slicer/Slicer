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

#-----------------------------------------------------------------------------
# Git protocol option
#-----------------------------------------------------------------------------
if(EP_GIT_PROTOCOL STREQUAL "https")
  # Verify that the global git config has been updated with the expected "insteadOf" option.
  # XXX CMake 3.8: Replace this with use of GIT_CONFIG option provided by ExternalProject
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
"option Slicer_USE_GIT_PROTOCOL set to FALSE. "
"See http://na-mic.org/Mantis/view.php?id=2731"
"\nYou could do so by running the command:\n"
"  ${GIT_EXECUTABLE} config --global url.${base}.insteadOf ${insteadof}\n")
    endif()
  endfunction()

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

# Provide a mechanism to disable/enable one or more modules.
mark_as_superbuild(
  Slicer_QTLOADABLEMODULES_DISABLED:STRING
  Slicer_QTLOADABLEMODULES_ENABLED:STRING
  Slicer_QTSCRIPTEDMODULES_DISABLED:STRING
  Slicer_QTSCRIPTEDMODULES_ENABLED:STRING
  Slicer_CLIMODULES_DISABLED:STRING
  Slicer_CLIMODULES_ENABLED:STRING
  )

#------------------------------------------------------------------------------
# Slicer dependency list
#------------------------------------------------------------------------------

set(ITK_EXTERNAL_NAME ITK)

set(VTK_EXTERNAL_NAME VTK)

set(Slicer_DEPENDENCIES
  curl
  CTKAppLauncherLib
  teem
  ${VTK_EXTERNAL_NAME}
  ${ITK_EXTERNAL_NAME}
  CTK
  LibArchive
  RapidJSON
  )

set(CURL_ENABLE_SSL ${Slicer_USE_PYTHONQT_WITH_OPENSSL})

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

if(Slicer_USE_CTKAPPLAUNCHER)
  list(APPEND Slicer_DEPENDENCIES CTKAPPLAUNCHER)
endif()

if(Slicer_USE_PYTHONQT)
  set(PYTHON_ENABLE_SSL ${Slicer_USE_PYTHONQT_WITH_OPENSSL})
  list(APPEND Slicer_DEPENDENCIES python)
  list(APPEND Slicer_DEPENDENCIES
    python-pythonqt-requirements  # This provides the "packaging.version.parse()" function
    )
  if(Slicer_USE_SCIPY)
    list(APPEND Slicer_DEPENDENCIES python-scipy)
  endif()
  if(Slicer_USE_NUMPY)
    list(APPEND Slicer_DEPENDENCIES python-numpy)
  endif()
  if(Slicer_BUILD_DICOM_SUPPORT AND Slicer_USE_PYTHONQT_WITH_OPENSSL)
    list(APPEND Slicer_DEPENDENCIES python-dicom-requirements)
  endif()
  if(Slicer_BUILD_EXTENSIONMANAGER_SUPPORT)
    list(APPEND Slicer_DEPENDENCIES
      python-extension-manager-requirements
      )
    if(Slicer_USE_PYTHONQT_WITH_OPENSSL OR Slicer_USE_SYSTEM_python)
      # python-PyGithub requires SSL support in Python
      list(APPEND Slicer_DEPENDENCIES
        python-extension-manager-ssl-requirements
        )
    else()
      message(STATUS "--------------------------------------------------")
      message(STATUS "Python was built without SSL support; "
                    "github integration will not be available. "
                    "Set Slicer_USE_PYTHONQT_WITH_OPENSSL=ON to enable this feature.")
      message(STATUS "--------------------------------------------------")
    endif()
  endif()
endif()

if(Slicer_USE_TBB)
  list(APPEND Slicer_DEPENDENCIES tbb)
endif()

include(SlicerCheckModuleEnabled)

# JsonCpp is required to build VolumeRendering module
slicer_is_loadable_builtin_module_enabled("VolumeRendering" _build_volume_rendering_module)
if(_build_volume_rendering_module)
  list(APPEND Slicer_DEPENDENCIES JsonCpp)
endif()

#------------------------------------------------------------------------------
include(ExternalProjectAddSource)

macro(list_conditional_append cond list)
  if(${cond})
    list(APPEND ${list} ${ARGN})
  endif()
endmacro()

#
# Support for remote sources:
#
# * Calling Slicer_Remote_Add downloads the corresponding sources and set the variable
#   <proj>_SOURCE_DIR.
#
# * Slicer_Remote_Add ensures <proj>_SOURCE_DIR is passed to inner build using mark_as_superbuild.
#
# * If <proj>_SOURCE_DIR is already defined, no sources are downloaded.
#
# * Specifying OPTION_NAME adds a CMake option allowing to disable the download of the sources.
#   It is enabled by default and passed to the inner build using mark_as_superbuild.
#
# * Specifying labels REMOTE_MODULE or REMOTE_EXTENSION allows the corresponding sources to be
#   be automatically added. See "Bundle remote modules and extensions adding source directories"
#   in top-level CMakeLists.txt.
#
# Corresponding logic is implemented in ExternalProjectAddSource.cmake
#

#------------------------------------------------------------------------------
# Include remote libraries
#------------------------------------------------------------------------------

Slicer_Remote_Add(vtkAddon
  GIT_REPOSITORY "${EP_GIT_PROTOCOL}://github.com/Slicer/vtkAddon"
  GIT_TAG a64d854cd826fb90b2aeb88907e948b3639f517f
  OPTION_NAME Slicer_BUILD_vtkAddon
  )
list_conditional_append(Slicer_BUILD_vtkAddon Slicer_REMOTE_DEPENDENCIES vtkAddon)

set(vtkAddon_CMAKE_DIR ${vtkAddon_SOURCE_DIR}/CMake)
mark_as_superbuild(vtkAddon_CMAKE_DIR:PATH)

set(vtkAddon_USE_UTF8 ON)
mark_as_superbuild(vtkAddon_USE_UTF8:BOOL)

set(vtkAddon_WRAP_PYTHON ${Slicer_USE_PYTHONQT})
mark_as_superbuild(vtkAddon_WRAP_PYTHON:BOOL)

#------------------------------------------------------------------------------
# Include remote modules
#------------------------------------------------------------------------------

if(Slicer_BUILD_WEBENGINE_SUPPORT)
  Slicer_Remote_Add(jqPlot
    URL https://github.com/Slicer/SlicerBinaryDependencies/releases/download/jqplot/jquery.jqplot.1.0.4r1115.tar.gz
    URL_MD5 5c5d73730145c3963f09e1d3ca355580
    LICENSE_FILES "MIT-LICENSE.txt"
    VERSION "1.0.4"
    SOURCE_DIR_VAR jqPlot_DIR
    LABELS FIND_PACKAGE
    )
  list(APPEND Slicer_REMOTE_DEPENDENCIES jqPlot)
endif()

option(Slicer_BUILD_MULTIVOLUME_SUPPORT "Build MultiVolume support." ON)
mark_as_advanced(Slicer_BUILD_MULTIVOLUME_SUPPORT)

Slicer_Remote_Add(MultiVolumeExplorer
  GIT_REPOSITORY ${EP_GIT_PROTOCOL}://github.com/fedorov/MultiVolumeExplorer.git
  GIT_TAG 8be7d7aa3bfb6878a30420ef68c23a9577cc7b1f
  OPTION_NAME Slicer_BUILD_MultiVolumeExplorer
  OPTION_DEPENDS "Slicer_BUILD_QTLOADABLEMODULES;Slicer_BUILD_MULTIVOLUME_SUPPORT;Slicer_USE_PYTHONQT"
  LABELS REMOTE_MODULE
  )
list_conditional_append(Slicer_BUILD_MultiVolumeExplorer Slicer_REMOTE_DEPENDENCIES MultiVolumeExplorer)

Slicer_Remote_Add(MultiVolumeImporter
  GIT_REPOSITORY ${EP_GIT_PROTOCOL}://github.com/fedorov/MultiVolumeImporter.git
  GIT_TAG b701ca17079dd855f9efc4002f7f734cf5fce5d8
  OPTION_NAME Slicer_BUILD_MultiVolumeImporter
  OPTION_DEPENDS "Slicer_BUILD_QTLOADABLEMODULES;Slicer_BUILD_MULTIVOLUME_SUPPORT;Slicer_USE_PYTHONQT"
  LABELS REMOTE_MODULE
  )
list_conditional_append(Slicer_BUILD_MultiVolumeImporter Slicer_REMOTE_DEPENDENCIES MultiVolumeImporter)

Slicer_Remote_Add(SimpleFilters
  GIT_REPOSITORY ${EP_GIT_PROTOCOL}://github.com/SimpleITK/SlicerSimpleFilters.git
  GIT_TAG 8686bf5722c642cca0b765cb3f8e5b24b0a25422
  OPTION_NAME Slicer_BUILD_SimpleFilters
  OPTION_DEPENDS "Slicer_BUILD_QTSCRIPTEDMODULES;Slicer_USE_SimpleITK"
  LABELS REMOTE_MODULE
  )
list_conditional_append(Slicer_BUILD_SimpleFilters Slicer_REMOTE_DEPENDENCIES SimpleFilters)


# BRAINSTools_hidden_options are internal options needed for BRAINSTools that should be hidden
set(BRAINSTools_hidden_options
  BRAINSTools_SUPERBUILD:INTERNAL=OFF
  BRAINSTools_BUILD_DICOM_SUPPORT:INTERNAL=${Slicer_BUILD_DICOM_SUPPORT}
  BRAINSTools_CLI_LIBRARY_OUTPUT_DIRECTORY:INTERNAL=${CMAKE_BINARY_DIR}/${Slicer_BINARY_INNER_SUBDIR}/${Slicer_CLIMODULES_LIB_DIR}
  BRAINSTools_CLI_ARCHIVE_OUTPUT_DIRECTORY:INTERNAL=${CMAKE_BINARY_DIR}/${Slicer_BINARY_INNER_SUBDIR}/${Slicer_CLIMODULES_LIB_DIR}
  BRAINSTools_CLI_RUNTIME_OUTPUT_DIRECTORY:INTERNAL=${CMAKE_BINARY_DIR}/${Slicer_BINARY_INNER_SUBDIR}/${Slicer_CLIMODULES_BIN_DIR}
  BRAINSTools_CLI_INSTALL_LIBRARY_DESTINATION:INTERNAL=${Slicer_INSTALL_CLIMODULES_LIB_DIR}
  BRAINSTools_CLI_INSTALL_ARCHIVE_DESTINATION:INTERNAL=${Slicer_INSTALL_CLIMODULES_LIB_DIR}
  BRAINSTools_CLI_INSTALL_RUNTIME_DESTINATION:INTERNAL=${Slicer_INSTALL_CLIMODULES_BIN_DIR}
  BRAINSTools_ExternalData_DATA_MANAGEMENT_TARGET:INTERNAL=${Slicer_ExternalData_DATA_MANAGEMENT_TARGET}
  BRAINSTools_DISABLE_TESTING:INTERNAL=ON
  # BRAINSTools comes with some extra tool that should not be compiled by default
  USE_DebugImageViewer:INTERNAL=OFF
  BRAINS_DEBUG_IMAGE_WRITE:INTERNAL=OFF
  USE_BRAINSRefacer:INTERNAL=OFF
  USE_AutoWorkup:INTERNAL=OFF
  USE_ReferenceAtlas:INTERNAL=OFF
  USE_ANTS:INTERNAL=OFF
  USE_BRAINSABC:INTERNAL=OFF
  USE_BRAINSTalairach:INTERNAL=OFF
  USE_BRAINSMush:INTERNAL=OFF
  USE_BRAINSInitializedControlPoints:INTERNAL=OFF
  USE_BRAINSMultiModeSegment:INTERNAL=OFF
  USE_ImageCalculator:INTERNAL=OFF
  USE_BRAINSSnapShotWriter:INTERNAL=OFF
  USE_ConvertBetweenFileFormats:INTERNAL=OFF
  USE_BRAINSMultiSTAPLE:INTERNAL=OFF
  USE_BRAINSCreateLabelMapFromProbabilityMaps:INTERNAL=OFF
  USE_BRAINSContinuousClass:INTERNAL=OFF
  USE_BRAINSPosteriorToContinuousClass:INTERNAL=OFF
  USE_GTRACT:INTERNAL=OFF
  USE_BRAINSConstellationDetector:INTERNAL=OFF
  USE_BRAINSLandmarkInitializer:INTERNAL=OFF
  )

# BRAINSTools_slicer_options are options exposed when building Slicer
set(BRAINSTools_slicer_options
  USE_BRAINSFit:BOOL=ON
  USE_BRAINSROIAuto:BOOL=ON
  USE_BRAINSResample:BOOL=ON
  USE_BRAINSTransformConvert:BOOL=ON
  USE_DWIConvert:BOOL=${Slicer_BUILD_DICOM_SUPPORT} ## Need to figure out library linking
)


Slicer_Remote_Add(BRAINSTools
  GIT_REPOSITORY ${EP_GIT_PROTOCOL}://github.com/Slicer/BRAINSTools.git
  GIT_TAG de71a695f605c7f17c52a168b35dce0231b62dd7 # slicer-2020-04-14-v5.2.0
  LICENSE_FILES "http://www.apache.org/licenses/LICENSE-2.0.txt"
  OPTION_NAME Slicer_BUILD_BRAINSTOOLS
  OPTION_DEPENDS "Slicer_BUILD_CLI_SUPPORT;Slicer_BUILD_CLI"
  LABELS REMOTE_MODULE
  VARS ${BRAINSTools_slicer_options} ${BRAINSTools_hidden_options}
  )
list_conditional_append(Slicer_BUILD_BRAINSTOOLS Slicer_REMOTE_DEPENDENCIES BRAINSTools)
if(Slicer_BUILD_BRAINSTOOLS)
  # This is added to SlicerConfig and is useful for extension depending on BRAINSTools
  set(BRAINSCommonLib_DIR "${Slicer_BINARY_DIR}/${Slicer_BINARY_INNER_SUBDIR}/Modules/Remote/BRAINSTools/BRAINSCommonLib")
  mark_as_superbuild(BRAINSCommonLib_DIR:PATH)
endif()

Slicer_Remote_Add(DataStore
  GIT_REPOSITORY "${EP_GIT_PROTOCOL}://github.com/Slicer/Slicer-DataStore"
  GIT_TAG d4d71e8a0c715d204f815cf1f49a965172ab3a39
  OPTION_NAME Slicer_BUILD_DataStore
  OPTION_DEPENDS "Slicer_BUILD_WEBENGINE_SUPPORT"
  LABELS REMOTE_MODULE
  )
list_conditional_append(Slicer_BUILD_DataStore Slicer_REMOTE_DEPENDENCIES DataStore)

Slicer_Remote_Add(CompareVolumes
  GIT_REPOSITORY "${EP_GIT_PROTOCOL}://github.com/pieper/CompareVolumes"
  GIT_TAG 94224926fa0b0900597343e4db991fd028136bd2
  OPTION_NAME Slicer_BUILD_CompareVolumes
  OPTION_DEPENDS "Slicer_USE_PYTHONQT"
  LABELS REMOTE_MODULE
  )
list_conditional_append(Slicer_BUILD_CompareVolumes Slicer_REMOTE_DEPENDENCIES CompareVolumes)

Slicer_Remote_Add(LandmarkRegistration
  GIT_REPOSITORY "${EP_GIT_PROTOCOL}://github.com/pieper/LandmarkRegistration"
  GIT_TAG c1385ba2cd5069a018cd97c19085788006276918
  OPTION_NAME Slicer_BUILD_LandmarkRegistration
  OPTION_DEPENDS "Slicer_BUILD_CompareVolumes;Slicer_USE_PYTHONQT"
  LABELS REMOTE_MODULE
 )
list_conditional_append(Slicer_BUILD_LandmarkRegistration Slicer_REMOTE_DEPENDENCIES LandmarkRegistration)

Slicer_Remote_Add(SurfaceToolbox
  GIT_REPOSITORY "${EP_GIT_PROTOCOL}://github.com/Slicer/SlicerSurfaceToolbox"
  GIT_TAG 28b2567f8c155d4665ae5c684c5d8ad9e7961934
  OPTION_NAME Slicer_BUILD_SurfaceToolbox
  OPTION_DEPENDS "Slicer_USE_PYTHONQT"
  LABELS REMOTE_MODULE
  )
list_conditional_append(Slicer_BUILD_SurfaceToolbox Slicer_REMOTE_DEPENDENCIES SurfaceToolbox)

#------------------------------------------------------------------------------
# Superbuild-type bundled extensions
#------------------------------------------------------------------------------

# The following logic is documented in the "Bundle remote modules and extensions adding source directories."
# section found in the top-level CMakeLists.txt

set(_all_extension_depends )

# Build only inner-build for superbuild-type extensions
set(Slicer_BUNDLED_EXTENSION_NAMES)
foreach(extension_dir ${Slicer_EXTENSION_SOURCE_DIRS})
  get_filename_component(extension_dir ${extension_dir} ABSOLUTE)
  get_filename_component(extension_name ${extension_dir} NAME) # The assumption is that source directories are named after the extension project
  if(EXISTS ${extension_dir}/SuperBuild OR EXISTS ${extension_dir}/Superbuild)
    set(${extension_name}_SUPERBUILD 0)
    mark_as_superbuild(${extension_name}_SUPERBUILD:BOOL)

    if(NOT DEFINED ${extension_name}_EXTERNAL_PROJECT_EXCLUDE_ALL)
      set(${extension_name}_EXTERNAL_PROJECT_EXCLUDE_ALL FALSE)
    endif()
    if(NOT ${extension_name}_EXTERNAL_PROJECT_EXCLUDE_ALL)
      list(APPEND EXTERNAL_PROJECT_ADDITIONAL_DIRS "${extension_dir}/SuperBuild")
      list(APPEND EXTERNAL_PROJECT_ADDITIONAL_DIRS "${extension_dir}/Superbuild")
    endif()
    if(NOT DEFINED ${extension_name}_EXTERNAL_PROJECT_DEPENDENCIES)
      set(${extension_name}_EXTERNAL_PROJECT_DEPENDENCIES )
    endif()

    set(_external_project_cmake_files)

    # SuperBuild
    file(GLOB _external_project_cmake_files1 RELATIVE "${extension_dir}/SuperBuild" "${extension_dir}/SuperBuild/External_*.cmake")
    list(APPEND _external_project_cmake_files ${_external_project_cmake_files1})

    # Superbuild
    file(GLOB _external_project_cmake_files2 RELATIVE "${extension_dir}/Superbuild" "${extension_dir}/Superbuild/External_*.cmake")
    list(APPEND _external_project_cmake_files ${_external_project_cmake_files2})

    list(REMOVE_DUPLICATES _external_project_cmake_files)

    set(_extension_depends)
    set(_msg_extension_depends)
    foreach (_external_project_cmake_file ${_external_project_cmake_files})
      string(REGEX MATCH "External_(.+)\.cmake" _match ${_external_project_cmake_file})
      set(_additional_project_name "${CMAKE_MATCH_1}")
      if(${extension_name}_EXTERNAL_PROJECT_EXCLUDE_ALL)
        set(_include FALSE)
      else()
        set(_include TRUE)
        if(NOT "${${extension_name}_EXTERNAL_PROJECT_DEPENDENCIES}" STREQUAL "")
          list(FIND ${extension_name}_EXTERNAL_PROJECT_DEPENDENCIES ${_additional_project_name} _index)
          if(_index EQUAL -1)
            set(_include FALSE)
          endif()
        endif()
      endif()
      if(_include)
          list(APPEND _extension_depends ${_additional_project_name})
          list(APPEND _msg_extension_depends ${_additional_project_name})
      else()
        list(APPEND _msg_extension_depends "exclude(${_additional_project_name})")
      endif()
    endforeach()

    list(APPEND Slicer_BUNDLED_EXTENSION_NAMES ${extension_name})

    message(STATUS "SuperBuild - ${extension_name} extension => ${_msg_extension_depends}")

    list(APPEND _all_extension_depends ${_extension_depends})
  endif()
endforeach()

if(_all_extension_depends)
  list(REMOVE_DUPLICATES _all_extension_depends)
endif()

list(APPEND Slicer_DEPENDENCIES ${_all_extension_depends})

mark_as_superbuild(Slicer_BUNDLED_EXTENSION_NAMES:STRING)


#------------------------------------------------------------------------------
# Slicer_ADDITIONAL_PROJECTS
#------------------------------------------------------------------------------

#
# List of <proj>_DIR variables that will be passed to the inner build.
# Then, the variables are:
# (1) associated with CPACK_INSTALL_CMAKE_PROJECTS in SlicerCPack
# (2) used to get <proj>_LIBRARY_DIRS and update "libs_path" in SlicerCPackBundleFixup.
#

list(APPEND Slicer_ADDITIONAL_PROJECTS ${Slicer_ADDITIONAL_DEPENDENCIES})
if(Slicer_ADDITIONAL_PROJECTS)
  list(REMOVE_DUPLICATES Slicer_ADDITIONAL_PROJECTS)
  foreach(additional_project ${Slicer_ADDITIONAL_PROJECTS})
    # needed to do find_package within Slicer
    mark_as_superbuild(${additional_project}_DIR:PATH)
  endforeach()
  mark_as_superbuild(Slicer_ADDITIONAL_PROJECTS:STRING)
endif()


#------------------------------------------------------------------------------
# Slicer_ADDITIONAL_DEPENDENCIES, EXTERNAL_PROJECT_ADDITIONAL_DIR, EXTERNAL_PROJECT_ADDITIONAL_DIRS
#------------------------------------------------------------------------------

#
# Setting the variable Slicer_ADDITIONAL_DEPENDENCIES allows to introduce additional
# Slicer external project dependencies.
#
# Additional external project files are looked up in the EXTERNAL_PROJECT_ADDITIONAL_DIR and EXTERNAL_PROJECT_ADDITIONAL_DIRS
#

if(DEFINED Slicer_ADDITIONAL_DEPENDENCIES)
  list(APPEND Slicer_DEPENDENCIES ${Slicer_ADDITIONAL_DEPENDENCIES})
endif()

mark_as_superbuild(Slicer_DEPENDENCIES:STRING)


#------------------------------------------------------------------------------
# Process external projects, aggregate variable marked as superbuild and set <proj>_EP_ARGS variable.
#------------------------------------------------------------------------------

ExternalProject_Include_Dependencies(Slicer DEPENDS_VAR Slicer_DEPENDENCIES)

#------------------------------------------------------------------------------
# Define list of additional options used to configure Slicer
#------------------------------------------------------------------------------

set(EXTERNAL_PROJECT_OPTIONAL_ARGS)
if(WIN32)
  list(APPEND EXTERNAL_PROJECT_OPTIONAL_ARGS -DSlicer_SKIP_ROOT_DIR_MAX_LENGTH_CHECK:BOOL=ON)
endif()

#------------------------------------------------------------------------------
# Customizing SlicerApp metadata
#------------------------------------------------------------------------------

# Configuring Slicer setting these variables allows to overwrite the properties
# associated with the SlicerApp application.

foreach(name IN ITEMS
  DESCRIPTION_SUMMARY
  DESCRIPTION_FILE
  LAUNCHER_SPLASHSCREEN_FILE
  APPLE_ICON_FILE
  WIN_ICON_FILE
  LICENSE_FILE
  )
  if(DEFINED SlicerApp_${name})
    list(APPEND EXTERNAL_PROJECT_OPTIONAL_ARGS
      -DSlicerApp_${name}:STRING=${SlicerApp_${name}}
      )
  endif()
endforeach()

#------------------------------------------------------------------------------
# Slicer_EXTENSION_SOURCE_DIRS
#------------------------------------------------------------------------------

#
# Configuring Slicer using
#
#   cmake -DSlicer_EXTENSION_SOURCE_DIRS:STRING=/path/to/ExtensionA;/path/to/ExtensionB [...] /path/to/source/Slicer
#
# will ensure the source of each extensions are *built* by Slicer. This is done
# as part of the Slicer inner build by adding each directory in the top-level CMakeLists.txt.
#
# Note that using 'Slicer_Remote_Add' specifying the label 'REMOTE_EXTENSION' (see above)
# will checkout the extension sources and append the corresponding source directory to
# the variable Slicer_EXTENSION_SOURCE_DIRS.
#

#------------------------------------------------------------------------------
# Slicer_EXTENSION_INSTALL_DIRS
#------------------------------------------------------------------------------

#
# Configuring Slicer using
#
#   cmake -DSlicer_EXTENSION_INSTALL_DIRS:STRING=/path/to/ExtensionA-install-tree;/path/to/ExtensionB-install-tree [...] /path/to/source/Slicer
#
# will ensure the content of each extensions install directories are *packaged*
# with Slicer.
#
# Corresponding install rules are found in "CMake/SlicerBlockInstallExtensionPackages.cmake"
#

#------------------------------------------------------------------------------
# Configure and build Slicer
#------------------------------------------------------------------------------
set(proj Slicer)

ExternalProject_Add(${proj}
  ${${proj}_EP_ARGS}
  DEPENDS ${Slicer_DEPENDENCIES} ${Slicer_REMOTE_DEPENDENCIES}
  SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}
  BINARY_DIR ${CMAKE_BINARY_DIR}/${Slicer_BINARY_INNER_SUBDIR}
  DOWNLOAD_COMMAND ""
  UPDATE_COMMAND ""
  CMAKE_CACHE_ARGS
    -DCMAKE_CXX_COMPILER:FILEPATH=${CMAKE_CXX_COMPILER}
    -DCMAKE_CXX_FLAGS:STRING=${ep_common_cxx_flags}
    -DCMAKE_C_COMPILER:FILEPATH=${CMAKE_C_COMPILER}
    -DCMAKE_C_FLAGS:STRING=${ep_common_c_flags}
    -DCMAKE_CXX_STANDARD:STRING=${CMAKE_CXX_STANDARD}
    -DCMAKE_CXX_STANDARD_REQUIRED:BOOL=${CMAKE_CXX_STANDARD_REQUIRED}
    -DCMAKE_CXX_EXTENSIONS:BOOL=${CMAKE_CXX_EXTENSIONS}
    -DADDITIONAL_C_FLAGS:STRING=${ADDITIONAL_C_FLAGS}
    -DADDITIONAL_CXX_FLAGS:STRING=${ADDITIONAL_CXX_FLAGS}
    -DSlicer_REQUIRED_C_FLAGS:STRING=${Slicer_REQUIRED_C_FLAGS}
    -DSlicer_REQUIRED_CXX_FLAGS:STRING=${Slicer_REQUIRED_CXX_FLAGS}
    -DSlicer_SUPERBUILD:BOOL=OFF
    -DSlicer_SUPERBUILD_DIR:PATH=${CMAKE_BINARY_DIR}
    -D${Slicer_MAIN_PROJECT}_APPLICATION_NAME:STRING=${${Slicer_MAIN_PROJECT}_APPLICATION_NAME}
    -DSlicer_EXTENSION_SOURCE_DIRS:STRING=${Slicer_EXTENSION_SOURCE_DIRS}
    -DSlicer_EXTENSION_INSTALL_DIRS:STRING=${Slicer_EXTENSION_INSTALL_DIRS}
    -DExternalData_OBJECT_STORES:PATH=${ExternalData_OBJECT_STORES}
    ${EXTERNAL_PROJECT_OPTIONAL_ARGS}
  INSTALL_COMMAND ""
  )

ExternalProject_AlwaysConfigure(${proj})
