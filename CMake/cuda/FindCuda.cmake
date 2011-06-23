###############################################################################
#  For more information, please see: http://software.sci.utah.edu
#
#  The MIT License
#
#  Copyright (c) 2007
#  Scientific Computing and Imaging Institute, University of Utah
#
#  License for the specific language governing rights and limitations under
#  Permission is hereby granted, free of charge, to any person obtaining a
#  copy of this software and associated documentation files (the "Software"),
#  to deal in the Software without restriction, including without limitation
#  the rights to use, copy, modify, merge, publish, distribute, sublicense,
#  and/or sell copies of the Software, and to permit persons to whom the
#  Software is furnished to do so, subject to the following conditions:
#
#  The above copyright notice and this permission notice shall be included
#  in all copies or substantial portions of the Software.
#
#  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
#  OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
#  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
#  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
#  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
#  DEALINGS IN THE SOFTWARE.
#
# This script locates the Nvidia Compute Unified Driver Architecture (CUDA)
# tools. It should on both linux and windows, and should be reasonably up to
# date with cuda releases.
#
# The script will prompt the user to specify CUDA_INSTALL_PREFIX if the
# prefix cannot be determined by the location of nvcc in the system path. To
# use a different installed version of the toolkit set the environment variable
# CUDA_BIN_PATH before running cmake (e.g. CUDA_BIN_PATH=/usr/local/cuda1.0
# instead of the default /usr/local/cuda).
#
# Set CUDA_BUILD_TYPE to "Device" or "Emulation" mode.
# _DEVICEEMU is defined in "Emulation" mode.
#
# Set CUDA_BUILD_CUBIN to "ON" or "OFF" to enable and extra compilation pass
# with the -cubin option in Device mode.
#
# The output is parsed and register, shared memory usage is printed during
# build. Default ON.
#
# The script creates the following macros:
# CUDA_INCLUDE_DIRECTORIES( path0 path1 ... )
# -- Sets the directories that should be passed to nvcc
#    (e.g. nvcc -Ipath0 -Ipath1 ... ). These paths usually contain other .cu
#    files.
#
# CUDA_ADD_LIBRARY( cuda_target file0 file1 ... )
# -- Creates a shared library "cuda_target" which contains all of the source
#    (*.c, *.cc, etc.) specified and all of the nvcc'ed .cu files specified.
#    All of the specified source files and generated .c files are compiled
#    using the standard CMake compiler, so the normal INCLUDE_DIRECTORIES,
#    LINK_DIRECTORIES, and TARGET_LINK_LIBRARIES can be used to affect their
#    build and link.
#
# CUDA_ADD_EXECUTABLE( cuda_target file0 file1 ... )
# -- Same as CUDA_ADD_LIBRARY except that an exectuable is created.
#
# The script defines the following variables:
#
# ( Note CUDA_ADD_* macros setup cuda/cut library dependencies automatically.
# These variables are only needed if a cuda API call must be made from code in
# a outside library or executable. )
#
# CUDA_INCLUDE         -- Include directory for cuda headers.
# CUDA_TARGET_LINK     -- Cuda RT library.
# CUDA_CUT_INCLUDE     -- Include directory for cuda SDK headers (cutil.h).
# CUDA_CUT_TARGET_LINK -- SDK libraries.
#
# -- Abe Stephens SCI Institute -- http://www.sci.utah.edu/~abe/FindCuda.html
###############################################################################

# FindCuda.cmake

set(CMAKE_BACKWARDS_COMPATIBILITY 2.2)

include(${CMAKE_SOURCE_DIR}/CMake/cuda/CudaDependency.cmake)

###############################################################################
###############################################################################
# Locate CUDA, Set Build Type, etc.
###############################################################################
###############################################################################

# Parse CUDA build type.
if(NOT CUDA_BUILD_TYPE)
  set(CUDA_BUILD_TYPE "Device" CACHE STRING "Cuda build type: Emulation or Device")
endif(NOT CUDA_BUILD_TYPE)

# Emulation if the card isn't present.
if(CUDA_BUILD_TYPE MATCHES "Emulation")
  # Emulation.
  set(CUDA_NVCC_FLAGS --device-emulation -D_DEVICEEMU -g)
