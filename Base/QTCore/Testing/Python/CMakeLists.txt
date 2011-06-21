

#
# Test templates
#

SET(Slicer_PYTHON_MODULE_TEST_TEMPLATES_DIR ${CMAKE_CURRENT_SOURCE_DIR}
  CACHE INTERNAL "Path to python test templates directory.")

# Install the file so that project building against the 'installed tree' can reference it.
IF(NOT Slicer_INSTALL_NO_DEVELOPMENT)
  INSTALL(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/qSlicerUtilsTest1.py.in
    DESTINATION ${Slicer_INSTALL_SHARE_DIR}/test-templates COMPONENT Development
    )
ENDIF()
