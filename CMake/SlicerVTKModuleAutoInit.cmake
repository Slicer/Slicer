#[==[.rst:
.. cmake:command:: slicer_module_autoinit

  Wrapper for `vtk_module_autoinit` that supports Slicer compatibility aliases,
  and skips non-VTK-modules targets.

  - `TARGETS`: Targets to initialize with given modules.
  - `MODULES`: List of modules to autoinit.

  .. code-block:: cmake

  slicer_module_autoinit(
    TARGETS <names...>
    MODULES <modules...>
  )
#]==]
function(slicer_module_autoinit)
  cmake_parse_arguments(PARSE_ARGV 0 arg
    ""
    ""
    "TARGETS;MODULES"
  )

  # Apply VTK module auto-initialization for linked modules that need it.
  set(modules)
  foreach(lib IN LISTS arg_MODULES)
    # Filter hardcoded paths/libnames, there are a few in slicer.
    if(NOT TARGET "${lib}")
      continue()
    endif()
    _slicer_real_target("${lib}" real_target)
    get_target_property(needs_autoinit "${real_target}" INTERFACE_vtk_module_needs_autoinit)
    if(needs_autoinit)
      list(APPEND modules "${real_target}")
    endif()
  endforeach()

  if(modules)
    list(REMOVE_DUPLICATES modules)
    vtk_module_autoinit(TARGETS ${arg_TARGETS} MODULES ${modules})
  endif()
endfunction()

# `name` can be one of:
# - the module name: Slicer::Module
# - the target name: Module
# - the compatibility alias, if different from target name: vtkSlicerModule
# Return in `${output}` the name of the real slicer target (== Module).
# Targets other than Slicer VTK modules can safely pass through this function. In such case, `output` is set to `name`.
function(_slicer_real_target name output)
  # Handles compatibility alias targets, such as vtkSlicerModule -> Module
  # Note that in this context, VTK imported modules are not aliases but imported targets,
  # so they are not concerned by this condition.
  set(lib "${name}")
  get_target_property(aliased "${lib}" ALIASED_TARGET)
  if(aliased)
    set(lib "${aliased}")
  endif()
  # Fetch back the module name from the real target, such as Module -> Slicer::Module
  # This is only defined on Slicer built module, c.f. slicer_vtk_module_build.
  get_target_property(module_name "${lib}" INTERFACE_vtk_module_name)
  if(module_name)
    set(lib "${module_name}")
  endif()

  set("${output}" "${lib}" PARENT_SCOPE)
endfunction()