else(CUDA_BUILD_TYPE MATCHES "Emulation")
  # Device present.
  set(CUDA_NVCC_FLAGS "")
endif(CUDA_BUILD_TYPE MATCHES "Emulation")

set(CUDA_BUILD_CUBIN TRUE CACHE BOOL "Generate and parse .cubin files in Device mode.")

# Search for the cuda distribution.
if(NOT CUDA_INSTALL_PREFIX)
  find_program(CUDA_NVCC
    nvcc
    PATHS $ENV{CUDA_BIN_PATH}
          /usr/local/bin
          /usr/local/cuda/bin
    )

  set(cuda_path "${CUDA_NVCC}")
  if(cuda_path)
    mark_as_advanced(CUDA_NVCC)
    string(REGEX REPLACE "[/\\\\]?bin[/\\\\]?nvcc[/\\\\]?$" "" cuda_path ${cuda_path})
    string(REGEX REPLACE "[/\\\\]?bin[/\\\\]?nvcc.exe[/\\\\]?$" "" cuda_path ${cuda_path})
  else(cuda_path)
    set(cuda_path "$ENV{CUDA_BIN_PATH}")
    #string(REGEX REPLACE "[/\\\\]?bin[/\\\\]?$" "" cuda_path ${cuda_path})
  endif(cuda_path)

  set(CUDA_INSTALL_PREFIX ${cuda_path} CACHE PATH "Prefix used during install")

  if(NOT EXISTS ${cuda_path})
    message(STATUS "Specify CUDA_INSTALL_PREFIX")
  endif(NOT EXISTS ${cuda_path})
endif(NOT CUDA_INSTALL_PREFIX)

# CUDA_NVCC
if(CUDA_INSTALL_PREFIX)
if(NOT CUDA_NVCC)
  find_program(CUDA_NVCC
    nvcc
    PATHS ${CUDA_INSTALL_PREFIX}/bin $ENV{CUDA_BIN_PATH}
          /usr/local/bin
          /usr/local/cuda/bin
    )

  if(NOT CUDA_NVCC)
    message(STATUS "Could not find nvcc")
  else(NOT CUDA_NVCC)
    mark_as_advanced(CUDA_NVCC)
  endif(NOT CUDA_NVCC)
endif(NOT CUDA_NVCC)

# CUDA_NVCC_INCLUDE_ARGS
# if(NOT FOUND_CUDA_NVCC_INCLUDE)
  find_path(FOUND_CUDA_NVCC_INCLUDE
    device_functions.h
    PATHS ${CUDA_INSTALL_PREFIX}/include $ENV{CUDA_INC_PATH}
    )

  if(NOT FOUND_CUDA_NVCC_INCLUDE)
    message(STATUS "Could not find Cuda headers")
  else(NOT FOUND_CUDA_NVCC_INCLUDE)
    # Set the initial include dir.
    set(CUDA_NVCC_INCLUDE_ARGS "-I"${FOUND_CUDA_NVCC_INCLUDE})
        set(CUDA_INCLUDE ${FOUND_CUDA_NVCC_INCLUDE})

    mark_as_advanced(
      FOUND_CUDA_NVCC_INCLUDE
      CUDA_NVCC_INCLUDE_ARGS
      )
  endif(NOT FOUND_CUDA_NVCC_INCLUDE)

# endif(NOT FOUND_CUDA_NVCC_INCLUDE)


