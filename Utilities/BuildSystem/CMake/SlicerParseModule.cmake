cmake_minimum_required(VERSION 2.5)
include(SlicerSetGetModule)

# ---------------------------------------------------------------------------
# slicer_parse_module: parse a module description from a string (XML).
#
# This function parses a module and creates the corresponding key/value pairs.
#
# Note: the list of modules that have been parsed so far can be retrieved
# using slicer_get_parsed_modules
#
# Arguments:
# in:
#   module_contents (string): contents of the module
# in/out:
#   module_varname (string): variable name to use to store all module values
# 
# Example:
#   set(module_contents "<Name>TestModule</Name><Group>Segmentation</Group>")
#   slicer_parse_module("${module_contents}" TestModule)
#   slicer_get_module_value(TestModule Name name)
#   message("Module name: ${name}")
#
# See also:
#   slicer_parse_module_file
#   slicer_parse_module_url
#   slicer_parse_modules_directory
#   slicer_get_parsed_modules
# ---------------------------------------------------------------------------

function(slicer_parse_module module_contents module_varname)
  
  # The XML elements to parse
  # This doesn't take into account any attributes at the moment

  set(elems 
    Acknowledgement
    Author
    CVSTag
    CVSModule
    Dependency
    Description 
    Group 
    HomePage 
    Icon 
    Name 
    SourceLocation 
    SVNUsername
    SVNPassword
    Version 
    )

  # For each element:
  # - make sure the variable is unset (say TestModule_Author)
  # - search for all matches (i.e. <Author>Bar1</Author>;<Author>Bar2</Author)
  # - loop over each match:
  #   * extract the value itself (i.e. Bar1, then Bar2)
  #   * insert the value at the end of the list for that element variable 

  foreach(elem ${elems})
    slicer_unset_module_value(${module_varname} ${elem})
    set(regexp "<${elem}>([^<]*)</${elem}>")
    string(REGEX MATCHALL "${regexp}" matches "${module_contents}")
    foreach(match ${matches})
      string(REGEX MATCH "${regexp}" value "${match}")
      if(CMAKE_MATCH_1)
        slicer_get_module_value(${module_varname} ${elem} var)
        set(var ${var} "${CMAKE_MATCH_1}")
        slicer_set_module_value(${module_varname} ${elem} ${var})
      endif(CMAKE_MATCH_1)
    endforeach(match)
  endforeach(elem)

  # If the module is unknown at this point, this content was invalid
  
  slicer_is_module_unknown(${module_varname} unknown)
  if(unknown)
    message(SEND_ERROR 
      "Incomplete module ${module_varname}! Invalid content?\n\n${module_contents}")
    foreach(elem ${elems})
      slicer_unset_module_value(${module_varname} ${elem})
    endforeach(elem)
    return()
  endif(unknown)

  # Keep track of the modules parsed so far

  slicer_get_module_value(__Meta__ ParsedModules parsed_modules)
  set(parsed_modules ${parsed_modules} ${module_varname})
  list(REMOVE_DUPLICATES parsed_modules)
  slicer_set_module_value(__Meta__ ParsedModules ${parsed_modules})

endfunction(slicer_parse_module)

# ---------------------------------------------------------------------------
# slicer_parse_module_file: parse a module description from a file (XML).
#
# This function loads a module description file into a variable and parse its
# contents by calling the slicer_parse_module function.
#
# The name of the file (without extension) will be used if the variable name
# to use to store all module values is omitted.
#
# Note: the file name can be a directory, say /home/foo/module1, in that case
# this function will try to read /home/foo/module1/module1.xml and exit
# quietly if none was found.
#
# Note: the list of modules that have been parsed so far can be retrieved
# using slicer_get_parsed_modules
#
# Arguments:
# in:
#   module_filename (filename): full path (filename) to the module
# optional in/out:
#   module_varname (string): variable name to use to store all module values
# 
# Example:
#   slicer_parse_module_file("C:/foo/TestModule/TestModule.xml" TestModule)
#   slicer_get_module_value(TestModule Name name)
#   message("Module name: ${name}")
#
#   slicer_parse_module_file("C:/foo/TestModule2/")
#   slicer_get_module_value(TestModule2 Name name)
#   message("Module2 name: ${name}")
#
# See also:
#   slicer_parse_module
#   slicer_parse_module_url
#   slicer_parse_modules_directory
#   slicer_get_parsed_modules
# ---------------------------------------------------------------------------

function(slicer_parse_module_file module_filename)

  # If filename is a directory, try to look for a XML file inside it with
  # the same name as the directory...

  set(module_filename_was_directory 0)
  if(IS_DIRECTORY "${module_filename}")
    get_filename_component(abs "${module_filename}" ABSOLUTE)
    get_filename_component(dirname "${abs}" NAME)
    get_filename_component(dirpath "${abs}" PATH)
    set(module_filename "${dirpath}/${dirname}/${dirname}.xml")
    set(module_filename_was_directory 1)
  endif(IS_DIRECTORY "${module_filename}")
    
  if(NOT EXISTS "${module_filename}")
    if(NOT module_filename_was_directory)
      message(SEND_ERROR "Unable to load and parse module ${module_filename}!")
    endif(NOT module_filename_was_directory)
    return()
  endif(NOT EXISTS "${module_filename}")

  # Module varname variable (use the filename if not found)

  if(ARG1)
    set(module_varname ${ARGV1})
  else(ARG1)
    get_filename_component(module_varname "${module_filename}" NAME_WE)
  endif(ARG1)

  # Parse it

  file(READ "${module_filename}" module_contents)
  slicer_parse_module("${module_contents}" ${module_varname})

