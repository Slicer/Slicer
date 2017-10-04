find_program(SPHINX_BUILD_EXECUTABLE sphinx-build)

if(SPHINX_BUILD_EXECUTABLE)
  # Set output directory
  set(pkg Slicer-${Slicer_VERSION_MAJOR}.${Slicer_VERSION_MINOR})
  set(docdir "${CMAKE_BINARY_DIR}/share/doc/${pkg}/SlicerWizard")
  set(tarball "${DOCUMENTATION_ARCHIVES_OUTPUT_DIRECTORY}/SlicerWizard.tar.gz")

  # Determine wiki documentation version string
  set(doc_version "${Slicer_VERSION_MAJOR}.${Slicer_VERSION_MINOR}")
  set(wiki_version "Nightly")
  if("${Slicer_RELEASE_TYPE}" STREQUAL "Stable")
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
  add_dependencies(doc SlicerWizard-doc)

  # Create documentation tarball
  add_custom_target(SlicerWizard-doc-tarball
    DEPENDS SlicerWizard-doc
    COMMAND "${CMAKE_COMMAND}"
            "-DINPUT_PATH:PATH=${docdir}"
            "-DOUTPUT_FILE:FILEPATH=${tarball}"
            -P "${CMAKE_CURRENT_SOURCE_DIR}/create_doc_tarball.cmake"
    )
  add_dependencies(doc-tarballs SlicerWizard-doc-tarball)

  set_target_properties(SlicerWizard-doc SlicerWizard-doc-tarball
    PROPERTIES LABELS Documentation)

else()
  message(WARNING "Warning: sphinx-build not found:"
                  " Python documentation will not be created")
endif()