# CUDA_TARGET_LINK
if(NOT CUDA_TARGET_LINK)

  find_library(FOUND_CUDART
    cudart
    PATHS ${CUDA_INSTALL_PREFIX}/lib $ENV{CUDA_LIB_PATH}
    DOC "\"cudart\" library"
    )

  # Check to see if cudart library was found.
  if(NOT FOUND_CUDART)
    message(STATUS "Could not find cudart library (cudart)")
  endif(NOT FOUND_CUDART)

  # 1.1 toolkit on linux doesn't appear to have a separate library.
  find_library(FOUND_CUDA
    cuda
    PATHS ${CUDA_INSTALL_PREFIX}/lib /usr/lib
    DOC "\"cuda\" library (older versions only)."
    NO_DEFAULT_PATH
    NO_CMAKE_ENVIRONMENT_PATH
    NO_CMAKE_PATH
    NO_SYSTEM_ENVIRONMENT_PATH
    NO_CMAKE_SYSTEM_PATH
    )

  # Add cuda library to the link line only if it is found.
  if(FOUND_CUDA)
    set(CUDA_TARGET_LINK ${FOUND_CUDA})
    mark_as_advanced(
      FOUND_CUDA
      )
  endif(FOUND_CUDA)

  # Always add cudart to the link line.
  if(FOUND_CUDART)
    set(CUDA_TARGET_LINK
      ${CUDA_TARGET_LINK} ${FOUND_CUDART}
      )
    mark_as_advanced(
      CUDA_TARGET_LINK
      CUDA_LIB
      FOUND_CUDART
      )
  else(FOUND_CUDART)
    message(STATUS "Could not find cuda libraries.")
  endif(FOUND_CUDART)

endif(NOT CUDA_TARGET_LINK)

# CUDA_CUT_INCLUDE
if(NOT CUDA_CUT_INCLUDE)
  if(APPLE)
    set(SEARCH_PATHS /Developer/CUDA/common/inc
                     $ENV{HOME}/NVIDIA_CUDA_SDK_MACOSX/common/inc)
  elseif(WIN32)
    set(SEARCH_PATHS $ENV{NVSDKCUDA_ROOT}/common/inc
                     "C:/Program Files/NVIDIA Corporation/NVIDIA SDK 10/NVIDIA CUDA SDK/common/inc"
                     "C:/Program Files/NVIDIA Corporation/NVIDIA CUDA SDK/common/inc")
#                     "C:/Documents and Settings/All Users/Application Data/NVIDIA Corporation/NVIDIA CUDA SDK/common/inc")
  else(APPLE)
     set(SEARCH_PATHS $ENV{HOME}/NVIDIA_CUDA_SDK/common/inc
                      ${CUDA_INSTALL_PREFIX}/local/NVSDK0.2/common/inc
                      ${CUDA_INSTALL_PREFIX}/NVSDK0.2/common/inc
                      ${CUDA_INSTALL_PREFIX}/NV_CUDA_SDK/common/inc)
  endif(APPLE)
  find_path(FOUND_CUT_INCLUDE
    cutil.h
    PATHS ${SEARCH_PATHS}
    DOC "Location of cutil.h"
    )
  if(FOUND_CUT_INCLUDE)
    set(CUDA_CUT_INCLUDE ${FOUND_CUT_INCLUDE})
    mark_as_advanced(
      FOUND_CUT_INCLUDE
      )
  endif(FOUND_CUT_INCLUDE)
endif(NOT CUDA_CUT_INCLUDE)


# CUDA_CUT_TARGET_LINK
if(NOT CUDA_CUT_TARGET_LINK)
  if(APPLE)
    set(SEARCH_PATHS /Developer/CUDA/lib
                     $ENV{HOME}/NVIDIA_CUDA_SDK_MACOSX/lib)
  elseif(WIN32)
    set(SEARCH_PATHS $ENV{NVSDKCUDA_ROOT}/common/lib
                     "C:/Program Files/NVIDIA Corporation/NVIDIA SDK 10/NVIDIA CUDA SDK/lib"
                     "C:/Program Files/NVIDIA Corporation/NVIDIA CUDA SDK/lib")
  else(APPLE)
     set(SEARCH_PATHS $ENV{HOME}/NVIDIA_CUDA_SDK/lib
                      ${CUDA_INSTALL_PREFIX}/local/NVSDK0.2/lib
                      ${CUDA_INSTALL_PREFIX}/NVSDK0.2/lib
                      ${CUDA_INSTALL_PREFIX}/NV_CUDA_SDK/lib)
  endif(APPLE)
  find_library(FOUND_CUT
    cutil
    cutil32
    PATHS ${SEARCH_PATHS}
    NO_DEFAULT_PATH
    NO_CMAKE_ENVIRONMENT_PATH
    NO_CMAKE_PATH
    NO_SYSTEM_ENVIRONMENT_PATH
    NO_CMAKE_SYSTEM_PATH
    DOC "Location of cutil.a"
    )
  if(FOUND_CUT)
    set(CUDA_CUT_TARGET_LINK ${FOUND_CUT})
    mark_as_advanced(
      FOUND_CUT
      )
  endif(FOUND_CUT)
