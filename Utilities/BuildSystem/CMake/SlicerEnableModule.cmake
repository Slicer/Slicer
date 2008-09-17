cmake_minimum_required(VERSION 2.5)
include(SlicerSetGetModule)

# ---------------------------------------------------------------------------
# slicer_create_use_module_option: create an option to use a module.
#
# This function can be used to create a boolean OPTION variable to control if
# a module should be used or not.
#
# Arguments:
# in:
#   module_varname (string): variable name used to store all module keys/values
#   option_name (string): name to use to for the option
# 
# Example:
#   slicer_parse_module_file("C:/foo/TestModule/TestModule.xml" TestModule)
#   slicer_create_use_module_option(TestModule USE_TEST_MODULE)
#   if(USE_TEST_MODULE)
#     ...
#   endif(USE_TEST_MODULE)
#
# See also:
#   slicer_get_module_value
#   slicer_set_module_value
#   slicer_get_module_short_description
# ---------------------------------------------------------------------------

function(slicer_create_use_module_option module_varname option_name)

  # Unknown module? Bail.

  slicer_is_module_unknown(
    ${module_varname} unknown "Unable to create option!")
  if(unknown)
    return()
  endif(unknown)

  # Create the option

  slicer_get_module_short_description(${module_varname} short_desc)
  option(${option_name} "Use ${short_desc}." OFF)

endfunction(slicer_create_use_module_option)

# ---------------------------------------------------------------------------
# slicer_create_use_modules_options: create options to use specific modules.
#
# This function can be used to create a boolean OPTION variable to control if
# a module should be used or not, for a list of specific modules.
#
# The option name will be created by prefixing the uppercase module name
# with "USE_". The option itself is created by calling 
# slicer_create_use_module_option
#
# Arguments:
# in:
#   modules (string): list of module variable names (*has* to be quoted)
# 
# Example:
#   slicer_parse_module_file("C:/foo/TestModule/TestModule.xml" TestModule)
#   slicer_parse_module_url("http://foo/bar/module/module2.xml" module2)
#   ...
#   slicer_get_parsed_modules(modules)
#   slicer_create_use_modules_options("${modules}")
#
# See also:
#   slicer_create_use_module_option
# ---------------------------------------------------------------------------

function(slicer_create_use_modules_options modules)

  # Loop over all modules

  foreach(module_varname ${modules})

    # Create the option name from the module name

    slicer_get_module_value(${module_varname} Name name)
    if(name)
      string(TOUPPER "${name}" upper_name)
      set(option_name "USE_${upper_name}")
      slicer_create_use_module_option(${module_varname} ${option_name})
    endif(name)
    
  endforeach(module_varname)

endfunction(slicer_create_use_modules_options)

# ---------------------------------------------------------------------------
# slicer_get_used_modules: get the list of used modules.
#
# This function can be used to retrieve the list of modules which 
# "USE_" option is set to ON, among a list of modules.
#
# Arguments:
# in:
#   modules (string): list of module variable names (*has* to be quoted)
# out:
#   list_varname (string): variable name to use to store the used modules list
# 
# Example:
#   slicer_parse_module_file("TestModule.xml" TestModule)
#   slicer_parse_module_file("TestModule2.xml" TestModule2)
#
#   slicer_get_parsed_modules(modules)
#   slicer_create_use_modules_options("${modules}")
#   slicer_get_used_modules("${modules}" used_modules)
#
# See also:
#   slicer_parse_module_file
#   slicer_get_parsed_modules
#   slicer_create_use_modules_options
# ---------------------------------------------------------------------------

function(slicer_get_used_modules modules list_varname)

  set(used_modules)

  # Loop over all modules

  foreach(module_varname ${modules})

    # Create the option name from the module name, check its value, if ON
    # collect the module name.

    slicer_get_module_value(${module_varname} Name name)
    if(name)
      string(TOUPPER "${name}" upper_name)
      set(option_name "USE_${upper_name}")
      if(${option_name})
        set(used_modules ${used_modules} ${module_varname})
      endif(${option_name})
    endif(name)
    
  endforeach(module_varname)

  set(${list_varname} ${used_modules} PARENT_SCOPE)
  
endfunction(slicer_get_used_modules)

