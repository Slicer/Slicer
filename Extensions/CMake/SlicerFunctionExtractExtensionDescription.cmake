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

cmake_minimum_required(VERSION 3.19.7 FATAL_ERROR)

include(CMakeParseArguments)

if(NOT DEFINED Slicer_EXTENSIONS_CMAKE_DIR)
  set(Slicer_EXTENSIONS_CMAKE_DIR ${CMAKE_CURRENT_LIST_DIR})
endif()

#
# slicerFunctionExtractExtensionDescription(EXTENSION_FILE <file> VAR_PREFIX <var-prefix>)
# is used to extract information associated with a given extension <file>.
#
# The function defines the following variables in the caller scope:
#  <var-prefix>_EXT_SCM - type of source repository (i.e. 'git', 'local')
#  <var-prefix>_EXT_SCMURL - URL of the associated source repository
#  <var-prefix>_EXT_SCMREVISION - REVISION associated with the source repository
#  <var-prefix>_EXT_BUILD_SUBDIRECTORY - Corresponds to the extension inner build directory (default is ".")
#  <var-prefix>_EXT_DEPENDS - list of runtime dependencies
#  <var-prefix>_EXT_HOMEPAGE - homepage
#  <var-prefix>_EXT_CATEGORY - category
#  <var-prefix>_EXT_ICONURL - url to icon (png, size 128x128 pixels)
#  <var-prefix>_EXT_STATUS - status
#  <var-prefix>_EXT_DESCRIPTION - one line description
#  <var-prefix>_EXT_SCREENSHOTURLS - space separated list of urls
#  <var-prefix>_EXT_ENABLED - indicate if the extension should be enabled after its installation (default is 1)
#