endif(NOT CUDA_CUT_TARGET_LINK)
endif(CUDA_INSTALL_PREFIX)

###############################################################################
# Add include directories to pass to the nvcc command.
macro(CUDA_INCLUDE_DIRECTORIES)
  foreach(dir ${ARGN})
    set(CUDA_NVCC_INCLUDE_ARGS ${CUDA_NVCC_INCLUDE_ARGS} -I${dir})
  endforeach(dir ${ARGN})
endmacro(CUDA_INCLUDE_DIRECTORIES)


##############################################################################
##############################################################################
# This helper macro populates the following variables and setups up custom commands and targets to
# invoke the nvcc compiler. The compiler is invoked once with -M to generate a dependency file and
# a second time with -cuda to generate a .c file
# ${target_srcs}
# ${cuda_cu_sources}
##############################################################################
##############################################################################

macro(CUDA_add_custom_commands)

  set(target_srcs "")
  set(cuda_cu_sources "")

  # Iterate over the macro arguments and create custom
  # commands for all the .cu files.
  foreach(file ${ARGN})
    if(${file} MATCHES ".*\\.cu$")

    # Add a custom target to generate a cpp file.
    set(generated_file  "${CMAKE_BINARY_DIR}/src/cuda/${file}_generated.cxx")
    set(generated_target "${file}_target")
    get_filename_component(file_we ${file} NAME_WE)
    set(generated_object "${file_we}.o")

    file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/src/cuda)

    set(source_file ${CMAKE_CURRENT_SOURCE_DIR}/${file})

    # Note that -cuda generates a .c file not a c++ file.
    set_source_files_properties(${source_file} PROPERTIES CPLUSPLUS ON)

    # message("${CUDA_NVCC} ${source_file} ${CUDA_NVCC_FLAGS} -cuda -o ${generated_file} ${CUDA_NVCC_INCLUDE_ARGS}")

    # Bring in the dependencies.  Creates a variable CUDA_NVCC_DEPEND
        set(cmake_dependency_file "${generated_file}.depend")
        CUDA_INCLUDE_NVCC_DEPENDENCIES(${cmake_dependency_file})
        set(NVCC_generated_dependency_file "${generated_file}.NVCC-depend")

   # Build object to be linked to library

   add_custom_command(
     OUTPUT ${generated_object}
      MAIN_DEPENDENCY ${source_file}
      DEPENDS ${CUDA_NVCC_DEPEND}
      DEPENDS ${cmake_dependency_file}
      DEPENDS ${NVCC_generated_cubin_file}
      COMMAND ${CUDA_NVCC}
      ARGS ${source_file}
           ${CUDA_NVCC_FLAGS}
           -DNVCC
           -c
           -use_fast_math
           -Xcompiler -fPIC
           -o ${generated_object}
           ${CUDA_NVCC_INCLUDE_ARGS}
       COMMENT "Building NVCC ${file}: ${generated_object}\n"
      )

        # Build the NVCC made dependency file
  if(CUDA_BUILD_TYPE MATCHES "Device" AND CUDA_BUILD_CUBIN)
    set(NVCC_generated_cubin_file "${generated_file}.NVCC-cubin.txt")
          add_custom_command(

      # Generate the .cubin output.
      OUTPUT ${NVCC_generated_cubin_file}
      COMMAND ${CUDA_NVCC}
      ARGS ${source_file}
      ${CUDA_NVCC_FLAGS}
      -DNVCC
      -cubin
      -o ${NVCC_generated_cubin_file}
      ${CUDA_NVCC_INCLUDE_ARGS}

      # Execute the parser script.
      COMMAND  ${CMAKE_COMMAND}
      ARGS
      -D input_file="${NVCC_generated_cubin_file}"
      -P "${CMAKE_SOURCE_DIR}/CMake/cuda/parse_cubin.cmake"


      # MAIN_DEPENDENCY ${source_file}
      DEPENDS ${source_file}
      DEPENDS ${CUDA_NVCC_DEPEND}



      COMMENT "Building NVCC -cubin File: ${NVCC_generated_cubin_file}\n"
      )
  else(CUDA_BUILD_TYPE MATCHES "Device" AND CUDA_BUILD_CUBIN)
    # Depend on something that will exist.
    set(NVCC_generated_cubin_file "${source_file}")
  endif(CUDA_BUILD_TYPE MATCHES "Device"AND CUDA_BUILD_CUBIN)

        # Build the NVCC made dependency file
        add_custom_command(
      OUTPUT ${NVCC_generated_dependency_file}
      COMMAND ${CUDA_NVCC}
      ARGS ${source_file}
           ${CUDA_NVCC_FLAGS}
           -DNVCC
           -M
           -o ${NVCC_generated_dependency_file}
           ${CUDA_NVCC_INCLUDE_ARGS}
      # MAIN_DEPENDENCY ${source_file}
      DEPENDS ${source_file}
      DEPENDS ${CUDA_NVCC_DEPEND}
          COMMENT "Building NVCC Dependency File: ${NVCC_generated_dependency_file}\n"
    )

    # Build the CMake readible dependency file
        add_custom_command(
          OUTPUT ${cmake_dependency_file}
      COMMAND ${CMAKE_COMMAND}
      ARGS
      -D input_file="${NVCC_generated_dependency_file}"
      -D output_file="${cmake_dependency_file}"
      -P "${CMAKE_SOURCE_DIR}/CMake/cuda/make2cmake.cmake"
      MAIN_DEPENDENCY ${NVCC_generated_dependency_file}
      COMMENT "Converting NVCC dependency to CMake (${cmake_dependency_file})"
    )

    add_custom_command(
      OUTPUT ${generated_file}
      MAIN_DEPENDENCY ${source_file}
      DEPENDS ${CUDA_NVCC_DEPEND}
      DEPENDS ${cmake_dependency_file}
      DEPENDS ${NVCC_generated_cubin_file}
      COMMAND ${CUDA_NVCC}
      ARGS ${source_file}
           ${CUDA_NVCC_FLAGS}
           -DNVCC
           --keep
           -cuda -o ${generated_file}
           ${CUDA_NVCC_INCLUDE_ARGS}
       COMMENT "Building NVCC ${source_file}: ${generated_file}\n"
      )

    set(cuda_cu_sources ${cuda_cu_sources} ${source_file})

    # Add the generated file name to the source list.
    if(UNIX)
      set(target_srcs ${target_srcs} ${generated_object})
    else(UNIX)
      set(target_srcs ${target_srcs} ${generated_file})
    endif(UNIX)

    else(${file} MATCHES ".*\\.cu$")

    # Otherwise add the file name to the source list.
    set(target_srcs ${target_srcs} ${file})

    endif(${file} MATCHES ".*\\.cu$")
  endforeach(file)

endmacro(CUDA_add_custom_commands)

###############################################################################
###############################################################################
# ADD LIBRARY
###############################################################################
###############################################################################
macro(CUDA_ADD_LIBRARY cuda_target)

  # Create custom commands and targets for each file.
  CUDA_add_custom_commands( ${ARGN} )

  # Add the library.
  add_library(${cuda_target}
    ${target_srcs}
    ${cuda_cu_sources}
    )

  target_link_libraries(${cuda_target}
    ${CUDA_TARGET_LINK}
    )

endmacro(CUDA_ADD_LIBRARY cuda_target)


###############################################################################
###############################################################################
# ADD EXECUTABLE
###############################################################################
###############################################################################
macro(CUDA_ADD_EXECUTABLE cuda_target)

  # Create custom commands and targets for each file.
  CUDA_add_custom_commands( ${ARGN} )

  # Add the library.
  add_executable(${cuda_target}
    ${target_srcs}
    ${cuda_cu_sources}
    )

  target_link_libraries(${cuda_target}
    ${CUDA_TARGET_LINK}
    )


endmacro(CUDA_ADD_EXECUTABLE cuda_target)

