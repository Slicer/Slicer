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
# SlicerMacroBuildModuleLogic
#

MACRO(SlicerMacroBuildModuleLogic)
  SLICER_PARSE_ARGUMENTS(MODULELOGIC
    "NAME;EXPORT_DIRECTIVE;SRCS;INCLUDE_DIRECTORIES;TARGET_LIBRARIES"
    "DISABLE_WRAP_PYTHON"
    ${ARGN}
    )

  # --------------------------------------------------------------------------
  # Sanity checks
  # --------------------------------------------------------------------------
  SET(expected_defined_vars NAME EXPORT_DIRECTIVE)
  FOREACH(var ${expected_defined_vars})
    IF(NOT DEFINED MODULELOGIC_${var})
      MESSAGE(FATAL_ERROR "${var} is mandatory")
    ENDIF()
  ENDFOREACH()
  
  # --------------------------------------------------------------------------
  # Define library name
  # --------------------------------------------------------------------------
  SET(lib_name ${MODULELOGIC_NAME})
  
  # --------------------------------------------------------------------------
  # Include dirs
  # --------------------------------------------------------------------------
  INCLUDE_DIRECTORIES(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${Slicer_Libs_INCLUDE_DIRS}
    ${Slicer_Base_INCLUDE_DIRS}
    ${Slicer_ModuleLogic_INCLUDE_DIRS}
    ${MODULELOGIC_INCLUDE_DIRECTORIES}
    )

  #-----------------------------------------------------------------------------
  # Update Slicer_ModuleLogic_INCLUDE_DIRS
  #-----------------------------------------------------------------------------
  SET(Slicer_ModuleLogic_INCLUDE_DIRS ${Slicer_ModuleLogic_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR} CACHE INTERNAL "Slicer Module logic includes" FORCE)

  #-----------------------------------------------------------------------------
  # Configure export header
  #-----------------------------------------------------------------------------
  SET(MY_LIBRARY_EXPORT_DIRECTIVE ${MODULELOGIC_EXPORT_DIRECTIVE})
  SET(MY_EXPORT_HEADER_PREFIX ${MODULELOGIC_NAME})
  SET(MY_LIBNAME ${lib_name})
  
  # Sanity checks
  IF (NOT EXISTS ${Slicer_EXPORT_HEADER_TEMPLATE})
    MESSAGE(FATAL_ERROR "error: Slicer_EXPORT_HEADER_TEMPLATE doesn't exist: ${Slicer_EXPORT_HEADER_TEMPLATE}")
  ENDIF()
  
  CONFIGURE_FILE(
    ${Slicer_EXPORT_HEADER_TEMPLATE}
    ${CMAKE_CURRENT_BINARY_DIR}/${MY_EXPORT_HEADER_PREFIX}Export.h
    )
  SET(dynamicHeaders
    "${dynamicHeaders};${CMAKE_CURRENT_BINARY_DIR}/${MY_EXPORT_HEADER_PREFIX}Export.h")

  #-----------------------------------------------------------------------------
  # Source groups
  # --------------------------------------------------------------------------
  SOURCE_GROUP("Generated" FILES
    ${dynamicHeaders}
    )

  # --------------------------------------------------------------------------
  # Build library
  # --------------------------------------------------------------------------
  ADD_LIBRARY(${lib_name}
    ${MODULELOGIC_SRCS}
    )

  # Set loadable modules output path
  SET_TARGET_PROPERTIES(${lib_name}
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY
    "${CMAKE_BINARY_DIR}/${Slicer_INSTALL_QTLOADABLEMODULES_BIN_DIR}"
    LIBRARY_OUTPUT_DIRECTORY
    "${CMAKE_BINARY_DIR}/${Slicer_INSTALL_QTLOADABLEMODULES_LIB_DIR}"
    ARCHIVE_OUTPUT_DIRECTORY
    "${CMAKE_BINARY_DIR}/${Slicer_INSTALL_QTLOADABLEMODULES_LIB_DIR}"
    )

  TARGET_LINK_LIBRARIES(${lib_name}
    ${Slicer_Libs_LIBRARIES}
    SlicerBaseLogic
    SlicerBaseCLI
    ${Slicer_ModuleLogic_LIBRARIES}
    ${MODULELOGIC_TARGET_LIBRARIES}
    )

  # Apply user-defined properties to the library target.
  IF(Slicer_LIBRARY_PROPERTIES)
    SET_TARGET_PROPERTIES(${lib_name} PROPERTIES ${Slicer_LIBRARY_PROPERTIES})
  ENDIF()
  
  # --------------------------------------------------------------------------
  # Update Slicer_ModuleLogic_LIBRARIES
  # --------------------------------------------------------------------------
  SET(Slicer_ModuleLogic_LIBRARIES ${Slicer_ModuleLogic_LIBRARIES} ${lib_name} CACHE INTERNAL "Slicer Module logic libraries" FORCE)
  
  # --------------------------------------------------------------------------
  # Install library
  # --------------------------------------------------------------------------
  INSTALL(TARGETS ${lib_name}
    RUNTIME DESTINATION ${Slicer_INSTALL_QTLOADABLEMODULES_BIN_DIR} COMPONENT RuntimeLibraries 
    LIBRARY DESTINATION ${Slicer_INSTALL_QTLOADABLEMODULES_LIB_DIR} COMPONENT RuntimeLibraries
    ARCHIVE DESTINATION ${Slicer_INSTALL_QTLOADABLEMODULES_LIB_DIR} COMPONENT Development
    )

  # --------------------------------------------------------------------------
  # Install headers
  # --------------------------------------------------------------------------
  IF(DEFINED Slicer_DEVELOPMENT_INSTALL)
    IF(NOT DEFINED ${MODULELOGIC_NAME}_DEVELOPMENT_INSTALL)
      SET(${MODULELOGIC_NAME}_DEVELOPMENT_INSTALL ${Slicer_DEVELOPMENT_INSTALL})
    ENDIF()
  ELSE()
    IF (NOT DEFINED ${MODULELOGIC_NAME}_DEVELOPMENT_INSTALL)
      SET(${MODULELOGIC_NAME}_DEVELOPMENT_INSTALL OFF)
    ENDIF()
  ENDIF()
  
  IF(${MODULELOGIC_NAME}_DEVELOPMENT_INSTALL)
    FILE(GLOB headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
    INSTALL(FILES
      ${headers}
      ${dynamicHeaders}
      DESTINATION ${Slicer_INSTALL_QTLOADABLEMODULES_INCLUDE_DIR}/${MODULELOGIC_NAME} COMPONENT Development
      )
  ENDIF()
  
  IF(NOT ${MODULELOGIC_DISABLE_WRAP_PYTHON} AND VTK_WRAP_PYTHON AND BUILD_SHARED_LIBS)

    # TODO: A parameter named 'WRAPPED_TARGET_LIBRARIES'
    # should be added to the macro.
    # VTK wrapped libraries
    SET(VTK_WRAPPED_LIBRARIES
      vtkVolumeRenderingPythonD
      )

    # TODO: Slicer's Use file should export this list automatically
    # (currently the wrapped and non-wrapped libs are mixed together)
    # See "HACK" above
    SET(Slicer_Wrapped_LIBRARIES
      vtkTeemPythonD vtkITKPythonD FreeSurferPythonD MRMLPythonD MRMLCLIPythonD MRMLLogicPythonD MRMLDisplayableManagerPythonD
      RemoteIOPythonD SlicerBaseLogicPythonD
      )
    LIST(APPEND Slicer_Wrapped_LIBRARIES ${VTK_WRAPPED_LIBRARIES})

    vtkMacroKitPythonWrap(
      KIT_NAME ${lib_name}
      KIT_SRCS ${MODULELOGIC_SRCS}
      KIT_INSTALL_BIN_DIR ${Slicer_INSTALL_QTLOADABLEMODULES_BIN_DIR}
      KIT_INSTALL_LIB_DIR ${Slicer_INSTALL_QTLOADABLEMODULES_LIB_DIR}
      KIT_PYTHON_LIBRARIES ${Slicer_Wrapped_LIBRARIES}
      )

    # Generate "Python/<lib_name>.py" file
    FILE(WRITE ${CMAKE_CURRENT_BINARY_DIR}/Python/slicer/modulelogic/${lib_name}.py "
\"\"\" This module loads all the classes from the ${lib_name} library into its
namespace.\"\"\"

from ${lib_name}Python import *
")

    FILE(GLOB PYFILES
      RELATIVE "${CMAKE_CURRENT_BINARY_DIR}/Python"
      "${CMAKE_CURRENT_BINARY_DIR}/Python/slicer/modulelogic/*.py")
    if ( PYFILES )
      ctkMacroCompilePythonScript(
        TARGET_NAME ${lib_name}
        SCRIPTS "${PYFILES}"
        RESOURCES ""
        SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/Python
        DESTINATION_DIR ${Slicer_BINARY_DIR}/bin/Python
        INSTALL_DIR ${Slicer_INSTALL_BIN_DIR}
        )
    ENDIF()

  ENDIF()
    
ENDMACRO()
