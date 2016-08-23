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
#  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
#  and was partially funded by NIH grant 3P41RR013218-12S1
#
################################################################################

if(NOT DEFINED Slicer_DONT_USE_EXTENSION)
  set(Slicer_DONT_USE_EXTENSION FALSE)
endif()
if(Slicer_DONT_USE_EXTENSION)
  message(STATUS "Skipping extension packaging - Extension support is disabled.")
  return()
endif()

# -------------------------------------------------------------------------
# Sanity checks
# -------------------------------------------------------------------------
set(expected_nonempty_vars EXTENSION_NAME Slicer_WC_REVISION Slicer_OS Slicer_ARCHITECTURE)
foreach(var ${expected_nonempty_vars})
  if("${${var}}" STREQUAL "")
    message(FATAL_ERROR "error: ${var} is either NOT defined or empty.")
  endif()
endforeach()

if(Slicer_SOURCE_DIR)
  message(STATUS "Skipping extension packaging: ${EXTENSION_NAME} - Slicer_SOURCE_DIR is defined.")
  return()
endif()

set(expected_existing_vars EXTENSION_README_FILE EXTENSION_LICENSE_FILE)
foreach(var ${expected_existing_vars})
  if(NOT EXISTS ${${var}})
    message(FATAL_ERROR "error: ${var} points to an inexistent file: ${${var}}")
  endif()
endforeach()

#-----------------------------------------------------------------------------
# Get working copy information
#-----------------------------------------------------------------------------
include(SlicerMacroExtractRepositoryInfo)
SlicerMacroExtractRepositoryInfo(VAR_PREFIX ${EXTENSION_NAME})

#-----------------------------------------------------------------------------
# Generate extension description
#-----------------------------------------------------------------------------
if(NOT "${Slicer_CPACK_SKIP_GENERATE_EXTENSION_DESCRIPTION}")
  include(SlicerFunctionGenerateExtensionDescription)

  set(${EXTENSION_NAME}_WC_READONLY_URL ${${EXTENSION_NAME}_WC_URL})
  set(${EXTENSION_NAME}_WC_READONLY_ROOT ${${EXTENSION_NAME}_WC_ROOT})
  # A git read-only repository url is expected
  if(${${EXTENSION_NAME}_WC_TYPE} STREQUAL "git")
    if(${${EXTENSION_NAME}_WC_READONLY_URL} MATCHES "^git@")
      string(REPLACE ":" "/" ${EXTENSION_NAME}_WC_READONLY_URL ${${EXTENSION_NAME}_WC_READONLY_URL})
      string(REPLACE "git@" "git://" ${EXTENSION_NAME}_WC_READONLY_URL ${${EXTENSION_NAME}_WC_READONLY_URL})
    endif()
    set(${EXTENSION_NAME}_WC_READONLY_ROOT ${${EXTENSION_NAME}_WC_READONLY_URL})
  endif()

  slicerFunctionGenerateExtensionDescription(
    EXTENSION_NAME ${EXTENSION_NAME}
    EXTENSION_CATEGORY ${EXTENSION_CATEGORY}
    EXTENSION_ICONURL ${EXTENSION_ICONURL}
    EXTENSION_STATUS ${EXTENSION_STATUS}
    EXTENSION_HOMEPAGE ${EXTENSION_HOMEPAGE}
    EXTENSION_CONTRIBUTORS ${EXTENSION_CONTRIBUTORS}
    EXTENSION_DESCRIPTION ${EXTENSION_DESCRIPTION}
    EXTENSION_SCREENSHOTURLS ${EXTENSION_SCREENSHOTURLS}
    EXTENSION_DEPENDS ${EXTENSION_DEPENDS}
    EXTENSION_ENABLED ${EXTENSION_ENABLED}
    EXTENSION_BUILD_SUBDIRECTORY ${EXTENSION_BUILD_SUBDIRECTORY}
    EXTENSION_WC_TYPE ${${EXTENSION_NAME}_WC_TYPE}
    EXTENSION_WC_REVISION ${${EXTENSION_NAME}_WC_REVISION}
    EXTENSION_WC_ROOT ${${EXTENSION_NAME}_WC_READONLY_ROOT}
    EXTENSION_WC_URL ${${EXTENSION_NAME}_WC_READONLY_URL}
    DESTINATION_DIR ${CMAKE_BINARY_DIR}
    SLICER_WC_REVISION ${Slicer_WC_REVISION}
    SLICER_WC_ROOT ${Slicer_WC_ROOT}
    )
  set(description_file "${CMAKE_BINARY_DIR}/${EXTENSION_NAME}.s4ext")
  if(NOT EXISTS "${description_file}")
    message(FATAL_ERROR "error: Failed to generate extension description file: ${description_file}")
  endif()

  set(description_install_dir ${Slicer_INSTALL_ROOT}${Slicer_SHARE_DIR})
  if(APPLE)
    set(description_install_dir ${Slicer_INSTALL_ROOT}${Slicer_BUNDLE_EXTENSIONS_LOCATION}${Slicer_SHARE_DIR})
  endif()
  install(FILES ${description_file} DESTINATION ${description_install_dir} COMPONENT RuntimeLibraries)
endif()

#-----------------------------------------------------------------------------
# Associate package name with date of last commit
#-----------------------------------------------------------------------------
string(REGEX REPLACE ".*([0-9][0-9][0-9][0-9]\\-[0-9][0-9]\\-[0-9][0-9]).*" "\\1"
  ${EXTENSION_NAME}_BUILDDATE "${${EXTENSION_NAME}_WC_LAST_CHANGED_DATE}")

# -------------------------------------------------------------------------
# Package properties
# -------------------------------------------------------------------------

set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${EXTENSION_DESCRIPTION}")

