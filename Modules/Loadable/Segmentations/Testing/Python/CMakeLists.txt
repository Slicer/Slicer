#-----------------------------------------------------------------------------
set(EXTENSION_TEST_PYTHON_SCRIPTS
  SegmentationsModuleTest1.py
  )

set(EXTENSION_TEST_PYTHON_RESOURCES
  )

#-----------------------------------------------------------------------------
if(CMAKE_CONFIGURATION_TYPES)
  set(MODULE_BUILD_DIR "")
  foreach(config ${CMAKE_CONFIGURATION_TYPES})
    list(APPEND MODULE_BUILD_DIR "${CMAKE_BINARY_DIR}/${Slicer_QTLOADABLEMODULES_LIB_DIR}/${config}")
  endforeach()
else()
  set(MODULE_BUILD_DIR "${CMAKE_BINARY_DIR}/${Slicer_QTLOADABLEMODULES_LIB_DIR}")
endif()

slicer_add_python_unittest(
  SCRIPT SegmentationsModuleTest1.py
  SLICER_ARGS --disable-cli-modules
              --additional-module-paths
                ${MODULE_BUILD_DIR}
                ${CMAKE_BINARY_DIR}/${Slicer_QTSCRIPTEDMODULES_LIB_DIR}
  )
# See http://na-mic.org/Mantis/view.php?id=4228
set_tests_properties(py_SegmentationsModuleTest1 PROPERTIES WILL_FAIL TRUE)
