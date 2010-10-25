project(PythonModules)

#-----------------------------------------------------------------------------
cmake_minimum_required(VERSION 2.8.2)
#-----------------------------------------------------------------------------

#-----------------------------------------------------------------------------
# See http://cmake.org/cmake/help/cmake-2-8-docs.html#section_Policies for details
#-----------------------------------------------------------------------------

SET(project_policies
  CMP0001 # NEW: CMAKE_BACKWARDS_COMPATIBILITY should no longer be used.
  CMP0002 # NEW: Logical target names must be globally unique.
  CMP0003 # NEW: Libraries linked via full path no longer produce linker search paths.
  CMP0004 # NEW: Libraries linked may NOT have leading or trailing whitespace.
  CMP0005 # NEW: Preprocessor definition values are now escaped automatically.
  CMP0006 # NEW: Installing MACOSX_BUNDLE targets requires a BUNDLE DESTINATION.
  CMP0007 # NEW: List command no longer ignores empty elements.
  CMP0008 # NEW: Libraries linked by full-path must have a valid library file name.
  CMP0009 # NEW: FILE GLOB_RECURSE calls should not follow symlinks by default.
  CMP0010 # NEW: Bad variable reference syntax is an error.
  CMP0011 # NEW: Included scripts do automatic cmake_policy PUSH and POP.
  CMP0012 # NEW: if() recognizes numbers and boolean constants.
  CMP0013 # NEW: Duplicate binary directories are not allowed.
  CMP0014 # NEW: Input directories must have CMakeLists.txt
  )
FOREACH(policy ${project_policies})
  IF(POLICY ${policy})
    CMAKE_POLICY(SET ${policy} NEW)
  ENDIF()
ENDFOREACH()

# --------------------------------------------------------------------------
# Find Slicer3

if(NOT Slicer_SOURCE_DIR)
  find_package(Slicer3 REQUIRED)
  include(${Slicer_USE_FILE})
  slicer3_set_default_install_prefix_for_external_projects()
endif(NOT Slicer_SOURCE_DIR)


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
