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
# The CMake code used to find Qt4 has been factored out into this CMake script so that 
# it can be used in both Slicer/CMakelists.txt and Slicer/UseSlicer.cmake
#

MACRO(__SlicerBlockFindQtAndCheckVersion_find_qt)
  FIND_PACKAGE(Qt4)
  IF(NOT QT4_FOUND)
    MESSAGE(FATAL_ERROR "error: Qt >= ${Slicer_REQUIRED_QT_VERSION} was not found on your system. You probably need to set the QT_QMAKE_EXECUTABLE variable.")
  ENDIF()

  # Check version
  IF("${QT_VERSION_MAJOR}.${QT_VERSION_MINOR}.${QT_VERSION_PATCH}" VERSION_LESS "${Slicer_REQUIRED_QT_VERSION}")
    MESSAGE(FATAL_ERROR "error: Slicer requires Qt >= ${Slicer_REQUIRED_QT_VERSION} -- you cannot use Qt ${QT_VERSION_MAJOR}.${QT_VERSION_MINOR}.${QT_VERSION_PATCH}. ${extra_error_message}")
  ENDIF()

  set(command_separated_module_list)
  # Check if all expected Qt modules have been discovered
  FOREACH(module ${Slicer_REQUIRED_QT_MODULES})
    IF(NOT ${QT_QT${module}_FOUND})
      MESSAGE(FATAL_ERROR "error: ")
    ENDIF()
    IF(NOT ${module} STREQUAL "CORE" AND NOT ${module} STREQUAL "GUI")
      SET(QT_USE_QT${module} ON)
    ENDIF()
    set(command_separated_module_list "${command_separated_module_list}${module}, ")
  ENDFOREACH()
ENDMACRO()

# Sanity checks - Check if variable are defined
SET(expected_defined_vars
  Slicer_REQUIRED_QT_VERSION 
  Slicer_REQUIRED_QT_MODULES
  )
FOREACH(v ${expected_defined_vars})
  IF(NOT DEFINED ${v})
    MESSAGE(FATAL_ERROR "error: ${v} CMake variable is not defined.")
  ENDIF()
ENDFOREACH()

# Check QT_QMAKE_EXECUTABLE provided by VTK
SET(extra_error_message)
IF(DEFINED VTK_QT_QMAKE_EXECUTABLE)
  MESSAGE(STATUS "Checking VTK_QT_QMAKE_EXECUTABLE ...")
  IF(NOT EXISTS "${VTK_QT_QMAKE_EXECUTABLE}")
    MESSAGE(FATAL_ERROR "error: You should probably re-configure VTK. VTK_QT_QMAKE_EXECUTABLE points to a nonexistent executable: ${VTK_QT_QMAKE_EXECUTABLE}")
  ENDIF()
  SET(QT_QMAKE_EXECUTABLE ${VTK_QT_QMAKE_EXECUTABLE})
  SET(extra_error_message "You should probably reconfigure VTK.")
  __SlicerBlockFindQtAndCheckVersion_find_qt()
  SET(qt_version_discovered_using_vtk "${QT_VERSION_MAJOR}.${QT_VERSION_MINOR}.${QT_VERSION_PATCH}")
ENDIF()

# Check QT_QMAKE_EXECUTABLE provided by CTK
IF(DEFINED CTK_QT_QMAKE_EXECUTABLE)
  MESSAGE(STATUS "Checking CTK_QT_QMAKE_EXECUTABLE ...")
  IF(NOT EXISTS "${CTK_QT_QMAKE_EXECUTABLE}")
    MESSAGE(FATAL_ERROR "error: You should probably re-configure CTK. CTK_QT_QMAKE_EXECUTABLE points to a nonexistent executable: ${CTK_QT_QMAKE_EXECUTABLE}")
  ENDIF()
  SET(QT_QMAKE_EXECUTABLE ${CTK_QT_QMAKE_EXECUTABLE})
  SET(extra_error_message "You should probably reconfigure VTK.")
  __SlicerBlockFindQtAndCheckVersion_find_qt()
  SET(qt_version_discovered_using_ctk "${QT_VERSION_MAJOR}.${QT_VERSION_MINOR}.${QT_VERSION_PATCH}")
ENDIF()

__SlicerBlockFindQtAndCheckVersion_find_qt()

INCLUDE(${QT_USE_FILE})

MESSAGE(STATUS "Configuring ${PROJECT_NAME} with Qt ${QT_VERSION_MAJOR}.${QT_VERSION_MINOR}.${QT_VERSION_PATCH} (using modules: ${command_separated_module_list})")
