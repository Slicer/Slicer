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
#  This file was originally developed by Johan Andruejol, Kitware Inc.
#  and was partially funded by NIH grant 3P41RR013218-12S1
#
################################################################################

#
# Simple wrapper around ctkMacroBuildQtPlugin to allow external project
# to easily build slicers plugins.
#
# Depends on:
#  cmake_parse_arguments ( >= CMake 2.8.3)
#

macro(slicerMacroBuildQtPlugin)
  cmake_parse_arguments(MY
    "" # no options
    "NAME;EXPORT_DIRECTIVE;PLUGIN_DIR" # one value args
    "SRCS;MOC_SRCS;UI_FORMS;INCLUDE_DIRECTORIES;TARGET_LIBRARIES;RESOURCES" # multi value args
    ${ARGN}
    )

  if(NOT DEFINED CMAKE_LIBRARY_OUTPUT_DIRECTORY)
    message("CMAKE_LIBRARY_OUTPUT_DIRECTORY not defined. "
            "Defaulting to CMAKE_CURRENT_BINARY_DIR: ${CMAKE_CURRENT_BINARY_DIR}")
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
  endif()

  ctkMacroBuildQtPlugin(
    NAME ${MY_NAME}
    EXPORT_DIRECTIVE ${MY_EXPORT_DIRECTIVE}
    PLUGIN_DIR ${MY_PLUGIN_DIR}
    SRCS ${MY_SRCS}
    MOC_SRCS ${MY_MOC_SRCS}
    UI_FORMS ${MY_UI_FORMS}
    TARGET_LIBRARIES ${MY_TARGET_LIBRARIES} ${Slicer_GUI_LIBRARY}
    INCLUDE_DIRECTORIES ${MY_INCLUDE_DIRECTORIES} ${Slicer_Base_INCLUDE_DIRS}
    RESOURCES ${MY_RESOURCES}
    )
endmacro()

macro(slicerMacroBuildQtDesignerPlugin)
  slicerMacroBuildQtPlugin(
    PLUGIN_DIR designer
    ${ARGN})
endmacro()

macro(slicerMacroBuildQtIconEnginesPlugin)
  slicerMacroBuildQtPlugin(
    PLUGIN_DIR iconengines
    ${ARGN})
endmacro()

macro(slicerMacroBuildQtStylesPlugin)
  slicerMacroBuildQtPlugin(
    PLUGIN_DIR styles
    ${ARGN})
endmacro()
