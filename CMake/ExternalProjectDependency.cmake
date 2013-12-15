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

include(CMakeParseArguments)

if(NOT DEFINED EP_LIST_SEPARATOR)
  set(EP_LIST_SEPARATOR "^^")
endif()

if(NOT EXISTS "${EXTERNAL_PROJECT_DIR}")
  set(EXTERNAL_PROJECT_DIR ${CMAKE_SOURCE_DIR}/SuperBuild)
endif()

if(NOT DEFINED EXTERNAL_PROJECT_FILE_PREFIX)
  set(EXTERNAL_PROJECT_FILE_PREFIX "External_")
endif()

# Compute -G arg for configuring external projects with the same CMake generator:
if(CMAKE_EXTRA_GENERATOR)
  set(EP_CMAKE_GENERATOR "${CMAKE_EXTRA_GENERATOR} - ${CMAKE_GENERATOR}")
else()
  set(EP_CMAKE_GENERATOR "${CMAKE_GENERATOR}")
endif()

#!
#! mark_as_superbuild(<varname1>[:<vartype1>] [<varname2>[:<vartype2>] [...]])
#!
#! mark_as_superbuild(
#!     VARS <varname1>[:<vartype1>] [<varname2>[:<vartype2>] [...]]
#!     [PROJECTS <projectname> [<projectname> [...]] | ALL_PROJECTS]
#!     [LABELS <label1> [<label2> [...]]]
#!     [CMAKE_CMD]
#!   )
#!
#! PROJECTS corresponds to a list of <projectname> that will be added using 'ExternalProject_Add' function.
#!          If not specified and called within a project file, it defaults to the value of 'SUPERBUILD_TOPLEVEL_PROJECT'
#!          Otherwise, it defaults to 'CMAKE_PROJECT_NAME'.
#!          If instead 'ALL_PROJECTS' is specified, the variables and labels will be passed to all projects.
#!
#! VARS is an expected list of variables specified as <varname>:<vartype> to pass to <projectname>
#!
#!
#! LABELS is an optional list of label to associate with the variable names specified using 'VARS' and passed to
#!        the <projectname> as CMake CACHE args of the form:
#!          -D<projectname>_EP_LABEL_<label1>=<varname1>;<varname2>[...]
#!          -D<projectname>_EP_LABEL_<label2>=<varname1>;<varname2>[...]
#!
function(mark_as_superbuild)
  set(options ALL_PROJECTS CMAKE_CMD)
  set(oneValueArgs)
  set(multiValueArgs VARS PROJECTS LABELS)
  cmake_parse_arguments(_sb "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  set(_vars ${_sb_UNPARSED_ARGUMENTS})

  set(_named_parameters_expected 0)
  if(_sb_PROJECTS OR _sb_ALL_PROJECTS  OR _sb_LABELS OR _sb_VARS)
    set(_named_parameters_expected 1)
    set(_vars ${_sb_VARS})
  endif()

  if(_named_parameters_expected AND _sb_UNPARSED_ARGUMENTS)
    message(FATAL_ERROR "Arguments '${_sb_UNPARSED_ARGUMENTS}' should be associated with VARS parameter !")
  endif()

  if(_sb_PROJECTS AND _sb_ALL_PROJECTS)
    message(FATAL_ERROR "Arguments 'PROJECTS' and 'ALL_PROJECTS' are mutually exclusive !")
  endif()

  foreach(var ${_vars})
    set(_type_specified 0)
    if(var MATCHES ":")
      set(_type_specified 1)
    endif()
    # XXX Display warning with variable type is also specified for cache variable.
    set(_var ${var})
    if(NOT _type_specified)
      get_property(_type_set_in_cache CACHE ${_var} PROPERTY TYPE SET)
      set(_var_name ${_var})
      set(_var_type "STRING")
      if(_type_set_in_cache)
        get_property(_var_type CACHE ${_var_name} PROPERTY TYPE)
      endif()
      set(_var ${_var_name}:${_var_type})
    endif()
    list(APPEND _vars_with_type ${_var})
  endforeach()

  if(_sb_CMAKE_CMD)
    set(optional_arg_CMAKE_CMD "CMAKE_CMD")
  endif()

  if(_sb_ALL_PROJECTS)
    set(optional_arg_ALL_PROJECTS "ALL_PROJECTS")
  else()
    set(optional_arg_ALL_PROJECTS PROJECTS ${_sb_PROJECTS})
  endif()

  _sb_append_to_cmake_args(
    VARS ${_vars_with_type} LABELS ${_sb_LABELS} ${optional_arg_ALL_PROJECTS} ${optional_arg_CMAKE_CMD})
endfunction()

#!
#! _sb_extract_varname_and_vartype(<cmake_varname_and_type> <varname_var> [<vartype_var>])
#!
#! <cmake_varname_and_type> corresponds to variable name and variable type passed as "<varname>:<vartype>"
#!
#! <varname_var> will be set to "<varname>"
#!
#! <vartype_var> is an optional variable name that will be set to "<vartype>"
function(_sb_extract_varname_and_vartype cmake_varname_and_type varname_var)
  set(_vartype_var "${ARGV2}")
  string(REPLACE ":" ";" varname_and_vartype ${cmake_varname_and_type})
  list(GET varname_and_vartype 0 _varname)
  list(GET varname_and_vartype 1 _vartype)
  set(${varname_var} ${_varname} PARENT_SCOPE)
  if(_vartype_var MATCHES ".+")
    set(${_vartype_var} ${_vartype} PARENT_SCOPE)
  endif()
endfunction()


function(_sb_list_to_string separator input_list output_string_var)
  set(_string "")
  cmake_policy(PUSH)
  cmake_policy(SET CMP0007 OLD)
  # Get list length
  list(LENGTH input_list list_length)
  # If the list has 0 or 1 element, there is no need to loop over.
  if(list_length LESS 2)
    set(_string  "${input_list}")
  else()
    math(EXPR last_element_index "${list_length} - 1")
    foreach(index RANGE ${last_element_index})
      # Get current item_value
      list(GET input_list ${index} item_value)
      # .. and append to output string
      set(_string  "${_string}${item_value}")
      # Append separator if current element is NOT the last one.
      if(NOT index EQUAL last_element_index)
        set(_string  "${_string}${separator}")
      endif()
    endforeach()
  endif()
  set(${output_string_var} ${_string} PARENT_SCOPE)
  cmake_policy(POP)
endfunction()


#!
#! _sb_cmakevar_to_cmakearg(<cmake_varname_and_type> <cmake_arg_var> <cmake_arg_type> [<varname_var> [<vartype_var>]])
#!
#! <cmake_varname_and_type> corresponds to variable name and variable type passed as "<varname>:<vartype>"
#!
#! <cmake_arg_var> is a variable name that will be set to "-D<varname>:<vartype>=${<varname>}"
#!
#! <cmake_arg_type> is set to either CMAKE_CACHE or CMAKE_CMD.
#!                  CMAKE_CACHE means that the generated cmake argument will be passed to
#!                  ExternalProject_Add as CMAKE_CACHE_ARGS.
#!                  CMAKE_CMD means that the generated cmake argument will be passed to
#!                  ExternalProject_Add as CMAKE_ARGS.
#!
#! <varname_var> is an optional variable name that will be set to "<varname>"
#!
#! <vartype_var> is an optional variable name that will be set to "<vartype>"
function(_sb_cmakevar_to_cmakearg cmake_varname_and_type cmake_arg_var cmake_arg_type)
  set(_varname_var "${ARGV3}")
  set(_vartype_var "${ARGV4}")

  # XXX Add check for <cmake_arg_type> value

  _sb_extract_varname_and_vartype(${cmake_varname_and_type} _varname _vartype)

  set(_var_value "${${_varname}}")
  get_property(_value_set_in_cache CACHE ${_varname} PROPERTY VALUE SET)
  if(_value_set_in_cache)
    get_property(_var_value CACHE ${_varname} PROPERTY VALUE)
  endif()

  if(cmake_arg_type STREQUAL "CMAKE_CMD")
    # Separate list item with <EP_LIST_SEPARATOR>
    _sb_list_to_string(${EP_LIST_SEPARATOR} "${_var_value}" _var_value)
  endif()

  set(${cmake_arg_var} -D${_varname}:${_vartype}=${_var_value} PARENT_SCOPE)

  if(_varname_var MATCHES ".+")
    set(${_varname_var} ${_varname} PARENT_SCOPE)
  endif()
  if(_vartype_var MATCHES ".+")
    set(${_vartype_var} ${_vartype} PARENT_SCOPE)
  endif()
endfunction()

set(_ALL_PROJECT_IDENTIFIER "ALLALLALL")

#!
#! _sb_append_to_cmake_args(
#!     VARS <varname1>:<vartype1> [<varname2>:<vartype2> [...]]
#!     [PROJECTS <projectname> [<projectname> [...]] | ALL_PROJECTS]
#!     [LABELS <label1> [<label2> [...]]]
#!     [CMAKE_CMD]
#!   )
#!
#! PROJECTS corresponds to a list of <projectname> that will be added using 'ExternalProject_Add' function.
#!          If not specified and called within a project file, it defaults to the value of 'SUPERBUILD_TOPLEVEL_PROJECT'
#!          Otherwise, it defaults to 'CMAKE_PROJECT_NAME'.
#!          If instead 'ALL_PROJECTS' is specified, the variables and labels will be passed to all projects.
#!
#! VARS is an expected list of variables specified as <varname>:<vartype> to pass to <projectname>
#!
#!
#! LABELS is an optional list of label to associate with the variable names specified using 'VARS' and passed to
#!        the <projectname> as CMake CACHE args of the form:
#!          -D<projectname>_EP_LABEL_<label1>=<varname1>;<varname2>[...]
#!          -D<projectname>_EP_LABEL_<label2>=<varname1>;<varname2>[...]
#!
function(_sb_append_to_cmake_args)
  set(options ALL_PROJECTS CMAKE_CMD)
  set(oneValueArgs)
  set(multiValueArgs VARS PROJECTS LABELS)
  cmake_parse_arguments(_sb "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  if(NOT _sb_PROJECTS AND NOT _sb_ALL_PROJECTS)
    if(SUPERBUILD_TOPLEVEL_PROJECT)
      set(_sb_PROJECTS ${SUPERBUILD_TOPLEVEL_PROJECT})
    else()
      set(_sb_PROJECTS ${CMAKE_PROJECT_NAME})
    endif()
  endif()

  if(_sb_ALL_PROJECTS)
    set(_sb_PROJECTS ${_ALL_PROJECT_IDENTIFIER})
  endif()

  foreach(_sb_PROJECT ${_sb_PROJECTS})
    set(_cmake_arg_type "CMAKE_CACHE")
    if(_sb_CMAKE_CMD)
      set(_cmake_arg_type "CMAKE")
      set(optional_arg_CMAKE_CMD "CMAKE_CMD")
    endif()
    set(_ep_property "${_cmake_arg_type}_ARGS")
    set(_ep_varnames "")
    foreach(varname_and_vartype ${_sb_VARS})
      if(NOT TARGET ${_sb_PROJECT})
        set_property(GLOBAL APPEND PROPERTY ${_sb_PROJECT}_EP_${_ep_property} ${varname_and_vartype})
        _sb_extract_varname_and_vartype(${varname_and_vartype} _varname)
      else()
        message(FATAL_ERROR "Function _sb_append_to_cmake_args not allowed because project '${_sb_PROJECT}' already added !")
      endif()
      list(APPEND _ep_varnames ${_varname})
    endforeach()

    if(_sb_LABELS)
      set_property(GLOBAL APPEND PROPERTY ${_sb_PROJECT}_EP_LABELS ${_sb_LABELS})
      foreach(label ${_sb_LABELS})
        set_property(GLOBAL APPEND PROPERTY ${_sb_PROJECT}_EP_LABEL_${label} ${_ep_varnames})
      endforeach()
    endif()
  endforeach()
endfunction()

function(_sb_get_external_project_arguments proj varname)

  mark_as_superbuild(${SUPERBUILD_TOPLEVEL_PROJECT}_USE_SYSTEM_${proj}:BOOL)

  function(_sb_collect_args proj)
    # Set list of CMake args associated with each label
    get_property(_labels GLOBAL PROPERTY ${proj}_EP_LABELS)
    if(_labels)
      list(REMOVE_DUPLICATES _labels)
      foreach(label ${_labels})
        get_property(${proj}_EP_LABEL_${label} GLOBAL PROPERTY ${proj}_EP_LABEL_${label})
        list(REMOVE_DUPLICATES ${proj}_EP_LABEL_${label})
        _sb_append_to_cmake_args(PROJECTS ${proj}
          VARS ${proj}_EP_LABEL_${label}:STRING)
      endforeach()
    endif()

    foreach(cmake_arg_type CMAKE_CMD CMAKE_CACHE)

      set(_ep_property "CMAKE_CACHE_ARGS")
      if(cmake_arg_type STREQUAL "CMAKE_CMD")
        set(_ep_property "CMAKE_ARGS")
      endif()

      get_property(_args GLOBAL PROPERTY ${proj}_EP_${_ep_property})
      foreach(var ${_args})
        _sb_cmakevar_to_cmakearg(${var} cmake_arg ${cmake_arg_type})
        set_property(GLOBAL APPEND PROPERTY ${proj}_EP_PROPERTY_${_ep_property} ${cmake_arg})
      endforeach()

    endforeach()
  endfunction()

  _sb_collect_args(${proj})
  _sb_collect_args(${_ALL_PROJECT_IDENTIFIER})

  set(_ep_arguments "")
  foreach(property CMAKE_ARGS CMAKE_CACHE_ARGS)
    get_property(${proj}_EP_PROPERTY_${property} GLOBAL PROPERTY ${proj}_EP_PROPERTY_${property})
    get_property(${_ALL_PROJECT_IDENTIFIER}_EP_PROPERTY_${property} GLOBAL PROPERTY ${_ALL_PROJECT_IDENTIFIER}_EP_PROPERTY_${property})
    set(_all ${${proj}_EP_PROPERTY_${property}} ${${_ALL_PROJECT_IDENTIFIER}_EP_PROPERTY_${property}})
    if(_all)
      list(REMOVE_DUPLICATES _all)
      list(APPEND _ep_arguments ${property} ${_all})
    endif()
  endforeach()

  list(APPEND _ep_arguments LIST_SEPARATOR ${EP_LIST_SEPARATOR})

  list(APPEND _ep_arguments CMAKE_GENERATOR ${EP_CMAKE_GENERATOR})

  set(${varname} ${_ep_arguments} PARENT_SCOPE)
endfunction()

function(_sb_update_indent proj)
  superbuild_stack_size(SB_PROJECT_STACK _stack_size)
  set(_indent "")
  if(_stack_size GREATER 0)
    foreach(not_used RANGE 1 ${_stack_size})
      set(_indent "  ${_indent}")
    endforeach()
  endif()
  set_property(GLOBAL PROPERTY SUPERBUILD_${proj}_INDENT ${_indent})
endfunction()

#!
#! ExternalProject_Message(<project_name> <msg> [condition])
#!
function(ExternalProject_Message proj msg)
  set(_display 1)
  if("${ARGV2}" MATCHES ".+")
    set(_display ${ARGN})
  endif()
  if(${_display})
    get_property(_indent GLOBAL PROPERTY SUPERBUILD_${proj}_INDENT)
    message(STATUS "SuperBuild - ${_indent}${msg}")
  endif()
endfunction()

#!
#! superbuild_stack_content(<stack_name> <output_var>)
#!
#! <stack_name> corresponds to the name of stack.
#!
#! <output_var> is the name of CMake variable that will be set with the content
#! of the stack identified by <stack_name>.
function(superbuild_stack_content stack_name output_var)
  get_property(_stack GLOBAL PROPERTY ${stack_name})
  set(${output_var} ${_stack} PARENT_SCOPE)
endfunction()

#!
#! superbuild_stack_size(<stack_name> <output_var>)
#!
#! <stack_name> corresponds to the name of stack.
#!
#! <output_var> is the name of CMake variable that will be set with the size
#! of the stack identified by <stack_name>.
function(superbuild_stack_size stack_name output_var)
  get_property(_stack GLOBAL PROPERTY ${stack_name})
  list(LENGTH _stack _stack_size)
  set(${output_var} ${_stack_size} PARENT_SCOPE)
endfunction()

#!
#! superbuild_stack_push(<stack_name> <value>)
#!
#! <stack_name> corresponds to the name of stack.
#!
#! <value> is appended to the stack identified by <stack_name>.
function(superbuild_stack_push stack_name value)
  set_property(GLOBAL APPEND PROPERTY ${stack_name} ${value})
endfunction()

#!
#! superbuild_stack_pop(<stack_name> <item_var>)
#!
#! <stack_name> corresponds to the name of stack.
#!
#! <item_var> names a CMake variable that will be set with the item
#! removed from the stack identified by <stack_name>.
function(superbuild_stack_pop stack_name item_var)
  get_property(_stack GLOBAL PROPERTY ${stack_name})
  list(LENGTH _stack _stack_size)
  if(_stack_size GREATER 0)
    math(EXPR _index_to_remove "${_stack_size} - 1")
    list(GET _stack ${_index_to_remove} _item)
    list(REMOVE_AT _stack ${_index_to_remove})
    set_property(GLOBAL PROPERTY ${stack_name} ${_stack})
    set(${item_var} ${_item} PARENT_SCOPE)
  endif()
endfunction()

function(_sb_is_optional proj output_var)
  set(_include_project 1)
  if(COMMAND superbuild_is_external_project_includable)
    superbuild_is_external_project_includable("${proj}" _include_project)
  endif()
  set(optional 1)
  if(_include_project)
    set(optional 0)
  endif()
  set(${output_var} ${optional} PARENT_SCOPE)
endfunction()


#!
#! ExternalProject_Include_Dependencies(<project_name>
#!     [PROJECT_VAR <project_var>]
#!     [EP_ARGS_VAR <external_project_args_var>]
#!     [DEPENDS_VAR <depends_var>]
#!     [USE_SYSTEM_VAR <use_system_var>]
#!     [SUPERBUILD_VAR <superbuild_var>]
#!   )
#!
macro(ExternalProject_Include_Dependencies project_name)
  set(options)
  set(oneValueArgs PROJECT_VAR DEPENDS_VAR EP_ARGS_VAR USE_SYSTEM_VAR SUPERBUILD_VAR)
  set(multiValueArgs)
  cmake_parse_arguments(_sb "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  # Sanity checks
  if(x${project_name} STREQUAL xPROJECT_VAR
      OR x${project_name} STREQUAL xEP_ARGS_VAR
      OR x${project_name} STREQUAL xDEPENDS_VAR
      OR x${project_name} STREQUAL xUSE_SYSTEM_VAR
      OR x${project_name} STREQUAL xSUPERBUILD_VAR
      )
    message(FATAL_ERROR "Argument <project_name> is missing !")
  endif()
  if(_sb_UNPARSED_ARGUMENTS)
    message(FATAL_ERROR "Invalid arguments: ${_sb_UNPARSED_ARGUMENTS}")
  endif()

  # Set default for optional PROJECT_VAR parameter
  if(NOT _sb_PROJECT_VAR)
    set(_sb_PROJECT_VAR proj)
    set(${_sb_PROJECT_VAR} ${project_name})
    #message("[${project_name}] Setting _sb_PROJECT_VAR with default value '${_sb_PROJECT_VAR}'")
  endif()

  if(_sb_PROJECT_VAR AND NOT x${project_name} STREQUAL x${${_sb_PROJECT_VAR}})
    message(FATAL_ERROR
      "Argument <project_name>:${project_name} and PROJECT_VAR:${_sb_PROJECT_VAR}:${${_sb_PROJECT_VAR}} are different !")
  endif()

  set(_sb_proj ${project_name})

  # Skip if project already included
  get_property(_is_included GLOBAL PROPERTY SB_${_sb_proj}_FILE_INCLUDED)
  if(_is_included)
    return()
  endif()

  # Set default for optional DEPENDS_VAR and EP_ARGS parameters
  foreach(param DEPENDS EP_ARGS)
    if(NOT _sb_${param}_VAR)
      set(_sb_${param}_VAR ${_sb_proj}_${param})
      #message("[${project_name}] Setting _sb_${param}_VAR with default value '${_sb_${param}_VAR}'")
    endif()
  endforeach()

  # Set top level project
  superbuild_stack_size(SB_PROJECT_STACK _stack_size)
  if(_stack_size EQUAL 0)
    set(SUPERBUILD_TOPLEVEL_PROJECT ${_sb_proj})
  endif()

  # Set default for optional USE_SYSTEM_VAR parameter
  if(NOT _sb_USE_SYSTEM_VAR)
    set(_sb_USE_SYSTEM_VAR ${SUPERBUILD_TOPLEVEL_PROJECT}_USE_SYSTEM_${_sb_proj})
    #message("[${project_name}] Setting _sb_USE_SYSTEM_VAR with default value '${_sb_USE_SYSTEM_VAR}'")
  endif()

  # Set default for optional SUPERBUILD_VAR parameter
  if(NOT _sb_SUPERBUILD_VAR)
    set(_sb_SUPERBUILD_VAR ${SUPERBUILD_TOPLEVEL_PROJECT}_SUPERBUILD)
    #message("[${project_name}] Setting _sb_SUPERBUILD_VAR with default value '${_sb_SUPERBUILD_VAR}'")
  endif()

  # Keeping track of variable name independently of the recursion
  if(NOT DEFINED _sb_SB_VAR)
    set(_sb_SB_VAR ${_sb_SUPERBUILD_VAR})
    #message("[${project_name}] Setting _sb_SB_VAR with default value '${_sb_SB_VAR}'")
  endif()

  # Set local variables
  set(_sb_DEPENDS ${${_sb_DEPENDS_VAR}})
  set(_sb_USE_SYSTEM ${${_sb_USE_SYSTEM_VAR}})

  _sb_update_indent(${_sb_proj})

  # Keep track of the projects
  list(APPEND SB_${SUPERBUILD_TOPLEVEL_PROJECT}_POSSIBLE_DEPENDS ${_sb_proj})

  # Use system ?
  get_property(_use_system_set GLOBAL PROPERTY SB_${_sb_proj}_USE_SYSTEM SET)
  if(_use_system_set)
    get_property(_sb_USE_SYSTEM GLOBAL PROPERTY SB_${_sb_proj}_USE_SYSTEM)
  endif()

  # Is this the first run ?
  if(${_sb_proj} STREQUAL ${SUPERBUILD_TOPLEVEL_PROJECT} AND NOT DEFINED SB_FIRST_PASS)
    message(STATUS "SuperBuild - First pass")
    set(SB_FIRST_PASS TRUE)
  endif()

  set(_sb_REQUIRED_DEPENDS)
  foreach(dep ${_sb_DEPENDS})
    if(NOT ${_sb_proj} STREQUAL ${SUPERBUILD_TOPLEVEL_PROJECT})
      if(_sb_USE_SYSTEM)
        set_property(GLOBAL PROPERTY SB_${dep}_USE_SYSTEM ${_sb_USE_SYSTEM})
        #message(${_sb_proj} "Property SB_${dep}_USE_SYSTEM set to [${_sb_USE_SYSTEM_VAR}:${_sb_USE_SYSTEM}]")
      endif()
    endif()
    _sb_is_optional(${dep} _optional)
    set_property(GLOBAL PROPERTY SB_${dep}_OPTIONAL ${_optional})
    #message(${_sb_proj} "[${_sb_proj}] Property SB_${dep}_OPTIONAL set to ${_optional}")
    if(NOT _optional)
      list(APPEND _sb_REQUIRED_DEPENDS ${dep})
    endif()
  endforeach()

  # Display dependency of project being processed
  if(_sb_REQUIRED_DEPENDS AND SB_SECOND_PASS AND ${_sb_SB_VAR})
    set(dependency_str "")
    foreach(dep ${_sb_REQUIRED_DEPENDS})
      get_property(_is_included GLOBAL PROPERTY SB_${dep}_FILE_INCLUDED)
      set(_include_status "")
      if(_is_included)
        set(_include_status "[INCLUDED]")
      endif()
      set(dependency_str "${dependency_str}${dep}${_include_status}, ")
    endforeach()
    ExternalProject_Message(${_sb_proj} "${_sb_proj} => Requires ${dependency_str}")
  endif()

  # Save variables
  set_property(GLOBAL PROPERTY SB_${_sb_proj}_REQUIRED_DEPENDS ${_sb_REQUIRED_DEPENDS})
  set_property(GLOBAL PROPERTY SB_${_sb_proj}_DEPENDS          ${_sb_DEPENDS})
  set_property(GLOBAL PROPERTY SB_${_sb_proj}_DEPENDS_VAR      ${_sb_DEPENDS_VAR})
  set_property(GLOBAL PROPERTY SB_${_sb_proj}_EP_ARGS_VAR      ${_sb_EP_ARGS_VAR})
  set_property(GLOBAL PROPERTY SB_${_sb_proj}_USE_SYSTEM       ${_sb_USE_SYSTEM})
  set_property(GLOBAL PROPERTY SB_${_sb_proj}_USE_SYSTEM_VAR   ${_sb_USE_SYSTEM_VAR})
  set_property(GLOBAL PROPERTY SB_${_sb_proj}_PROJECT_VAR      ${_sb_PROJECT_VAR})
  superbuild_stack_push(SB_PROJECT_STACK ${_sb_proj})

  # Include dependencies
  foreach(dep ${_sb_DEPENDS})
    get_property(_included GLOBAL PROPERTY SB_${dep}_FILE_INCLUDED)
    if(NOT _included)
      # XXX - Refactor - Add a single variable named 'EXTERNAL_PROJECT_DIRS'
      if(EXISTS "${EXTERNAL_PROJECT_DIR}/${EXTERNAL_PROJECT_FILE_PREFIX}${dep}.cmake")
        include(${EXTERNAL_PROJECT_DIR}/${EXTERNAL_PROJECT_FILE_PREFIX}${dep}.cmake)
      elseif(EXISTS "${${dep}_FILEPATH}")
        include(${${dep}_FILEPATH})
      elseif(EXISTS "${EXTERNAL_PROJECT_ADDITIONAL_DIR}/${EXTERNAL_PROJECT_FILE_PREFIX}${dep}.cmake")
        include(${EXTERNAL_PROJECT_ADDITIONAL_DIR}/${EXTERNAL_PEXCLUDEDROJECT_FILE_PREFIX}${dep}.cmake)
      else()
        message(FATAL_ERROR "Can't find ${EXTERNAL_PROJECT_FILE_PREFIX}${dep}.cmake")
      endif()
      set_property(GLOBAL PROPERTY SB_${dep}_FILE_INCLUDED 1)
    endif()
  endforeach()

  # Restore variables
  superbuild_stack_pop(SB_PROJECT_STACK _sb_proj)
  get_property(_sb_PROJECT_VAR      GLOBAL PROPERTY SB_${_sb_proj}_PROJECT_VAR)
  get_property(_sb_USE_SYSTEM_VAR   GLOBAL PROPERTY SB_${_sb_proj}_USE_SYSTEM_VAR)
  get_property(_sb_USE_SYSTEM       GLOBAL PROPERTY SB_${_sb_proj}_USE_SYSTEM)
  get_property(_sb_EP_ARGS_VAR      GLOBAL PROPERTY SB_${_sb_proj}_EP_ARGS_VAR)
  get_property(_sb_DEPENDS_VAR      GLOBAL PROPERTY SB_${_sb_proj}_DEPENDS_VAR)
  get_property(_sb_DEPENDS          GLOBAL PROPERTY SB_${_sb_proj}_DEPENDS)
  get_property(_sb_REQUIRED_DEPENDS GLOBAL PROPERTY SB_${_sb_proj}_REQUIRED_DEPENDS)

  # Use system ?
  set(_include_type "")
  if(_sb_USE_SYSTEM)
    set(_include_type " (SYSTEM)")
  endif()
  get_property(_optional GLOBAL PROPERTY SB_${_sb_proj}_OPTIONAL)
  ExternalProject_Message(${_sb_proj} "${_sb_proj}[OK]${_include_type}" SB_SECOND_PASS AND ${_sb_SB_VAR} AND NOT _optional)

  if(${_sb_proj} STREQUAL ${SUPERBUILD_TOPLEVEL_PROJECT} AND SB_FIRST_PASS)
    set(SB_FIRST_PASS FALSE)
    ExternalProject_Message(${_sb_proj} "First pass - done")

    if(${_sb_SB_VAR})
      foreach(possible_proj ${SB_${SUPERBUILD_TOPLEVEL_PROJECT}_POSSIBLE_DEPENDS})
        get_property(_optional GLOBAL PROPERTY SB_${possible_proj}_OPTIONAL)
        if(_optional)
          ExternalProject_Message(${_sb_proj} "${possible_proj}[OPTIONAL]")
        endif()
        set_property(GLOBAL PROPERTY SB_${possible_proj}_FILE_INCLUDED 0)
      endforeach()

      set(${_sb_PROJECT_VAR} ${_sb_proj})

      set(SB_SECOND_PASS TRUE)
      ExternalProject_Include_Dependencies(${_sb_proj}
        PROJECT_VAR ${_sb_PROJECT_VAR}
        DEPENDS_VAR ${_sb_DEPENDS_VAR}
        EP_ARGS_VAR ${_sb_EP_ARGS_VAR}
        USE_SYSTEM_VAR _sb_USE_SYSTEM
        SUPERBUILD_VAR ${_sb_SB_VAR}
        )
      set(SB_SECOND_PASS FALSE)
    endif()
  endif()

  if(SB_FIRST_PASS OR _optional)
    if(NOT ${_sb_proj} STREQUAL ${SUPERBUILD_TOPLEVEL_PROJECT})
      return()
    endif()
  endif()

  if(SB_SECOND_PASS)
    _sb_get_external_project_arguments(${_sb_proj} ${_sb_EP_ARGS_VAR})
  endif()

  if(NOT SB_FIRST_PASS AND NOT SB_SECOND_PASS
      AND ${_sb_proj} STREQUAL ${SUPERBUILD_TOPLEVEL_PROJECT})
    #ExternalProject_Message(${_sb_proj} "Clean up")
    unset(_sb_SB_VAR)
    unset(SB_FIRST_PASS)
    unset(SB_SECOND_PASS)
  endif()

  # Set public variables
  set(${_sb_PROJECT_VAR} ${_sb_proj})
  set(${_sb_DEPENDS_VAR} ${_sb_REQUIRED_DEPENDS})
  set(${_sb_USE_SYSTEM_VAR} ${_sb_USE_SYSTEM})

  #message("[${_sb_proj}] #################################")
  #message("[${_sb_proj}] Setting ${_sb_PROJECT_VAR}:${_sb_proj}")
  #message("[${_sb_proj}] Setting ${_sb_EP_ARGS_VAR}:${${_sb_EP_ARGS_VAR}}")
  #message("[${_sb_proj}] Setting ${_sb_DEPENDS_VAR}:${${_sb_DEPENDS_VAR}}")
  #message("[${_sb_proj}] Setting ${_sb_USE_SYSTEM_VAR}:${_sb_USE_SYSTEM}")
endmacro()


#!
#! ExternalProject_Add_Empty(<project_name>
#!     DEPENDS <depends>
#!   )
#!
macro(ExternalProject_Add_Empty project_name)
  set(options)
  set(oneValueArgs)
  set(multiValueArgs DEPENDS)
  cmake_parse_arguments(_sb "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  # Sanity checks
  if(x${project_name} STREQUAL xDEPENDS)
    message(FATAL_ERROR "Argument <project_name> is missing !")
  endif()
  if(_sb_UNPARSED_ARGUMENTS)
    message(FATAL_ERROR "Invalid arguments: ${_sb_UNPARSED_ARGUMENTS}")
  endif()

  ExternalProject_Add(${project_name}
    SOURCE_DIR ${CMAKE_BINARY_DIR}/${project_name}
    BINARY_DIR ${project_name}-build
    DOWNLOAD_COMMAND ""
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    DEPENDS ${_sb_DEPENDS}
    )
endmacro()
