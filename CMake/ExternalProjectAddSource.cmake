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


#!
#! ExternalProject_Add_Source(<projectname>
#!     SOURCE_DIR_VAR <source_dir_var>
#!     [DOWNLOAD_DIR <dir>]
#!     (
#!       URL <url> [URL_MD5 md5] |
#!       GIT_REPOSITORY <url> [GIT_TAG <tag>] |
#!       SVN_REPOSITORY url [SVN_REVISION -r 123][SVN_USERNAME john][SVN_PASSWORD doe][SVN_TRUST_CERT 1]
#!     )
#!     [PROJECTS <projectname> [<projectname> [...]]]
#!     [LABELS <label1> [<label2> [...]]]
#!   )
#!
function(ExternalProject_Add_Source projectname)
  set(options)
  set(_ep_one_args BINARY_DIR DOWNLOAD_DIR URL URL_MD5 GIT_REPOSITORY GIT_TAG SVN_REPOSITORY SVN_USERNAME SVN_PASSWORD SVN_TRUST_CERT)
  set(oneValueArgs ${_ep_one_args} SOURCE_DIR_VAR)
  set(_ep_multi_args SVN_REVISION)
  set(multiValueArgs ${_ep_multi_args} LABELS PROJECTS)
  cmake_parse_arguments(_ep "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  if(NOT _ep_SOURCE_DIR_VAR)
    message(FATAL_ERROR "Parameter SOURCE_DIR_VAR is not specified !")
  endif()

  # Sanity checks
  if(DEFINED ${_ep_SOURCE_DIR_VAR} AND NOT EXISTS ${${_ep_SOURCE_DIR_VAR}})
    message(FATAL_ERROR "${_ep_SOURCE_DIR_VAR} variable is defined but corresponds to non-existing directory")
  endif()

  if(NOT DEFINED ${_ep_SOURCE_DIR_VAR})

    if(NOT ${_ep_SOURCE_DIR_VAR})
      set(${_ep_SOURCE_DIR_VAR} ${CMAKE_BINARY_DIR}/${projectname})
    endif()

    if(NOT _ep_BINARY_DIR)
      set(_ep_BINARY_DIR CMakeFiles/${projectname}-build)
    endif()

    set(_ep_args_to_pass)
    foreach(arg ${_ep_one_args} ${_ep_multi_args})
      if(_ep_${arg})
        list(APPEND _ep_args_to_pass ${arg} ${_ep_${arg}})
      endif()
    endforeach()

    ExternalProject_Add(${projectname}
      ${_ep_args_to_pass}
      SOURCE_DIR ${${_ep_SOURCE_DIR_VAR}}
      CONFIGURE_COMMAND ""
      BUILD_COMMAND ""
      INSTALL_COMMAND ""
      )

    set(${_ep_SOURCE_DIR_VAR} ${CMAKE_BINARY_DIR}/${projectname} PARENT_SCOPE)

  endif()

  if(NOT _ep_LABELS)
    set(_ep_LABELS "")
  endif()
  if(NOT _ep_PROJECTS)
    set(_ep_PROJECTS "")
  endif()
  mark_as_superbuild(VARS ${_ep_SOURCE_DIR_VAR}:PATH LABELS ${_ep_LABELS} PROJECTS ${_ep_PROJECTS})

endfunction()


#!
#! Slicer_RemoteModule_Add(<projectname>
#!     [DOWNLOAD_DIR <dir>]
#!     (
#!       URL <url> [URL_MD5 md5] |
#!       GIT_REPOSITORY <url> [GIT_TAG <tag>] |
#!       SVN_REPOSITORY url [SVN_REVISION -r 123][SVN_USERNAME john][SVN_PASSWORD doe][SVN_TRUST_CERT 1]
#!     )
#!     [OPTION_NAME <option_name>
#!       [OPTION_DEFAULT ON]
#!       [OPTION_DEPENDS <option_depends>]
#!       [OPTION_FORCE OFF]
#!     ]
#!   )
#!
macro(Slicer_Remote_Add projectname)
  set(options)
  set(_add_source_args
    BINARY_DIR DOWNLOAD_DIR URL URL_MD5 GIT_REPOSITORY GIT_TAG SVN_REPOSITORY SVN_USERNAME SVN_PASSWORD SVN_TRUST_CERT)
  set(oneValueArgs OPTION_NAME OPTION_DEFAULT OPTION_FORCE SOURCE_DIR_VAR ${_add_source_args})
  set(_add_source_multi_args SVN_REVISION LABELS PROJECTS)
  set(multiValueArgs OPTION_DEPENDS ${_add_source_multi_args})
  cmake_parse_arguments(_ep "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  if(NOT _ep_SOURCE_DIR_VAR)
    set(_ep_SOURCE_DIR_VAR ${projectname}_SOURCE_DIR)
  endif()

  if(_ep_OPTION_NAME AND NOT ${_ep_OPTION_DEFAULT} MATCHES ".+")
    set(_ep_OPTION_DEFAULT ON)
    #message("[${projectname}] Setting default value for OPTION_DEFAULT:${_ep_OPTION_DEFAULT}")
  endif()

  if(_ep_OPTION_NAME AND NOT ${_ep_OPTION_FORCE} MATCHES ".+")
    set(_ep_OPTION_FORCE OFF)
    #message("[${projectname}] Setting default value for OPTION_FORCE:${_ep_OPTION_FORCE}")
  endif()

  set(_add_source 1)
  if(_ep_OPTION_NAME)
    #message("[${projectname}] Adding option ${_ep_OPTION_NAME}")
    cmake_dependent_option(
      ${_ep_OPTION_NAME} "Download and integrate ${projectname} sources." ${_ep_OPTION_DEFAULT}
      "${_ep_OPTION_DEPENDS}" ${_ep_OPTION_FORCE})
    mark_as_advanced(${_ep_OPTION_NAME})
    mark_as_superbuild(${_ep_OPTION_NAME})
    set(_add_source ${${_ep_OPTION_NAME}})
  else()
    foreach(_arg_name OPTION_DEFAULT OPTION_FORCE OPTION_DEPENDS)
      if(_ep_${_arg_name})
        message(FATAL_ERROR "Argument ${_arg_name} expects OPTION_NAME to be specified !")
      endif()
    endforeach()
  endif()

  #message("[${projectname}] Option (${_ep_OPTION_NAME}) value ${${_ep_OPTION_NAME}}")
  if(_add_source)
    message(STATUS "Remote - ${projectname} [OK]")

    set(_ep_args_to_pass)
    foreach(arg ${_add_source_args} ${_add_source_multi_args})
      if(_ep_${arg})
        list(APPEND _ep_args_to_pass ${arg} ${_ep_${arg}})
      endif()
    endforeach()

    ExternalProject_Add_Source(${projectname}
      ${_ep_args_to_pass}
      SOURCE_DIR_VAR ${_ep_SOURCE_DIR_VAR}
      )
  endif()

endmacro()
