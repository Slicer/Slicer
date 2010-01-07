# Configure Qt Designer launcher
IF(Slicer3_USE_QT)
  GET_TARGET_PROPERTY(output_directory ${PROJECT_NAME} LIBRARY_OUTPUT_DIRECTORY)
  #MESSAGE(STATUS CMAKE_CONFIGURATION_TYPES:${CMAKE_CONFIGURATION_TYPES})
  #MESSAGE(STATUS CMAKE_BUILD_TYPE:${CMAKE_BUILD_TYPE})
  IF(CMAKE_CONFIGURATION_TYPES)
    FOREACH(config ${CMAKE_CONFIGURATION_TYPES})
      SET(LIBRARY_OUTPUT_DIRECTORY ${output_directory}/${config})
      SET(designer_launcher "${Slicer3_BINARY_DIR}/Designer-${config}.py")
      CONFIGURE_FILE(
        ${CMAKE_CURRENT_SOURCE_DIR}/Designer.py.in
        ${designer_launcher}
        IMMEDIATE
        )
      MESSAGE(STATUS "Configuring Designer launcher:"${designer_launcher})
    ENDFOREACH(config)
  ELSE(CMAKE_CONFIGURATION_TYPES)
    SET(LIBRARY_OUTPUT_DIRECTORY ${output_directory})
    SET(designer_launcher "${Slicer3_BINARY_DIR}/Designer.py")
    CONFIGURE_FILE(
      ${CMAKE_CURRENT_SOURCE_DIR}/Designer.py.in
      ${designer_launcher}
      IMMEDIATE
      )
    MESSAGE(STATUS "Configuring Designer launcher:"${designer_launcher})
  ENDIF(CMAKE_CONFIGURATION_TYPES)
ENDIF(Slicer3_USE_QT)
