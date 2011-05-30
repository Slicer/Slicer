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


MACRO(SLICER_ADD_PYTHON_TEST)
  set(options)
  set(oneValueArgs TESTNAME_PREFIX SLICER_ARGS SCRIPT)
  set(multiValueArgs SCRIPT_ARGS)
  cmake_parse_arguments(MY "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
  get_filename_component(test_name ${MY_SCRIPT} NAME_WE)
  ADD_TEST(
    NAME py_${MY_TESTNAME_PREFIX}${test_name}
    COMMAND ${Slicer_LAUNCH_COMMAND} $<TARGET_FILE:SlicerQT${SlicerQT_EXE_SUFFIX}>
    --no-splash --ignore-slicerrc ${MY_SLICER_ARGS} --python-script ${CMAKE_CURRENT_SOURCE_DIR}/${MY_SCRIPT} ${MY_SCRIPT_ARGS}
    )
ENDMACRO()

MACRO(SLICER_ADD_PYTHON_UNITTEST)
  set(options)
  set(oneValueArgs TESTNAME_PREFIX SCRIPT)
  set(multiValueArgs SLICER_ARGS)
  cmake_parse_arguments(MY "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
  get_filename_component(test_name ${MY_SCRIPT} NAME_WE)
  ADD_TEST(
    NAME py_${MY_TESTNAME_PREFIX}${test_name}
    COMMAND ${Slicer_LAUNCH_COMMAND} $<TARGET_FILE:SlicerQT${SlicerQT_EXE_SUFFIX}>
    --no-splash --ignore-slicerrc ${MY_SLICER_ARGS}
    --python-code "import slicer.testing; slicer.testing.runUnitTest('${CMAKE_CURRENT_SOURCE_DIR}', '${test_name}')"
    )
ENDMACRO()
