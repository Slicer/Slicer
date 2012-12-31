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

if(NOT EXISTS "${EXTERNAL_PROJECT_DIR}")
  set(EXTERNAL_PROJECT_DIR ${${CMAKE_PROJECT_NAME}_SOURCE_DIR}/SuperBuild)
endif()

macro(SlicerMacroCheckExternalProjectDependency proj)
  # Set indent variable if needed
  if(NOT DEFINED __indent)
    set(__indent "")
  else()
    set(__indent "${__indent}  ")
  endif()

  # Sanity checks
  if(NOT DEFINED ${proj}_DEPENDENCIES)
    message(FATAL_ERROR "${__indent}${proj}_DEPENDENCIES variable is NOT defined !")
  endif()

  # Display dependency of project being processed
  if("${${proj}_DEPENDENCIES}" STREQUAL "")
    message(STATUS "SuperBuild - ${__indent}${proj}[OK]")
  else()
    set(dependency_str " ")
    foreach(dep ${${proj}_DEPENDENCIES})
      if(External_${dep}_FILE_INCLUDED)
        set(dependency_str "${dependency_str}${dep}[INCLUDED], ")
      else()
        set(dependency_str "${dependency_str}${dep}, ")
      endif()
    endforeach()
    message(STATUS "SuperBuild - ${__indent}${proj} => Requires${dependency_str}")
  endif()

  # Include dependencies
  foreach(dep ${${proj}_DEPENDENCIES})
    if(NOT External_${dep}_FILE_INCLUDED)
      if(EXISTS "${EXTERNAL_PROJECT_DIR}/External_${dep}.cmake")
        include(${EXTERNAL_PROJECT_DIR}/External_${dep}.cmake)
      elseif(EXISTS "${Slicer_ADDITIONAL_EXTERNAL_PROJECT_DIR}/External_${dep}.cmake")
        include(${Slicer_ADDITIONAL_EXTERNAL_PROJECT_DIR}/External_${dep}.cmake)
      else()
        message(FATAL_ERROR "Can't find External_${dep}.cmake")
      endif()
    endif()
  endforeach()

  set(__${proj}_superbuild_message "SuperBuild - ${__indent}${proj}[OK]")
  set(__${proj}_indent ${__indent})

  # If project being process has dependencies, indicates it has also been added.
  if(NOT "${${proj}_DEPENDENCIES}" STREQUAL "")
    message(STATUS ${__${proj}_superbuild_message})
  endif()


  # Update indent variable
  string(LENGTH "${__indent}" __indent_length)
  math(EXPR __indent_length "${__indent_length}-2")
  if(NOT ${__indent_length} LESS 0)
    string(SUBSTRING "${__indent}" 0 ${__indent_length} __indent)
  endif()
endmacro()
