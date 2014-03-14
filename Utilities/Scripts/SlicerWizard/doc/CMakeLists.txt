find_program(SPHINX_BUILD_EXECUTABLE sphinx-build)

if(SPHINX_BUILD_EXECUTABLE)
  # Set output directory
  set(pkg Slicer-${Slicer_VERSION_MAJOR}.${Slicer_VERSION_MINOR})
  set(docdir "${CMAKE_BINARY_DIR}/share/doc/${pkg}/SlicerWizard")

  # Determine wiki documentation version string
  set(doc_version "${Slicer_VERSION_MAJOR}.${Slicer_VERSION_MINOR}")
  set(wiki_version "Nightly")
  if(DEFINED Slicer_VERSION_TWEAK) # This is defined only for release
    set(wiki_version "${doc_version}")
  endif()

  # Generate documentation
  add_custom_target(SlicerWizard-doc
    COMMAND "${CMAKE_COMMAND}" -E make_directory "${docdir}"
    COMMAND "${SPHINX_BUILD_EXECUTABLE}"
            "-Dversion=${doc_version}"
            "-Dwikidoc_version=${wiki_version}"
            "${CMAKE_CURRENT_SOURCE_DIR}"
            "${docdir}"
    )
  set_target_properties(SlicerWizard-doc PROPERTIES LABELS Documentation)
  add_dependencies(doc SlicerWizard-doc)
else()
  message(WARNING "Warning: sphinx-build not found:"
                  " Python documentation will not be created")
endif()
