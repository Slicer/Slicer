
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
# SlicerMacroPythonWrapModuleVTKLibrary
#

MACRO(SlicerMacroPythonWrapModuleVTKLibrary)
  SLICER_PARSE_ARGUMENTS(PYTHONWRAPMODULEVTKLIBRARY
    "NAME;SRCS;WRAPPED_TARGET_LIBRARIES;RELATIVE_PYTHON_DIR"
    ""
    ${ARGN}
    )
    
  # --------------------------------------------------------------------------
  # Sanity checks
  # --------------------------------------------------------------------------
  SET(expected_defined_vars VTK_LIBRARIES Slicer_Libs_VTK_WRAPPED_LIBRARIES)
  FOREACH(var ${expected_defined_vars})
    IF(NOT DEFINED ${var})
      MESSAGE(FATAL_ERROR "error: ${var} CMake variable is not defined !")
    ENDIF()
  ENDFOREACH()
  
  SET(expected_nonempty_vars NAME SRCS)
  FOREACH(var ${expected_nonempty_vars})
    IF("${PYTHONWRAPMODULEVTKLIBRARY_${var}}" STREQUAL "")
      MESSAGE(FATAL_ERROR "error: ${var} CMake variable is empty !")
    ENDIF()
  ENDFOREACH()

  SET(VTK_PYTHON_WRAPPED_LIBRARIES)
  FOREACH(lib ${VTK_LIBRARIES})
    LIST(APPEND VTK_PYTHON_WRAPPED_LIBRARIES ${lib}PythonD)
  ENDFOREACH()
  
  SET(Slicer_Libs_VTK_PYTHON_WRAPPED_LIBRARIES)
  FOREACH(lib ${Slicer_Libs_VTK_WRAPPED_LIBRARIES})
    LIST(APPEND Slicer_Libs_VTK_PYTHON_WRAPPED_LIBRARIES ${lib}PythonD)
  ENDFOREACH()

  SET(PYTHONWRAPMODULEVTKLIBRARY_Wrapped_LIBRARIES
    ${VTK_PYTHON_WRAPPED_LIBRARIES}
    ${Slicer_Libs_VTK_PYTHON_WRAPPED_LIBRARIES}
    ${PYTHONWRAPMODULEVTKLIBRARY_WRAPPED_TARGET_LIBRARIES}
    )

  vtkMacroKitPythonWrap(
    KIT_NAME ${PYTHONWRAPMODULEVTKLIBRARY_NAME}
    KIT_SRCS ${PYTHONWRAPMODULEVTKLIBRARY_SRCS}
    KIT_INSTALL_BIN_DIR ${Slicer_INSTALL_QTLOADABLEMODULES_BIN_DIR}
    KIT_INSTALL_LIB_DIR ${Slicer_INSTALL_QTLOADABLEMODULES_LIB_DIR}
    KIT_PYTHON_LIBRARIES ${PYTHONWRAPMODULEVTKLIBRARY_Wrapped_LIBRARIES}
    )

  # Generate "Python/<lib_name>.py" file
  FILE(WRITE ${CMAKE_CURRENT_BINARY_DIR}/Python/${PYTHONWRAPMODULEVTKLIBRARY_RELATIVE_PYTHON_DIR}/${lib_name}.py "
\"\"\" This module loads all the classes from the ${lib_name} library into its
namespace.\"\"\"

from ${lib_name}Python import *
")

  FILE(GLOB PYFILES
    RELATIVE "${CMAKE_CURRENT_BINARY_DIR}/Python"
    "${CMAKE_CURRENT_BINARY_DIR}/Python/${PYTHONWRAPMODULEVTKLIBRARY_RELATIVE_PYTHON_DIR}/*.py")
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
    
ENDMACRO()
