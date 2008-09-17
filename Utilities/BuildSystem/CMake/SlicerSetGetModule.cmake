cmake_minimum_required(VERSION 2.5)

# ---------------------------------------------------------------------------
# slicer_set_module_value: set a module value.
#
# This function can be used to set a module value without worrying too much
# about the underlying data structure. 
# Implementation: values are stored as key/value pairs at the moment.
#
# Arguments:
# in:
#   key (string): key (no spaces, should be similar to a variable name)
#   value (string): value (or list of values)
# in/out:
#   module_varname (string): var name to be used to store all module values
# 
# Example:
#   slicer_set_module_value(TestModule Author "John Doe")
#   slicer_set_module_value(TestModule MyList Elem1 Elem2 Elem3)
#
# See also:
#   slicer_get_module_value
#   slicer_unset_module_value
# ---------------------------------------------------------------------------

function(slicer_set_module_value module_varname key)
  
  set_property(GLOBAL PROPERTY "_${module_varname}_${key}" ${ARGN})

endfunction(slicer_set_module_value)

# ---------------------------------------------------------------------------
# slicer_get_module_value: get a module value.
#
# This function can be used to retrieve a module value without worrying too much
# about the underlying data structure. 
# Implementation: values are stored as key/value pairs at the moment.
# If the module has no value for that key, the resulting variable 
# (value_varname) will be unset.
#
# Arguments:
# in:
#   module_varname (string): var name used to store all module values
#   key (string): key
# in/out:
#   value_varname (string): var name to use to store the specific value
# 
# Example:
#   slicer_get_module_value(TestModule Author authors)
#   message("Author(s): ${author}")
#
# See also:
#   slicer_set_module_value
#   slicer_unset_module_value
# ---------------------------------------------------------------------------

function(slicer_get_module_value module_varname key value_varname)

  get_property(is_set GLOBAL PROPERTY "_${module_varname}_${key}" SET)

  if(is_set)
    get_property(value GLOBAL PROPERTY "_${module_varname}_${key}")
    set(${value_varname} ${value} PARENT_SCOPE)
  else(is_set)
    set(${value_varname} PARENT_SCOPE)
  endif(is_set)

endfunction(slicer_get_module_value)

# ---------------------------------------------------------------------------
# slicer_unset_module_value: unset a module value.
#
# This function can be used to unset a module value without worrying too much
# about the underlying data structure. 
# Implementation: values are stored as key/value pairs at the moment.
#
# Arguments:
# in:
#   key (string): key
# in/out:
#   module_varname (string): var name used to store all module values
# 
# Example:
#   slicer_unset_module_value(TestModule "Author")
#
# See also:
#   slicer_get_module_value
#   slicer_set_module_value
# ---------------------------------------------------------------------------

function(slicer_unset_module_value module_varname key)
 
  # until Ken gets one 
  set_property(GLOBAL PROPERTY "_${module_varname}_${key}")

endfunction(slicer_unset_module_value)

# ---------------------------------------------------------------------------
# slicer_is_module_unknown: check if a module is unknown.
#
# This function can be used to check if a module is unknown, i.e. if
# some of its core values are not in memory (most likely the module name was
# mistyped, or the module was not parsed yet).
# If the module is unknown and an optional error message is passed, it will
# be output using MESSAGE(SEND_ERROR ...) .
#
# Arguments:
# in:
#   module_varname (string): var name used to store all module values
# out:
#   bool_varname (string): var name to use to store if module is unknown
# optional in:
#   error_msg (string): optional error message
# 
# Example:
#   slicer_is_module_unknown(TestModule unknown "Unknown module TestModule!")
#
# See also:
#   slicer_get_module_value
#   slicer_set_module_value
# ---------------------------------------------------------------------------

function(slicer_is_module_unknown module_varname bool_varname)
  
  slicer_get_module_value(${module_varname} Name name)
  if(NOT name)
    set(${bool_varname} 1 PARENT_SCOPE)
    if(ARGN)
      message(SEND_ERROR "Unknown module ${module_varname}. ${ARGN}")
    endif(ARGN)
  else(NOT name)
    set(${bool_varname} 0 PARENT_SCOPE)
  endif(NOT name)

endfunction(slicer_is_module_unknown)

# ---------------------------------------------------------------------------
# slicer_get_module_short_description: get a module short description.
#
# This function uses the current module variables (key/values) to create a 
# short module description.
#
# Arguments:
# in:
#   module_varname (string): var name used to store all module keys/values
# out:
#   desc_varname (string): var name to use to store the short description
# 
# Example:
#   slicer_get_module_short_description(TestModule desc)
#   message("TestModule Short Description: ${desc}")
#
# See also:
#   slicer_get_module_value
#   slicer_set_module_value
# ---------------------------------------------------------------------------

