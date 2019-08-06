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
# Required C/CXX flags
#

#
# Usage: The variable Slicer_REQUIRED_C_FLAGS and Slicer_REQUIRED_CXX_FLAGS
#        will set appropriately.
#

if(NOT DEFINED Slicer_REQUIRED_C_FLAGS OR NOT DEFINED Slicer_REQUIRED_CXX_FLAGS)

  include(SlicerFunctionCheckCompilerFlags)
  include(SlicerFunctionGetGccVersion)

  set(tmp_c_flags)
  set(tmp_cxx_flags)

  if(CMAKE_COMPILER_IS_GNUCXX)
    set(cflags "-Wall -Wextra -Wpointer-arith -Winvalid-pch -Wcast-align -Wwrite-strings -D_FORTIFY_SOURCE=2")
    SlicerFunctionCheckCompilerFlags("-fdiagnostics-show-option" cflags)
    SlicerFunctionCheckCompilerFlags("-Wl,--no-undefined" cflags)

    slicerFunctionGetGccVersion(${CMAKE_CXX_COMPILER} GCC_VERSION)
    # With older version of gcc supporting the flag -fstack-protector-all, an extra dependency to libssp.so
    # is introduced. If gcc is smaller than 4.4.0 and the build type is Release let's not include the flag.
    # Doing so should allow to build package made for distribution using older linux distro.
    if(GCC_VERSION VERSION_GREATER "4.4.0" OR (CMAKE_BUILD_TYPE STREQUAL "Debug" AND GCC_VERSION VERSION_LESS "4.4.0"))
      SlicerFunctionCheckCompilerFlags("-fstack-protector-all" cflags)
    endif()
    if(MINGW)
      # suppress warnings about auto imported symbols
      set(tmp_cxx_flags "-Wl,--enable-auto-import ${tmp_cxx_flags}")
    endif()

    # Note: -Wold-style-cast is too verbose
    #       Let's postpone the use of -Wsign-promo
    set(tmp_c_flags "${cflags} ${tmp_c_flags}")
    set(tmp_cxx_flags "${cflags} -Wno-deprecated -Woverloaded-virtual -Wstrict-null-sentinel ${tmp_cxx_flags}")
  elseif(MSVC)
    # if 64-bit Windows link with /bigobj
    if(CMAKE_SIZEOF_VOID_P MATCHES 8)
       set(tmp_c_flags /bigobj)
       set(tmp_cxx_flags /bigobj)
    endif()
    if(MSVC_VERSION GREATER 1400)
      set(CMAKE_CXX_MP_FLAG OFF CACHE BOOL "Build with /MP flag enabled")
      set(PROCESSOR_COUNT "$ENV{NUMBER_OF_PROCESSORS}")
      set(CMAKE_CXX_MP_NUM_PROCESSORS ${PROCESSOR_COUNT} CACHE STRING "The maximum number of processes for the /MP flag")
      if(CMAKE_CXX_MP_FLAG)
        set(tmp_c_flags "${tmp_c_flags} /MP${CMAKE_CXX_MP_NUM_PROCESSORS}")
        set(tmp_cxx_flags "${tmp_cxx_flags} /MP${CMAKE_CXX_MP_NUM_PROCESSORS}")
      endif()
    endif()
  endif()

  set(Slicer_REQUIRED_C_FLAGS ${tmp_c_flags})
  set(Slicer_REQUIRED_CXX_FLAGS ${tmp_cxx_flags})

endif()

if(MSVC)
  # Update CMake C/CXX flags associated with the current project.
  string(REPLACE "/W3" "/W2" _cmake_c_flags "${CMAKE_C_FLAGS}")
  set(CMAKE_C_FLAGS ${_cmake_c_flags} CACHE STRING "CMake C Flags" FORCE)
  string(REPLACE "/W3" "/W2" _cmake_cxx_flags "${CMAKE_CXX_FLAGS}")
  set(CMAKE_CXX_FLAGS ${_cmake_cxx_flags} CACHE STRING "CMake CXX Flags" FORCE)

  # Also update CMAKE_{C,CXX}_FLAGS_INIT variables since they can be used to
  # configure external projects. Note that settings these variables
  # has no effect on the current project.
  string(REPLACE "/W3" "/W2" CMAKE_C_FLAGS_INIT "${CMAKE_C_FLAGS_INIT}")
  string(REPLACE "/W3" "/W2" CMAKE_CXX_FLAGS_INIT "${CMAKE_CXX_FLAGS_INIT}")
endif()
