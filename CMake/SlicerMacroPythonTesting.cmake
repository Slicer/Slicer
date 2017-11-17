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

set(UNITTEST_LIB_PATHS
  "--additional-module-paths"
  ${CMAKE_BINARY_DIR}/${Slicer_QTSCRIPTEDMODULES_LIB_DIR}
  ${CMAKE_BINARY_DIR}/${Slicer_CLIMODULES_LIB_DIR}
  ${CMAKE_BINARY_DIR}/${Slicer_QTLOADABLEMODULES_LIB_DIR}
  )

macro(slicer_add_python_test)
  set(options)
  set(oneValueArgs TESTNAME_PREFIX SCRIPT)
  set(multiValueArgs SLICER_ARGS SCRIPT_ARGS)
  cmake_parse_arguments(MY "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
  get_filename_component(test_name ${MY_SCRIPT} NAME_WE)
  if(NOT IS_ABSOLUTE ${MY_SCRIPT})
    set(MY_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/${MY_SCRIPT}")
  endif()
  ExternalData_add_test(${Slicer_ExternalData_DATA_MANAGEMENT_TARGET}
    NAME py_${MY_TESTNAME_PREFIX}${test_name}
    COMMAND ${Slicer_LAUNCHER_EXECUTABLE}
    --no-splash
    --testing
    ${Slicer_ADDITIONAL_LAUNCHER_SETTINGS}
    ${UNITTEST_LIB_PATHS}
    ${MY_SLICER_ARGS}
    --python-script ${MY_SCRIPT} ${MY_SCRIPT_ARGS}
    )
  set_property(TEST py_${MY_TESTNAME_PREFIX}${test_name} PROPERTY RUN_SERIAL TRUE)
endmacro()

macro(slicer_add_python_unittest)
  set(options)
  set(oneValueArgs TESTNAME_PREFIX SCRIPT)
  set(multiValueArgs SLICER_ARGS)
  cmake_parse_arguments(MY "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
  get_filename_component(test_name ${MY_SCRIPT} NAME_WE)
  get_filename_component(_script_source_dir ${MY_SCRIPT} PATH)
  if("${_script_source_dir}" STREQUAL "")
    set(_script_source_dir ${CMAKE_CURRENT_SOURCE_DIR})
  endif()
  ExternalData_add_test(${Slicer_ExternalData_DATA_MANAGEMENT_TARGET}
    NAME py_${MY_TESTNAME_PREFIX}${test_name}
    COMMAND ${Slicer_LAUNCHER_EXECUTABLE}
    --no-splash
    --testing
    ${Slicer_ADDITIONAL_LAUNCHER_SETTINGS}
    ${MY_SLICER_ARGS}
    ${UNITTEST_LIB_PATHS}
    --python-code "import slicer.testing\\; slicer.testing.runUnitTest(['${CMAKE_CURRENT_BINARY_DIR}', '${_script_source_dir}'], '${test_name}')"
    )
  set_property(TEST py_${MY_TESTNAME_PREFIX}${test_name} PROPERTY RUN_SERIAL TRUE)
endmacro()
