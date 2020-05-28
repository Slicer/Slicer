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
# SlicerInitializeOSXVariables
#

#
# Adapted from Paraview/Superbuild/CMakeLists.txt
#

# Note: Change architecture *before* any enable_language() or project()
#       calls so that it's set properly to detect 64-bit-ness, and
#       deployment target for the standard c++ library.
#
if(APPLE)

  # Disable universal binaries
  if(NOT "${CMAKE_OSX_ARCHITECTURES}" STREQUAL "")
    list(LENGTH CMAKE_OSX_ARCHITECTURES arch_count)
    if(arch_count GREATER 1)
      message(FATAL_ERROR "error: Only one value (i386 or x86_64) should be associated with CMAKE_OSX_ARCHITECTURES.")
    endif()
  endif()

  if("x${CMAKE_OSX_DEPLOYMENT_TARGET}x" STREQUAL "xx")
    string(REGEX MATCH "MacOSX([0-9]+\\.[0-9]+)\\.sdk" _match "${CMAKE_OSX_SYSROOT}")
    set(SDK_VERSION "${CMAKE_MATCH_1}")
    if( "${SDK_VERSION}" VERSION_GREATER "10.8" )
      # add to cache to allow interactive editing after fatal error
      set(CMAKE_OSX_DEPLOYMENT_TARGET "" CACHE PATH "Deployment target needs to be explicitly set." FORCE)
      message(FATAL_ERROR
        "The OSX_SYSROOT is set to version ${SDK_VERSION} (>10.8) and OSX_DEPLOYMENT_TARGET is not explicitly set!\n"
        "Since:\n"
        " (1) the default runtime associated with >=10.9 deployment target is 'libc++'.[1]\n"
        " (2) the default runtime associated with <=10.8 deployment target is 'libstdc++'.\n"
        " (3) Qt support for 'macx-clang-libc++' is listed as 'unsupported' mkspecs.\n"
        " (4) Qt binaries may be build against 'libstdc++' or 'libc++'.\n"
        " (5) Mixing the two different runtime in binaries is unstable.\n"
        "  [1]http://stackoverflow.com/questions/19637164/c-linking-error-after-upgrading-to-mac-os-x-10-9-xcode-5-0-1/19637199#19637199\n"
        "--------------------------------\n"
        "Run '$otool -L $(which qmake) |grep lib.\\*c++' to check what library Qt is built against:\n"
        " (1) if it is libstdc++ then add '-DCMAKE_OSX_DEPLOYMENT_TARGET:STRING=10.8' (or older) to the cmake command line.\n"
        " (2) if it is libc++ then add '-DCMAKE_OSX_DEPLOYMENT_TARGET:STRING=10.9' (or newer) to the cmake command line.\n"
        )
    endif()
  endif()

  # Starting with 10.9, libc++ replaces libstdc++ as the default runtime.
  set(required_deployment_target "10.13")

  if(CMAKE_OSX_DEPLOYMENT_TARGET VERSION_LESS ${required_deployment_target})
    message(FATAL_ERROR "CMAKE_OSX_DEPLOYMENT_TARGET ${CMAKE_OSX_DEPLOYMENT_TARGET} must be ${required_deployment_target} or greater.")
  endif()

  if(NOT "${CMAKE_OSX_SYSROOT}" STREQUAL "")
    if(NOT EXISTS "${CMAKE_OSX_SYSROOT}")
      message(FATAL_ERROR "error: CMAKE_OSX_SYSROOT='${CMAKE_OSX_SYSROOT}' does not exist")
    endif()
  endif()
endif()
