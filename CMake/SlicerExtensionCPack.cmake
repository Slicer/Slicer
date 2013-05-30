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
  message(STATUS "Skipping extension packaging: ${EXTENSION_NAME}")
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
    EXTENSION_WC_ROOT ${${EXTENSION_NAME}_WC_ROOT}
    EXTENSION_WC_URL ${${EXTENSION_NAME}_WC_URL}
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
# Get today's date
#-----------------------------------------------------------------------------
include(SlicerFunctionToday)
TODAY(${EXTENSION_NAME}_BUILDDATE)

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
  # HACK - For a given directory, "install(SCRIPT ...)" rule will be evaluated first,
  #        let's make sure the following install rule is evaluated within its own directory.
  #        Otherwise, the associated script will be executed before any other relevant install rules.
  file(WRITE ${slicer_extension_cpack_bundle_fixup_directory}/CMakeLists.txt
    "install(SCRIPT \"${slicer_extension_cpack_bundle_fixup_directory}/SlicerExtensionCPackBundleFixup.cmake\")")
  add_subdirectory(${slicer_extension_cpack_bundle_fixup_directory} ${slicer_extension_cpack_bundle_fixup_directory}-binary)
endif()

#-----------------------------------------------------------------------------
# Set EXTENSION_UPLOAD_ONLY_COMMAND
set(EXTENSION_UPLOAD_ONLY_COMMAND_ARG_LIST "${EXTENSION_COMMAND_ARG_LIST}
set(RUN_CTEST_CONFIGURE \"FALSE\")
set(RUN_CTEST_BUILD \"FALSE\")
set(RUN_CTEST_TEST \"FALSE\")
set(RUN_CTEST_PACKAGES \"FALSE\")
set(RUN_CTEST_UPLOAD \"TRUE\")
set(EXTENSION_ARCHITECTURE \"${EXTENSION_ARCHITECTURE}\")
set(EXTENSION_BITNESS \"${EXTENSION_BITNESS}\")
set(EXTENSION_OPERATING_SYSTEM \"${EXTENSION_OPERATING_SYSTEM}\")
set(CPACK_PACKAGE_FILE_NAME \"${CPACK_PACKAGE_FILE_NAME}\")")

set(script_args_file ${CMAKE_CURRENT_BINARY_DIR}/${EXTENSION_NAME}-upload-only-command-args.cmake)
file(WRITE ${script_args_file} ${EXTENSION_UPLOAD_ONLY_COMMAND_ARG_LIST})

set(CTEST_MODEL "Experimental")
set(EXTENSION_UPLOAD_ONLY_COMMAND ${CMAKE_CTEST_COMMAND} ${EXTENSION_COMMAND_BUILD_CONF_ARG_LIST} -DCTEST_MODEL:STRING=${CTEST_MODEL} -DSCRIPT_ARGS_FILE:FILEPATH=${script_args_file} -S ${EXTENSION_SCRIPT} -V${CTEST_EXTRA_VERBOSE_ARG})

add_custom_target(Experimental${target_qualifier}UploadOnly
  COMMAND ${EXTENSION_UPLOAD_ONLY_COMMAND}
  COMMENT "Upload extension"
  )

#-----------------------------------------------------------------------------
include(CPack)
