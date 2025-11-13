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
    # 1920-1929 = VS 16.0 (v142 toolset)
    # 1930-1949 = VS 17.0 (v143 toolset, see https://devblogs.microsoft.com/cppblog/msvc-toolset-minor-version-number-14-40-in-vs-2022-v17-10/)
    # 1950-1959 = VS 18.0 (v145 toolset)
    if(NOT MSVC_VERSION VERSION_GREATER_EQUAL 1920)
      message(FATAL_ERROR "Microsoft Visual C/C++ toolset 142 or newer is required !")
    endif()
  elseif(APPLE)
    # See CMake/Modules/Platform/Darwin.cmake)
    # and https://en.wikipedia.org/wiki/Darwin_(operating_system)#Release_history
    #  23.x == macOS 14 (Sonoma)
    #  24.x == macOS 15 (Sequoia)
    #  25.x == macOS 26 (Sequoia)
    if(NOT DARWIN_MAJOR_VERSION GREATER_EQUAL "23")
      message(FATAL_ERROR "Only macOS >= 14 is supported !")
    endif()
  # elseif(UNIX)
    # No UNIX platform checks currently
  endif()
endif()
