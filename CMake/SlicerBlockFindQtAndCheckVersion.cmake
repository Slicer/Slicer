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
# The CMake code used to find Qt4 has been factored out into this CMake script so that
# it can be used in both Slicer/CMakelists.txt and Slicer/UseSlicer.cmake
#

macro(__SlicerBlockFindQtAndCheckVersion_find_qt)
  if(Slicer_REQUIRED_QT_VERSION VERSION_LESS "5")
    find_package(Qt4)
    set(_found_var QT4_FOUND)
    set(_error_details "You probably need to set the QT_QMAKE_EXECUTABLE variable.")
    set(_major ${QT_VERSION_MAJOR})
    set(_minor ${QT_VERSION_MINOR})
    set(_patch ${QT_VERSION_PATCH})
    set(_found_prefix "QT_")
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
    if(NOT ${module} STREQUAL "QTCORE"
        AND NOT ${module} STREQUAL "QTGUI"
        AND Slicer_REQUIRED_QT_VERSION VERSION_LESS "5"
        )
      set(QT_USE_${module} ON)
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

# Check QT_QMAKE_EXECUTABLE provided by VTK
set(extra_error_message)
if(DEFINED VTK_QT_QMAKE_EXECUTABLE AND Slicer_REQUIRED_QT_VERSION VERSION_LESS "5")
  #message(STATUS "Checking VTK_QT_QMAKE_EXECUTABLE ...")
  if(NOT EXISTS "${VTK_QT_QMAKE_EXECUTABLE}")
    message(FATAL_ERROR "error: You should probably re-configure VTK. VTK_QT_QMAKE_EXECUTABLE points to a nonexistent executable: ${VTK_QT_QMAKE_EXECUTABLE}")
  endif()
  set(QT_QMAKE_EXECUTABLE ${VTK_QT_QMAKE_EXECUTABLE})
  set(extra_error_message "You should probably reconfigure VTK.")
  __SlicerBlockFindQtAndCheckVersion_find_qt()
endif()

# Check QT_QMAKE_EXECUTABLE provided by CTK
if(DEFINED CTK_QT_QMAKE_EXECUTABLE AND Slicer_REQUIRED_QT_VERSION VERSION_LESS "5")
  if(NOT EXISTS "${CTK_QT_QMAKE_EXECUTABLE}")
    message(FATAL_ERROR "error: You should probably re-configure CTK. CTK_QT_QMAKE_EXECUTABLE points to a nonexistent executable: ${CTK_QT_QMAKE_EXECUTABLE}")
  endif()
  set(QT_QMAKE_EXECUTABLE ${CTK_QT_QMAKE_EXECUTABLE})
  set(extra_error_message "You should probably reconfigure CTK.")
  __SlicerBlockFindQtAndCheckVersion_find_qt()
endif()

if(Slicer_REQUIRED_QT_VERSION VERSION_GREATER "4.9")
  if(DEFINED CTK_Qt5_DIR AND NOT EXISTS "${CTK_Qt5_DIR}")
    message(FATAL_ERROR "error: You should probably re-configure CTK. CTK_Qt5_DIR points to a nonexistent directory: ${CTK_Qt5_DIR}")
  endif()
endif()

__SlicerBlockFindQtAndCheckVersion_find_qt()

if(Slicer_REQUIRED_QT_VERSION VERSION_LESS "5")
  include(${QT_USE_FILE})
endif()

set(_project_name ${Slicer_MAIN_PROJECT_APPLICATION_NAME})
if(NOT Slicer_SOURCE_DIR)
  set(_project_name ${PROJECT_NAME})
endif()
message(STATUS "Configuring ${_project_name} with Qt ${QT_VERSION_MAJOR}.${QT_VERSION_MINOR}.${QT_VERSION_PATCH} (using modules: ${command_separated_module_list})")

# Since Qt5 does not set CMake variables for plugins and binary directories,
# we explicitly set them here.
if(Slicer_REQUIRED_QT_VERSION VERSION_GREATER "4.9")
  set(QT_PLUGINS_DIR "${Qt5_DIR}/../../../plugins")
  get_filename_component(QT_PLUGINS_DIR ${QT_PLUGINS_DIR} ABSOLUTE)
  message(STATUS "Setting QT_PLUGINS_DIR: ${QT_PLUGINS_DIR}")

  set(QT_BINARY_DIR "${Qt5_DIR}/../../../bin")
  get_filename_component(QT_BINARY_DIR ${QT_BINARY_DIR} ABSOLUTE)
  message(STATUS "Setting QT_BINARY_DIR: ${QT_BINARY_DIR}")
endif()
