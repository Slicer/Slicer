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

include(CMakeParseArguments)

if(NOT DEFINED Slicer_CMAKE_DIR)
  set(Slicer_CMAKE_DIR ${CMAKE_CURRENT_LIST_DIR}/../../CMake)
endif()
include(${Slicer_CMAKE_DIR}/ListToString.cmake)

if(NOT DEFINED Slicer_EXTENSIONS_CMAKE_DIR)
  set(Slicer_EXTENSIONS_CMAKE_DIR ${CMAKE_CURRENT_LIST_DIR})
endif()

function(slicerFunctionGenerateExtensionDescription)
  set(options)
  set(oneValueArgs
    DESTINATION_DIR
    EXTENSION_BUILD_SUBDIRECTORY
    EXTENSION_CATEGORY
    EXTENSION_CONTRIBUTORS
    EXTENSION_DESCRIPTION
    EXTENSION_ENABLED
    EXTENSION_HOMEPAGE
    EXTENSION_ICONURL
    EXTENSION_NAME
    EXTENSION_SCREENSHOTURLS
    EXTENSION_STATUS
    EXTENSION_WC_REVISION
    EXTENSION_WC_ROOT
    EXTENSION_WC_TYPE
    EXTENSION_WC_URL
    SLICER_REVISION
    SLICER_WC_ROOT
    )
  set(multiValueArgs
    EXTENSION_DEPENDS
    )
  cmake_parse_arguments(MY "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  # Sanity checks
  set(expected_nonempty_vars
    EXTENSION_NAME
    EXTENSION_WC_REVISION
    EXTENSION_WC_ROOT
    EXTENSION_WC_TYPE
    EXTENSION_WC_URL
    SLICER_REVISION
    SLICER_WC_ROOT

    )
  foreach(var ${expected_nonempty_vars})
    if("${MY_${var}}" STREQUAL "")
      message(FATAL_ERROR "CMake variable ${var} is empty !")
    endif()
  endforeach()

  # Convert to space separated list
  list_to_string(" " "${MY_EXTENSION_DEPENDS}" MY_EXTENSION_DEPENDS)

  set(expected_existing_vars DESTINATION_DIR)
  foreach(var ${expected_existing_vars})
    if(NOT EXISTS "${MY_${var}}")
      message(FATAL_ERROR "error: ${var} CMake variable points to a inexistent file or directory: ${MY_${var}}")
    endif()
  endforeach()

  include(${Slicer_EXTENSIONS_CMAKE_DIR}/SlicerExtensionDescriptionSpec.cmake)
  slicer_extension_metadata_set_defaults(MY_EXTENSION)

  set(filename ${MY_DESTINATION_DIR}/${MY_EXTENSION_NAME}.s4ext)

  set(scm_type ${MY_EXTENSION_WC_TYPE})
  set(scm_url ${MY_EXTENSION_WC_URL})

  #message(MY_SLICER_WC_ROOT:${MY_SLICER_WC_ROOT})
  #message(MY_SLICER_REVISION:${MY_SLICER_REVISION})
  #message(MY_EXTENSION_WC_TYPE:${MY_EXTENSION_WC_TYPE})
  #message(MY_EXTENSION_WC_ROOT:${MY_EXTENSION_WC_ROOT})
  #message(MY_EXTENSION_WC_REVISION:${MY_EXTENSION_WC_REVISION})

  # If both Root and Revision matches, let's assume both Slicer source and Extension source
  # are checkout on the same filesystem.
  # This is useful for testing purposes
  if(${MY_EXTENSION_WC_TYPE} STREQUAL "local" OR (${MY_EXTENSION_WC_ROOT} STREQUAL ${MY_SLICER_WC_ROOT}
     AND ${MY_EXTENSION_WC_REVISION} STREQUAL ${MY_SLICER_REVISION}))
    set(scm_type local)
    set(scm_url ${CMAKE_CURRENT_SOURCE_DIR})
  endif()

  configure_file(
    ${Slicer_EXTENSIONS_CMAKE_DIR}/../../Utilities/Templates/Extensions/extension_description.s4ext.in
    ${filename}
    )

  message(STATUS "Extension description has been written to: ${filename}")
endfunction()

################################################################################
# Testing
################################################################################

#
# cmake -DTEST_<testfunction>:BOOL=ON -P <this_script>.cmake
#

function(slicer_generate_extension_description_test)

  if(NOT DEFINED Slicer_SOURCE_DIR)
    set(Slicer_SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/../..)
  endif()

  set(common_args
    DESTINATION_DIR ${CMAKE_CURRENT_BINARY_DIR}
    EXTENSION_DESCRIPTION "The SlicerToKiwiExporter module provides Slicer user with any easy way to export models into a KiwiViewer scene file."
    EXTENSION_CATEGORY "Exporter"
    EXTENSION_CONTRIBUTORS "Jean-Christophe Fillion-Robin (Kitware), Pat Marion (Kitware), Steve Pieper (Isomics), Atsushi Yamada (Shiga University of Medical Science)"
    EXTENSION_HOMEPAGE "http://www.slicer.org/slicerWiki/index.php/Documentation/Nightly/Extensions/SlicerToKiwiExporter"
    EXTENSION_ICONURL "http://www.slicer.org/slicerWiki/images/6/64/SlicerToKiwiExporterLogo.png"
    EXTENSION_NAME "SlicerToKiwiExporter"
    EXTENSION_SCREENSHOTURLS "http://www.slicer.org/slicerWiki/images/9/9e/SlicerToKiwiExporter_Kiwiviewer_8.PNG http://www.slicer.org/slicerWiki/images/a/ab/SlicerToKiwiExporter_Kiwiviewer_9.PNG http://www.slicer.org/slicerWiki/images/9/9a/SlicerToKiwiExporter_SaveDialog_Select-file-format_1.png"
    EXTENSION_WC_REVISION "9d7341e978df954a2c875240290833d7528ef29c"
    EXTENSION_WC_ROOT "git://github.com/jcfr/SlicerToKiwiExporter.git"
    EXTENSION_WC_TYPE "git"
    EXTENSION_WC_URL "git://github.com/jcfr/SlicerToKiwiExporter.git"
    SLICER_REVISION "21510"
    SLICER_WC_ROOT "http://svn.slicer.org/Slicer4"
    )

  # Generate description file of an extension *without* dependencies
  slicerFunctionGenerateExtensionDescription(
    ${common_args}
    #EXTENSION_BUILD_SUBDIRECTORY
    #EXTENSION_DEPENDS
    #EXTENSION_ENABLED
    EXTENSION_STATUS "Development"
    )
  execute_process(
    COMMAND ${CMAKE_COMMAND} -E compare_files
      ${CMAKE_CURRENT_BINARY_DIR}/SlicerToKiwiExporter.s4ext
      ${Slicer_SOURCE_DIR}/Extensions/CMake/Testing/extension_description_without_depends.s4ext
    ERROR_VARIABLE error
    RESULT_VARIABLE result
    )
  if(NOT result EQUAL 0)
    message(FATAL_ERROR "${error}")
  endif()

  # Generate description file of an extension *with* dependencies
  # where EXTENSION_DEPENDS is a space separated string
  slicerFunctionGenerateExtensionDescription(
    ${common_args}
    EXTENSION_BUILD_SUBDIRECTORY "inner/inner-inner-build"
    EXTENSION_DEPENDS "Foo Bar"
    EXTENSION_ENABLED 0
    EXTENSION_STATUS ""
    )
  execute_process(
    COMMAND ${CMAKE_COMMAND} -E compare_files
      ${CMAKE_CURRENT_BINARY_DIR}/SlicerToKiwiExporter.s4ext
      ${Slicer_SOURCE_DIR}/Extensions/CMake/Testing/extension_description_with_depends.s4ext
    ERROR_VARIABLE error
    RESULT_VARIABLE result
    )
  if(NOT result EQUAL 0)
    message(FATAL_ERROR "${error}")
  endif()

  # Generate description file of an extension *with* dependencies
  # where EXTENSION_DEPENDS is a list
  slicerFunctionGenerateExtensionDescription(
    ${common_args}
    EXTENSION_BUILD_SUBDIRECTORY "inner/inner-inner-build"
    EXTENSION_DEPENDS Foo Bar
    EXTENSION_ENABLED 0
    EXTENSION_STATUS ""
    )
  execute_process(
    COMMAND ${CMAKE_COMMAND} -E compare_files
      ${CMAKE_CURRENT_BINARY_DIR}/SlicerToKiwiExporter.s4ext
      ${Slicer_SOURCE_DIR}/Extensions/CMake/Testing/extension_description_with_depends.s4ext
    ERROR_VARIABLE error
    RESULT_VARIABLE result
    )
  if(NOT result EQUAL 0)
    message(FATAL_ERROR "${error}")
  endif()

  message("SUCCESS")
endfunction()
if(TEST_slicer_generate_extension_description_test)
  slicer_generate_extension_description_test()
endif()

