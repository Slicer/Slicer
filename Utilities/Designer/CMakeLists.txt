#
# Configure Qt Designer launcher (Designer.py.in) allowing to:
# - update the environment variable QT_PLUGIN_PATH with Slicer QT Plugin path
# - update PATH with Slicer libraries
# - start the designer
#

# --------------------------------------------------------------
SET(designer_launcher_project_name SlicerQT)
SET(designer_launcher_directory ${Slicer_BINARY_DIR})
# --------------------------------------------------------------

GET_TARGET_PROPERTY(output_directory ${designer_launcher_project_name} LIBRARY_OUTPUT_DIRECTORY)
#MESSAGE(STATUS CMAKE_CONFIGURATION_TYPES:${CMAKE_CONFIGURATION_TYPES})
#MESSAGE(STATUS CMAKE_BUILD_TYPE:${CMAKE_BUILD_TYPE})
#MESSAGE(STATUS output_directory:${output_directory})

IF(CMAKE_CONFIGURATION_TYPES)
  # Loop through all Configuration types
  # For further details, see http://www.cmake.org/cmake/help/cmake-2-8-docs.html#variable:CMAKE_CONFIGURATION_TYPES
  FOREACH(config ${CMAKE_CONFIGURATION_TYPES})
    SET(LIBRARY_OUTPUT_DIRECTORY ${output_directory}/${config})
    SET(designer_launcher "${designer_launcher_directory}/Designer-${config}.py")
    CONFIGURE_FILE(
      ${CMAKE_CURRENT_SOURCE_DIR}/Designer.py.in
      ${CMAKE_CURRENT_BINARY_DIR}/Designer.py
      )
    FILE(COPY ${CMAKE_CURRENT_BINARY_DIR}/Designer.py
      DESTINATION ${designer_launcher_directory}
      FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ WORLD_READ)
    MESSAGE(STATUS "Configuring Qt Designer launcher: "${designer_launcher})
  ENDFOREACH(config)
  
ELSE(CMAKE_CONFIGURATION_TYPES)

  SET(LIBRARY_OUTPUT_DIRECTORY ${output_directory})
  SET(designer_launcher "${designer_launcher_directory}/Designer.py")
  CONFIGURE_FILE(
    ${CMAKE_CURRENT_SOURCE_DIR}/Designer.py.in
    ${CMAKE_CURRENT_BINARY_DIR}/Designer.py
    )
  FILE(COPY ${CMAKE_CURRENT_BINARY_DIR}/Designer.py
    DESTINATION ${designer_launcher_directory}
    FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ WORLD_READ )
  MESSAGE(STATUS "Configuring Qt Designer launcher: "${designer_launcher})
  
ENDIF(CMAKE_CONFIGURATION_TYPES)
