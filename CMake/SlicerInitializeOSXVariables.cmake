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

  # Waiting universal binaries are supported and tested, complain if
  # multiple architectures are specified.
  if(NOT "${CMAKE_OSX_ARCHITECTURES}" STREQUAL "")
    list(LENGTH CMAKE_OSX_ARCHITECTURES arch_count)
    if(arch_count GREATER 1)
      message(FATAL_ERROR "error: Only one value (i386 or x86_64) should be associated with CMAKE_OSX_ARCHITECTURES.")
    endif()
  endif()

  # See CMake/Modules/Platform/Darwin.cmake and https://en.wikipedia.org/wiki/MacOS#Release_history
  #   8.x == Mac OSX 10.4 (Tiger)
  #   9.x == Mac OSX 10.5 (Leopard)
  #  10.x == Mac OSX 10.6 (Snow Leopard)
  #  11.x == Mac OSX 10.7 (Lion)
  #  12.x == Mac OSX 10.8 (Mountain Lion)
  #  13.x == Mac OSX 10.9 (Mavericks)
  #  14.x == Mac OSX 10.10 (Yosemite)
  #  15.x == Mac OSX 10.11 (El Capitan)
  #  16.x == Mac OSX 10.12 (Sierra)
  #  17.x == Mac OSX 10.13 (High Sierra)
  #  18.x == Mac OSX 10.14 (Mojave)
  set(OSX_SDK_104_NAME "Tiger")
  set(OSX_SDK_105_NAME "Leopard")
  set(OSX_SDK_106_NAME "Snow Leopard")
  set(OSX_SDK_107_NAME "Lion")
  set(OSX_SDK_108_NAME "Mountain Lion")
  set(OSX_SDK_109_NAME "Mavericks")
  set(OSX_SDK_1010_NAME "Yosemite")
  set(OSX_SDK_1011_NAME "El Capitan")
  set(OSX_SDK_1012_NAME "Sierra")
  set(OSX_SDK_1013_NAME "High Sierra")
  set(OSX_SDK_1014_NAME "Mojave")

  set(OSX_SDK_ROOTS
    /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs
    /Developer/SDKs
    )

  # Explicitly set the OSX_SYSROOT to the latest one, as its required
  #       when the SX_DEPLOYMENT_TARGET is explicitly set
  foreach(SDK_ROOT ${OSX_SDK_ROOTS})
    if( "x${CMAKE_OSX_SYSROOT}x" STREQUAL "xx")
      file(GLOB SDK_SYSROOTS "${SDK_ROOT}/MacOSX*.sdk")

      if(NOT "x${SDK_SYSROOTS}x" STREQUAL "xx")
        set(SDK_SYSROOT_NEWEST "")
        set(SDK_VERSION "0")
        # find the latest SDK
        foreach(SDK_ROOT_I ${SDK_SYSROOTS})
          # extract version from SDK
          string(REGEX MATCH "MacOSX([0-9]+\\.[0-9]+)\\.sdk" _match "${SDK_ROOT_I}")
          if("${CMAKE_MATCH_1}" VERSION_GREATER "${SDK_VERSION}")
            set(SDK_SYSROOT_NEWEST "${SDK_ROOT_I}")
            set(SDK_VERSION "${CMAKE_MATCH_1}")
          endif()
        endforeach()

        if(NOT "x${SDK_SYSROOT_NEWEST}x" STREQUAL "xx")
          string(REPLACE "." "" sdk_version_no_dot ${SDK_VERSION})
          set(OSX_NAME ${OSX_SDK_${sdk_version_no_dot}_NAME})
          set(CMAKE_OSX_ARCHITECTURES "x86_64" CACHE STRING "Force build for 64-bit ${OSX_NAME}." FORCE)
          set(CMAKE_OSX_SYSROOT "${SDK_SYSROOT_NEWEST}" CACHE PATH "Force build for 64-bit ${OSX_NAME}." FORCE)
          message(STATUS "Setting OSX_ARCHITECTURES to '${CMAKE_OSX_ARCHITECTURES}' as none was specified.")
          message(STATUS "Setting OSX_SYSROOT to latest '${CMAKE_OSX_SYSROOT}' as none was specified.")
        endif()
      endif()
    endif()
  endforeach()

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
  set(required_deployment_target "10.11")

  if(CMAKE_OSX_DEPLOYMENT_TARGET VERSION_LESS ${required_deployment_target})
    message(FATAL_ERROR "CMAKE_OSX_DEPLOYMENT_TARGET must be ${required_deployment_target} or greater.")
  endif()

  if(NOT "${CMAKE_OSX_SYSROOT}" STREQUAL "")
    if(NOT EXISTS "${CMAKE_OSX_SYSROOT}")
      message(FATAL_ERROR "error: CMAKE_OSX_SYSROOT='${CMAKE_OSX_SYSROOT}' does not exist")
    endif()
  endif()
endif()
