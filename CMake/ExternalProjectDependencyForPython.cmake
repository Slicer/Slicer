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
#  and was partially funded by NIH grant 1U24CA194354-01
#
################################################################################

#
#.rst:
# ExternalProjectDependencyForPython
# ----------------------------------
#
# This module provides functions intended to be used in ``External_*.cmake`` files.
#
# .. command:: ExternalProject_FindPythonPackage
#
#   ExternalProject_FindPythonPackage(
#     MODULE_NAME <python-module-name>
#     [VERSION_PROPERTY <version-property>]
#     [PYTHON_EXECUTABLE /path/to/python-interpreter]
#     [OUTPUT_VAR_PREFIX <output-var-prefix>]
#     )
#
#  This function tries to import ``<python-module-name>`` module.
#
#  By default, it displays corresponding module path and version and set variables
#  ``<python-module-name>_PATH`` and ``<python-module-name>_VERSION``.
#
#  The options are:
#
#  ``MODULE_NAME <python-module-name>``
#    Name of the python module to try to import using ``/path/to/python-interpreter -c "import <python-module-name>"``.
#
#  ``REQUIRED``
#    Report a fatal error if module can not be imported.
#
#  ``QUIET``
#    Do not display path and version.
#
#  ``VERSION_PROPERTY <version-property>``
#    Name of the version property to query. It defaults to ``<python-module-name>.__version__``.
#
#  ``NO_VERSION_PROPERTY``
#    If specified, do not attempt to extract version information and set output variable.
#
#  ``PYTHON_EXECUTABLE /path/to/python-interpreter``
#    Path of the python executable to search module with. It defaults to value
#    of ``PYTHON_EXECUTABLE`` variable set in the calling scope.
#
#  ``OUTPUT_VAR_PREFIX <output-var-prefix>``
#    Specify the prefix of output variables ``<output-var-prefix>_PATH`` and ``<output-var-prefix>_VERSION``.
#    It defaults to upercase value of ``<python-module-name>``.
#

function(ExternalProject_FindPythonPackage)
  set(options REQUIRED QUIET NO_VERSION_PROPERTY)
  set(oneValueArgs MODULE_NAME VERSION_PROPERTY PYTHON_EXECUTABLE OUTPUT_VAR_PREFIX)
  set(multiValueArgs )
  cmake_parse_arguments(MY "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  # Sanity checks
  if(NOT DEFINED MY_MODULE_NAME)
    message(FATAL_ERROR "Parameter MODULE_NAME is not specified !")
  endif()

  # Defaults
  if(NOT DEFINED MY_VERSION_PROPERTY)
    set(MY_VERSION_PROPERTY "__version__")
  endif()
  if(NOT DEFINED MY_OUTPUT_VAR_PREFIX)
    string(TOUPPER "${MY_MODULE_NAME}" MY_OUTPUT_VAR_PREFIX)
  endif()

  # Python interpreter
  if(NOT DEFINED MY_PYTHON_EXECUTABLE)
    if(NOT DEFINED PYTHON_EXECUTABLE)
      find_package(PythonInterp REQUIRED)
    endif()
    set(MY_PYTHON_EXECUTABLE ${PYTHON_EXECUTABLE})
    if(NOT EXISTS "${MY_PYTHON_EXECUTABLE}")
      if(MY_REQUIRED)
        message(FATAL_ERROR "Could not find Python interpreter for required dependency ${MY_MODULE_NAME}. Path '${MY_PYTHON_EXECUTABLE}' corresponds to a nonexistent file.")
      else()
        message(STATUS "Could not find ${MY_MODULE_NAME} because no Python interpreter was found")
        return()
      endif()
    endif()
  endif()

  # Check if module can be imported
  execute_process(
    COMMAND ${PYTHON_EXECUTABLE} -c "import ${MY_MODULE_NAME}"
    RESULT_VARIABLE _process_status
    OUTPUT_STRIP_TRAILING_WHITESPACE
    )
  if(NOT _process_status EQUAL 0)
    if(MY_REQUIRED)
      message(FATAL_ERROR "Could not import module for required dependency '${MY_MODULE_NAME}' using '${MY_PYTHON_EXECUTABLE}' interpreter.")
    else()
      message(STATUS "Could not find ${MY_MODULE_NAME} using '${MY_PYTHON_EXECUTABLE}' interpreter.")
      return()
    endif()
  endif()

  # Get path
  execute_process(
    COMMAND
      ${PYTHON_EXECUTABLE} -c 
        "import ${MY_MODULE_NAME}; print(${MY_MODULE_NAME}.__file__)"
    RESULT_VARIABLE _process_status
    OUTPUT_VARIABLE _process_output
    OUTPUT_STRIP_TRAILING_WHITESPACE
    )
  if(NOT _process_status EQUAL 0)
    message(FATAL_ERROR "Could not get module '${MY_MODULE_NAME}' path using '${MY_MODULE_NAME}.__file__'.")
  endif()
  set(_path "${_process_output}")
  string(STRIP ${_path} _path)
  get_filename_component(_path ${_path} DIRECTORY)

  # Get version
  if(NOT MY_NO_VERSION_PROPERTY)
    execute_process(
      COMMAND
        ${PYTHON_EXECUTABLE} -c 
          "import ${MY_MODULE_NAME}; print(${MY_MODULE_NAME}.${MY_VERSION_PROPERTY})"
      RESULT_VARIABLE _process_status
      OUTPUT_VARIABLE _process_output
      OUTPUT_STRIP_TRAILING_WHITESPACE
      )
    if(NOT _process_status EQUAL 0)
      message(FATAL_ERROR "Could not get module '${MY_MODULE_NAME}' version using '${MY_MODULE_NAME}.${MY_VERSION_PROPERTY}'.")
    endif()
    set(_version "${_process_output}")
    string(STRIP ${_version} _version)
  endif()

  # Path
  if(NOT MY_QUIET)
    message(STATUS "${MY_OUTPUT_VAR_PREFIX}_PATH: ${_path}")
  endif()
  set(${MY_OUTPUT_VAR_PREFIX}_PATH "${_path}" PARENT_SCOPE)

  # Version
  if(NOT MY_NO_VERSION_PROPERTY)
    if(NOT MY_QUIET)
      message(STATUS "${MY_OUTPUT_VAR_PREFIX}_VERSION: ${_version}")
    endif()
    set(${MY_OUTPUT_VAR_PREFIX}_VERSION "${_version}" PARENT_SCOPE)
  endif()

endfunction()
