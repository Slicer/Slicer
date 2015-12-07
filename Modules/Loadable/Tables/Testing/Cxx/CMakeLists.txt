set(KIT qSlicer${MODULE_NAME}Module)

#-----------------------------------------------------------------------------
set(INPUT "${MRMLCore_SOURCE_DIR}/Testing/TestData/")

#-----------------------------------------------------------------------------
set(KIT_TEST_SRCS
  vtkSlicerTablesLogicAddFileTest.cxx
  )

#-----------------------------------------------------------------------------
slicerMacroConfigureModuleCxxTestDriver(
  NAME ${KIT}
  SOURCES ${KIT_TEST_SRCS}
  WITH_VTK_DEBUG_LEAKS_CHECK
  WITH_VTK_ERROR_OUTPUT_CHECK
  )

#-----------------------------------------------------------------------------
simple_test(vtkSlicerTablesLogicAddFileTest ${INPUT}/table.csv)
