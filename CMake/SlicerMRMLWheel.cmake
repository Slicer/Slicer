set(Slicer_BUILD_MRML_WHEEL ON CACHE STRING "Build Slicer MRML wheel for external python project use!")

if(Slicer_BUILD_MRML_WHEEL)
  message("Building MRML Python wheel")

  set(Slicer_MRML_WHEEL_DIRECTORY ${Slicer_BINARY_DIR}/MRMLWheel)
  set(Slicer_MRML_WHEEL_PACKAGE_DIRECTORY ${Slicer_MRML_WHEEL_DIRECTORY}/slicer)
  execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory ${Slicer_MRML_WHEEL_DIRECTORY} ${Slicer_MRML_WHEEL_PACKAGE_DIRECTORY})

  set(Slicer_MRML_WHEEL_SETUP_FILE ${Slicer_MRML_WHEEL_DIRECTORY}/setup.py)
  configure_file(${CMAKE_CURRENT_LIST_DIR}/setup.py.in ${Slicer_MRML_WHEEL_SETUP_FILE})

  # store all targets to wait before invoking python bdist
  # Use a cache variable to prevent any scope problem
  set(Slicer_MRML_WHEEL_DEPENDENCIES "" CACHE INTERNAL "" FORCE)

  set(Slicer_MRML_WHEEL_SCRIPTS
    ${CMAKE_SOURCE_DIR}/Base/Python/slicer/__init__.py
    ${CMAKE_SOURCE_DIR}/Base/Python/slicer/logic.py
    ${CMAKE_SOURCE_DIR}/Base/Python/slicer/util.py
    ${CMAKE_SOURCE_DIR}/Base/Python/mrml.py
    )
endif()

function(SlicerRegisterMRMLWheelTarget target)
  if(Slicer_BUILD_MRML_WHEEL)
    add_custom_command(TARGET ${target} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${target}> ${Slicer_MRML_WHEEL_PACKAGE_DIRECTORY}
    )

    set(Slicer_MRML_WHEEL_DEPENDENCIES "${Slicer_MRML_WHEEL_DEPENDENCIES};${target}" CACHE INTERNAL "" FORCE)
  endif()
endfunction()

function(SlicerBUILDMRMLWheel)
  set(byproducts)
  foreach(script ${Slicer_MRML_WHEEL_SCRIPTS})
    cmake_path(GET script FILENAME name)
    list(APPEND byproducts ${Slicer_MRML_WHEEL_PACKAGE_DIRECTORY}/${name})
  endforeach()

  message("Building MRML wheel with ${Slicer_MRML_WHEEL_DEPENDENCIES}")

  add_custom_target(SetupMRMLWheel ALL
    COMMAND ${CMAKE_COMMAND} -E copy -t ${Slicer_MRML_WHEEL_PACKAGE_DIRECTORY} ${Slicer_MRML_WHEEL_SCRIPTS}
    DEPENDS ${Slicer_MRML_WHEEL_DEPENDENCIES}
    BYPRODUCTS ${byproducts}
  )

  add_custom_target(BuildMRMLWheel ALL
    COMMAND ${PYTHON_EXECUTABLE} -m pip install build
    COMMAND ${PYTHON_EXECUTABLE} -m build --no-isolation
    DEPENDS SetupMRMLWheel
    WORKING_DIRECTORY ${Slicer_MRML_WHEEL_DIRECTORY}
  )
endfunction()
