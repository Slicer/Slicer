FIND_PACKAGE( Doxygen QUIET)

IF( DOXYGEN_FOUND )
  IF( DOXYGEN_DOT_FOUND )

    # Automatically generate documentation at build time
    SET(all_arg)
    IF (DOCUMENTATION_TARGET_IN_ALL)
      SET(all_arg "ALL")
    ENDIF()

    #
    # User documentation
    #
    SET( Slicer_USER_DOCUMENTATION "NO" )
    SET( Slicer_ADDITIONAL_EXCLUDE_PATTERN "*_p.h\  *Private.*" )
    SET( Slicer_DOCUMENTATION_HTML_TARZ TRUE)

    CONFIGURE_FILE( Doxyfile.txt.in
      ${CMAKE_CURRENT_BINARY_DIR}/UserDoxyfile.txt
      )

    CONFIGURE_FILE(
      ${CMAKE_CURRENT_SOURCE_DIR}/generate_doc.cmake.in
      ${CMAKE_CURRENT_BINARY_DIR}/generate_userdoc.cmake
      @ONLY
      )

    SET(userdoc_output "${CMAKE_CURRENT_BINARY_DIR}/doc/html/index.html")

    # Run doxygen as a custom command driven by a custom target.
    ADD_CUSTOM_COMMAND(
      OUTPUT ${userdoc_output}
      COMMAND ${CMAKE_COMMAND} -P "${CMAKE_CURRENT_BINARY_DIR}/generate_userdoc.cmake"
      DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/generate_userdoc.cmake"
              "${CMAKE_CURRENT_BINARY_DIR}/UserDoxyfile.txt"
      )

    ADD_CUSTOM_TARGET(doc ${all_arg} DEPENDS ${userdoc_output})
    SET_TARGET_PROPERTIES(doc PROPERTIES LABELS Documentation)

    #
    # Dev documentation
    #
    SET( Slicer_USER_DOCUMENTATION "YES" )
    SET( Slicer_ADDITIONAL_EXCLUDE_PATTERN "" )
    SET( Slicer_ADDITIONAL_FILE_PATTERN "*.c \  *.cc \  *.cxx \  *.cpp \  *.c++ \  *.hxx \  *.hpp \  *.h++ \  *.txx \  *.C \  *.CC \  *.C++ \  *.moc" )


    CONFIGURE_FILE( Doxyfile.txt.in
      ${CMAKE_CURRENT_BINARY_DIR}/DevDoxyfile.txt
      )

    # TODO Let's disable target 'doc-dev', it crashes using doxygen 1.7.3
    #ADD_CUSTOM_TARGET( doc-dev
    #  ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/DevDoxyfile.txt
    #  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/DevDoxyfile.txt
    #  )
    #SET_TARGET_PROPERTIES(doc-dev PROPERTIES LABELS Documentation)

  ELSE( DOXYGEN_DOT_FOUND )
    MESSAGE( WARNING
      "Warning: Dot not found - Documentation will not be created"
      )
  ENDIF( DOXYGEN_DOT_FOUND )
ENDIF( DOXYGEN_FOUND )
