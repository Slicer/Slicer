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

macro(slicerMacroBuildScriptedModule)
  set(options
    VERBOSE
    )
  set(oneValueArgs
    NAME
    )
  set(multiValueArgs
    SCRIPTS
    RESOURCES
    )
  CMAKE_PARSE_ARGUMENTS(MY_SLICER
    "${options}"
    "${oneValueArgs}"
    "${multiValueArgs}"
    ${ARGN}
    )
    
  message(STATUS "Configuring Scripted module: ${MY_SLICER_NAME}")
  
  # --------------------------------------------------------------------------
  # Print information helpful for debugging checks
  # --------------------------------------------------------------------------
  if(MY_SLICER_VERBOSE)
    list(APPEND ALL_OPTIONS ${options} ${oneValueArgs} ${multiValueArgs})
    foreach(curr_opt ${ALL_OPTIONS})
      message(STATUS "${curr_opt} = ${MY_SLICER_${curr_opt}}")
    endforeach()
  endif()

  # --------------------------------------------------------------------------
  # Sanity checks
  # --------------------------------------------------------------------------
  if(MY_SLICER_UNPARSED_ARGUMENTS)
    message(FATAL_ERROR "Unknown keywords given to slicerMacroBuildScriptedModule(): \"${MY_SLICER_UNPARSED_ARGUMENTS}\"")
  endif()

  if(NOT DEFINED MY_SLICER_NAME)
    message(FATAL_ERROR "NAME is mandatory")
  endif()

  set(expected_existing_vars SCRIPTS RESOURCES)
  foreach(var ${expected_existing_vars})
    foreach(value ${MY_SLICER_${var}})
      if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${value}"
         AND NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${value}.py")
        message(FATAL_ERROR "Variable ${var} contains a value pointing to an inexistent directory or file ! [${CMAKE_CURRENT_SOURCE_DIR}/${value}]")
      endif()
    endforeach()
  endforeach()

  ctkMacroCompilePythonScript(
    TARGET_NAME ${MY_SLICER_NAME}
    SCRIPTS "${MY_SLICER_SCRIPTS}"
    RESOURCES "${MY_SLICER_RESOURCES}"
    DESTINATION_DIR ${CMAKE_BINARY_DIR}/${Slicer_QTSCRIPTEDMODULES_LIB_DIR}
    INSTALL_DIR ${Slicer_INSTALL_QTSCRIPTEDMODULES_LIB_DIR}
    NO_INSTALL_SUBDIR
    )

endmacro()

