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


#
# slicerFunctionExtractExtensionDescription(EXTENSION_FILE <file> VAR_PREFIX <var-prefix>)
# is used to extract information associated with a given extension <file>.
#
# The function defines the following variables in the caller scope:
#  <var-prefix>_SEXT_SCM - type of source repository (i.e. 'svn', 'git', 'local')
#  <var-prefix>_SEXT_SCMURL - URL of the associated source repository
#  <var-prefix>_SEXT_BUILD_SUBDIRECTORY - Corresponds to the extension inner build directory (default is ".")
#  <var-prefix>_SEXT_DEPENDS - list of dependencies
#  <var-prefix>_SEXT_HOMEPAGE - homepage
#  <var-prefix>_SEXT_CATEGORY - category
#  <var-prefix>_SEXT_ICONURL - url to icon (png, size 128x128 pixels)
#  <var-prefix>_SEXT_STATUS - status
#  <var-prefix>_SEXT_DESCRIPTION - one line description
#  <var-prefix>_SEXT_ENABLED - indicate if the extension should be enabled after its installation (default is 1)
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

  # <token_name>:<default_value>
  set(extension_description_tokens
    scm:
    scmurl:
    depends:
    build_subdirectory:.
    homepage:
    contributors:
    category:
    iconurl:
    status:
    description:
    enabled:1
    )

  foreach(token_and_default ${extension_description_tokens})

    # Extract token and its associated default value
    string(REPLACE ":" ";" token_and_default_as_list ${token_and_default})
    list(GET token_and_default_as_list 0 token)
    list(GET token_and_default_as_list 1 token_default_value)

    string(TOUPPER ${token} upper_case_token)
    string(REGEX REPLACE "^(.*\n)?${token}[ ]+([^\n]+).*"
          "\\2" sext_${upper_case_token} "${extension_file_content}")

    # If there was no match, set to the default value specified above or an empty string if any
    if(sext_${upper_case_token} STREQUAL "${extension_file_content}")
      set(sext_${upper_case_token} ${token_default_value})
    endif()

    # Trim value
    set(str ${sext_${upper_case_token}})
    string(REGEX REPLACE "^[ \t\r\n]+" "" str "${str}")
    string(REGEX REPLACE "[ \t\r\n]+$" "" str "${str}")
    set(sext_${upper_case_token} ${str})

    set(${MY_VAR_PREFIX}_SEXT_${upper_case_token} ${sext_${upper_case_token}} PARENT_SCOPE)
  endforeach()

endfunction()

