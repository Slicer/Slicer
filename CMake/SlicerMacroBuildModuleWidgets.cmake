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
# SlicerMacroBuildModuleWidgets
#

macro(SlicerMacroBuildModuleWidgets)
  set(options
    WRAP_PYTHONQT
    NO_INSTALL
    )
  set(oneValueArgs
    NAME
    EXPORT_DIRECTIVE
    FOLDER
    )
  set(multiValueArgs
    SRCS
    MOC_SRCS
    UI_SRCS
    INCLUDE_DIRECTORIES
    TARGET_LIBRARIES
    RESOURCES
    )
  CMAKE_PARSE_ARGUMENTS(MODULEWIDGETS
    "${options}"
    "${oneValueArgs}"
    "${multiValueArgs}"
    ${ARGN}
    )

  if(MODULEWIDGETS_UNPARSED_ARGUMENTS)
    message(FATAL_ERROR "Unknown keywords given to SlicerMacroBuildModuleWidgets(): \"${MODULEWIDGETS_UNPARSED_ARGUMENTS}\"")
  endif()

  list(APPEND MODULEWIDGETS_INCLUDE_DIRECTORIES
    ${Slicer_Libs_INCLUDE_DIRS}
    ${Slicer_Base_INCLUDE_DIRS}
    ${Slicer_ModuleLogic_INCLUDE_DIRS}
    ${Slicer_ModuleMRML_INCLUDE_DIRS}
    ${Slicer_ModuleWidgets_INCLUDE_DIRS}
    )

  list(APPEND MODULEWIDGETS_TARGET_LIBRARIES
    ${Slicer_GUI_LIBRARY}
    )

  if(NOT DEFINED MODULEWIDGETS_FOLDER AND DEFINED MODULE_NAME)
    set(MODULEWIDGETS_FOLDER "Module-${MODULE_NAME}")
  endif()
  if(NOT "${MODULEWIDGETS_FOLDER}" STREQUAL "")
    set_target_properties(${lib_name} PROPERTIES FOLDER ${MODULEWIDGETS_FOLDER})
  endif()

  set(MODULEWIDGETS_WRAP_PYTHONQT_OPTION)
  if(MODULEWIDGETS_WRAP_PYTHONQT)
    set(MODULEWIDGETS_WRAP_PYTHONQT_OPTION "WRAP_PYTHONQT")
  endif()
  set(MODULEWIDGETS_NO_INSTALL_OPTION)
  if(MODULEWIDGETS_NO_INSTALL)
    set(MODULEWIDGETS_NO_INSTALL_OPTION "NO_INSTALL")
  endif()

  SlicerMacroBuildModuleQtLibrary(
    NAME ${MODULEWIDGETS_NAME}
    EXPORT_DIRECTIVE ${MODULEWIDGETS_EXPORT_DIRECTIVE}
    FOLDER ${MODULEWIDGETS_FOLDER}
    INCLUDE_DIRECTORIES ${MODULEWIDGETS_INCLUDE_DIRECTORIES}
    SRCS ${MODULEWIDGETS_SRCS}
    MOC_SRCS ${MODULEWIDGETS_MOC_SRCS}
    UI_SRCS ${MODULEWIDGETS_UI_SRCS}
    TARGET_LIBRARIES ${MODULEWIDGETS_TARGET_LIBRARIES}
    RESOURCES ${MODULEWIDGETS_RESOURCES}
    ${MODULEWIDGETS_WRAP_PYTHONQT_OPTION}
    ${MODULEWIDGETS_NO_INSTALL_OPTION}
    )

  set_property(GLOBAL APPEND PROPERTY SLICER_MODULE_WIDGET_TARGETS ${MODULEWIDGETS_NAME})

  #-----------------------------------------------------------------------------
  # Update Slicer_ModuleWidgets_INCLUDE_DIRS
  #-----------------------------------------------------------------------------
  set(Slicer_ModuleWidgets_INCLUDE_DIRS
    ${Slicer_ModuleWidgets_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    CACHE INTERNAL "Slicer Module widgets includes" FORCE)

endmacro()

