project(PythonModules)

#-----------------------------------------------------------------------------
cmake_minimum_required(VERSION 2.8.2)
#-----------------------------------------------------------------------------

#-----------------------------------------------------------------------------
# See http://cmake.org/cmake/help/cmake-2-8-docs.html#section_Policies for details
#-----------------------------------------------------------------------------
if(POLICY CMP0016)
  CMAKE_POLICY(SET CMP0016 NEW)
endif()
if(POLICY CMP0017)
  CMAKE_POLICY(SET CMP0017 OLD)
endif()


# --------------------------------------------------------------------------
# Copy over the Python files

if(Slicer_USE_PYTHON)

  file(GLOB PYTHONFILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.py")

  foreach(file ${PYTHONFILES})
    configure_file(
      ${CMAKE_CURRENT_SOURCE_DIR}/${file}
      ${CMAKE_BINARY_DIR}/${Slicer_INSTALL_PLUGINS_BIN_DIR}/${file}
      COPYONLY)
  endforeach(file)

  install(
    FILES ${PYTHONFILES}
    DESTINATION ${Slicer_INSTALL_PLUGINS_BIN_DIR}
    )

  file(GLOB PYTHONFILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "StochasticTractographyServer/*.py")
  foreach(pythonfile ${PYTHONFILES})
    configure_file(
      ${CMAKE_CURRENT_SOURCE_DIR}/${pythonfile}
      ${CMAKE_BINARY_DIR}/${Slicer_INSTALL_PLUGINS_BIN_DIR}/${pythonfile}
      COPYONLY)
  endforeach(pythonfile)

  install(
    FILES ${PYTHONFILES}
    DESTINATION ${Slicer_INSTALL_PLUGINS_BIN_DIR}/StochasticTractographyServer
    )


  file(GLOB PYTHONFILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "StochasticTractographyServerMP/*.py")
  foreach(pythonfile ${PYTHONFILES})
    configure_file(
      ${CMAKE_CURRENT_SOURCE_DIR}/${pythonfile}
      ${CMAKE_BINARY_DIR}/${Slicer_INSTALL_PLUGINS_BIN_DIR}/${pythonfile}
      COPYONLY)
  endforeach(pythonfile)

  install(
    FILES ${PYTHONFILES}
    DESTINATION ${Slicer_INSTALL_PLUGINS_BIN_DIR}/StochasticTractographyServerMP
    )

if ( 0 )
  message ( STATUS "Configuration of Python modules, python is ${PYTHON_EXECUTABLE} path is $ENV{PATH}" )

  # To build the shared library, we run the BuildFilteredTractography.py script, but the weave tools must be installed
  file ( COPY FilteredTractography/weave DESTINATION ${CMAKE_BINARY_DIR}/${Slicer_INSTALL_PLUGINS_BIN_DIR} )
  add_custom_command ( OUTPUT ${CMAKE_BINARY_DIR}/${Slicer_INSTALL_PLUGINS_BIN_DIR}/filtered_ext.so
    COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/FilteredTractography/BuildFilteredTractography.py $ENV{PATH}
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/FilteredTractography/BuildFilteredTractography.py ${CMAKE_CURRENT_SOURCE_DIR}/FilteredTractography/FilteredTractography.py
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/${Slicer_INSTALL_PLUGINS_BIN_DIR}"
    COMMENT "Building shared library for FilteredTractography"
    )

  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/FilteredTractography/FilteredTractography.py
    ${CMAKE_BINARY_DIR}/${Slicer_INSTALL_PLUGINS_BIN_DIR}/FilteredTractography.py
    COPYONLY)
  install(
    FILES FilteredTractography/FilteredTractography.py
    DESTINATION ${Slicer_INSTALL_PLUGINS_BIN_DIR}/FilteredTractography.py
    )

  add_custom_target ( filtered1t_Library ALL DEPENDS ${CMAKE_BINARY_DIR}/${Slicer_INSTALL_PLUGINS_BIN_DIR}/filtered_ext.so )
endif()

endif(Slicer_USE_PYTHON)
