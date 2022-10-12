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
# Slicer Platform check
#

#
# Usage:
#   The check can be enabled/disabled by setting Slicer_PLATFORM_CHECK to
#   True/False before including the block.
#

if(NOT DEFINED Slicer_PLATFORM_CHECK)
  set(Slicer_PLATFORM_CHECK true)
endif()

if(Slicer_PLATFORM_CHECK)
  if(WIN32)
    # See https://cmake.org/cmake/help/latest/variable/MSVC_VERSION.html
    # and https://en.wikipedia.org/wiki/Microsoft_Visual_Studio#Version_history
    # 1910-1919 = VS 15.0 (v141 toolset) 1914 = VS 15.7
    # 1920-1929 = VS 16.0 (v142 toolset)
    # 1930-1939 = VS 17.0 (v143 toolset)
    # VS 15.7 was announced to officially conform with the C++ standard of C++11, C++14 and C++17
    # https://devblogs.microsoft.com/cppblog/announcing-msvc-conforms-to-the-c-standard/
    if(NOT MSVC_VERSION VERSION_GREATER_EQUAL 1914)
      message(FATAL_ERROR "Microsoft Visual C/C++ toolset 141 (VS 15.7) or newer is required !")
    endif()
  elseif(APPLE)
    # See CMake/Modules/Platform/Darwin.cmake)
    # and https://en.wikipedia.org/wiki/Darwin_(operating_system)#Release_history
    #  17.x == macOS 10.13 (High Sierra)
    #  18.x == macOS 10.14 (Mojave)
    #  19.x == macOS 10.15 (Catalina)
    #  20.x == macOS 11 (Big Sur)
    #  21.x == macOS 12 (Monterey)
    if(NOT DARWIN_MAJOR_VERSION GREATER_EQUAL "17")
      message(FATAL_ERROR "Only macOS >= 10.13 is supported !")
    endif()
  # elseif(UNIX)
    # No UNIX platform checks currently
  endif()
endif()
