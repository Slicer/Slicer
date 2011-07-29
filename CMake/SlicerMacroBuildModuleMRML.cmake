################################################################################
#
#  Program: 3D Slicer
#
#  Copyright (c) 2010 Kitware Inc.
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
# SlicerMacroBuildModuleMRML
#

macro(SlicerMacroBuildModuleMRML)
  SLICER_PARSE_ARGUMENTS(MODULEMRML
    "NAME;EXPORT_DIRECTIVE;SRCS;INCLUDE_DIRECTORIES;TARGET_LIBRARIES"
    "DISABLE_WRAP_PYTHON;NO_INSTALL"
    ${ARGN}
    )

  list(APPEND MODULEMRML_INCLUDE_DIRECTORIES
    ${Slicer_Libs_INCLUDE_DIRS}
    ${Slicer_ModuleMRML_INCLUDE_DIRS}
    )

  set(MODULEMRML_NO_INSTALL_OPTION)
  if(MODULEMRML_NO_INSTALL)
    set(MODULEMRML_NO_INSTALL_OPTION "NO_INSTALL")
  endif()

  SlicerMacroBuildModuleVTKLibrary(
    NAME ${MODULEMRML_NAME}
    EXPORT_DIRECTIVE ${MODULEMRML_EXPORT_DIRECTIVE}
    SRCS ${MODULEMRML_SRCS}
    INCLUDE_DIRECTORIES ${MODULEMRML_INCLUDE_DIRECTORIES}
    TARGET_LIBRARIES ${MODULEMRML_TARGET_LIBRARIES}
    ${MODULEMRML_NO_INSTALL_OPTION}
    )

  #-----------------------------------------------------------------------------
  # Update Slicer_ModuleMRML_INCLUDE_DIRS
  #-----------------------------------------------------------------------------
  if(Slicer_SOURCE_DIR)
    set(Slicer_ModuleMRML_INCLUDE_DIRS
      ${Slicer_ModuleMRML_INCLUDE_DIRS}
      ${CMAKE_CURRENT_SOURCE_DIR}
      ${CMAKE_CURRENT_BINARY_DIR}
      CACHE INTERNAL "Slicer Module MRML includes" FORCE)
  endif()

  # --------------------------------------------------------------------------
  # Python wrapping
  # --------------------------------------------------------------------------
  if(NOT ${MODULEMRML_DISABLE_WRAP_PYTHON} AND VTK_WRAP_PYTHON)

    set(Slicer_Wrapped_LIBRARIES
      )

    SlicerMacroPythonWrapModuleVTKLibrary(
      NAME ${MODULEMRML_NAME}
      SRCS ${MODULEMRML_SRCS}
      WRAPPED_TARGET_LIBRARIES ${Slicer_Wrapped_LIBRARIES}
      RELATIVE_PYTHON_DIR "."
      )

    # Set python module logic output
    set_target_properties(${MODULEMRML_NAME}Python ${MODULEMRML_NAME}PythonD PROPERTIES
      RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${Slicer_QTLOADABLEMODULES_BIN_DIR}"
      LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${Slicer_QTLOADABLEMODULES_LIB_DIR}"
      ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${Slicer_QTLOADABLEMODULES_LIB_DIR}"
      )

    # Export target
    set_property(GLOBAL APPEND PROPERTY Slicer_TARGETS ${MODULEMRML_NAME}Python ${MODULEMRML_NAME}PythonD)
  endif()

endmacro()
