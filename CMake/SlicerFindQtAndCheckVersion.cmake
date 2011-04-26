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

SET(extra_error_message)
IF(NOT DEFINED QT_QMAKE_EXECUTABLE AND EXISTS "${VTK_QT_QMAKE_EXECUTABLE}")
  SET(QT_QMAKE_EXECUTABLE ${VTK_QT_QMAKE_EXECUTABLE})
  SET(extra_error_message "You should probably reconfigure VTK.")
ENDIF()

FIND_PACKAGE(Qt4)
IF(NOT QT4_FOUND)
  MESSAGE(FATAL_ERROR "error: Qt >= ${Slicer_REQUIRED_QT_VERSION} was not found on your system. You probably need to set the QT_QMAKE_EXECUTABLE variable.")
ENDIF()

# Check version, note that ${QT_VERSION_PATCH} could also be used
IF("${QT_VERSION_MAJOR}.${QT_VERSION_MINOR}.${QT_VERSION_PATCH}" VERSION_LESS "${Slicer_REQUIRED_QT_VERSION}")
  MESSAGE(FATAL_ERROR "error: Slicer requires Qt >= ${Slicer_REQUIRED_QT_VERSION} -- you cannot use Qt ${QT_VERSION_MAJOR}.${QT_VERSION_MINOR}.${QT_VERSION_PATCH}. ${extra_error_message}")
ENDIF()
