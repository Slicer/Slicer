set(KIT qSlicer${MODULE_NAME}Module)

#-----------------------------------------------------------------------------
set(MODEL_FILE "${MRMLCore_SOURCE_DIR}/Testing/TestData/cube.vtk")

#-----------------------------------------------------------------------------
set(KIT_TEST_SRCS
  vtkSlicerDataLogicAutoRemoveTest
  )

#-----------------------------------------------------------------------------
slicerMacroConfigureModuleCxxTestDriver(
  NAME ${KIT}
  SOURCES ${KIT_TEST_SRCS}
  WITH_VTK_DEBUG_LEAKS_CHECK
  WITH_VTK_ERROR_OUTPUT_CHECK
  )

#-----------------------------------------------------------------------------
simple_test(vtkSlicerDataLogicAutoRemoveTest)

