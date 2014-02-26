
if(Slicer_EP_LABEL_REMOTE_MODULE)
  if(BUILD_TESTING)
    # XXX Some project (ie BRAINSTools) expects 'LAUNCH_EXE' to be defined.
    #     For reference see commits r16677 and r18537
    set(LAUNCH_EXE ${SEM_LAUNCH_COMMAND})
  endif()

  foreach(varname ${Slicer_EP_LABEL_REMOTE_MODULE})
    set(_source_dir ${${varname}})
    get_filename_component(_project_name ${_source_dir} NAME_WE)
    add_subdirectory(
      ${_source_dir}
      ${CMAKE_CURRENT_BINARY_DIR}/${_project_name}
      )
  endforeach()
endif()
