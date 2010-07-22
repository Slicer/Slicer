project(PythonModules)

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
IF(COMMAND CMAKE_POLICY)
  CMAKE_POLICY(SET CMP0003 NEW)
ENDIF(COMMAND CMAKE_POLICY)
MARK_AS_ADVANCED(CMAKE_BACKWARDS_COMPATIBILITY)

 
# --------------------------------------------------------------------------
# Find Slicer3

if(NOT Slicer3_SOURCE_DIR)
  find_package(Slicer3 REQUIRED)
  include(${Slicer3_USE_FILE})
  slicer3_set_default_install_prefix_for_external_projects()
endif(NOT Slicer3_SOURCE_DIR)


# --------------------------------------------------------------------------
# Copy over the Python files

if(Slicer3_USE_PYTHON)

  file(GLOB PYTHONFILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.py")

  foreach(file ${PYTHONFILES})
    configure_file(
      ${CMAKE_CURRENT_SOURCE_DIR}/${file}
      ${CMAKE_BINARY_DIR}/${Slicer3_INSTALL_PLUGINS_BIN_DIR}/${file}
      COPYONLY)
  endforeach(file)

  install(
    FILES ${PYTHONFILES}
    DESTINATION ${Slicer3_INSTALL_PLUGINS_BIN_DIR}
    )

  file(GLOB PYTHONFILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "StochasticTractographyServer/*.py")
  foreach(pythonfile ${PYTHONFILES})
    configure_file(
      ${CMAKE_CURRENT_SOURCE_DIR}/${pythonfile}
      ${CMAKE_BINARY_DIR}/${Slicer3_INSTALL_PLUGINS_BIN_DIR}/${pythonfile}
      COPYONLY)
  endforeach(pythonfile)

  install(
    FILES ${PYTHONFILES}
    DESTINATION ${Slicer3_INSTALL_PLUGINS_BIN_DIR}/StochasticTractographyServer
    )


  file(GLOB PYTHONFILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "StochasticTractographyServerMP/*.py")
  foreach(pythonfile ${PYTHONFILES})
    configure_file(
      ${CMAKE_CURRENT_SOURCE_DIR}/${pythonfile}
      ${CMAKE_BINARY_DIR}/${Slicer3_INSTALL_PLUGINS_BIN_DIR}/${pythonfile}
      COPYONLY)
  endforeach(pythonfile)

  install(
    FILES ${PYTHONFILES}
    DESTINATION ${Slicer3_INSTALL_PLUGINS_BIN_DIR}/StochasticTractographyServerMP
    )

if ( 0 )
  message ( STATUS "Configuration of Python modules, python is ${PYTHON_EXECUTABLE} path is $ENV{PATH}" )

  # To build the shared library, we run the BuildFilteredTractography.py script, but the weave tools must be installed
  file ( COPY FilteredTractography/weave DESTINATION ${CMAKE_BINARY_DIR}/${Slicer3_INSTALL_PLUGINS_BIN_DIR} )
  add_custom_command ( OUTPUT ${CMAKE_BINARY_DIR}/${Slicer3_INSTALL_PLUGINS_BIN_DIR}/filtered_ext.so
    COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/FilteredTractography/BuildFilteredTractography.py $ENV{PATH}
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/FilteredTractography/BuildFilteredTractography.py ${CMAKE_CURRENT_SOURCE_DIR}/FilteredTractography/FilteredTractography.py
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/${Slicer3_INSTALL_PLUGINS_BIN_DIR}"
    COMMENT "Building shared library for FilteredTractography"
    )

  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/FilteredTractography/FilteredTractography.py
    ${CMAKE_BINARY_DIR}/${Slicer3_INSTALL_PLUGINS_BIN_DIR}/FilteredTractography.py
    COPYONLY)
  install(
    FILES FilteredTractography/FilteredTractography.py
    DESTINATION ${Slicer3_INSTALL_PLUGINS_BIN_DIR}/FilteredTractography.py
    )

  add_custom_target ( filtered1t_Library ALL DEPENDS ${CMAKE_BINARY_DIR}/${Slicer3_INSTALL_PLUGINS_BIN_DIR}/filtered_ext.so )
endif()

endif(Slicer3_USE_PYTHON)
