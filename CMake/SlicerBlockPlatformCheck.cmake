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
  # See CMake/Modules/Platform/Darwin.cmake)
  #   6.x == Mac OSX 10.2 (Jaguar)
  #   7.x == Mac OSX 10.3 (Panther)
  #   8.x == Mac OSX 10.4 (Tiger)
  #   9.x == Mac OSX 10.5 (Leopard)
  #  10.x == Mac OSX 10.6 (Snow Leopard)
  #  11.x == Mac OSX 10.7 (Lion)
  #  12.x == Mac OSX 10.8 (Mountain Lion)
  if(DARWIN_MAJOR_VERSION LESS "9")
    message(FATAL_ERROR "Only Mac OSX >= 10.5 is supported !")
  endif()

  if(MSVC)
    # See http://www.cmake.org/cmake/help/v2.8.10/cmake.html#variable:MSVC_VERSION
    # and https://en.wikipedia.org/wiki/Microsoft_Visual_Studio#Version_history
    #   1200 = VS  6.0 (Visual Studio 6.0)
    #   1300 = VS  7.0 (Visual Studio .NET (2002))
    #   1310 = VS  7.1 (Visual Studio .NET 2003)
    #   1400 = VS  8.0 (Visual Studio 2005)
    #   1500 = VS  9.0 (Visual Studio 2008)
    #   1600 = VS 10.0 (Visual Studio 2010)
    #   1700 = VS 11.0 (Visual Studio 2012)
    if(MSVC_VERSION VERSION_LESS 1500)
      message(FATAL_ERROR "Visual Studio >= 2008 is required !")
    endif()

    # See https://github.com/Kitware/CMake/blob/master/Modules/CMakeDetermineVSServicePack.cmake
    #  14.00.50727.42  - vc80
    #  14.00.50727.762 - vc80sp1
    #  15.00.21022.08  - vc90
    #  15.00.30729.01  - vc90sp1
    #  16.00.30319.01  - vc100
    #  16.00.40219.01  - vc100sp1
    #  17.00.50727.1   - vc110
    if(MSVC90 AND NOT "${CMAKE_CXX_COMPILER_VERSION}" VERSION_EQUAL "15.00.30729.01")
      message(FATAL_ERROR "Slicer requires Microsoft Visual Studio 2008 (VS9) SP1 or greater!"
                          "See http://www.microsoft.com/en-us/download/details.aspx?id=10986" )
    endif()
  endif()
endif()
