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

IF(NOT DEFINED CMAKE_CURRENT_LIST_DIR)
  GET_FILENAME_COMPONENT(CMAKE_CURRENT_LIST_DIR ${CMAKE_CURRENT_LIST_FILE} PATH)
ENDIF()
INCLUDE("${CMAKE_CURRENT_LIST_DIR}/SlicerMacroParseArguments.cmake")

FUNCTION(SlicerFunctionCTestPackage)
  SLICER_PARSE_ARGUMENTS(MY
    "BINARY_DIR;CONFIG;RETURN_VAR"
    ""
    ${ARGN})

  # Sanity checks
  IF(NOT DEFINED MY_BINARY_DIR)
    MESSAGE(SEND_ERROR "BINARY_DIR is mandatory")
  ENDIF()

  IF(NOT DEFINED MY_CONFIG)
    MESSAGE(SEND_ERROR "CONFIG is mandatory")
  ENDIF()
  
  # The following variable could be used while testing the macro ...
  set(SlicerFunctionCTestPackage_build_target TRUE)

  set(cpack_output_file ${MY_BINARY_DIR}/SlicerFunctionCTestPackage-make-package-output.txt)
  if (SlicerFunctionCTestPackage_build_target)
    EXECUTE_PROCESS(
      COMMAND ${CMAKE_COMMAND} --build ${MY_BINARY_DIR} --target package --config ${MY_CONFIG}
      WORKING_DIRECTORY ${MY_BINARY_DIR}
      OUTPUT_STRIP_TRAILING_WHITESPACE
      OUTPUT_FILE ${cpack_output_file}
      RESULT_VARIABLE rv
      )
  endif()
    
  # TODO Display errors so that they can be sent to CDash

  set(rv 0)
  
  # List of filepath corresponding to the generated packages or installers
  set(package_list)

  IF(rv EQUAL 0)

    set(regexp ".*CPack: - package: (.*) generated\\.")

    # Extract list of generated packages
    set(raw_package_list)
    FILE(STRINGS ${cpack_output_file} raw_package_list REGEX ${regexp})

    FOREACH(package ${raw_package_list})
      STRING(REGEX REPLACE ${regexp} "\\1" package_path "${package}" )
      LIST(APPEND package_list ${package_path})
    ENDFOREACH()
    
  ENDIF()

  SET(${MY_RETURN_VAR} ${package_list} PARENT_SCOPE)
ENDFUNCTION()
