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

include(SlicerMacroExtractRepositoryInfo)

function(slicerFunctionGenerateExtensionDescription)
  set(options)
  set(oneValueArgs EXTENSION_NAME EXTENSION_CATEGORY EXTENSION_ICONURL EXTENSION_CONTRIBUTORS EXTENSION_STATUS EXTENSION_HOMEPAGE EXTENSION_DESCRIPTION EXTENSION_SCREENSHOTURLS EXTENSION_DEPENDS EXTENSION_BUILD_SUBDIRECTORY EXTENSION_ENABLED DESTINATION_DIR SLICER_WC_REVISION SLICER_WC_ROOT)
  set(multiValueArgs)
  cmake_parse_arguments(MY "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  # Sanity checks
  set(expected_nonempty_vars EXTENSION_NAME EXTENSION_CATEGORY EXTENSION_STATUS EXTENSION_HOMEPAGE EXTENSION_DEPENDS SLICER_WC_REVISION SLICER_WC_ROOT)
  foreach(var ${expected_nonempty_vars})
    if("${MY_${var}}" STREQUAL "")
      message(FATAL_ERROR "CMake variable ${var} is empty !")
    endif()
  endforeach()

  set(expected_defined_vars EXTENSION_ICONURL EXTENSION_CONTRIBUTORS EXTENSION_DESCRIPTION EXTENSION_SCREENSHOTURLS)
  foreach(var ${expected_defined_vars})
    if("${MY_${var}}" STREQUAL "")
      message(AUTHOR_WARNING "CMake variable ${var} is empty !")
    endif()
  endforeach()

  if(NOT "${MY_EXTENSION_DEPENDS}" STREQUAL "NA")
    # A list of extension names without spaces is expected
    #TODO if()
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

  SlicerMacroExtractRepositoryInfo(VAR_PREFIX Extension)

  set(scm_type ${Extension_WC_TYPE})
  #set(scm_path_token ${Extension_WC_TYPE}path)
  set(scm_path_token scmurl)
  set(scm_url ${Extension_WC_URL})

  #message(MY_SLICER_WC_ROOT:${MY_SLICER_WC_ROOT})
  #message(MY_SLICER_WC_REVISION:${MY_SLICER_WC_REVISION})
  #message(Extension_WC_TYPE:${Extension_WC_TYPE})
  #message(Extension_WC_ROOT:${Extension_WC_ROOT})
  #message(Extension_WC_REVISION:${Extension_WC_REVISION})

  # If both Root and Revision matches, let's assume both Slicer source and Extension source
  # are checkout on the same filesystem.
  # This is useful for testing purposes
  if(${Extension_WC_TYPE} STREQUAL "local" OR ("${Extension_WC_ROOT}" STREQUAL ${MY_SLICER_WC_ROOT}
     AND "${Extension_WC_REVISION}" STREQUAL ${MY_SLICER_WC_REVISION}))
    set(scm_type local)
    #set(scm_path_token localpath)
    set(scm_url ${CMAKE_CURRENT_SOURCE_DIR})
  endif()


  file(WRITE ${filename}
"#
# First token of each non-comment line is the keyword and the rest of the line
# (including spaces) is the value.
# - the value can be blank
#

# This is source code manager (i.e. svn)
scm ${scm_type}
${scm_path_token} ${scm_url}

# list dependencies
# - These should be names of other modules that have .s4ext files
# - The dependencies will be built first
depends     ${MY_EXTENSION_DEPENDS}

# Inner build directory (default is ".")
build_subdirectory ${MY_EXTENSION_BUILD_SUBDIRECTORY}

# homepage
homepage    ${MY_EXTENSION_HOMEPAGE}

# Firstname1 Lastname1 ([SubOrg1, ]Org1), Firstname2 Lastname2 ([SubOrg2, ]Org2)
# For example: Jane Roe (Superware), John Doe (Lab1, Nowhere), Joe Bloggs (Noware)
contributors ${MY_EXTENSION_CONTRIBUTORS}

# Match category in the xml description of the module (where it shows up in Modules menu)
category    ${MY_EXTENSION_CATEGORY}

# url to icon (png, size 128x128 pixels)
iconurl     ${MY_EXTENSION_ICONURL}

# Give people an idea what to expect from this code
#  - Is it just a test or something you stand beind?
status      ${MY_EXTENSION_STATUS}

# One line stating what the module does
description ${MY_EXTENSION_DESCRIPTION}

# Space separated list of urls
screenshoturls ${MY_EXTENSION_SCREENSHOTURLS}

# 0 or 1: Define if the extension should be enabled after its installation.
enabled ${MY_EXTENSION_ENABLED}
")

message(STATUS "Extension description has been written to: ${filename}")


endfunction()

