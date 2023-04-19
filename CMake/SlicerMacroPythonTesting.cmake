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

# Setting this policy to NEW avoids error due to use of IN_LIST in build-system
# of Slicer extension specifying a minimum required version older than 3.3.
cmake_policy(SET CMP0057 NEW)

# When called from an extension build-system, this function updates the given
# list of arguments by appending appending "--additional-module-paths" along
# with the loadable, scripted loadable and CLI module paths if not explicitly
# disabled passing options like "--disable-modules" or "--disable-<module_type>-modules"
function(_slicer_python_test_append_module_paths arguments_var)
  if(DEFINED Slicer_SOURCE_DIR)
    return()
  endif()
  if("--disable-modules" IN_LIST "${arguments_var}")
    return()
  endif()
  set(module_paths)
  if(NOT "--disable-scripted-loadable-modules" IN_LIST "${arguments_var}" )
    list(APPEND module_paths ${CMAKE_BINARY_DIR}/${Slicer_QTSCRIPTEDMODULES_LIB_DIR})
  endif()
  if(NOT "--disable-cli-modules" IN_LIST "${arguments_var}" )
    list(APPEND module_paths ${CMAKE_BINARY_DIR}/${Slicer_CLIMODULES_LIB_DIR})
  endif()
  if(NOT "--disable-loadable-modules" IN_LIST "${arguments_var}" )
    list(APPEND module_paths ${CMAKE_BINARY_DIR}/${Slicer_QTLOADABLEMODULES_LIB_DIR})
  endif()
  if(module_paths)
    list(INSERT module_paths 0 "--additional-module-paths")
  endif()
  list(APPEND ${arguments_var} ${module_paths})
  set(${arguments_var} ${${arguments_var}} PARENT_SCOPE)
endfunction()

macro(slicer_add_python_test)
  set(options)
  set(oneValueArgs TESTNAME_PREFIX SCRIPT)
  set(multiValueArgs SLICER_ARGS SCRIPT_ARGS)
  cmake_parse_arguments(MY "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
  get_filename_component(test_name ${MY_SCRIPT} NAME_WE)
  if(NOT IS_ABSOLUTE ${MY_SCRIPT})
    set(MY_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/${MY_SCRIPT}")
  endif()
  _slicer_python_test_append_module_paths(MY_SLICER_ARGS)
  ExternalData_add_test(${Slicer_ExternalData_DATA_MANAGEMENT_TARGET}
    NAME py_${MY_TESTNAME_PREFIX}${test_name}
    COMMAND ${Slicer_LAUNCHER_EXECUTABLE}
    --no-splash
    --testing
    ${Slicer_ADDITIONAL_LAUNCHER_SETTINGS}
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
  _slicer_python_test_append_module_paths(MY_SLICER_ARGS)
  ExternalData_add_test(${Slicer_ExternalData_DATA_MANAGEMENT_TARGET}
    NAME py_${MY_TESTNAME_PREFIX}${test_name}
    COMMAND ${Slicer_LAUNCHER_EXECUTABLE}
    --no-splash
    --testing
    ${Slicer_ADDITIONAL_LAUNCHER_SETTINGS}
    ${MY_SLICER_ARGS}
    --python-code "import slicer.testing\\; slicer.testing.runUnitTest(['${CMAKE_CURRENT_BINARY_DIR}', '${_script_source_dir}'], '${test_name}')"
    )
  set_property(TEST py_${MY_TESTNAME_PREFIX}${test_name} PROPERTY RUN_SERIAL TRUE)
endmacro()
