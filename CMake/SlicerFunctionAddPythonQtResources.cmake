set_property(GLOBAL PROPERTY _SLICER_PYTHON_RESOURCE_TARGETS)

function(slicerFunctionAddPythonQtResources RESOURCE_NAMES)
  set(out_paths)

  # Generate compiler resource scripts
  foreach(in_path ${ARGN})
    set(rc_depends)
    if(IS_ABSOLUTE ${in_path})
      file(RELATIVE_PATH out_dir ${CMAKE_CURRENT_SOURCE_DIR} ${in_path})
      get_filename_component(out_dir ${CMAKE_CURRENT_BINARY_DIR}/${out_dir} PATH)
    else()
      get_filename_component(out_dir ${CMAKE_CURRENT_BINARY_DIR}/${in_path} PATH)
      get_filename_component(in_path ${in_path} ABSOLUTE)
    endif()
    get_filename_component(out_name ${in_path} NAME_WE)
    get_filename_component(rc_path ${in_path} PATH)
    set(out_path ${out_dir}/${out_name}Resources.py)

    if(EXISTS ${in_path})
      # Parse file for dependencies
      file(READ ${in_path} rc_file_contents)
      string(REGEX MATCHALL "<file[^<]+" rc_files "${rc_file_contents}")
      foreach(rc_file ${rc_files})
        string(REGEX REPLACE "^<file[^>]*>" "" rc_file ${rc_file})
        if(NOT IS_ABSOLUTE ${rc_file})
          set(rc_file ${rc_path}/${rc_file})
        endif()
        list(APPEND rc_depends ${rc_file})
      endforeach()

      # Copy the input qrc script to enforce recalculation of dependencies on
      # changes to the same
      configure_file(
        ${in_path}
        ${CMAKE_CURRENT_BINARY_DIR}/.${out_name}Resources.py.deps
        COPYONLY
        )
    endif()

    if(NOT DEFINED QT_RCC_EXECUTABLE AND TARGET Qt5::rcc)
      get_target_property(QT_RCC_EXECUTABLE Qt5::rcc IMPORTED_LOCATION)
    endif()

    # Create command to generate the compiled resource script
    add_custom_command(
      OUTPUT ${out_path}
      COMMAND ${CMAKE_COMMAND} -E make_directory ${out_dir}
      COMMAND ${PYTHON_EXECUTABLE}
        ${Slicer_SOURCE_DIR}/Utilities/Scripts/qrcc.py
        --rcc ${QT_RCC_EXECUTABLE}
        -o ${out_path}
        ${in_path}
      VERBATIM
      WORKING_DIRECTORY ${rc_path}
      MAIN_DEPENDENCY ${in_path}
      DEPENDS ${Slicer_SOURCE_DIR}/Utilities/Scripts/qrcc.py ${rc_depends}
      )

    list(APPEND out_paths ${out_path})
  endforeach()

  # Create target to generate resource files
  get_filename_component(target ${CMAKE_CURRENT_BINARY_DIR}-Resources NAME)
  add_custom_target(${target} DEPENDS ${out_paths})

  get_property(resource_targets
    GLOBAL PROPERTY _SLICER_PYTHON_RESOURCE_TARGETS
    )
  list(APPEND resource_targets ${target})
  set_property(GLOBAL PROPERTY _SLICER_PYTHON_RESOURCE_TARGETS
    ${resource_targets}
    )

  set(${RESOURCE_NAMES} ${out_paths} PARENT_SCOPE)
endfunction()

function(slicerFunctionAddPythonQtResourcesTargets NAME)
  get_property(resource_files GLOBAL PROPERTY _SLICER_PYTHON_RESOURCE_TARGETS)
  add_custom_target(${NAME} DEPENDS ${resource_files})
endfunction()
