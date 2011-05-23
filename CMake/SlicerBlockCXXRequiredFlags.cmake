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
# Required C/CXX flags
#

#
# Usage: The variable Slicer_REQUIRED_C_FLAGS and Slicer_REQUIRED_CXX_FLAGS 
#        will set appropriately.
#

IF(NOT DEFINED Slicer_REQUIRED_C_FLAGS OR NOT DEFINED Slicer_REQUIRED_CXX_FLAGS)
  
  INCLUDE(SlicerFunctionCheckCompilerFlags)
  INCLUDE(SlicerFunctionGetGccVersion)

  SET(tmp_c_flags)
  SET(tmp_cxx_flags)
  
  IF(CMAKE_COMPILER_IS_GNUCXX)
    SET(cflags "-Wall -Wextra -Wpointer-arith -Winvalid-pch -Wcast-align -Wwrite-strings -D_FORTIFY_SOURCE=2")
    SlicerFunctionCheckCompilerFlags("-fdiagnostics-show-option" cflags)
    SlicerFunctionCheckCompilerFlags("-Wl,--no-undefined" cflags)
    
    slicerFunctionGetGccVersion(${CMAKE_CXX_COMPILER} GCC_VERSION)
    # With older version of gcc supporting the flag -fstack-protector-all, an extra dependency to libssp.so
    # is introduced. If gcc is smaller than 4.4.0 and the build type is Release let's not include the flag.
    # Doing so should allow to build package made for distribution using older linux distro.
    IF(GCC_VERSION VERSION_GREATER "4.4.0" OR (CMAKE_BUILD_TYPE STREQUAL "Debug" AND GCC_VERSION VERSION_LESS "4.4.0"))
      SlicerFunctionCheckCompilerFlags("-fstack-protector-all" cflags)
    ENDIF()
    IF(MINGW)
      # suppress warnings about auto imported symbols
      SET(tmp_cxx_flags "-Wl,--enable-auto-import ${tmp_cxx_flags}")
    ENDIF()

    # Note: -Wold-style-cast is too verbose
    #       Let's postpone the use of -Wsign-promo 
    SET(tmp_c_flags "${cflags} ${tmp_c_flags}")
    SET(tmp_cxx_flags "${cflags} -Wno-deprecated -Woverloaded-virtual -Wstrict-null-sentinel ${tmp_cxx_flags}")
  ELSEIF(MSVC)
     # if 64-bit Windows link with /bigobj
    IF(CMAKE_SIZEOF_VOID_P MATCHES 8)
       SET(tmp_c_flags /bigobj)
       SET(tmp_cxx_flags /bigobj)
    ENDIF()

    SET(cflags "/Zm1000 /W3")
    SET(tmp_c_flags "${cflags} ${tmp_c_flags}")
    SET(tmp_cxx_flags "${cflags} /EHsc /GR ${tmp_cxx_flags}")
  ENDIF()
  
  SET(Slicer_REQUIRED_C_FLAGS ${tmp_c_flags})
  SET(Slicer_REQUIRED_CXX_FLAGS ${tmp_cxx_flags})
  
ENDIF()
