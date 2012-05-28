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
# SlicerMacroGetOperatingSystemArchitectureBitness(<var-prefix>)
# is used to extract information associated with the current platform.
#
# The macro defines the following variables:
#  <var-prefix>_BITNESS - bitness of the platform: 32 or 64
#  <var-prefix>_OS - which is on the this value: linux, macosx, win
#  <var-prefix>_ARCHITECTURE - which is on the this value: i386, amd64, ppc

include(${CMAKE_CURRENT_LIST_DIR}/SlicerBlockOperatingSystemNames.cmake)

macro(SlicerMacroGetOperatingSystemArchitectureBitness)
  set(options)
  set(oneValueArgs VAR_PREFIX)
  set(multiValueArgs)
  cmake_parse_arguments(MY "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  # Sanity checks
  if("${MY_VAR_PREFIX}" STREQUAL "")
    message(FATAL_ERROR "error: VAR_PREFIX should be specified !")
  endif()

  set(${MY_VAR_PREFIX}_ARCHITECTURE "")

  set(${MY_VAR_PREFIX}_ARCHITECTURE i386)
  set(${MY_VAR_PREFIX}_BITNESS 32)
  if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(${MY_VAR_PREFIX}_BITNESS 64)
    set(${MY_VAR_PREFIX}_ARCHITECTURE amd64)
  endif()

  if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(${MY_VAR_PREFIX}_OS "${Slicer_OS_WIN_NAME}")

  elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(${MY_VAR_PREFIX}_OS "${Slicer_OS_LINUX_NAME}")

  elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(${MY_VAR_PREFIX}_OS "${Slicer_OS_MAC_NAME}")
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "powerpc")
      set(${MY_VAR_PREFIX}_ARCHITECTURE "ppc")
    endif()

  #elseif(CMAKE_SYSTEM_NAME STREQUAL "Solaris")

  #  set(${MY_VAR_PREFIX}_BUILD "solaris8") # What about solaris9 and solaris10 ?

  endif()

endmacro()

