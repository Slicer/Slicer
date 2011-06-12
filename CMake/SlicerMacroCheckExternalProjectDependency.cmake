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

MACRO(SlicerMacroCheckExternalProjectDependency proj)
  # Set indent variable if needed
  IF(NOT DEFINED __indent)
    SET(__indent "")
  ELSE()
    SET(__indent "${__indent}  ")
  ENDIF()
  
  # Sanity checks
  IF(NOT DEFINED ${proj}_DEPENDENCIES)
    message(FATAL_ERROR "${__indent}${proj}_DEPENDENCIES variable is NOT defined !")
  ENDIF()
  
  # Display dependency of project being processed
  IF("${${proj}_DEPENDENCIES}" STREQUAL "")
    MESSAGE(STATUS "SuperBuild - ${__indent}${proj}[OK]")
  ELSE()
    SET(dependency_str " ")
    FOREACH(dep ${${proj}_DEPENDENCIES})
      IF(External_${dep}_FILE_INCLUDED)
        SET(dependency_str "${dependency_str}${dep}[INCLUDED], ")
      ELSE()
        SET(dependency_str "${dependency_str}${dep}, ")
      ENDIF()
    ENDFOREACH()
    MESSAGE(STATUS "SuperBuild - ${__indent}${proj} => Requires${dependency_str}")
  ENDIF()
  
  # Include dependencies
  FOREACH(dep ${${proj}_DEPENDENCIES})
    IF(NOT External_${dep}_FILE_INCLUDED)
      INCLUDE(${Slicer_SOURCE_DIR}/SuperBuild/External_${dep}.cmake)
    ENDIF()
  ENDFOREACH()
  
  # If project being process has dependencies, indicates it has also been added.
  IF(NOT "${${proj}_DEPENDENCIES}" STREQUAL "")
    MESSAGE(STATUS "SuperBuild - ${__indent}${proj}[OK]")
  ENDIF()
  
  # Update indent variable
  STRING(LENGTH "${__indent}" __indent_length)
  MATH(EXPR __indent_length "${__indent_length}-2")
  IF(NOT ${__indent_length} LESS 0)
    STRING(SUBSTRING "${__indent}" 0 ${__indent_length} __indent)
  ENDIF()
ENDMACRO()
