project(Endoscopy)

CMAKE_MINIMUM_REQUIRED(VERSION 2.8.2)
IF(COMMAND CMAKE_POLICY)
  CMAKE_POLICY(SET CMP0003 NEW)
ENDIF(COMMAND CMAKE_POLICY)
MARK_AS_ADVANCED(CMAKE_BACKWARDS_COMPATIBILITY)

 
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
      ${CMAKE_BINARY_DIR}/${Slicer_INSTALL_MODULES_LIB_DIR}/${PROJECT_NAME}/${file}
      COPYONLY)
  endforeach(file)

  install(
    FILES ${PYTHONFILES}
    DESTINATION ${Slicer_INSTALL_MODULES_LIB_DIR}/${PROJECT_NAME}
    )

endif(Slicer_USE_PYTHON)
