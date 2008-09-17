cmake_minimum_required(VERSION 2.5)
include(SlicerSetGetModule)
find_package(Subversion)
find_package(CVS)

# ---------------------------------------------------------------------------
# slicer_create_download_module_target: create a download module target.
#
# This function can be used to create a target to download a module
# repository (CVS or SVN).
#
# Note: while one could create multiple download targets with different 
# names, it is assumed that only one is needed; the name of this download
# target can be later on retrieved using slicer_get_download_module_target.
#
# Arguments:
# in:
#   module_varname (string): variable name used to store all module values
#   target_name (string): name of the target
#   dir (string): where the module should be downloaded
# 
# Example:
#   slicer_create_download_module_target(TestModule testmodule_download "/src/TestModule")
#
# See also:
#   slicer_get_download_module_target
#   slicer_create_update_module_target
# ---------------------------------------------------------------------------

function(slicer_create_download_module_target module_varname target_name dir)

  # Unknown module? Bail.

  set(err_msg "Unable to create download target!")
  slicer_is_module_unknown(${module_varname} unknown ${err_msg})
  if(unknown)
    return()
  endif(unknown)

  # Missing source location? Bye.

  slicer_get_module_value(${module_varname} SourceLocation source_loc)
  if(NOT source_loc)
    message(SEND_ERROR 
      "Unknown source location for module ${module_varname}. ${err_msg}")
    return()
  endif(NOT source_loc)

  slicer_get_module_source_repository_type(${module_varname} type)

  # If SVN create a command to checkout the repository, or a switch

  if(type STREQUAL "svn")

    if(NOT Subversion_FOUND)
      message(SEND_ERROR "Subversion (svn) was not found. ${err_msg}")
      return()
    endif(NOT Subversion_FOUND)
    
    set(svn_args)

    # username/password, if any

    slicer_get_module_value(${module_varname} SVNUsername svn_username)
    if(svn_username)
      set(svn_args ${svn_args} "--username" "${svn_username}")
    endif(svn_username)

    slicer_get_module_value(${module_varname} SVNPassword svn_password)
    if(svn_password)
      set(svn_args ${svn_args} "--password" "${svn_password}")
    endif(svn_password)

    get_filename_component(abs_dir "${dir}" ABSOLUTE)

    # We need those args in both lists form and flattened string form

    set(svn_args ${svn_args} "${source_loc}" "${abs_dir}")
    foreach(svn_arg ${svn_args})
      set(svn_args_flat "${svn_args_flat} ${svn_arg}")
    endforeach(svn_arg)

    # Create a CMake script that will checkout the SVN repository
    # if it doesn't exist. This can't be done just by adding a custom
    # command which OUTPUT would be a hand-picked file in the repository
    # (i.e. a command that would semantically represent: "check the repo
    # out, and by doing so guarantee that this hand-picked file exist", then
    # by creating a custom target which dependencies would be that OUTPUT.
    # This can not be done because the OUTPUT file always depends on the
    # build.cmake file, i.e. the Makefile that stores the rules to build 
    # the custom target. This is done so that changing the command itself
    # would ensure the target/command is re-run (i.e., if you change the tool
    # that is supposed to produce this OUTPUT, you probably want that OUTPUT
    # to be re-generated). However, anything that would update the build.cmake
    # file would then force the target to be *always* re-run, since the OUTPUT
    # file, i.e. a file in the repository, will *not* be re-downloaded again 
    # (it exists already), and therefore would never be newer than build.cmake.
    # That's because 99% of the time the OUTPUT of a custom command is 
    # assumed to be always up-to-date.
    # Solve this by creating a CMake script that will actually perform
    # the checkout only if that hand-picked file in the repo is not found
    # *and* will create a dummy up-to-date file that will act as our 
    # real dependency and *will* be newer than build.cmake if it is ever
    # modified.

    slicer_get_module_cache_directory(${module_varname} module_cache_dir)

    set(dep_file "${module_cache_dir}/${target_name}.uptodate")
    set(svn_dep_file "${abs_dir}/.svn")
   
    set(cmake_script "${module_cache_dir}/${target_name}.cmake")
    file(WRITE "${cmake_script}"
      "if(NOT EXISTS \"${svn_dep_file}\") 
         execute_process(COMMAND \"${Subversion_SVN_EXECUTABLE}\" checkout ${svn_args_flat}) 
       else(NOT EXISTS \"${svn_dep_file}\")
         execute_process(COMMAND \"${Subversion_SVN_EXECUTABLE}\" switch ${svn_args_flat}) 
       endif(NOT EXISTS \"${svn_dep_file}\")
       file(WRITE \"${dep_file}\" \"\")"
      )
    
    add_custom_command(
      OUTPUT "${dep_file}"
      COMMAND "${CMAKE_COMMAND}" ARGS "-P" "${cmake_script}"
      )

    add_custom_target(${target_name}
      DEPENDS "${dep_file}"
      )

    slicer_set_module_value(
      ${module_varname} __DownloadTarget__ ${target_name})

  # If CVS create a command to checkout the repository, assume it is
  # there if CVS/ sub-directory can be found.
  # Use the CVSModule parameter to get the module to checkout

  elseif(type STREQUAL "cvs")

    if(NOT CVS_FOUND)
      message(SEND_ERROR "CVS was not found. ${err_msg}")
      return()
    endif(NOT CVS_FOUND)

    slicer_get_module_value(${module_varname} CVSModule cvs_module)
    if(NOT cvs_module)
      message(SEND_ERROR 
        "Unknown CVS module for module ${module_varname}. ${err_msg}")
      return()
    endif(NOT cvs_module)

    # Make sure the directory exists (actually only the parent needs to exist)

    set(cmake_args "-E" "make_directory" "${dir}")

    add_custom_command(
      OUTPUT "${dir}"
      COMMAND "${CMAKE_COMMAND}" ARGS ${cmake_args}
      )

    # Checkout

    get_filename_component(dirname "${dir}" NAME)
    get_filename_component(dirpath "${dir}" PATH)

    set(cmake_args "-E" "chdir" "${dirpath}" "${CVS_EXECUTABLE}" "-d" "${source_loc}" "checkout" "-d" "${dirname}")

    slicer_get_module_source_tag(${module_varname} cvs_tag)
    if(cvs_tag)
      set(cmake_args ${cmake_args} "-r" "${cvs_tag}")
    endif(cvs_tag)

    set(cmake_args ${cmake_args} "${cvs_module}")

    add_custom_command(
      OUTPUT "${dir}/CVS/Root"
      DEPENDS "${dir}"
      COMMAND "${CMAKE_COMMAND}" ARGS ${cmake_args}
      )

    add_custom_target(${target_name} DEPENDS "${dir}/CVS/Root")
  
    slicer_set_module_value(
      ${module_varname} __DownloadTarget__ ${target_name})

  # Don't know the repository

  else(type STREQUAL "svn")

      message(SEND_ERROR "Unknown source repository type for module ${module_varname}. ${err_msg}")
      return()

    slicer_unset_module_value(
      ${module_varname} __DownloadTarget__)

  endif(type STREQUAL "svn")

endfunction(slicer_create_download_module_target)

# ---------------------------------------------------------------------------
# slicer_get_download_module_target: get the name of a download module target.
#
# This function can be used to retrieve the name of the download target
# associated to a module, if any.
#
# Arguments:
# in:
#   module_varname (string): variable name used to store all module values
# out:
#   target_varname (string): variable name to use to store the target name
# 
# Example:
#   slicer_get_download_module_target(TestModule download_target_name)
#   message("download target for TestModule: ${download_target_name}")
#
# See also:
#   slicer_create_download_module_target
# ---------------------------------------------------------------------------

function(slicer_get_download_module_target module_varname target_varname)

  # Unknown module? Bail.

  set(err_msg "Unable to get download module target!")
  slicer_is_module_unknown(${module_varname} unknown ${err_msg})
  if(unknown)
    return()
  endif(unknown)

  slicer_get_module_value(${module_varname} __DownloadTarget__ value)
  set(${target_varname} ${value} PARENT_SCOPE)

endfunction(slicer_get_download_module_target)

# ---------------------------------------------------------------------------
# slicer_create_update_module_target: create an update module target.
#
# This function can be used to create a target to update a module 
# repository (CVS or SVN).
#
# Note: while one could create multiple update targets with different 
# names, it is assumed that only one is needed; the name of this update
# target can be later on retrieved using slicer_get_update_module_target.
#
# Note: this function does not automatically create a download target. This
# can however be done using the add_dependencies() command; the name of
# the download target itself, if any for that module, can be retrieved
# using slicer_get_download_module_target.
#
# Arguments:
# in:
#   module_varname (string): variable name used to store all module values
#   target_name (string): name of the target
#   dir (string): where the module has been downloaded/checked out
# 
# Example:
#   slicer_create_update_module_target(TestModule testmodule_update "/src/TestModule")
#
# See also:
#   slicer_create_download_module_target
#   slicer_get_update_module_target
# ---------------------------------------------------------------------------

function(slicer_create_update_module_target module_varname target_name dir)

  # Unknown module? Bail.

  set(err_msg "Unable to create update target!")
  slicer_is_module_unknown(${module_varname} unknown ${err_msg})
  if(unknown)
    return()
  endif(unknown)

  slicer_get_module_source_repository_type(${module_varname} type)

  # If SVN create a command to update the repository.

  if(type STREQUAL "svn")

    if(NOT Subversion_FOUND)
      message(SEND_ERROR "Subversion (svn) was not found. ${err_msg}")
      return()
    endif(NOT Subversion_FOUND)
    
    set(svn_args "update" "--non-interactive" "${dir}")

    add_custom_target(${target_name}
      "${Subversion_SVN_EXECUTABLE}" ${svn_args}
      )

    slicer_set_module_value(
      ${module_varname} __UpdateTarget__ ${target_name})

  # If CVS create a command to update the repository.
  # CVS can not update a specific directory, one has to chdir in the directory
  # to update. Use the CMake command-line client in Execute mode (-E) to
  # change to the directory, and run the CVS update command.

  elseif(type STREQUAL "cvs")

    if(NOT CVS_FOUND)
      message(SEND_ERROR "CVS was not found. ${err_msg}")
      return()
    endif(NOT CVS_FOUND)

    set(cmake_args "-E" "chdir" "${dir}" "${CVS_EXECUTABLE}" "update" "-dP")

    add_custom_target(${target_name}
      ${CMAKE_COMMAND} ${cmake_args}
      )
  
    slicer_set_module_value(
      ${module_varname} __UpdateTarget__ ${target_name})

  # Don't know the repository

  else(type STREQUAL "svn")

      message(SEND_ERROR "Unknown source repository type for module ${module_varname}. ${err_msg}")
      return()

    slicer_unset_module_value(
      ${module_varname} __UpdateTarget__)

  endif(type STREQUAL "svn")

endfunction(slicer_create_update_module_target)

# ---------------------------------------------------------------------------
# slicer_get_update_module_target: get the name of a update module target.
#
# This function can be used to retrieve the name of the update target
# associated to a module, if any.
#
# Arguments:
# in:
#   module_varname (string): variable name used to store all module values
# out:
#   target_varname (string): variable name to use to store the target name
# 
# Example:
#   slicer_get_update_module_target(TestModule update_target_name)
#   message("update target for TestModule: ${update_target_name}")
#
# See also:
#   slicer_create_update_module_target
# ---------------------------------------------------------------------------

function(slicer_get_update_module_target module_varname target_varname)

  # Unknown module? Bail.

  set(err_msg "Unable to get update module target!")
  slicer_is_module_unknown(${module_varname} unknown ${err_msg})
  if(unknown)
    return()
  endif(unknown)

  slicer_get_module_value(${module_varname} __UpdateTarget__ value)
  set(${target_varname} ${value} PARENT_SCOPE)

endfunction(slicer_get_update_module_target)

# ---------------------------------------------------------------------------
# slicer_create_download_and_update_modules_targets: create download and update targets for specific modules.
#
# This function can be used to create and connect both download and update
# targets for a list of specific modules.
#
# The download target name will be created by appending "_download" to the
# module name, unless that target exists already.
#
# The update target name will be created by appending "_update" to the
# module name, unless that target exists already.
#
# Each module's update target will depend on the module's download target
# so that downloading occurs automatically when the repository is not found.
#
# A unique "download_modules" and "update_modules" target will be created
# that will depend on all modules download and update targets (respectively).
#
# Each module is downloaded/checked out in its own sub-directory, named after
# the module's name.
#
# Arguments:
# in:
#   modules (string): list of module variable names (*has* to be quoted)
#   dir (string): where the modules should be downloaded (parent dir)
# 
# Example:
#   slicer_parse_module_file("C:/foo/TestModule/TestModule.xml" TestModule)
#   slicer_parse_module_url("http://foo/bar/module/module2.xml" module2)
#   ...
#   slicer_get_parsed_modules(modules)
#   slicer_create_download_and_update_modules_targets("${modules}" "/src")
#
# See also:
#   slicer_create_update_module_target
# ---------------------------------------------------------------------------

function(slicer_create_download_and_update_modules_targets modules dir)

  # Create the global download and update targets

  set(download_modules_target download_modules)
  add_custom_target(${download_modules_target})

  set(update_modules_target update_modules)
  add_custom_target(${update_modules_target})

  foreach(module_varname ${modules})

    # Find and/or create download target
    
    slicer_get_download_module_target(${module_varname} download_target)
    if(NOT download_target)
      set(download_target "${module_varname}_download")
      slicer_create_download_module_target(
        ${module_varname} ${download_target} "${dir}/${module_varname}")
    endif(NOT download_target)
    add_dependencies(${download_modules_target} ${download_target})

    # Find and/or create update target
    
    slicer_get_update_module_target(${module_varname} update_target)
    if(NOT update_target)
      set(update_target "${module_varname}_update")
      slicer_create_update_module_target(
        ${module_varname} ${update_target} "${dir}/${module_varname}")
    endif(NOT update_target)
    add_dependencies(${update_modules_target} ${update_target})

    # Attach download to update

    add_dependencies(${update_target} ${download_target})
    
  endforeach(module_varname)

endfunction(slicer_create_download_and_update_modules_targets)

