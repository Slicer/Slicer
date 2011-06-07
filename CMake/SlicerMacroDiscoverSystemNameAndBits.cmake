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
# SlicerMacroDiscoverSystemNameAndBits(<var-prefix>)
# is used to extract information associated with the current platform.
#
# The macro defines the following variables:
#  <var-prefix>_BUILD_BITS - bitness of the platform: 32 or 64
#  <var-prefix>_BUILD - which is on the this value: linux-i386, linux-amd64, macosx-ppc, macosx-i386, macosx-amd64, win32-x86, win64-x86

MACRO(SlicerMacroDiscoverSystemNameAndBits)
  SET(options)
  SET(oneValueArgs VAR_PREFIX)
  SET(multiValueArgs)
  cmake_parse_arguments(MY "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  # Sanity checks
  if("${MY_VAR_PREFIX}" STREQUAL "")
    message(FATAL_ERROR "error: VAR_PREFIX should be specified !")
  endif()

  SET(${MY_VAR_PREFIX}_BUILD "")

  SET(${MY_VAR_PREFIX}_BUILD_BITS "32")
  IF(CMAKE_SIZEOF_VOID_P EQUAL 8)
    SET(${MY_VAR_PREFIX}_BUILD_BITS "64")
  ENDIF()

  IF(CMAKE_SYSTEM_NAME STREQUAL "Windows")

    SET(${MY_VAR_PREFIX}_BUILD "win32-x86")
    IF(${MY_VAR_PREFIX}_BUILD_BITS STREQUAL "64")
      SET(${MY_VAR_PREFIX}_BUILD "win64-x86")
    ENDIF()

  ELSEIF(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  
    SET(${MY_VAR_PREFIX}_BUILD "linux-i386")
    IF(${MY_VAR_PREFIX}_BUILD_BITS STREQUAL "64")
      SET(${MY_VAR_PREFIX}_BUILD "linux-amd64")
    ENDIF()

  ELSEIF(CMAKE_SYSTEM_NAME STREQUAL "Darwin")

    IF(CMAKE_SYSTEM_PROCESSOR MATCHES "powerpc")
      SET(${MY_VAR_PREFIX}_BUILD "macosx-ppc")
    ELSE()
      SET(${MY_VAR_PREFIX}_BUILD "macosx-i386")
      IF (${MY_VAR_PREFIX}_BUILD_BITS STREQUAL "64")
        SET(${MY_VAR_PREFIX}_BUILD "macosx-amd64")
      ENDIF()
    ENDIF()

  #ELSEIF(CMAKE_SYSTEM_NAME STREQUAL "Solaris")

  #  SET(${MY_VAR_PREFIX}_BUILD "solaris8") # What about solaris9 and solaris10 ?

  ENDIF()

ENDMACRO()

