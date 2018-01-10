
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
# SlicerMacroPythonWrapModuleVTKLibrary
#

macro(SlicerMacroPythonWrapModuleVTKLibrary)
  set(options
    )
  set(oneValueArgs
    NAME
    RELATIVE_PYTHON_DIR
    )
  set(multiValueArgs
    SRCS
    WRAPPED_TARGET_LIBRARIES
    )
  cmake_parse_arguments(PYTHONWRAPMODULEVTKLIBRARY
    "${options}"
    "${oneValueArgs}"
    "${multiValueArgs}"
    ${ARGN}
    )

  if(Slicer_USE_PYTHONQT AND NOT VTK_WRAP_PYTHON)
    message(FATAL_ERROR "Since Slicer_USE_PYTHONQT is ON, VTK_WRAP_PYTHON is expected to be ON. "
                        "Re-configure VTK with python wrapping.")
  endif()

  # --------------------------------------------------------------------------
  # Sanity checks
  # --------------------------------------------------------------------------
  set(expected_defined_vars VTK_LIBRARIES Slicer_Libs_VTK_WRAPPED_LIBRARIES)
  foreach(var ${expected_defined_vars})
    if(NOT DEFINED ${var})
      message(FATAL_ERROR "error: ${var} CMake variable is not defined !")
    endif()
  endforeach()

  set(expected_nonempty_vars NAME SRCS)
  foreach(var ${expected_nonempty_vars})
    if("${PYTHONWRAPMODULEVTKLIBRARY_${var}}" STREQUAL "")
      message(FATAL_ERROR "error: ${var} CMake variable is empty !")
    endif()
  endforeach()

  set(Slicer_Libs_VTK_PYTHON_WRAPPED_LIBRARIES)
  foreach(lib ${Slicer_Libs_VTK_WRAPPED_LIBRARIES})
    list(APPEND Slicer_Libs_VTK_PYTHON_WRAPPED_LIBRARIES ${lib}PythonD)
  endforeach()

  set(PYTHONWRAPMODULEVTKLIBRARY_Wrapped_LIBRARIES
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

endmacro()
