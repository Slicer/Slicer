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

include(ExternalProject)
include(${CMAKE_CURRENT_LIST_DIR}/ExternalProjectGenerateProjectDescription.cmake)

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
#!     [VARS <name1>:<type1>=<value1> [<name2>:<type2>=<value2> [...]]]
#!     #
#!     # See ExternalProject_GenerateProjectDescription_Step
#!     #
#!     [VERSION <version>]
#!     [LICENSE_FILES <file> [...]]
#!   )
#!
function(ExternalProject_Add_Source projectname)
  set(options)
  set(_ep_one_args DOWNLOAD_DIR URL URL_MD5 GIT_REPOSITORY GIT_TAG SVN_REPOSITORY SVN_USERNAME SVN_PASSWORD SVN_TRUST_CERT)
  set(_epgpd_one_args VERSION LICENSE_FILES)
  set(oneValueArgs ${_ep_one_args} ${_epgpd_one_args} SOURCE_DIR_VAR)
  set(_ep_multi_args SVN_REVISION)
  set(multiValueArgs ${_ep_multi_args} LABELS PROJECTS VARS)
  cmake_parse_arguments(_ep "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  if(NOT _ep_SOURCE_DIR_VAR)
    message(FATAL_ERROR "Parameter SOURCE_DIR_VAR is not specified !")
  endif()

  # Sanity checks
  if(DEFINED ${_ep_SOURCE_DIR_VAR} AND NOT EXISTS ${${_ep_SOURCE_DIR_VAR}})
    message(FATAL_ERROR "${_ep_SOURCE_DIR_VAR} variable is defined but corresponds to nonexistent directory")
  endif()

  if(NOT DEFINED ${_ep_SOURCE_DIR_VAR})

    if(NOT ${_ep_SOURCE_DIR_VAR})
      set(${_ep_SOURCE_DIR_VAR} ${CMAKE_BINARY_DIR}/${projectname})
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
      BUILD_IN_SOURCE 1
      CONFIGURE_COMMAND ""
      BUILD_COMMAND ""
      INSTALL_COMMAND ""
      )

    set(_epgpd_args_to_pass)
    foreach(arg ${_epgpd_one_args})
      if(_ep_${arg})
        list(APPEND _epgpd_args_to_pass ${arg} ${_ep_${arg}})
      endif()
    endforeach()

    ExternalProject_GenerateProjectDescription_Step(${projectname}
      ${_epgpd_args_to_pass}
      )

    set(${_ep_SOURCE_DIR_VAR} ${CMAKE_BINARY_DIR}/${projectname} PARENT_SCOPE)

  else()

    ExternalProject_Add_Empty(${projectname})

  endif()

  if(NOT _ep_LABELS)
    set(_ep_LABELS "")
  endif()
  if(NOT _ep_PROJECTS)
    set(_ep_PROJECTS "")
  endif()
  mark_as_superbuild(VARS ${_ep_SOURCE_DIR_VAR}:PATH LABELS ${_ep_LABELS} PROJECTS ${_ep_PROJECTS})

  foreach(_var ${_ep_VARS})
    string(REPLACE "=" ";" _nametype_and_value ${_var})
    list(LENGTH _nametype_and_value _nametype_and_value_value_length)
    if(NOT _nametype_and_value_value_length EQUAL 2)
      message(FATAL_ERROR "VARS variable expects item [${_var}] to be specified as [<name>:<type>=<value>]")
    endif()
    list(GET _nametype_and_value 0 _nametype)
    list(GET _nametype_and_value 1 _value)
    string(REPLACE ":" ";" _name_and_type ${_nametype})
    list(LENGTH _name_and_type _name_and_type_length)
    if(NOT _name_and_type_length EQUAL 2)
      message(FATAL_ERROR "VARS variable expects item [${_var}] to be specified as [<name>:<type>=<value>]")
    endif()
    list(GET _name_and_type 0 _name)
    list(GET _name_and_type 1 _type)
    set(${_name} ${_value} CACHE ${_type} "Variable set by 'ExternalProject_Add_Source' based on VARS parameter." FORCE)
    #message(STATUS "mark_as_superbuild - _name:${_name} - _type:${_type} - value:${${_name}} - _ep_PROJECTS:${_ep_PROJECTS}")
    mark_as_superbuild(VARS ${_name}:${_type} PROJECTS ${_ep_PROJECTS})
  endforeach()

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
#!     [LABELS REMOTE_MODULE]
#!     [VARS <name1>:<type1>=<value1> [<name2>:<type2>=<value2> [...]]]
#!     #
#!     # See ExternalProject_GenerateProjectDescription_Step
#!     #
#!     [VERSION <version>]
#!     [LICENSE_FILES <file> [...]]
#!   )
#!
#! If no <option_name> is specified or if the option is enabled, the variable "Foo_SOURCE_DIR" set
#! by default to '${CMAKE_BINARY_DIR}/${projectname}' will be passed to Slicer inner build.
#!
#! OPTION_NAME If specified, it adds an advanced option allowing to easily toggle the inclusion of the
#!             associated remote module. That option will be passed to the Slicer inner build.
#!
#! VARS is an expected list of variables specified as <varname>:<vartype>=<varvalue> to pass to <projectname>
#!
#! LABELS By associating the "REMOTE_MODULE" label, the corresponding source directory will be added
#!        automatically using the call 'add_directory'.
#!        This happen in the Slicer/Modules/Remote/CMakeLists.txt file.
#!
macro(Slicer_Remote_Add projectname)
  set(options)
  set(_add_source_args
    DOWNLOAD_DIR URL URL_MD5 GIT_REPOSITORY GIT_TAG SVN_REPOSITORY SVN_USERNAME SVN_PASSWORD SVN_TRUST_CERT
    VERSION LICENSE_FILES
    )
  set(oneValueArgs OPTION_NAME OPTION_DEFAULT OPTION_FORCE SOURCE_DIR_VAR ${_add_source_args})
  set(_add_source_multi_args SVN_REVISION LABELS PROJECTS VARS)
  set(multiValueArgs OPTION_DEPENDS ${_add_source_multi_args})
  cmake_parse_arguments(_ep "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  if(NOT _ep_SOURCE_DIR_VAR)
    set(_ep_SOURCE_DIR_VAR ${projectname}_SOURCE_DIR)
  endif()

  if(_ep_OPTION_NAME AND NOT ${_ep_OPTION_DEFAULT} MATCHES ".+")
    set(_ep_OPTION_DEFAULT ON)
    #message("[${projectname}] Setting default value for ${_ep_OPTION_NAME} OPTION_DEFAULT to ${_ep_OPTION_DEFAULT}")
  endif()

  if(_ep_OPTION_NAME AND NOT ${_ep_OPTION_FORCE} MATCHES ".+")
    set(_ep_OPTION_FORCE OFF)
    #message("[${projectname}] Setting default value for ${_ep_OPTION_NAME} OPTION_FORCE to ${_ep_OPTION_FORCE}")
  endif()

  set(_add_source 1)
  if(_ep_OPTION_NAME)
    #message("[${projectname}] Adding option ${_ep_OPTION_NAME}")

    # If it applies, also account for dependent options.
    foreach(_option IN LISTS _ep_OPTION_DEPENDS)
      set(_prop_name "SLICER_REMOTE_${_option}_OPTION_DEPENDS")
      get_property(_option_depends GLOBAL PROPERTY ${_prop_name})
      if(NOT "${_option_depends}" STREQUAL "")
        list(APPEND _ep_OPTION_DEPENDS ${_option_depends})
      endif()
    endforeach()

    cmake_dependent_option(
      ${_ep_OPTION_NAME} "Download and integrate ${projectname} sources." ${_ep_OPTION_DEFAULT}
      "${_ep_OPTION_DEPENDS}" ${_ep_OPTION_FORCE})
    mark_as_advanced(${_ep_OPTION_NAME})
    mark_as_superbuild(${_ep_OPTION_NAME})
    set(_add_source ${${_ep_OPTION_NAME}})

    # Keep track of dependent options so that options depending on *this* option
    # can expand their own list of dependent options.
    set_property(GLOBAL PROPERTY "SLICER_REMOTE_${_ep_OPTION_NAME}_OPTION_DEPENDS" ${_ep_OPTION_DEPENDS})

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
  elseif(_ep_LABELS)
    if(NOT _ep_PROJECTS)
      set(_ep_PROJECTS "")
    endif()
    ExternalProject_DeclareLabels(
      LABELS ${_ep_LABELS} PROJECTS ${_ep_PROJECTS}
      )
  endif()

endmacro()
