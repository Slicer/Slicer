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

macro(__SlicerBlockFindQtAndCheckVersion_find_qt)
  if(Slicer_REQUIRED_QT_VERSION VERSION_GREATER_EQUAL "6")
    find_package(Qt6 COMPONENTS ${Slicer_REQUIRED_QT_MODULES})
    set(_found_var Qt6_FOUND)
    set(_error_details "You probably need to set the Qt6_DIR variable.")
    set(_major ${Qt6_VERSION_MAJOR})
    set(_minor ${Qt6_VERSION_MINOR})
    set(_patch ${Qt6_VERSION_PATCH})
    set(_found_prefix "Qt6")
  else()
    find_package(Qt5 COMPONENTS ${Slicer_REQUIRED_QT_MODULES})
    set(_found_var Qt5_FOUND)
    set(_error_details "You probably need to set the Qt5_DIR variable.")
    set(_major ${Qt5_VERSION_MAJOR})
    set(_minor ${Qt5_VERSION_MINOR})
    set(_patch ${Qt5_VERSION_PATCH})
    set(_found_prefix "Qt5")
  endif()
  if(NOT ${_found_var})
    message(FATAL_ERROR "error: Qt ${Slicer_REQUIRED_QT_VERSION} was not found on your system."
                        "${_error_details}")
  endif()

  # Check version
  if("${_major}.${_minor}.${_patch}" VERSION_LESS "${Slicer_REQUIRED_QT_VERSION}")
    message(FATAL_ERROR "error: Slicer requires at least Qt ${Slicer_REQUIRED_QT_VERSION} "
                        "-- you cannot use Qt ${_major}.${_minor}.${_patch}. ${extra_error_message}")
  endif()

  set(command_separated_module_list)
  # Check if all expected Qt modules have been discovered
  foreach(module ${Slicer_REQUIRED_QT_MODULES})
    if(NOT "${${_found_prefix}${module}_FOUND}")
      message(FATAL_ERROR "error: Missing Qt module named \"${module}\"")
    endif()
    set(command_separated_module_list "${command_separated_module_list}${module}, ")
  endforeach()
endmacro()

# Sanity checks - Check if variable are defined
set(expected_defined_vars
  Slicer_REQUIRED_QT_VERSION
  Slicer_REQUIRED_QT_MODULES
  )
foreach(v ${expected_defined_vars})
  if(NOT DEFINED ${v})
    message(FATAL_ERROR "error: ${v} CMake variable is not defined.")
  endif()
endforeach()

# Check that either Qt5_DIR or Qt6_DIR is provided by CTK
set(extra_error_message)
  if(DEFINED CTK_Qt5_DIR AND NOT EXISTS "${CTK_Qt5_DIR}")
    message(FATAL_ERROR "error: You should probably re-configure CTK. CTK_Qt5_DIR points to a nonexistent directory: ${CTK_Qt5_DIR}")
  endif()
if(DEFINED CTK_Qt6_DIR AND NOT EXISTS "${CTK_Qt6_DIR}")
  message(FATAL_ERROR "error: You should probably re-configure CTK. CTK_Qt6_DIR points to a nonexistent directory: ${CTK_Qt6_DIR}")
endif()

__SlicerBlockFindQtAndCheckVersion_find_qt()

set(_project_name ${Slicer_MAIN_PROJECT_APPLICATION_NAME})
if(NOT Slicer_SOURCE_DIR)
  set(_project_name ${PROJECT_NAME})
endif()
message(STATUS "Configuring ${_project_name} with Qt ${_major}.${_minor}.${_patch} (using modules: ${command_separated_module_list})")

# Since neither Qt5 or Qt6 set CMake variables for plugins and binary directories,
# we explicitly set them here.
get_target_property(QMAKE_EXECUTABLE Qt${_major}::qmake LOCATION)

execute_process(
    COMMAND ${QMAKE_EXECUTABLE} -query QT_INSTALL_PLUGINS
    OUTPUT_VARIABLE QT_PLUGINS_DIR
    OUTPUT_STRIP_TRAILING_WHITESPACE
    RESULT_VARIABLE qmake_result
)
if(NOT qmake_result EQUAL 0)
    message(FATAL_ERROR "Failed to get QT_PLUGINS_DIR using qmake")
endif()
message(STATUS "Setting QT_PLUGINS_DIR: ${QT_PLUGINS_DIR}")

execute_process(
    COMMAND ${QMAKE_EXECUTABLE} -query QT_INSTALL_BINS
    OUTPUT_VARIABLE QT_BINARY_DIR
    OUTPUT_STRIP_TRAILING_WHITESPACE
    RESULT_VARIABLE qmake_result
)
if(NOT qmake_result EQUAL 0)
    message(FATAL_ERROR "Failed to get QT_BINARY_DIR using qmake")
endif()
message(STATUS "Setting QT_BINARY_DIR: ${QT_BINARY_DIR}")

execute_process(
    COMMAND ${QMAKE_EXECUTABLE} -query QT_INSTALL_LIBS
    OUTPUT_VARIABLE QT_LIBRARY_DIR
    OUTPUT_STRIP_TRAILING_WHITESPACE
    RESULT_VARIABLE qmake_result
)
if(NOT qmake_result EQUAL 0)
    message(FATAL_ERROR "Failed to get QT_LIBRARY_DIR using qmake")
endif()
message(STATUS "Setting QT_LIBRARY_DIR: ${QT_LIBRARY_DIR}")

# Sanity checks
foreach(varname IN ITEMS QT_PLUGINS_DIR QT_BINARY_DIR QT_LIBRARY_DIR)
  if("${${varname}}" STREQUAL "" OR NOT EXISTS "${${varname}}")
    message(FATAL_ERROR "Variable ${varname} is expected to be set to an existing path [${${varname}}]")
  endif()
endforeach()
