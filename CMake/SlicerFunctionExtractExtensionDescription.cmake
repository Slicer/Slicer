################################################################################
#
#  Program: 3D Slicer
#
#  Copyright (c) 2010 Kitware Inc.
#
#  See Doc/copyright/copyright.txt
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
#  <var-prefix>_SEXT_DEPENDS - list of dependencies
#  <var-prefix>_SEXT_HOMEPAGE - homepage 
#  <var-prefix>_SEXT_CATEGORY - category
#  <var-prefix>_SEXT_STATUS - status
#  <var-prefix>_SEXT_DESCRIPTION - one line description 
#

FUNCTION(slicerFunctionExtractExtensionDescription)
  set(options)
  set(oneValueArgs EXTENSION_FILE VAR_PREFIX)
  set(multiValueArgs)
  cmake_parse_arguments(MY "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
  
  # Sanity checks
  SET(expected_nonempty_vars VAR_PREFIX)
  FOREACH(var ${expected_nonempty_vars})
    IF("${MY_${var}}" STREQUAL "")
      MESSAGE(FATAL_ERROR "error: ${var} CMake variable is empty !")
    ENDIF()
  ENDFOREACH()
  
  SET(expected_existing_vars EXTENSION_FILE)
  FOREACH(var ${expected_existing_vars})
    IF(NOT EXISTS "${MY_${var}}")
      MESSAGE(FATAL_ERROR "error: ${var} CMake variable points to a inexistent file or directory: ${MY_${var}}")
    ENDIF()
  ENDFOREACH()
  
  # Read file
  FILE(READ ${MY_EXTENSION_FILE} extension_file_content)
  
  SET(extension_description_tokens scm scmurl depends homepage category status description)
  
  FOREACH(token ${extension_description_tokens})
    
    STRING(TOUPPER ${token} upper_case_token)
    STRING(REGEX REPLACE "^(.*\n)?${token}[ ]+([^\n]+).*"
          "\\2" sext_${upper_case_token} "${extension_file_content}")
    
    # If there was no match, set to an empty string
    if (sext_${upper_case_token} STREQUAL "${extension_file_content}")
      set(sext_${upper_case_token} "")
    endif()
    
    SET(${MY_VAR_PREFIX}_SEXT_${upper_case_token} ${sext_${upper_case_token}} PARENT_SCOPE)
  ENDFOREACH()

ENDFUNCTION()

