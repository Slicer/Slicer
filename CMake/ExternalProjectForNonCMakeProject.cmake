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
#  W A R N I N G
#  -------------
#
# This file is not part of the Slicer API.  It exists purely as an
# implementation detail.  This CMake module may change from version to
# version without notice, or even be removed.
#
# We mean it.
#

set(_this_list_file ${CMAKE_CURRENT_LIST_FILE})

#!
#! ExternalProject_Execute(<proj> <stepname> <executable>)
#!
#! Run <executable> with the list of arguments past the last expected argument as parameters
#! and the working directory previously set to <proj>_WORKING_DIR.
#!
#! Standard output and standard error will be captured and written into two files:
#!   <CMAKE_BINARY_DIR>/<proj>_<stepname>_step_output.txt
#!   <CMAKE_BINARY_DIR>/<proj>_<stepname>_step_error.txt
#!
#! If the command execute successfully, the following message will be printed:
#!   <proj>: '<stepname>' step successfully completed.
#!
#! In case of error, path to the two log files and their content will be displayed.
#!
function(ExternalProject_Execute proj stepname)
  set(cmd ${ARGN})
  ExternalProject_Remove_Execute_Logs(${proj} ${stepname})
  message(STATUS "${proj}: ${proj}_WORKING_DIR: ${${proj}_WORKING_DIR}")
  message(STATUS "${proj}: ${cmd}")
  if(NOT DEFINED ${proj}_WORKING_DIR)
    message(FATAL_ERROR "${proj}: Variable ${proj}_WORKING_DIR is not defined !")
  endif()
  if(NOT EXISTS ${${proj}_WORKING_DIR})
    message(FATAL_ERROR "${proj}: Variable ${proj}_WORKING_DIR is set to a non-existent directory !")
  endif()
  execute_process(
    COMMAND ${PYTHON_EXECUTABLE} ${cmd}
    WORKING_DIRECTORY ${${proj}_WORKING_DIR}
    RESULT_VARIABLE result
    OUTPUT_VARIABLE output
    ERROR_VARIABLE error
    )

  set(output_file "${CMAKE_BINARY_DIR}/${proj}_${stepname}_step_output.txt")
  file(WRITE ${output_file} ${output})

  set(error_file "${CMAKE_BINARY_DIR}/${proj}_${stepname}_step_error.txt")
  file(WRITE ${error_file} ${error})

  if(NOT ${result} EQUAL 0)
    message(STATUS "${proj}: Errors detected - See below.\n${output}\n${error}")
    message(FATAL_ERROR "${proj}: Error in ${stepname} step. See ${output_file} and ${error_file}")
  endif()

  message(STATUS "${proj}: '${stepname}' step successfully completed.")
endfunction()

#!
#! ExternalProject_Remove_Execute_Logs(<proj> <stepnames>)
#!
#! Convenience function removing log file created by calling
#! ExternalProject_Execute(<proj> <stepname>)
#!
#! <stepnames> is a list of <stepname>
#!
function(ExternalProject_Remove_Execute_Logs proj stepnames)
  message(STATUS "${proj}: Removing '${stepnames}' log files")
  foreach(stepname ${stepnames})
    foreach(logtype error output)
      file(REMOVE ${CMAKE_BINARY_DIR}/${proj}_${stepname}_step_${logtype}.txt)
    endforeach()
  endforeach()
endfunction()

#!
#! ExternalProject_Write_SetBuildEnv_Commands(<file> [APPEND])
#!
#! Write (or append) to <file> the CMake command setting the environment
#! variables required to build a non-CMake project.
#!
function(ExternalProject_Write_SetBuildEnv_Commands file)
  set(append_or_write WRITE)
  if("${ARGV1}" MATCHES "APPEND")
    set(append_or_write ${ARGV1})
  endif()
  file(${append_or_write} ${file}
"#------------------------------------------------------------------------------
# Added by 'ExternalProject_Write_SetBuildEnv_Commands'

include(\"${_this_list_file}\")

set(CMAKE_BINARY_DIR \"${CMAKE_BINARY_DIR}\")

set(ENV{VS_UNICODE_OUTPUT} \"\")

set(ENV{CC} \"${CMAKE_C_COMPILER} ${CMAKE_C_COMPILER_ARG1}\")
set(ENV{CFLAGS} \"${CMAKE_C_FLAGS} ${CMAKE_C_FLAGS_RELEASE}\")
set(ENV{CXX} \"${CMAKE_CXX_COMPILER} ${CMAKE_CXX_COMPILER_ARG1}\")
set(ENV{CXXFLAGS} \"${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_RELEASE}\")
set(ENV{LDFLAGS} \"${CMAKE_LINKER_FLAGS} ${CMAKE_LINKER_FLAGS_RELEASE}\")

if(APPLE)
  set(CMAKE_CXX_HAS_ISYSROOT \"${CMAKE_CXX_HAS_ISYSROOT}\")
  set(CMAKE_OSX_ARCHITECTURES \"${CMAKE_OSX_ARCHITECTURES}\")
  set(CMAKE_OSX_DEPLOYMENT_TARGET \"${CMAKE_OSX_DEPLOYMENT_TARGET}\")
  set(CMAKE_OSX_SYSROOT \"${CMAKE_OSX_SYSROOT}\")

  set(osx_arch_flags)
  foreach(arch \${CMAKE_OSX_ARCHITECTURES})
    set(osx_arch_flags \"\${osx_arch_flags} -arch \${arch}\")
  endforeach()
  set(osx_version_flag \"-mmacosx-version-min=\${CMAKE_OSX_DEPLOYMENT_TARGET}\")
  set(osx_sysroot)
  if(CMAKE_CXX_HAS_ISYSROOT)
    set(osx_sysroot \"-isysroot \${CMAKE_OSX_SYSROOT}\")
  endif()
  set(CMAKE_OSX_FLAGS \"\${osx_arch_flags} \${osx_version_flag} \${osx_sysroot}\")

  set(ENV{CFLAGS} \"\$ENV{CFLAGS} \${CMAKE_OSX_FLAGS}\")
  set(ENV{CXXFLAGS} \"\$ENV{CXXFLAGS} \${CMAKE_OSX_FLAGS}\")
  set(ENV{LDFLAGS} \"\$ENV{LDFLAGS} \${CMAKE_OSX_FLAGS}\")
endif()
")
endfunction()

#!
#! ExternalProject_Write_SetPythonSetupEnv_Commands(<file> [APPEND])
#!
#! Write (or append) to <file> the CMake command setting the environment
#! variables required to setup a python module.
#!
function(ExternalProject_Write_SetPythonSetupEnv_Commands file)
  set(append_or_write WRITE)
  if("${ARGV1}" MATCHES "APPEND")
    set(append_or_write ${ARGV1})
  endif()
  file(${append_or_write} ${file}
"#------------------------------------------------------------------------------
# Added by 'ExternalProject_Write_SetPythonSetupEnv_Commands'

include(\"${_this_list_file}\")

set(CMAKE_BINARY_DIR \"${CMAKE_BINARY_DIR}\")

set(ENV{VS_UNICODE_OUTPUT} \"\")
")
endfunction()
