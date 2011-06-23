find_package( Doxygen QUIET)

if( DOXYGEN_FOUND )
  if( DOXYGEN_DOT_FOUND )

    # Automatically generate documentation at build time
    set(all_arg)
    if(DOCUMENTATION_TARGET_IN_ALL)
      set(all_arg "ALL")
    endif()

    #
    # User documentation
    #
    set( Slicer_USER_DOCUMENTATION "NO" )
    set( Slicer_ADDITIONAL_EXCLUDE_PATTERN "*_p.h\  *Private.*" )
    set( Slicer_DOCUMENTATION_HTML_TARZ TRUE)

    configure_file( Doxyfile.txt.in
      ${CMAKE_CURRENT_BINARY_DIR}/UserDoxyfile.txt
      )

    configure_file(
      ${CMAKE_CURRENT_SOURCE_DIR}/generate_doc.cmake.in
      ${CMAKE_CURRENT_BINARY_DIR}/generate_userdoc.cmake
      @ONLY
      )

    set(userdoc_output "${CMAKE_CURRENT_BINARY_DIR}/doc/html/index.html")

    # Run doxygen as a custom command driven by a custom target.
    add_custom_command(
      OUTPUT ${userdoc_output}
      COMMAND ${CMAKE_COMMAND} -P "${CMAKE_CURRENT_BINARY_DIR}/generate_userdoc.cmake"
      DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/generate_userdoc.cmake"
              "${CMAKE_CURRENT_BINARY_DIR}/UserDoxyfile.txt"
      )

    add_custom_target(doc ${all_arg} DEPENDS ${userdoc_output})
    set_target_properties(doc PROPERTIES LABELS Documentation)

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
