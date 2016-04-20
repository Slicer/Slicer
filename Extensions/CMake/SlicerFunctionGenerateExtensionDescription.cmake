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

function(slicerFunctionGenerateExtensionDescription)
  set(options)
  set(oneValueArgs
    DESTINATION_DIR
    EXTENSION_BUILD_SUBDIRECTORY
    EXTENSION_CATEGORY
    EXTENSION_CONTRIBUTORS
    EXTENSION_DEPENDS
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
    SLICER_WC_REVISION
    SLICER_WC_ROOT
    )
  set(multiValueArgs)
  cmake_parse_arguments(MY "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  # Sanity checks
  set(expected_nonempty_vars
    EXTENSION_NAME
    EXTENSION_WC_REVISION
    EXTENSION_WC_ROOT
    EXTENSION_WC_TYPE
    EXTENSION_WC_URL
    SLICER_WC_REVISION
    SLICER_WC_ROOT

    )
  foreach(var ${expected_nonempty_vars})
    if("${MY_${var}}" STREQUAL "")
      message(FATAL_ERROR "CMake variable ${var} is empty !")
    endif()
  endforeach()


  # A list of extension names without spaces is expected
  #TODO if()

  if("${MY_EXTENSION_DEPENDS}" STREQUAL "")
    set(MY_EXTENSION_DEPENDS "NA")
  endif()

  if(NOT DEFINED MY_EXTENSION_ENABLED)
    set(MY_EXTENSION_ENABLED 1)
  endif()

  # If not specified, EXTENSION_BUILD_SUBDIRECTORY default to "."
  if("${MY_EXTENSION_BUILD_SUBDIRECTORY}" STREQUAL "")
    set(MY_EXTENSION_BUILD_SUBDIRECTORY ".")
  endif()

  set(expected_existing_vars DESTINATION_DIR)
  foreach(var ${expected_existing_vars})
    if(NOT EXISTS "${MY_${var}}")
      message(FATAL_ERROR "error: ${var} CMake variable points to a inexistent file or directory: ${MY_${var}}")
    endif()
  endforeach()

  set(filename ${MY_DESTINATION_DIR}/${MY_EXTENSION_NAME}.s4ext)

  set(scm_type ${MY_EXTENSION_WC_TYPE})
  #set(scm_path_token ${MY_EXTENSION_WC_TYPE}path)
  set(scm_path_token scmurl)
  set(scm_url ${MY_EXTENSION_WC_URL})

  #message(MY_SLICER_WC_ROOT:${MY_SLICER_WC_ROOT})
  #message(MY_SLICER_WC_REVISION:${MY_SLICER_WC_REVISION})
  #message(MY_EXTENSION_WC_TYPE:${MY_EXTENSION_WC_TYPE})
  #message(MY_EXTENSION_WC_ROOT:${MY_EXTENSION_WC_ROOT})
  #message(MY_EXTENSION_WC_REVISION:${MY_EXTENSION_WC_REVISION})

  # If both Root and Revision matches, let's assume both Slicer source and Extension source
  # are checkout on the same filesystem.
  # This is useful for testing purposes
  if(${MY_EXTENSION_WC_TYPE} STREQUAL "local" OR (${MY_EXTENSION_WC_ROOT} STREQUAL ${MY_SLICER_WC_ROOT}
     AND ${MY_EXTENSION_WC_REVISION} STREQUAL ${MY_SLICER_WC_REVISION}))
    set(scm_type local)
    #set(scm_path_token localpath)
    set(scm_url ${CMAKE_CURRENT_SOURCE_DIR})
  endif()

 configure_file(
 ${CMAKE_CURRENT_LIST_DIR}/../Utilities/Templates/Extensions/extension_description.s4ext.in
 ${filename})

message(STATUS "Extension description has been written to: ${filename}")


endfunction()