function(slicer_get_module_short_description module_varname desc_varname)

  # Unknown module? Bail.

  slicer_is_module_unknown(
    ${module_varname} unknown "Unable to create short description!")
  if(unknown)
    return()
  endif(unknown)

  # Create the short description by assembling various values

  slicer_get_module_value(${module_varname} Name name)
  if(name)
    set(short_desc "${name}")
  endif(name)

  slicer_get_module_value(${module_varname} Version version)
  if(version)
    set(short_desc "${short_desc} ${version}")
  endif(version)

  slicer_get_module_value(${module_varname} Author authors)
  if(authors)
    set(short_desc "${short_desc} (${authors})")
  endif(authors)

  set(short_desc "${short_desc}.")

  slicer_get_module_value(${module_varname} Description desc)
  if(desc)
    set(short_desc "${short_desc} ${desc}")
  endif(desc)

  set(${desc_varname} ${short_desc} PARENT_SCOPE)

endfunction(slicer_get_module_short_description)

# ---------------------------------------------------------------------------
# slicer_get_module_source_repository_type: get a module source repository type.
#
# This function can be used to retrieve the type of source repository the 
# module is using.
# Will return either "cvs", "svn", or unset the var if unknown.
#
# Arguments:
# in:
#   module_varname (string): var name used to store all module keys/values
# out:
#   type_varname (string): var name to use to store the type
# 
# Example:
#   slicer_get_module_source_repository_type(TestModule type)
#   if(type STREQUAL "svn")
#     ...
#   endif(type STREQUAL "svn")
#
# See also:
#   slicer_get_module_value
#   slicer_set_module_value
# ---------------------------------------------------------------------------

function(slicer_get_module_source_repository_type module_varname type_varname)

  # Unknown module? Bail.

  slicer_is_module_unknown(
    ${module_varname} unknown "Unable to get repository type!")
  if(unknown)
    return()
  endif(unknown)

  # Parse the SourceLocation for some hings about the repository type

  set(${type_varname} PARENT_SCOPE)

  slicer_get_module_value(${module_varname} SourceLocation source_loc)
  if(source_loc)
    string(REGEX MATCH "^https?://.+$" found_svn "${source_loc}")
    if(found_svn)
      set(${type_varname} "svn" PARENT_SCOPE)
    else(found_svn)
      string(REGEX MATCH "^:.+:.+:.+$" found_cvs "${source_loc}")
      if(found_cvs)
        set(${type_varname} "cvs" PARENT_SCOPE)
      endif(found_cvs)
    endif(found_svn)
  endif(source_loc)

endfunction(slicer_get_module_source_repository_type)

# ---------------------------------------------------------------------------
# slicer_get_module_source_tag: get a module source tag.
#
# This function can be used to retrieve the tag/branch for the source 
# repository the module is using (if any).
# Will return either the tag, or unset the var if unknown.
#
# Arguments:
# in:
#   module_varname (string): var name used to store all module keys/values
# out:
#   tag_varname (string): var name to use to store the type
# 
# Example:
#
# See also:
#   slicer_get_module_value
#   slicer_set_module_value
# ---------------------------------------------------------------------------

function(slicer_get_module_source_tag module_varname tag_varname)

  # Unknown module? Bail.

  slicer_is_module_unknown(
    ${module_varname} unknown "Unable to get repository type!")
  if(unknown)
    return()
  endif(unknown)

  slicer_get_module_source_repository_type(${module_varname} type)

  set(${tag_varname} PARENT_SCOPE)

  # If SVN 

  if(type STREQUAL "svn")

    slicer_get_module_value(${module_varname} SourceLocation source_loc)
    if(source_loc)
      string(REGEX MATCH "/branches/([^/]+)/?$" tag "${source_loc}")
      # TODO!!
      message("${tag}; ${CMAKE_MATCH_0}; ${CMAKE_MATCH_1}")
    endif(source_loc)

  # If CVS

  elseif(type STREQUAL "cvs")

    slicer_get_module_value(${module_varname} CVSTag cvs_tag)
    if(cvs_tag)
      set(${tag_varname} ${cvs_tag} PARENT_SCOPE)
    endif(cvs_tag)

  endif(type STREQUAL "svn")

endfunction(slicer_get_module_source_tag)

# ---------------------------------------------------------------------------
# slicer_get_module_cache_directory: get a module cache directory.
#
# This function can be used to get the path to a local cache directory where
# module-specific files can be stored. This can be used to write configured
# file for a specific module, or intermediate CMake scripts that are used
# in module targets, etc.
#
# Note: this function does not create the cache directory automatically.
#
# Arguments:
# in:
#   module_varname (string): var name used to store all module keys/values
# out:
#   dir_varname (string): var name to use to store the module car dir
# 
# Example:
#   slicer_get_module_cache_directory(TestModule dir)
#   configure_file("${CMAKE_CURRENT_SOURCE_DIR}/foo.in" "${dir}/foo")
#
# See also:
#   slicer_get_module_value
#   slicer_set_module_value
# ---------------------------------------------------------------------------

function(slicer_get_module_cache_directory module_varname dir_varname)

  set(${dir_varname} 
    "${PROJECT_BINARY_DIR}/ModuleCache/${module_varname}" PARENT_SCOPE)
  
endfunction(slicer_get_module_cache_directory)

