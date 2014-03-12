find_program(SPHINX_BUILD_EXECUTABLE sphinx-build)

if(SPHINX_BUILD_EXECUTABLE)
  # Automatically generate documentation at build time
  set(all_arg)
  if(DOCUMENTATION_TARGET_IN_ALL)
    set(all_arg "ALL")
  endif()

  # Set output directory
  set(pkg Slicer-${Slicer_VERSION_MAJOR}.${Slicer_VERSION_MINOR})
  set(docdir "${CMAKE_BINARY_DIR}/share/doc/${pkg}/SlicerWizard")

  # Generate documentation
  add_custom_target(SlicerWizard-doc ${all_arg}
    COMMAND "${CMAKE_COMMAND}" -E make_directory "${docdir}"
    COMMAND "${SPHINX_BUILD_EXECUTABLE}"
            "${CMAKE_CURRENT_SOURCE_DIR}"
            "${docdir}"
    )
  add_dependencies(doc SlicerWizard-doc)
else()
  message(WARNING "Warning: sphinx-build not found:"
                  " Python documentation will not be created")
endif()
