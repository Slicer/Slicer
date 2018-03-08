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
# SlicerMacroBuildModuleLogic
#

macro(SlicerMacroBuildModuleLogic)
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
  cmake_parse_arguments(MODULELOGIC
    "${options}"
    "${oneValueArgs}"
    "${multiValueArgs}"
    ${ARGN}
    )

  if(MODULELOGIC_UNPARSED_ARGUMENTS)
    message(FATAL_ERROR "Unknown keywords given to SlicerMacroBuildModuleLogic(): \"${MODULELOGIC_UNPARSED_ARGUMENTS}\"")
  endif()

  list(APPEND MODULELOGIC_INCLUDE_DIRECTORIES
    ${Slicer_Libs_INCLUDE_DIRS}
    ${Slicer_Base_INCLUDE_DIRS}
    ${Slicer_ModuleLogic_INCLUDE_DIRS}
    ${Slicer_ModuleMRML_INCLUDE_DIRS}
    )

  if(Slicer_BUILD_CLI_SUPPORT)
    # Third-party library
    find_package(SlicerExecutionModel REQUIRED ModuleDescriptionParser)
    list(APPEND MODULELOGIC_INCLUDE_DIRECTORIES
      ${ModuleDescriptionParser_INCLUDE_DIRS}
      )

    # Note: Linking against qSlicerBaseQTCLI provides logic with
    #       access to the core application modulemanager. Using the module manager
    #       a module logic can then use the services provided by registrered
    #       command line module (CLI).

    list(APPEND MODULELOGIC_TARGET_LIBRARIES
      qSlicerBaseQTCLI
      )
  else()
    list(APPEND MODULELOGIC_TARGET_LIBRARIES
      SlicerBaseLogic
      MRMLDisplayableManager
      )
  endif()

  if(NOT DEFINED MODULELOGIC_FOLDER AND DEFINED MODULE_NAME)
    set(MODULELOGIC_FOLDER "Module-${MODULE_NAME}")
  endif()

  set(MODULELOGIC_NO_INSTALL_OPTION)
  if(MODULELOGIC_NO_INSTALL)
    set(MODULELOGIC_NO_INSTALL_OPTION "NO_INSTALL")
  endif()

  SlicerMacroBuildModuleVTKLibrary(
    NAME ${MODULELOGIC_NAME}
    EXPORT_DIRECTIVE ${MODULELOGIC_EXPORT_DIRECTIVE}
    FOLDER ${MODULELOGIC_FOLDER}
    SRCS ${MODULELOGIC_SRCS}
    INCLUDE_DIRECTORIES ${MODULELOGIC_INCLUDE_DIRECTORIES}
    TARGET_LIBRARIES ${MODULELOGIC_TARGET_LIBRARIES}
    ${MODULELOGIC_NO_INSTALL_OPTION}
    )

  set_property(GLOBAL APPEND PROPERTY SLICER_MODULE_LOGIC_TARGETS ${MODULELOGIC_NAME})

  #-----------------------------------------------------------------------------
  # Update Slicer_ModuleLogic_INCLUDE_DIRS
  #-----------------------------------------------------------------------------
  set(Slicer_ModuleLogic_INCLUDE_DIRS
    ${Slicer_ModuleLogic_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    CACHE INTERNAL "Slicer Module logic includes" FORCE)

  # --------------------------------------------------------------------------
  # Python wrapping
  # --------------------------------------------------------------------------
  if(NOT ${MODULELOGIC_DISABLE_WRAP_PYTHON} AND VTK_WRAP_PYTHON AND BUILD_SHARED_LIBS)

    set(Slicer_Wrapped_LIBRARIES
      SlicerBaseLogicPythonD
      )

    foreach(library ${MODULELOGIC_TARGET_LIBRARIES})
      if(TARGET ${library}PythonD)
        list(APPEND Slicer_Wrapped_LIBRARIES ${library}PythonD)
      endif()
    endforeach()

    SlicerMacroPythonWrapModuleVTKLibrary(
      NAME ${MODULELOGIC_NAME}
      SRCS ${MODULELOGIC_SRCS}
      WRAPPED_TARGET_LIBRARIES ${Slicer_Wrapped_LIBRARIES}
      RELATIVE_PYTHON_DIR "."
      )

    # Set python module logic output
    set_target_properties(${MODULELOGIC_NAME}Python ${MODULELOGIC_NAME}PythonD PROPERTIES
      RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${Slicer_QTLOADABLEMODULES_BIN_DIR}"
      LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${Slicer_QTLOADABLEMODULES_LIB_DIR}"
      ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${Slicer_QTLOADABLEMODULES_LIB_DIR}"
      )

    if(NOT "${MODULELOGIC_FOLDER}" STREQUAL "")
      set_target_properties(${MODULELOGIC_NAME}Python PROPERTIES FOLDER ${MODULELOGIC_FOLDER})
      set_target_properties(${MODULELOGIC_NAME}PythonD PROPERTIES FOLDER ${MODULELOGIC_FOLDER})
      if(TARGET ${MODULELOGIC_NAME}Hierarchy)
        set_target_properties(${MODULELOGIC_NAME}Hierarchy PROPERTIES FOLDER ${MODULELOGIC_FOLDER})
      endif()
    endif()

    # Export target
    set_property(GLOBAL APPEND PROPERTY Slicer_TARGETS ${MODULELOGIC_NAME}Python ${MODULELOGIC_NAME}PythonD)
  endif()

endmacro()
