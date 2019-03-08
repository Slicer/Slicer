set(KIT qSlicer${MODULE_NAME}Module)

#-----------------------------------------------------------------------------
set(KIT_TEST_SRCS
  qSlicer${MODULE_NAME}ModuleWidgetTest.cxx
  )

#-----------------------------------------------------------------------------
set(KIT_TEST_GENERATE_MOC_SRCS
  qSlicer${MODULE_NAME}ModuleWidgetTest.cxx
  )
  QT5_GENERATE_MOCS(${KIT_TEST_GENERATE_MOC_SRCS})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

#-----------------------------------------------------------------------------
slicerMacroConfigureModuleCxxTestDriver(
  NAME ${KIT}
  SOURCES ${KIT_TEST_SRCS}
  WITH_VTK_DEBUG_LEAKS_CHECK
  WITH_VTK_ERROR_OUTPUT_CHECK
  )

#-----------------------------------------------------------------------------
simple_test(qSlicerTransformsModuleWidgetTest)
