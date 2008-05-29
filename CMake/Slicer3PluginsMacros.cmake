#-----------------------------------------------------------------------------
# Set the default output paths for one or more plugins/CLP
# 
macro(slicer3_set_plugins_output_path)
  set_target_properties(${ARGN}
    PROPERTIES 
    RUNTIME_OUTPUT_DIRECTORY 
    "${CMAKE_BINARY_DIR}/${Slicer3_INSTALL_PLUGINS_BIN_DIR}"
    LIBRARY_OUTPUT_DIRECTORY 
    "${CMAKE_BINARY_DIR}/${Slicer3_INSTALL_PLUGINS_LIB_DIR}"
    ARCHIVE_OUTPUT_DIRECTORY 
    "${CMAKE_BINARY_DIR}/${Slicer3_INSTALL_PLUGINS_LIB_DIR}"
    )
endmacro(slicer3_set_plugins_output_path)

#-----------------------------------------------------------------------------
# Install one or more plugins to the default plugin location
# 
macro(slicer3_install_plugins)
  install(TARGETS ${ARGN}
    RUNTIME DESTINATION ${Slicer3_INSTALL_PLUGINS_BIN_DIR} COMPONENT RuntimeLibraries
    LIBRARY DESTINATION ${Slicer3_INSTALL_PLUGINS_LIB_DIR} COMPONENT RuntimeLibraries
    )
endmacro(slicer3_install_plugins)

#-----------------------------------------------------------------------------
# Test a plugin
# 
include("${KWWidgets_CMAKE_DIR}/KWWidgetsTestingMacros.cmake")
macro(slicer3_add_plugins_test test_name clp_to_test)
  set(build_type ".")
  if(WIN32 AND CMAKE_CONFIGURATION_TYPES)
    # Sadly, there is no way to know if the user picked Debug or Release
    # here, so we are going to have to stick to the value of CMAKE_BUILD_TYPE
    # if it has been set explicitly (by a dashboard for example), or the
    # first value in CMAKE_CONFIGURATION_TYPES (i.e. Debug)/
    KWWidgets_GET_CMAKE_BUILD_TYPE(build_type)
  endif(WIN32 AND CMAKE_CONFIGURATION_TYPES)
  add_test(${test_name} ${Slicer3_DIR}/Slicer3 --launch ${CMAKE_BINARY_DIR}/${Slicer3_INSTALL_PLUGINS_BIN_DIR}/${build_type}/${clp_to_test} ${ARGN}) 
endmacro(slicer3_add_plugins_test)