set(CPACK_MONOLITHIC_INSTALL ON)

set(CMAKE_PROJECT_NAME ${EXTENSION_NAME})
set(CPACK_PACKAGE_VENDOR "NA-MIC")
set(CPACK_PACKAGE_DESCRIPTION_FILE "${EXTENSION_README_FILE}")
set(CPACK_RESOURCE_FILE_LICENSE "${EXTENSION_LICENSE_FILE}")
set(CPACK_PACKAGE_FILE_NAME "${Slicer_WC_REVISION}-${Slicer_OS}-${Slicer_ARCHITECTURE}-${EXTENSION_NAME}-${${EXTENSION_NAME}_WC_TYPE}${${EXTENSION_NAME}_WC_REVISION}-${${EXTENSION_NAME}_BUILDDATE}")
#set(CPACK_PACKAGE_VERSION_MAJOR "${Slicer_VERSION_MAJOR}")
#set(CPACK_PACKAGE_VERSION_MINOR "${Slicer_VERSION_MINOR}")
#set(CPACK_PACKAGE_VERSION_PATCH "${Slicer_VERSION_PATCH}")

#if(APPLE)
#  set(CPACK_PACKAGE_ICON "${Slicer_SOURCE_DIR}/Resources/Slicer.icns")
#endif()

# Slicer does *NOT* require setting the windows path
set(CPACK_NSIS_MODIFY_PATH OFF)

# -------------------------------------------------------------------------
# Disable source generator enabled by default
# -------------------------------------------------------------------------
set(CPACK_SOURCE_TBZ2 OFF CACHE BOOL "Enable to build TBZ2 source packages")
set(CPACK_SOURCE_TZ   OFF CACHE BOOL "Enable to build TZ source packages")

# -------------------------------------------------------------------------
# Enable generator
# -------------------------------------------------------------------------
set(CPACK_GENERATOR "TGZ")
if(WIN32)
  set(CPACK_GENERATOR "ZIP")
endif()

if(APPLE)
  set(fixup_path @rpath)
  set(slicer_extension_cpack_bundle_fixup_directory ${CMAKE_BINARY_DIR}/SlicerExtensionBundle)
  set(EXTENSION_BINARY_DIR ${EXTENSION_SUPERBUILD_BINARY_DIR}/${EXTENSION_BUILD_SUBDIRECTORY})
  set(EXTENSION_SUPERBUILD_DIR ${EXTENSION_SUPERBUILD_BINARY_DIR})
  get_filename_component(Slicer_SUPERBUILD_DIR ${Slicer_DIR}/.. ABSOLUTE)

  configure_file(
    ${Slicer_EXTENSION_CPACK_BUNDLE_FIXUP}
    "${slicer_extension_cpack_bundle_fixup_directory}/SlicerExtensionCPackBundleFixup.cmake"
    @ONLY)

  set(msg "Checking if extension type is SuperBuild")
  message(STATUS "${msg}")
  if(${EXTENSION_NAME}_SUPERBUILD)
    message(STATUS "${msg} - true")
    set(_is_superbuild_extension 1)
  else()
    message(STATUS "${msg} - false")
    set(_is_superbuild_extension 0)
  endif()

  if(NOT _is_superbuild_extension)

    message(STATUS "Extension fixup mode: adding <cpack_bundle_fixup_directory>")
    # HACK - For a given directory, "install(SCRIPT ...)" rule will be evaluated first,
    #        let's make sure the following install rule is evaluated within its own directory.
    #        Otherwise, the associated script will be executed before any other relevant install rules.
    file(WRITE ${slicer_extension_cpack_bundle_fixup_directory}/CMakeLists.txt
      "install(SCRIPT \"${slicer_extension_cpack_bundle_fixup_directory}/SlicerExtensionCPackBundleFixup.cmake\")")
    add_subdirectory(${slicer_extension_cpack_bundle_fixup_directory} ${slicer_extension_cpack_bundle_fixup_directory}-binary)

  else()

    message(STATUS "Extension fixup mode: updating CPACK_INSTALL_CMAKE_PROJECTS with <cpack_bundle_fixup_directory>")
    # Configure project and append the build directory to the
    # list of project to install. This will ensure the fixup happen last
    # for SuperBuild extensions.

    file(WRITE ${slicer_extension_cpack_bundle_fixup_directory}/CMakeLists.txt
    "cmake_minimum_required(VERSION 3.0)
install(SCRIPT \"${slicer_extension_cpack_bundle_fixup_directory}/SlicerExtensionCPackBundleFixup.cmake\")")
    set(source_dir "${slicer_extension_cpack_bundle_fixup_directory}")
    set(build_dir "${slicer_extension_cpack_bundle_fixup_directory}-binary")
    execute_process(
      COMMAND ${CMAKE_COMMAND} -E make_directory ${build_dir}
      RESULT_VARIABLE result
      )
    if(NOT result EQUAL 0)
      message(FATAL_ERROR "${EXTENSION_NAME}-Fixup: Failed to create build directory:${build_dir}")
    endif()
    execute_process(
      COMMAND ${CMAKE_COMMAND} ${source_dir}
      WORKING_DIRECTORY ${build_dir}
      RESULT_VARIABLE result)
    if(NOT result EQUAL 0)
      message(FATAL_ERROR "${EXTENSION_NAME}-Fixup: Failed to configure project [source_dir:${source_dir}, build_dir:${build_dir}")
    endif()
    set(CPACK_INSTALL_CMAKE_PROJECTS "${CPACK_INSTALL_CMAKE_PROJECTS};${build_dir};${EXTENSION_NAME}-Fixup;ALL;/")
  endif()
endif()

#-----------------------------------------------------------------------------
include(SlicerExtensionPackageAndUploadTarget)

#-----------------------------------------------------------------------------
include(CPack)
