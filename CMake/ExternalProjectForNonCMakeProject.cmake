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
#! By default, standard output and standard error will be captured and written into
#! two files and displated only if exit code is not zero:
#!   <CMAKE_BINARY_DIR>/<proj>_<stepname>_step_output.txt
#!   <CMAKE_BINARY_DIR>/<proj>_<stepname>_step_error.txt
#!
#! Logging to files help prevent CTest or IDE like Visual Studio for improperly
#! identifying process output as errors or warnings.
#!
#! For debugging purpose, logging to files of standard error and output
#! can be disabled setting the environment variable 'EP_EXECUTE_DISABLE_CAPTURE_OUTPUTS'
#! to 1.
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

  # Check if output and error should be captured into files
  set(capture_outputs 1)
  if("$ENV{EP_EXECUTE_DISABLE_CAPTURE_OUTPUTS}")
    set(capture_outputs 0)
    set(_reason " (EP_EXECUTE_DISABLE_CAPTURE_OUTPUTS env. variable set to '$ENV{EP_EXECUTE_DISABLE_CAPTURE_OUTPUTS}')")
    message(STATUS "${proj}: '${stepname}' Disabling capture of outputs${_reason}")
  endif()

  # Execute command
  set(_args)
  if(capture_outputs)
    set(_args
      OUTPUT_VARIABLE output
      ERROR_VARIABLE error
      )
  endif()
  execute_process(
    COMMAND ${cmd}
    WORKING_DIRECTORY ${${proj}_WORKING_DIR}
    RESULT_VARIABLE result
    ${_args}
    )

  # If it applies, write output to files
  if(capture_outputs)
    set(output_file "${CMAKE_BINARY_DIR}/${proj}_${stepname}_step_output.txt")
    file(WRITE ${output_file} ${output})

    set(error_file "${CMAKE_BINARY_DIR}/${proj}_${stepname}_step_error.txt")
    file(WRITE ${error_file} ${error})
  endif()

  if(NOT ${result} EQUAL 0)
    if(capture_outputs)
      message(STATUS "${proj}: Errors detected - See below.\n${output}\n${error}")
      message(FATAL_ERROR "${proj}: ${stepname} step failed with exit code '${result}'.
Outputs also captured in ${output_file} and ${error_file}.
Setting env. variable EP_EXECUTE_DISABLE_CAPTURE_OUTPUTS to 1 allows to disable file capture.
")
    else()
      message(FATAL_ERROR "${proj}: ${stepname} step failed with exit code '${result}'.")
    endif()
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
#! _ep_var_append(<var_name> <value>)
#!
#! Append <value> to the variable <var_name> if <value>
#! is not an empty string.
#!
function(_ep_var_append var_name value)
  string(STRIP "${value}" value)
  if(NOT "${value}" STREQUAL "")
    if("${${var_name}}" STREQUAL "")
      set(${var_name} ${value})
    else()
      set(${var_name} "${${var_name}} ${value}")
    endif()
    set(${var_name} ${${var_name}} PARENT_SCOPE)
  endif()
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

set(CMAKE_C_COMPILER \"${CMAKE_C_COMPILER}\")
set(CMAKE_C_COMPILER_ARG1 \"${CMAKE_C_COMPILER_ARG1}\")

set(CMAKE_C_FLAGS_INIT \"${CMAKE_C_FLAGS_INIT}\")
set(CMAKE_C_FLAGS_RELEASE \"${CMAKE_C_FLAGS_RELEASE}\")

set(CMAKE_CXX_COMPILER \"${CMAKE_CXX_COMPILER}\")
set(CMAKE_CXX_COMPILER_ARG1 \"${CMAKE_CXX_COMPILER_ARG1}\")

set(CMAKE_CXX_FLAGS_INIT \"${CMAKE_CXX_FLAGS_INIT}\")
set(CMAKE_CXX_FLAGS_RELEASE \"${CMAKE_CXX_FLAGS_RELEASE}\")

set(CMAKE_LINKER_FLAGS \"${CMAKE_LINKER_FLAGS}\")
set(CMAKE_LINKER_FLAGS_RELEASE \"${CMAKE_LINKER_FLAGS_RELEASE}\")

_ep_var_append(_ep_CC \"\${CMAKE_C_COMPILER}\")
_ep_var_append(_ep_CC \"\${CMAKE_C_COMPILER_ARG1}\")

_ep_var_append(_ep_CFLAGS \"\${CMAKE_C_FLAGS_INIT}\")
_ep_var_append(_ep_CFLAGS \"\${CMAKE_C_FLAGS_RELEASE}\")

_ep_var_append(_ep_CXX \"\${CMAKE_CXX_COMPILER}\")
_ep_var_append(_ep_CXX \"\${CMAKE_CXX_COMPILER_ARG1}\")

_ep_var_append(_ep_CXXFLAGS \"\${CMAKE_CXX_FLAGS_INIT}\")
_ep_var_append(_ep_CXXFLAGS \"\${CMAKE_CXX_FLAGS_RELEASE}\")

_ep_var_append(_ep_LDFLAGS \"\${CMAKE_LINKER_FLAGS}\")
_ep_var_append(_ep_LDFLAGS \"\${CMAKE_LINKER_FLAGS_RELEASE}\")

if(APPLE)
  set(CMAKE_CXX_SYSROOT_FLAG \"${CMAKE_CXX_SYSROOT_FLAG}\")
  set(CMAKE_OSX_ARCHITECTURES \"${CMAKE_OSX_ARCHITECTURES}\")
  set(CMAKE_OSX_DEPLOYMENT_TARGET \"${CMAKE_OSX_DEPLOYMENT_TARGET}\")
  set(CMAKE_OSX_SYSROOT \"${CMAKE_OSX_SYSROOT}\")

  set(osx_arch_flags)
  foreach(arch \${CMAKE_OSX_ARCHITECTURES})
    set(osx_arch_flags \"\${osx_arch_flags} -arch \${arch}\")
  endforeach()
  set(osx_version_flag \"-mmacosx-version-min=\${CMAKE_OSX_DEPLOYMENT_TARGET}\")
  set(osx_sysroot)
  if(CMAKE_CXX_SYSROOT_FLAG)
    set(osx_sysroot \"\${CMAKE_CXX_SYSROOT_FLAG} \${CMAKE_OSX_SYSROOT}\")
  endif()
  set(CMAKE_OSX_FLAGS \"\${osx_arch_flags} \${osx_version_flag} \${osx_sysroot}\")

  _ep_var_append(_ep_CFLAGS \"\${CMAKE_OSX_FLAGS}\")
  _ep_var_append(_ep_CXXFLAGS \"\${CMAKE_OSX_FLAGS}\")
  _ep_var_append(_ep_LDFLAGS \"\${CMAKE_OSX_FLAGS}\")
endif()

set(ENV{VS_UNICODE_OUTPUT} \"\")

set(ENV{CC} \"\${_ep_CC}\")
set(ENV{CXX} \"\${_ep_CXX}\")

if(NOT \"\${_ep_CFLAGS}\" STREQUAL \"\")
  set(ENV{CFLAGS} \"\${_ep_CFLAGS}\")
endif()
if(NOT \"\${_ep_CXXFLAGS}\" STREQUAL \"\")
  set(ENV{CXXFLAGS} \"\${_ep_CXXFLAGS}\")
endif()
if(NOT \"\${_ep_LDFLAGS}\" STREQUAL \"\")
  set(ENV{LDFLAGS} \"\${_ep_LDFLAGS}\")
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
