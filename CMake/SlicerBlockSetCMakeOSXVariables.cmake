################################################################################
#
#  Program: 3D Slicer
#
#  Copyright (c) 2010 Kitware Inc.
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
# SlicerBlockSetCMakeOSXVariables
#

#
# Adapted from Paraview/Superbuild/CMakeLists.txt
#

# Note: Change architecture *before* any enable_language() or project()
#       calls so that it's set properly to detect 64-bit-ness...
#       Currently, we default to 10.5. Obviously this may need to be re-evaluated when Lion comes out.
#
if(APPLE)

  set(SDK_VERSIONS_TO_CHECK "")
  set(SDK_VERSIONS_TO_CHECK ${SDK_VERSIONS_TO_CHECK} "10.5")
  set(SDK_VERSIONS_TO_CHECK ${SDK_VERSIONS_TO_CHECK} "10.6")
  foreach(SDK_VERSION ${SDK_VERSIONS_TO_CHECK}) ## Prefer 10.5, but use 10.6 if necessary
    if(NOT CMAKE_OSX_DEPLOYMENT_TARGET OR "${CMAKE_OSX_DEPLOYMENT_TARGET}" STREQUAL "")
      set(TEST_OSX_SYSROOT "/Developer/SDKs/MacOSX${SDK_VERSION}.sdk")
      if(EXISTS "${TEST_OSX_SYSROOT}")
        set(CMAKE_OSX_ARCHITECTURES "x86_64" CACHE STRING "force build for 64-bit Leopard" FORCE)
        set(CMAKE_OSX_DEPLOYMENT_TARGET "${SDK_VERSION}" CACHE STRING "force build for 64-bit Leopard" FORCE)
        set(CMAKE_OSX_SYSROOT "${TEST_OSX_SYSROOT}" CACHE PATH "force build for 64-bit Leopard" FORCE)
      endif()
    endif()
  endforeach()

  if(NOT "${CMAKE_OSX_SYSROOT}" STREQUAL "")
    if(NOT EXISTS "${CMAKE_OSX_SYSROOT}")
      message(FATAL_ERROR "error: CMAKE_OSX_SYSROOT='${CMAKE_OSX_SYSROOT}' does not exist")
    endif()
  endif()
endif()