# ---------------------------------------------------------------------------
# slicer_get_unresolved_modules_dependencies: get the unresolved modules dependencies for specific modules 
#
# This function can be used to retrieve the list of unique unresolved modules 
# dependencies among a list of specific modules; this is the list of 
# dependent modules that are not part of the list of modules passed as 
# parameter.
#
# Arguments:
# in:
#   modules (string): list of module variable names (*has* to be quoted)
# out:
#   list_varname (string): variable name to use to store the unresolved list
# optional:
#   msg_varname (string): variable name to store a message detailing the unresolved dependencies.
# 
# Example:
#   slicer_parse_module_file("C:/foo/TestModule/TestModule.xml" TestModule)
#   slicer_parse_module_url("http://foo/bar/module/module2.xml" module2)
#   ...
#   slicer_get_parsed_modules(modules)
#   slicer_create_use_modules_options("${modules}")
#   slicer_get_used_modules("${modules}" used_modules)
#   slicer_get_unresolved_modules_dependencies("${used_modules}" unresolved_dependencies)
#
# See also:
#   slicer_parse_module_file
#   slicer_get_parsed_modules
#   slicer_create_use_modules_options
#   slicer_get_used_modules
# ---------------------------------------------------------------------------

function(slicer_get_unresolved_modules_dependencies modules list_varname)

  set(unresolved_dependencies)

  # Loop over all modules

  foreach(module_varname ${modules})

    # Loop over all dependencies for this module, and try to find them
    
    slicer_get_module_value(${module_varname} Dependency dependencies)
    foreach(dependency ${dependencies})

      # If the dependency was not found, collect it in a list, and
      # update the "reversed-list", i.e. the list of modules that depends
      # on it.
      
      list(FIND modules ${dependency} index)
      if(index EQUAL -1)
        set(unresolved_dependencies ${unresolved_dependencies} ${dependency})
        set(__${dependency}_modules 
          ${__${dependency}_modules} ${module_varname})
      endif(index EQUAL -1)
      
    endforeach(dependency)
    
  endforeach(module_varname)

  # Remove duplicates to get the unique list of dependencies

  if(unresolved_dependencies)
    list(REMOVE_DUPLICATES unresolved_dependencies)
  endif(unresolved_dependencies)
  set(${list_varname} ${unresolved_dependencies} PARENT_SCOPE)
  
  # If an optional arg was specified, use it to create a more detailed
  # message listing all unresolved dependencies with the list of modules 
  # that actually depend on them (per-dependency).

  if(ARGV2)
    set(msg)
    foreach(dependency ${unresolved_dependencies})
      list(REMOVE_DUPLICATES __${dependency}_modules)
      set(msg "${msg}- ${dependency}: ${__${dependency}_modules}\n")
    endforeach(dependency)
    set(${ARGV2} ${msg} PARENT_SCOPE)
  endif(ARGV2)

endfunction(slicer_get_unresolved_modules_dependencies)

# ---------------------------------------------------------------------------
# slicer_get_resolved_modules: get the list of resolved modules for specific modules 
#
# This function can be used to retrieve the list of resolved modules 
# among a list of specific modules; this is the list of modules which
# dependencies are resolved (i.e. all dependencies are already part of the
# list of modules passed as parameter).
#
# Arguments:
# in:
#   modules (string): list of module variable names (*has* to be quoted)
# out:
#   list_varname (string): variable name to use to store the resolved list
# 
# Example:
#   slicer_parse_module_file("C:/foo/TestModule/TestModule.xml" TestModule)
#   slicer_parse_module_url("http://foo/bar/module/module2.xml" module2)
#   ...
#   slicer_get_parsed_modules(modules)
#   slicer_create_use_modules_options("${modules}")
#   slicer_get_used_modules("${modules}" used_modules)
#   slicer_get_resolved_modules("${used_modules}" resolved_modules)
#
# See also:
#   slicer_parse_module_file
#   slicer_get_parsed_modules
#   slicer_create_use_modules_options
#   slicer_get_used_modules
# ---------------------------------------------------------------------------

function(slicer_get_resolved_modules modules list_varname)

  set(resolved_modules)

  # Loop over all modules

  foreach(module_varname ${modules})

    # Loop over all dependencies for this module, and try to find them
    
    set(module_has_unresolved_dependency 0)
    slicer_get_module_value(${module_varname} Dependency dependencies)
    foreach(dependency ${dependencies})

      list(FIND modules ${dependency} index)
      if(index EQUAL -1)
        set(module_has_unresolved_dependency 1)
        break()
      endif(index EQUAL -1)
      
    endforeach(dependency)
    
    if(NOT module_has_unresolved_dependency)
      set(resolved_modules ${resolved_modules} ${module_varname})
    endif(NOT module_has_unresolved_dependency)

  endforeach(module_varname)

  # Remove duplicates to get a unique list (for convenience)

  if(resolved_modules)
    list(REMOVE_DUPLICATES resolved_modules)
  endif(resolved_modules)
  set(${list_varname} ${resolved_modules} PARENT_SCOPE)

endfunction(slicer_get_resolved_modules)