function(slicerFunctionExtractExtensionDescription)
  set(options)
  set(oneValueArgs EXTENSION_FILE VAR_PREFIX)
  set(multiValueArgs)
  cmake_parse_arguments(MY "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  # Sanity checks
  set(expected_nonempty_vars VAR_PREFIX)
  foreach(var ${expected_nonempty_vars})
    if("${MY_${var}}" STREQUAL "")
      message(FATAL_ERROR "error: ${var} CMake variable is empty !")
    endif()
  endforeach()

  set(expected_existing_vars EXTENSION_FILE)
  foreach(var ${expected_existing_vars})
    if(NOT EXISTS "${MY_${var}}")
      message(FATAL_ERROR "error: ${var} CMake variable points to a inexistent file or directory: ${MY_${var}}")
    endif()
  endforeach()

  # Read file
  file(READ ${MY_EXTENSION_FILE} extension_file_content)

  include(${Slicer_EXTENSIONS_CMAKE_DIR}/SlicerExtensionDescriptionSpec.cmake)

  foreach(name IN LISTS Slicer_EXT_METADATA_NAMES)
    set(token_default_value "${Slicer_EXT_METADATA_${name}_DEFAULT}")
    set(upper_case_token ${name})
    string(TOLOWER ${name} token)

    string(REGEX REPLACE "^(.*\n)?${token}[ ]+([^\n]+).*"
          "\\2" ext_${upper_case_token} "${extension_file_content}")

    # If there was no match, set to the default value specified above or an empty string if any
    if(ext_${upper_case_token} STREQUAL "${extension_file_content}")
      set(ext_${upper_case_token} ${token_default_value})
    endif()

    # Trim value
    set(str ${ext_${upper_case_token}})
    string(REGEX REPLACE "^[ \t\r\n]+" "" str "${str}")
    string(REGEX REPLACE "[ \t\r\n]+$" "" str "${str}")
    set(ext_${upper_case_token} ${str})

    # depends
    if(${token} STREQUAL "depends")
      # Replace "NA"
      string(REGEX REPLACE "^NA$" "" ext_${upper_case_token} "${ext_${upper_case_token}}")
      # Convert to list
      string(REPLACE " " ";" ext_${upper_case_token} "${ext_${upper_case_token}}")
    endif()
    set(${MY_VAR_PREFIX}_EXT_${upper_case_token} "${ext_${upper_case_token}}" PARENT_SCOPE)
  endforeach()

endfunction()

#
# slicerFunctionExtractExtensionDescriptionFromJson(EXTENSION_FILE <file> VAR_PREFIX <var-prefix>)
# is used to extract information associated with a given extension <file>.
#
# The function defines the following variables in the caller scope:
#  <var-prefix>_EXT_SCM_TYPE - type of source repository ('git' or 'local', default is 'git')
#  <var-prefix>_EXT_SCM_URL - URL of the associated source repository
#  <var-prefix>_EXT_SCM_REVISION - REVISION associated with the source repository
#  <var-prefix>_EXT_BUILD_DEPENDENCIES - list of Slicer extensions required at build-time
#  <var-prefix>_EXT_CATEGORY - category
#  <var-prefix>_EXT_ENABLED - indicate if the extension should be enabled after its installation (default is 1)
#  <var-prefix>_EXT_TIER - extension tier (default is 1)
#  <var-prefix>_EXT_DICOM_SUPPORT_RULE - logical string representing dicom plugin support
#  <var-prefix>_EXT_RECOMMENDS - list of Slicer extensions that should be installed with this extension
#  <var-prefix>_EXT_KEYWORDS - list of keywords that are used to describe the extension
#

function(slicerFunctionExtractExtensionDescriptionFromJson)
  set(options)
  set(oneValueArgs EXTENSION_FILE VAR_PREFIX)
  set(multiValueArgs)
  cmake_parse_arguments(MY "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  # Sanity checks
  set(expected_nonempty_vars VAR_PREFIX)
  foreach(var ${expected_nonempty_vars})
    if("${MY_${var}}" STREQUAL "")
      message(FATAL_ERROR "error: ${var} CMake variable is empty !")
    endif()
  endforeach()

  set(expected_existing_vars EXTENSION_FILE)
  foreach(var ${expected_existing_vars})
    if(NOT EXISTS "${MY_${var}}")
      message(FATAL_ERROR "error: ${var} CMake variable points to a inexistent file or directory: ${MY_${var}}")
    endif()
  endforeach()

  # Read file
  file(READ ${MY_EXTENSION_FILE} extension_file_content)

  set(Slicer_EXT_REQUIRED_METADATA_NAMES
    "SCM_URL"
    "SCM_REVISION"
    "CATEGORY"
    )
  set(Slicer_EXT_OPTIONAL_METADATA_NAMES
    "SCM_TYPE"
    "BUILD_SUBDIRECTORY"
    "BUILD_DEPENDENCIES"
    "ENABLED"
    "TIER"
    "DICOM_SUPPORT_RULE"
    )
  set(SCM_TYPE_DEFAULT "git")
  set(BUILD_SUBDIRECTORY_DEFAULT ".")
  set(BUILD_DEPENDENCIES_DEFAULT "")
  set(ENABLED_DEFAULT "1")
  set(TIER_DEFAULT "1")
  set(DICOM_SUPPORT_RULE_DEFAULT "")

  foreach(name IN LISTS Slicer_EXT_REQUIRED_METADATA_NAMES Slicer_EXT_OPTIONAL_METADATA_NAMES)
    set(upper_case_token ${name})
    string(TOLOWER ${name} token)

    if(name IN_LIST Slicer_EXT_OPTIONAL_METADATA_NAMES)
      string(JSON type ERROR_VARIABLE error TYPE "${extension_file_content}" "${token}")
      if(error)
        set(${MY_VAR_PREFIX}_EXT_${upper_case_token} "${${upper_case_token}_DEFAULT}" PARENT_SCOPE)
        continue()
      endif()
    else()
      string(JSON type TYPE "${extension_file_content}" "${token}")
    endif()

    if(type STREQUAL "ARRAY")
      set(values)
      string(JSON arrayLength LENGTH "${extension_file_content}" "${token}")
      math(EXPR arrayLength "${arrayLength}-1")
      if(arrayLength GREATER_EQUAL 0)
        foreach(index RANGE ${arrayLength})
          string(JSON value GET "${extension_file_content}" "${token}" ${index})
          list(APPEND values ${value})
        endforeach()
      endif()
      set(ext_${upper_case_token} ${values})
    elseif(type STREQUAL "BOOLEAN")
      string(JSON value GET "${extension_file_content}" "${token}")
      if(value)
        set(ext_${upper_case_token} "1")
      else()
        set(ext_${upper_case_token} "0")
      endif()
    else()
      string(JSON ext_${upper_case_token} GET "${extension_file_content}" "${token}")
    endif()

    set(${MY_VAR_PREFIX}_EXT_${upper_case_token} "${ext_${upper_case_token}}" PARENT_SCOPE)
  endforeach()
endfunction()

################################################################################
# Testing
################################################################################

#
# cmake -DTEST_<testfunction>:BOOL=ON -P <this_script>.cmake
#

function(slicer_extract_extension_description_test)

  # Common properties
  set(required
    SCM
    SCMURL
    SCMREVISION
    )
  set(optional
    DEPENDS
    RECOMMENDS
    BUILD_SUBDIRECTORY
    HOMEPAGE
    CONTRIBUTORS
    CATEGORY
    ICONURL
    DESCRIPTION
    SCREENSHOTURLS
    ENABLED
    STATUS
    KEYWORDS
    )

  set(expected_BUILD_SUBDIRECTORY ".")
  set(expected_CATEGORY "Exporter")
  set(expected_CONTRIBUTORS "Jean-Christophe Fillion-Robin (Kitware), Pat Marion (Kitware), Steve Pieper (Isomics), Atsushi Yamada (Shiga University of Medical Science)")
  set(expected_DESCRIPTION "The SlicerToKiwiExporter module provides Slicer user with any easy way to export models into a KiwiViewer scene file.<br>This is a line of text.<br>And another one.")
  set(expected_ENABLED "1")
  set(expected_HOMEPAGE "https://www.slicer.org/w/index.php/Documentation/Nightly/Extensions/SlicerToKiwiExporter")
  set(expected_ICONURL "https://www.slicer.org/w/images/6/64/SlicerToKiwiExporterLogo.png")
  set(expected_SCM "git")
  set(expected_SCMREVISION "9d7341e978df954a2c875240290833d7528ef29c")
  set(expected_SCMURL "https://github.com/jcfr/SlicerToKiwiExporter.git")
  set(expected_SCREENSHOTURLS "https://www.slicer.org/w/images/9/9e/SlicerToKiwiExporter_Kiwiviewer_8.PNG https://www.slicer.org/w/images/a/ab/SlicerToKiwiExporter_Kiwiviewer_9.PNG https://www.slicer.org/w/images/9/9a/SlicerToKiwiExporter_SaveDialog_Select-file-format_1.png")
  set(expected_STATUS "")

  # Extract extension description without depends
  set(test_s4ext ${CMAKE_CURRENT_BINARY_DIR}/slicer_extract_extension_description_without_depends_test.s4ext)
  file(WRITE ${test_s4ext}
"build_subdirectory ${expected_BUILD_SUBDIRECTORY}
category ${expected_CATEGORY}
contributors ${expected_CONTRIBUTORS}
depends NA
description ${expected_DESCRIPTION}
enabled ${expected_ENABLED}
homepage ${expected_HOMEPAGE}
iconurl ${expected_ICONURL}
scm ${expected_SCM}
scmrevision ${expected_SCMREVISION}
scmurl ${expected_SCMURL}
screenshoturls ${expected_SCREENSHOTURLS}
status ${expected_STATUS}")

  slicerFunctionExtractExtensionDescription(
    EXTENSION_FILE ${test_s4ext}
    VAR_PREFIX foo
  )

  set(expected_DEPENDS "")

  foreach(name IN LISTS required optional)
    if(NOT foo_EXT_${name} STREQUAL "${expected_${name}}")
      message(FATAL_ERROR "Problem with foo_EXT_${name}
  Expected: [${expected_${name}}]
  Actual: [${foo_EXT_${name}}]")
    endif()
  endforeach()

  list(LENGTH foo_EXT_DEPENDS depends_count)
  if(NOT depends_count EQUAL 0)
    message(FATAL_ERROR "Problem with foo_EXT_DEPENDS")
  endif()


  # Extract extension description with depends
  set(test_s4ext ${CMAKE_CURRENT_BINARY_DIR}/slicer_extract_extension_description_with_depends_test.s4ext)
  file(WRITE ${test_s4ext}
"build_subdirectory ${expected_BUILD_SUBDIRECTORY}
category ${expected_CATEGORY}
contributors ${expected_CONTRIBUTORS}
depends Foo Bar
description ${expected_DESCRIPTION}
enabled ${expected_ENABLED}
homepage ${expected_HOMEPAGE}
iconurl ${expected_ICONURL}
scm ${expected_SCM}
scmrevision ${expected_SCMREVISION}
scmurl ${expected_SCMURL}
screenshoturls ${expected_SCREENSHOTURLS}
status ${expected_STATUS}")

  slicerFunctionExtractExtensionDescription(
    EXTENSION_FILE ${test_s4ext}
    VAR_PREFIX bar
  )

  set(expected_DEPENDS Foo Bar)

  foreach(name IN LISTS required optional)
    if(NOT bar_EXT_${name} STREQUAL "${expected_${name}}")
      message(FATAL_ERROR "Problem with bar_EXT_${name}
  Expected: [${expected_${name}}]
  Actual: [${bar_EXT_${name}}]")
    endif()
  endforeach()

  list(LENGTH bar_EXT_DEPENDS depends_count)
  if(NOT depends_count EQUAL 2)
    message(FATAL_ERROR "Problem with bar_EXT_DEPENDS")
  endif()

  message("SUCCESS")
endfunction()
if(TEST_slicer_extract_extension_description_test)
  slicer_extract_extension_description_test()
endif()

function(slicer_extract_extension_description_from_json_test)

  # Common properties
  set(required
    SCM_URL
    SCM_REVISION
    CATEGORY
    )
  set(optional
    SCM_TYPE
    BUILD_DEPENDENCIES
    BUILD_SUBDIRECTORY
    DICOM_SUPPORT_RULE
    ENABLED
    TIER
    RECOMMENDS
    KEYWORDS
    )

  set(expected_CATEGORY "Exporter")
  set(expected_SCM_TYPE "git")
  set(expected_SCM_REVISION "9d7341e978df954a2c875240290833d7528ef29c")
  set(expected_SCM_URL "https://github.com/jcfr/SlicerToKiwiExporter.git")

  # Extract extension description without depends
  set(test_s4ext ${CMAKE_CURRENT_BINARY_DIR}/slicer_extract_extension_description_without_depends_test.json)
  file(WRITE ${test_s4ext}
"{
    \"category\": \"${expected_CATEGORY}\",
    \"scm_revision\": \"${expected_SCM_REVISION}\",
    \"scm_url\": \"${expected_SCM_URL}\"
}
")

  slicerFunctionExtractExtensionDescriptionFromJson(
    EXTENSION_FILE ${test_s4ext}
    VAR_PREFIX foo
  )

  set(expected_BUILD_SUBDIRECTORY ".")
  set(expected_BUILD_DEPENDENCIES "")
  set(expected_ENABLED "1")
  set(expected_TIER "1")
  set(expected_DICOM_SUPPORT_RULE "")

  foreach(name IN LISTS required optional)
    if(NOT foo_EXT_${name} STREQUAL "${expected_${name}}")
      message(FATAL_ERROR "Problem with foo_EXT_${name}
  Expected: [${expected_${name}}]
  Actual: [${foo_EXT_${name}}]")
    endif()
  endforeach()

  list(LENGTH foo_EXT_BUILD_DEPENDENCIES build_dependencies_count)
  if(NOT build_dependencies_count EQUAL 0)
    message(FATAL_ERROR "Problem with foo_EXT_BUILD_DEPENDENCIES")
  endif()


  # Extract extension description with depends
  set(test_s4ext ${CMAKE_CURRENT_BINARY_DIR}/slicer_extract_extension_description_with_depends_test.json)
  file(WRITE ${test_s4ext}
"{
    \"build_subdirectory\": \"inner-build\",
    \"category\": \"${expected_CATEGORY}\",
    \"build_dependencies\": [\"Foo\", \"Bar\"],
    \"scm_revision\": \"${expected_SCM_REVISION}\",
    \"scm_url\": \"${expected_SCM_URL}\",
    \"enabled\": false,
    \"tier\": 5,
    \"dicom_support_rule\": \"Modality = \\\"SEG\\\"\",
}
")

  slicerFunctionExtractExtensionDescriptionFromJson(
    EXTENSION_FILE ${test_s4ext}
    VAR_PREFIX bar
  )

  set(expected_BUILD_SUBDIRECTORY "inner-build")
  set(expected_BUILD_DEPENDENCIES Foo Bar)
  set(expected_ENABLED "0")
  set(expected_TIER "5")
  set(expected_DICOM_SUPPORT_RULE "Modality = \"SEG\"")
  set(expected_RECOMMENDS Foo Bar)
  set(expected_KEYWORDS Foo Bar)

  foreach(name IN LISTS required optional)
    if(NOT bar_EXT_${name} STREQUAL "${expected_${name}}")
      message(FATAL_ERROR "Problem with bar_EXT_${name}
  Expected: [${expected_${name}}]
  Actual: [${bar_EXT_${name}}]")
    endif()
  endforeach()

  list(LENGTH bar_EXT_BUILD_DEPENDENCIES build_dependencies_count)
  if(NOT build_dependencies_count EQUAL 2)
    message(FATAL_ERROR "Problem with bar_EXT_BUILD_DEPENDENCIES")
  endif()

  message("SUCCESS")
endfunction()
if(TEST_slicer_extract_extension_description_from_json_test)
  slicer_extract_extension_description_from_json_test()
endif()
