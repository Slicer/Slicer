function(slicer_module_autoinit)
  cmake_parse_arguments(PARSE_ARGV 0 arg
    ""
    ""
    "TARGETS;MODULES"
  )

  # Apply VTK module auto-initialization for linked modules that need it.
  set(modules)
  foreach(lib IN LISTS arg_MODULES)
    if(TARGET "${lib}")
      get_target_property(aliased "${lib}" ALIASED_TARGET)
      if(aliased)
        set(lib "${aliased}")
      endif()
      # Handles compatibility targets, such as MRMLCore -> Slicer::MRMLCore
      get_target_property(module_name "${lib}" INTERFACE_vtk_module_name)
      if(module_name)
        set(lib "${module_name}")
      endif()
      get_target_property(needs_autoinit "${lib}" INTERFACE_vtk_module_needs_autoinit)
      if(needs_autoinit)
        list(APPEND modules "${lib}")
      endif()
    endif()
  endforeach()
  if(modules)
    list(REMOVE_DUPLICATES modules)
    vtk_module_autoinit(TARGETS ${arg_TARGETS} MODULES ${modules})
  endif()
endfunction()
