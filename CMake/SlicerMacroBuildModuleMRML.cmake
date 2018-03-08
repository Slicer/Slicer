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
# SlicerMacroBuildModuleMRML
#

macro(SlicerMacroBuildModuleMRML)
  set(options
    DISABLE_WRAP_PYTHON
    NO_INSTALL
    )
  set(oneValueArgs
    NAME
    EXPORT_DIRECTIVE
    FOLDER
    )
  set(multiValueArgs
    SRCS
    INCLUDE_DIRECTORIES
    TARGET_LIBRARIES
    )
  cmake_parse_arguments(MODULEMRML
    "${options}"
    "${oneValueArgs}"
    "${multiValueArgs}"
    ${ARGN}
    )

  if(MODULEMRML_UNPARSED_ARGUMENTS)
    message(FATAL_ERROR "Unknown keywords given to SlicerMacroBuildModuleMRML(): \"${MODULEMRML_UNPARSED_ARGUMENTS}\"")
  endif()

  list(APPEND MODULEMRML_INCLUDE_DIRECTORIES
    ${Slicer_Libs_INCLUDE_DIRS}
    ${Slicer_ModuleMRML_INCLUDE_DIRS}
    )

  if(NOT DEFINED MODULEMRML_FOLDER AND DEFINED MODULE_NAME)
    set(MODULEMRML_FOLDER "Module-${MODULE_NAME}")
  endif()

  set(MODULEMRML_NO_INSTALL_OPTION)
  if(MODULEMRML_NO_INSTALL)
    set(MODULEMRML_NO_INSTALL_OPTION "NO_INSTALL")
  endif()

  SlicerMacroBuildModuleVTKLibrary(
    NAME ${MODULEMRML_NAME}
    EXPORT_DIRECTIVE ${MODULEMRML_EXPORT_DIRECTIVE}
    FOLDER ${MODULEMRML_FOLDER}
    SRCS ${MODULEMRML_SRCS}
    INCLUDE_DIRECTORIES ${MODULEMRML_INCLUDE_DIRECTORIES}
    TARGET_LIBRARIES ${MODULEMRML_TARGET_LIBRARIES}
    ${MODULEMRML_NO_INSTALL_OPTION}
    )

  set_property(GLOBAL APPEND PROPERTY SLICER_MODULE_MRML_TARGETS ${MODULEMRML_NAME})

  #-----------------------------------------------------------------------------
  # Update Slicer_ModuleMRML_INCLUDE_DIRS
  #-----------------------------------------------------------------------------
  set(Slicer_ModuleMRML_INCLUDE_DIRS
    ${Slicer_ModuleMRML_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    CACHE INTERNAL "Slicer Module MRML includes" FORCE)

  # --------------------------------------------------------------------------
  # Python wrapping
  # --------------------------------------------------------------------------
  if(NOT ${MODULEMRML_DISABLE_WRAP_PYTHON} AND VTK_WRAP_PYTHON AND BUILD_SHARED_LIBS)

    set(Slicer_Wrapped_LIBRARIES
      )

    foreach(library ${MODULEMRML_TARGET_LIBRARIES})
      if(TARGET ${library}PythonD)
        list(APPEND Slicer_Wrapped_LIBRARIES ${library}PythonD)
      endif()
    endforeach()

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

    if(NOT "${MODULEMRML_FOLDER}" STREQUAL "")
      set_target_properties(${MODULEMRML_NAME}Python PROPERTIES FOLDER ${MODULEMRML_FOLDER})
      set_target_properties(${MODULEMRML_NAME}PythonD PROPERTIES FOLDER ${MODULEMRML_FOLDER})
      if(TARGET ${MODULEMRML_NAME}Hierarchy)
        set_target_properties(${MODULEMRML_NAME}Hierarchy PROPERTIES FOLDER ${MODULEMRML_FOLDER})
      endif()
    endif()

    # Export target
    set_property(GLOBAL APPEND PROPERTY Slicer_TARGETS ${MODULEMRML_NAME}Python ${MODULEMRML_NAME}PythonD)
  endif()

endmacro()
