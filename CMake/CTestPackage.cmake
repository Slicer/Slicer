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

function(ctest_package)
  set(options)
  set(oneValueArgs BINARY_DIR CONFIG RETURN_VAR)
  set(multiValueArgs)
  cmake_parse_arguments(MY "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  # Sanity checks
  if(NOT DEFINED MY_BINARY_DIR)
    message(SEND_ERROR "BINARY_DIR is mandatory")
  endif()

  if(NOT DEFINED MY_CONFIG)
    message(SEND_ERROR "CONFIG is mandatory")
  endif()

  # The following variable could be used while testing the macro ...
  set(_build_target TRUE)

  set(cpack_output_file ${MY_BINARY_DIR}/ctest_package_make_package_output.txt)
  if(_build_target)
    execute_process(
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

  if(rv EQUAL 0)

    set(regexp ".*CPack: - package: (.*) generated\\.")

    # Extract list of generated packages
    set(raw_package_list)
    file(STRINGS ${cpack_output_file} raw_package_list REGEX ${regexp})

    foreach(package ${raw_package_list})
      string(REGEX REPLACE ${regexp} "\\1" package_path "${package}" )
      list(APPEND package_list ${package_path})
    endforeach()

  endif()

  set(${MY_RETURN_VAR} ${package_list} PARENT_SCOPE)
endfunction()
