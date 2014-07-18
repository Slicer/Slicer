find_package(Doxygen 1.8.4 QUIET)

if( DOXYGEN_FOUND )
  if( DOXYGEN_DOT_FOUND )
    #
    # User documentation
    #
    set(userdoc_doxyfile ${CMAKE_CURRENT_BINARY_DIR}/UserDoxyfile.txt)
    set(userdoc_tarball
      ${DOCUMENTATION_ARCHIVES_OUTPUT_DIRECTORY}/Slicer-cpp.tar.gz
      )

    set( Slicer_USER_DOCUMENTATION "NO" )
    set( Slicer_ADDITIONAL_EXCLUDE_PATTERN "*_p.h\  *Private.*" )

    configure_file(Doxyfile.txt.in ${userdoc_doxyfile})

    # Generate doxygen documentation
    add_custom_target(cpp-doc
      COMMAND ${DOXYGEN_EXECUTABLE} ${userdoc_doxyfile}
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
      )
    add_dependencies(doc cpp-doc)

    # Create documentation tarball
    add_custom_target(cpp-doc-tarball-clean
      COMMAND ${CMAKE_COMMAND} -E remove ${userdoc_tarball}
      )

    add_custom_target(cpp-doc-tarball
      DEPENDS cpp-doc cpp-doc-tarball-clean
      COMMAND ${CMAKE_COMMAND} -E tar cfz ${userdoc_tarball} html
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
      )
    add_dependencies(doc-tarballs cpp-doc-tarball)

    set_target_properties(cpp-doc cpp-doc-tarball
      PROPERTIES LABELS Documentation)

    #
    # Dev documentation
    #
    set( Slicer_USER_DOCUMENTATION "YES" )
    set( Slicer_ADDITIONAL_EXCLUDE_PATTERN "" )
    set( Slicer_ADDITIONAL_FILE_PATTERN "*.c \  *.cc \  *.cxx \  *.cpp \  *.c++ \  *.hxx \  *.hpp \  *.h++ \  *.txx \  *.C \  *.CC \  *.C++ \  *.moc" )


    configure_file( Doxyfile.txt.in
      ${CMAKE_CURRENT_BINARY_DIR}/DevDoxyfile.txt
      )

    # TODO Let's disable target 'doc-dev', it crashes using doxygen 1.7.3
    #add_custom_target( doc-dev
    #  ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/DevDoxyfile.txt
    #  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/DevDoxyfile.txt
    #  )
    #set_target_properties(doc-dev PROPERTIES LABELS Documentation)

  else( DOXYGEN_DOT_FOUND )
    message( WARNING
      "Warning: Dot not found - Documentation will not be created"
      )
  endif( DOXYGEN_DOT_FOUND )
endif( DOXYGEN_FOUND )