endfunction(slicer_parse_module_file)

# ---------------------------------------------------------------------------
# slicer_parse_module_url: parse a module description from a remote file.
#
# This function loads a remote module file into a variable and parse its
# contents by calling the slicer_parse_module function.
#
# The last component of the url (without extension) will be used if the
# variable name to use to store all module values is omitted.
#
# Note: the remote file is first downloaded to a local file that will be
# stored in the local module's cache directory (as retrieved by
# slicer_get_module_cache_directory)
# 
# Note: the list of modules that have been parsed so far can be retrieved
# using slicer_get_parsed_modules
#
# Arguments:
# in:
#   module_url (string): URL to the module file
# optional in/out:
#   module_varname (string): variable name to use to store all module values
# 
# Example:
#   slicer_parse_module_url("http://www.na-mic.org/modules/test/test.xml" TestModule)
#   slicer_get_module_value(TestModule Name name)
#   message("Module name: ${name}")
#
# See also:
#   slicer_parse_module
#   slicer_parse_module_file
#   slicer_parse_modules_directory
#   slicer_get_parsed_modules
#   slicer_get_module_cache_directory
# ---------------------------------------------------------------------------

function(slicer_parse_module_url module_url)

  # Retrieve the last component (trimmed from potential query string)

  get_filename_component(module_filename "${module_url}" NAME)
  string(REGEX REPLACE "\\?.+$" "" module_filename "${module_filename}")

  # Module varname variable (use the last component if not found)

  if(ARG1)
    set(module_varname ${ARGV1})
  else(ARG1)
    get_filename_component(module_varname "${module_filename}" NAME_WE)
  endif(ARG1)

  # Download the remote module file to a local file

  if(NOT module_filename)
    set(module_filename "${module_varname}.xml")
  endif(NOT module_filename)

  slicer_get_module_cache_directory(${module_varname} module_cache_dir)

  set(module_filename "${module_cache_dir}/${module_filename}")

  file(DOWNLOAD "${module_url}" "${module_filename}" 
    TIMEOUT 30 
    STATUS status 
    LOG log)

  list(GET status 0 status_code)
  list(GET status 1 status_string)
  if(NOT status_code EQUAL 0)
    message(SEND_ERROR "Failed to download module ${module_url} to ${module_filename}! (status Code: ${status_code}, status string: ${status_string})")
    return()
  endif(NOT status_code EQUAL 0)

  # Parse the local file 

  slicer_parse_module_file("${module_filename}" ${module_varname})

endfunction(slicer_parse_module_url)

# ---------------------------------------------------------------------------
# slicer_parse_modules_directory: parse modules sub-directories in a directory.
#
# This function goes through every sub-directories in a given directory
# and tries to parse the corresponding module files (i.e. in a /home/foo
# directory, the /home/foo/module1 sub-directory will be inspected for a
# /home/foo/module1/module1.xml module file).
#
# Note: the list of modules that have been parsed so far can be retrieved
# using slicer_get_parsed_modules
#
# Arguments:
# in:
#   dir (string): full path to a directory
# 
# Example:
#   slicer_parse_modules_directory("/home/foo/")
#   slicer_get_module_value(module1 Name name)
#   message("Module name: ${name}")
#
# See also:
#   slicer_parse_module
#   slicer_parse_module_file
#   slicer_parse_module_url
#   slicer_get_parsed_modules
# ---------------------------------------------------------------------------

function(slicer_parse_modules_directory dir)

  # Is it really a directory?

  if(NOT IS_DIRECTORY "${dir}")
    message(SEND_ERROR "Unable to access non-existing directory ${dir}!")
  endif(NOT IS_DIRECTORY "${dir}")
    
  # Glob the directory and inspect each subdirs

  get_filename_component(abs_dir "${dir}" ABSOLUTE)
  file(GLOB files "${abs_dir}/*")
  foreach(file ${files})
    slicer_parse_module_file(${file})
  endforeach(file ${files})

endfunction(slicer_parse_modules_directory)

# ---------------------------------------------------------------------------
# slicer_get_parsed_modules: get the list of parsed modules.
#
# This function can be used to retrieve the list of modules that have
# been parsed so far.
#
# Arguments:
# out:
#   list_varname (string): variable name to use to store all modules list
# 
# Example:
#   slicer_parse_module_file("TestModule.xml" TestModule)
#   slicer_parse_module_file("TestModule2.xml" TestModule2)
#   slicer_get_parsed_modules(modules)
#
# See also:
#   slicer_parse_module
#   slicer_parse_module_file
# ---------------------------------------------------------------------------

function(slicer_get_parsed_modules list_varname)

  slicer_get_module_value(__Meta__ ParsedModules value)
  set(${list_varname} ${value} PARENT_SCOPE)
  
endfunction(slicer_get_parsed_modules)

