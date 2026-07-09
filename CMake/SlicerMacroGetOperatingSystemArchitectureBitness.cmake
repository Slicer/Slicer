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
#  <var-prefix>_ARCHITECTURE - which is on the this value: i386, amd64, arm64, ppc

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

  set(${MY_VAR_PREFIX}_ARCHITECTURE i386)
  set(${MY_VAR_PREFIX}_BITNESS 32)
  if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(${MY_VAR_PREFIX}_BITNESS 64)
    set(${MY_VAR_PREFIX}_ARCHITECTURE amd64)  # initial default before platform specific logic below
  endif()

  if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(${MY_VAR_PREFIX}_OS "${Slicer_OS_WIN_NAME}")
    # For Visual Studio generators, CMAKE_GENERATOR_PLATFORM (e.g. set via
    # "-A ARM64") reflects the architecture actually being targeted, and
    # should be preferred over CMAKE_SYSTEM_PROCESSOR, which may instead
    # reflect an amd64 build of CMake running under x64 emulation on an
    # arm64 host.
    if(NOT "${CMAKE_GENERATOR_PLATFORM}" STREQUAL "")
      if(CMAKE_GENERATOR_PLATFORM MATCHES "^ARM64$")
        set(${MY_VAR_PREFIX}_ARCHITECTURE "arm64")
      endif()
    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "^(arm64|aarch64)$")
      set(${MY_VAR_PREFIX}_ARCHITECTURE "arm64")
    endif()

  elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(${MY_VAR_PREFIX}_OS "${Slicer_OS_LINUX_NAME}")
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "^(arm64|aarch64)$")
      set(${MY_VAR_PREFIX}_ARCHITECTURE "arm64")
    endif()

  elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(${MY_VAR_PREFIX}_OS "${Slicer_OS_MAC_NAME}")
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "powerpc")
      set(${MY_VAR_PREFIX}_ARCHITECTURE "ppc")
    endif()
    # CMAKE_OSX_ARCHITECTURES (e.g. set via "-DCMAKE_OSX_ARCHITECTURES=arm64")
    # reflects the architecture actually being targeted, and should be
    # preferred over CMAKE_SYSTEM_PROCESSOR, which reflects the host machine
    # and may be wrong, for example when CMake is run under Rosetta 2
    # translation.
    if(NOT "${CMAKE_OSX_ARCHITECTURES}" STREQUAL "")
      if(CMAKE_OSX_ARCHITECTURES MATCHES "arm64")
        set(${MY_VAR_PREFIX}_ARCHITECTURE "arm64")
      endif()
    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "^(arm64|aarch64)$")
      set(${MY_VAR_PREFIX}_ARCHITECTURE "arm64")
    endif()

  #elseif(CMAKE_SYSTEM_NAME STREQUAL "Solaris")

  #  set(${MY_VAR_PREFIX}_BUILD "solaris8") # What about solaris9 and solaris10 ?

  endif()

endmacro()
